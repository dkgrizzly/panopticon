/*
 *  VerilogBoy
 *
 *  Copyright (C) 2019  Wenting Zhang <zephray@outlook.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms and conditions of the GNU General Public License,
 *  version 2, as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "usb.h"
#include "usb_gamepad.h"

// #define LOG_TO_SERIAL
#define DEBUG_LOGGING
#define VERBOSE_DEBUG_LOGGING
#include "log.h"

// This is driver is designed to be a generic USB HID gamepad driver with these
// limitations:
//   no auto scailing to the analog values
//   no naming for analog values
//   all analog values should be uint8 (16 bit number will be truncated)
//   only short items is allowed in the descriptor
//   only input is support
//   only 1 report ID is accepted
// With these limitations... it should works with:
//   SONY DualShock 3
//   SONY DualShock 4
//   Nintendo Switch Pro Controller
//   Nintendo Switch NGC Controller
// Without support of any kind of:
//   analog triggers (they are generally defined in Vendor-Specific sections)
//   analog buttons (the same)
//   gyroscopes (the same)
//   accelerometers (the same)
//   touchpad (the same)
//   rumble (requires output)
//   audio (this is not a audio driver at all)
// And I assume most other generic HID-compliant controllers.
//   **No, it doesn't work with Xbox 360/One controllers!**

// acceptable usage page:
#define USAGE_PAGE_GEN_DESKTOP         0x01
#define USAGE_PAGE_BUTTON              0x09
//   otherwise ignored. (like vendor defined or consumer)

// accpetable usage:
#define USAGE_JOYSTICK                 0x04
#define USAGE_GAMEPAD                  0x05
#define USAGE_X                        0x30
#define USAGE_Y                        0x31
#define USAGE_Z                        0x32
#define USAGE_RZ                       0x35
#define USAGE_HAT_SWITCH               0x39
//   otherwise ignored. (like pointer)

uint32_t gp_num_buttons;
uint32_t gp_buttons;
uint32_t gp_num_analogs;
uint8_t  gp_analog[MAX_ANALOG];

// Private
int32_t dpad_bit_offset[MAX_DPAD];
int32_t button_bit_offset[MAX_BUTTON];
int32_t analog_byte_offset[MAX_ANALOG];
int32_t analog_bit_size[MAX_ANALOG];
int32_t dpad_count = 0;
int32_t button_count = 0;
int32_t analog_count = 0;
int32_t accepted_report_id = -1;
uint32_t report_length;
uint8_t report[MAX_BITS / 8];

#define DEVNAME "usbgamepad"

static int32_t usb_gp_getstatus(void)
{
  usb_event_poll();
  return 0;
}

static int32_t usb_gp_probe(struct usb_device *dev, uint32_t ifnum);

// check if any joysticks is present
int32_t drv_usb_gp_init(void)
{
  int32_t error,i;
  struct usb_device *dev;

  /* scan all USB Devices */
  for(i = 0; i < USB_MAX_DEVICE; i++) {
    dev = usb_get_dev_index(i);
    if(dev == NULL)
      return -1; // what does this mean?
    if(dev->devnum != -1) {
      if(usb_gp_probe(dev, 0) == 1) {
        LOG("USB gamepad found.\n");
      }
    }
  }
  return -1;
}

// deregister
int32_t usb_gp_deregister(void)
{
  return 1;
}

void usb_gp_parse_descriptor(uint8_t *descriptor, uint32_t descriptor_length) {
    // status values
    uint32_t report_size = 0;
    uint32_t report_count = 0;
    int32_t report_id = -1;
    uint32_t usage = 0;
    uint32_t usage_page = 0;
    uint32_t bit_count = 0;
  uint32_t byte_count;
    uint32_t collection_count = 0;
    // variable for current item
    uint8_t item_type;
    uint8_t item_length;
    uint32_t item_value;
    uint32_t i = 0;

    for (int32_t i = 0; i < MAX_DPAD; i++)
        dpad_bit_offset[i] = -1;
    for (int32_t i = 0; i < MAX_BUTTON; i++)
        button_bit_offset[i] = -1;
    for (int32_t i = 0; i < MAX_ANALOG; i++)
        analog_byte_offset[i] = -1;

    while(i < descriptor_length) {
        item_type = descriptor[i] & 0xfc;
        item_length = descriptor[i] & 0x03;
        item_length = (item_length == 3) ? (4) : (item_length); // 3 means 4
        i += 1;
        item_value = 0;
        for (int32_t bc = 0; bc < 4; bc++)
            if (item_length > bc)
                item_value |= (uint32_t )descriptor[i + bc] << (8 * bc);

        switch (item_type) {
        case 0x04: // Usage Page
            usage_page = item_value;
            break;
        case 0x08: // Usage
            usage = item_value & 0xff; // workaround for NS pro controller
            break;
        case 0x74: // Report Size
            report_size = item_value;
            break;
        case 0x94: // Report Count
            report_count = item_value;
            break;
        case 0x84: // Report ID
            report_id = item_value;
            if (accepted_report_id == -1) {
                // The report will contain the report ID
                bit_count += 8;
                accepted_report_id = report_id;
            }
        case 0x80: // Input
            // Check if the report id is correct or not
            if (report_id != accepted_report_id)
                break;
            // First check if it is a valid data or it is used for padding
            if (item_value & 0x01) {
                // This is for padding only
                bit_count += report_count * report_size;
            }
            else {
                // This is valid data
                if ((usage_page == USAGE_PAGE_GEN_DESKTOP) &&
                        (usage == USAGE_HAT_SWITCH) && (report_size == 4) &&
                        (bit_count % 4 == 0)) {
                    for (int32_t j = 0; j < report_count; j++) {
                        if ((dpad_count < MAX_DPAD) &&
                (bit_count < MAX_BITS))
                            dpad_bit_offset[dpad_count++] = bit_count;
                        bit_count += 4;
                    }
                }
                else if (((usage == USAGE_JOYSTICK) ||
                        (usage == USAGE_X) || (usage == USAGE_Y) ||
                        (usage == USAGE_Z) || (usage == USAGE_RZ)) &&
                        ((report_size == 8) || (report_size == 16)) &&
                        (bit_count % 8 == 0))  {
                    for (int32_t j = 0; j < report_count; j++) {
                        if ((analog_count < MAX_ANALOG) &&
                (bit_count < MAX_BITS))
                            analog_byte_offset[analog_count++] = bit_count / 8;
                        bit_count += report_size;
                    }
                }
                else if (((usage_page == USAGE_PAGE_BUTTON) ||
                        (usage_page == USAGE_PAGE_GEN_DESKTOP)) &&
                        (report_size == 1)) {
                    // anything goes...
                    for (int32_t j = 0; j < report_count; j++) {
                        if ((button_count < MAX_BUTTON) &&
                (bit_count < MAX_BITS))
                            button_bit_offset[button_count++] = bit_count;
                        bit_count++;
                    }
                }
                else {
                ignore:
                    // unknown or ignored, use as padding
                    bit_count += report_count * report_size;
                }
            }
            break;
        case 0xa0: // Collection
            collection_count ++;
            break;
        case 0xc0: // End Collection
            collection_count --;
            break;
        case 0x18: // Usage minimum
        case 0x28: // Usage maximum
        case 0x14: // Logical Minimum
        case 0x24: // Logical Maximum
        case 0x34: // Physical Minimum
        case 0x44: // Physical Maximum
        case 0x54: // Unit Exponent
        case 0x64: // Unit
        case 0x90: // Output
        case 0xb0: // Feature
            // Known but ignored
            break;
        }
        i += item_length;
    }
  if (collection_count != 0)
    printf("WARNING: Incomplete descriptor\n");

  byte_count = (bit_count / 8) + ((bit_count % 8 != 0) ? (1) : (0));
    printf("Expected total report size %d bits (%d bytes), with %d buttons, "
      "%d d-pads, and %d analog controls.\n",
            bit_count, byte_count, button_count, dpad_count, analog_count);
    for (i = 0; i < button_count; i++) {
        LOG("Button %d at byte %d bit %d\n", i,
        button_bit_offset[i] / 8, button_bit_offset[i] % 8);
    }
    for (i = 0; i < dpad_count; i++) {
        LOG("Dpad %d at byte %d bit %d\n", i,
        dpad_bit_offset[i] / 8, dpad_bit_offset[i] % 8);
    }
    for (i = 0; i < analog_count; i++) {
        LOG("Analog %d at byte %d\n", i, analog_byte_offset[i]);
    }
    gp_num_buttons = button_count + 4 * dpad_count;
    gp_num_analogs = analog_count;

  report_length = (bit_count > MAX_BITS) ? (MAX_BITS / 8) : (byte_count);

    return;
}

// DPAD are mapped as: Left Down Right Up
#define GP_DPAD_LEFT  (1 << 3)
#define GP_DPAD_DOWN  (1 << 2)
#define GP_DPAD_RIGHT (1 << 1)
#define GP_DPAD_UP    (1 << 0)
uint8_t dpad_lut[8] = {
    GP_DPAD_UP,
    GP_DPAD_UP | GP_DPAD_RIGHT,
    GP_DPAD_RIGHT,
    GP_DPAD_DOWN | GP_DPAD_RIGHT,
    GP_DPAD_DOWN,
    GP_DPAD_DOWN | GP_DPAD_LEFT,
    GP_DPAD_LEFT,
    GP_DPAD_UP | GP_DPAD_LEFT
};

void usb_gp_parse_report() {
    // Process DPADs
    int32_t i;
    uint8_t tmp_in;
    uint8_t tmp_out;

    if ((accepted_report_id == -1) || (report[0] == accepted_report_id)) {
        gp_buttons = 0;
        for (i = 0; i < dpad_count; i++) {
            tmp_in = report[dpad_bit_offset[i] / 8] >> (dpad_bit_offset[i] % 8);
            tmp_out = (tmp_in >= 8) ? (0) : (dpad_lut[tmp_in]);
            gp_buttons |= tmp_out << (4 * i);
        }
        for (i = 0; i < button_count; i++) {
            tmp_in = report[button_bit_offset[i] / 8]
                & (1 << (button_bit_offset[i] % 8));
            gp_buttons |= ((tmp_in) ? (1u) : (0u)) << (dpad_count * 4 + i);
        }
        for (i = 0; i < analog_count; i++) {
            gp_analog[i] = report[analog_byte_offset[i]];
        }
    }
}

/**************************************************************************
 * Low Level drivers
 */

static int32_t usb_gp_irq(struct usb_device *dev, unsigned long pipe) {
  LOG(".\n");
  usb_gp_parse_report();

  // install the IRQ handler again
  return 1;
}

// check if a device is a joystick
static int32_t usb_gp_probe(struct usb_device *dev, uint32_t ifnum) {
  struct usb_interface_descriptor *iface;
  struct usb_endpoint_descriptor *ep;
  int32_t pipe, max_packet_size;
  uint8_t report_descriptor[256];
  uint32_t report_descriptor_length;

  if (dev->descriptor.bNumConfigurations != 1) return 0;
  iface = &dev->config.if_desc[ifnum];

  if (iface->bInterfaceClass != 3) return 0;
  if (iface->bInterfaceSubClass != 0) return 0;
  if (iface->bInterfaceProtocol != 0) return 0;

  // This driver actually accepts all HID devices other than mouse and
  // keyboard that can return a valid HID report descriptor.
        if(dev->hid_descriptor.wItemLength > sizeof(report_descriptor)) return 0;
  //report_descriptor = malloc(dev->hid_descriptor.wItemLength);
  report_descriptor_length = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
      USB_REQ_GET_DESCRIPTOR, 0x81, (USB_DT_REPORT << 8), 0,
      report_descriptor, dev->hid_descriptor.wItemLength,
      USB_CNTL_TIMEOUT);

  if (report_descriptor_length != dev->hid_descriptor.wItemLength) {
    printf("Requested %d, actually got %d.\n",
        dev->hid_descriptor.wItemLength, report_descriptor_length);
  }

  // Dump the whole descriptor
  for (int32_t i = 0; i < report_descriptor_length; i++) {
    LOG("%02x ", report_descriptor[i]);
  }
  LOG("\n");

  // Parse the descriptor
  usb_gp_parse_descriptor(report_descriptor, report_descriptor_length);

  //free(report_descriptor);

  // Find an IN endpoint
  for (int32_t i = 0; i < iface->bNumEndpoints; i++) {
    ep = &iface->ep_desc[i];
    if (ep->bEndpointAddress & 0x80) break;
  }
  // Check if it is valid
  if (!(ep->bEndpointAddress & 0x80)) {
    printf("Unable to find an IN endpoint!\n");
    return 0;
  }

  pipe = usb_rcvintpipe(dev, ep->bEndpointAddress);
  max_packet_size = usb_maxpacket(dev, pipe);
  // What is maximum packet size is larger than report length?
  dev->irq_handle = usb_gp_irq;
  LOG("USB gamepad enable interrupt pipe...\n");
  usb_submit_int_msg(dev, pipe, report, report_length, ep->bInterval);

  return 1;
}


