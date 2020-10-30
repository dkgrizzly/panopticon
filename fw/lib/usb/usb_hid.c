/*
 *  Copyright (C) 2019  Skip Hansen
 *
 *  This file is derived from Verilogboy project:
 *  Copyright (C) 2019  Wenting Zhang <zephray@outlook.com>
 *
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * Part of this source has been derived from the Linux USB
 * project.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "misc.h"
#include "usb.h"

// #define LOG_TO_SERIAL
//#define DEBUG_LOGGING
//#define VERBOSE_DEBUG_LOGGING
#define DISABLE_LOGGING
#include "log.h"

// Define the following to send ANSI escape sequences for certain special keys
#undef ANSI_SUPPORT

/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
extern int32_t overwrite_console (void);
#else
int32_t overwrite_console (void)
{
  return(0);
}
#endif


#define REPEAT_RATE  40/4 /* 40msec -> 25cps */
#define REPEAT_DELAY 10 /* 10 x REAPEAT_RATE = 400msec */

#include "usb_keys.h"

/* Modifier bits */
#define LEFT_CNTR    0
#define LEFT_SHIFT   1
#define LEFT_ALT     2
#define LEFT_GUI     3
#define RIGHT_CNTR   4
#define RIGHT_SHIFT  5
#define RIGHT_ALT    6
#define RIGHT_GUI    7

#define ANSI_ESC 0x1b

#define USB_KBD_BUFFER_LEN 0x20  /* size of the keyboardbuffer */

static volatile uint16_t usb_kbd_buffer[USB_KBD_BUFFER_LEN];
static volatile int32_t usb_in_pointer = 0;
static volatile int32_t usb_out_pointer = 0;

uint8_t new_ms_pkt[8];
uint8_t old_ms_pkt[8];

uint8_t new_kb_pkt[8];
uint8_t old_kb_pkt[8];
int32_t repeat_delay;
#define DEVNAME "usbhid"
static uint8_t num_lock = 0;
static uint8_t caps_lock = 0;
static uint8_t scroll_lock = 0;
static uint8_t ctrl = 0;

static bool kbd_raw = 0;

static uint8_t leds __attribute__ ((aligned (0x4)));

static uint8_t usb_kbd_numkey[] = {
  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0','\r',0x1b,'\b','\t',' ', '-',
  '=', '[', ']','\\', '#', ';', '\'', '`', ',', '.', '/'
};
static uint8_t usb_kbd_numkey_shifted[] = {
  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')','\r',0x1b,'\b','\t',' ', '_',
  '+', '{', '}', '|', '~', ':', '"', '~', '<', '>', '?'
};

unsigned long kb_pipe = 0xffffffff;
unsigned long ms_pipe = 0xffffffff;

/******************************************************************
 * Queue handling
 ******************************************************************/
/* puts character in the queue and sets up the in and out pointer */
static void usb_kbd_put_queue(uint16_t data)
{
#ifdef VERBOSE_DEBUG_LOGGING
  if(isprint(data)) {
    LOG_R("queued '%c'\n",data);
  }
  else {
    LOG_R("queued 0x%x\n",data);
  }
#endif
  if((usb_in_pointer+1)==USB_KBD_BUFFER_LEN) {
    if(usb_out_pointer==0) {
      return; /* buffer full */
    }
    else {
      usb_in_pointer=0;
    }
  }
  else {
    if((usb_in_pointer+1)==usb_out_pointer)
      return; /* buffer full */
    usb_in_pointer++;
  }
  usb_kbd_buffer[usb_in_pointer]=data;
  return;
}

/* test if a character is in the queue */
int32_t usb_kbd_testc(void)
{
#ifdef CONFIG_SYS_USB_EVENT_POLL
  usb_event_poll();
#endif
  if(usb_in_pointer==usb_out_pointer)
    return(0); /* no data */
  else
    return(1);
}
/* gets the character from the queue */
uint16_t usb_kbd_getc(void)
{
  uint16_t c;
  while(usb_in_pointer==usb_out_pointer) {
#ifdef CONFIG_SYS_USB_EVENT_POLL
    usb_event_poll();
#endif
  }
  if((usb_out_pointer+1)==USB_KBD_BUFFER_LEN)
    usb_out_pointer=0;
  else
    usb_out_pointer++;
  c=usb_kbd_buffer[usb_out_pointer];
  return c;
}

/* forward decleration */
static int32_t usb_hid_probe(struct usb_device *dev, uint32_t ifnum);

/* search for hid class devices and register if found */
int32_t drv_usb_hid_init(void)
{
  int32_t error,i;
  struct usb_device *dev;

  usb_in_pointer=0;
  usb_out_pointer=0;
  /* scan all USB Devices */
  for(i=0;i<USB_MAX_DEVICE;i++) {
    dev=usb_get_dev_index(i); /* get device */
    if(dev == NULL)
      return -1;
    if(dev->devnum!=-1) {
      /* Skip devices with more than one configuration */
      if(dev->descriptor.bNumConfigurations == 1) {
        for(int ifnum = 0; ifnum < dev->config.bNumInterfaces; ifnum++) {
          if(usb_hid_probe(dev,ifnum)==1) {
            /* Ok, we found a keyboard or mouse */
          }
        }
      }
    }
  }
  /* no USB HID Devices found */
  return -1;
}


/* deregistering the keyboard */
int32_t usb_hid_deregister(void)
{
#ifdef CONFIG_SYS_STDIO_DEREGISTER
  return stdio_deregister(DEVNAME);
#else
  return 1;
#endif
}

/**************************************************************************
 * Low Level drivers
 */

/* set the LEDs. Since this is used in the irq routine, the control job
  is issued with a timeout of 0. This means, that the job is queued without
  waiting for job completion */

static void usb_kbd_setled(struct usb_device *dev)
{
  struct usb_interface_descriptor *iface;
  iface = &dev->config.if_desc[0];
  leds=0;
  if(scroll_lock!=0)
    leds|=1;
  leds<<=1;
  if(caps_lock!=0)
    leds|=1;
  leds<<=1;
  if(num_lock!=0)
    leds|=1;
  usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
                 USB_REQ_SET_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                 0x200, iface->bInterfaceNumber,(void *)&leds, 1, 0);

}


#define CAPITAL_MASK 0x20
/* Translate the scancode in ASCII
  Modifier:
    0 - key released
    1 - key pressed
    2 - key repeat
*/
static int32_t usb_kbd_translate(uint8_t scancode,uint8_t modifier,int pressed)
{
  uint8_t keycode;
#ifdef ANSI_SUPPORT
  char EscKey = 0;
#endif
  static uint8_t RepeatScanCode;

#ifdef VERBOSE_DEBUG_LOGGING
  VLOG("0x%x,0x%x,%d\n",scancode,modifier,pressed);
#endif

  if(pressed==0) {
    /* key released */
    repeat_delay=0;
    if(RepeatScanCode == scancode) {
      RepeatScanCode = 0;
    }
#ifdef ANSI_SUPPORT
    if(scancode == KEYCODE_ARROW_U) {
      EscKey = 'A';
    }
    else if(scancode == KEYCODE_ARROW_D) {
      EscKey = 'B';
    }
    else if(scancode == KEYCODE_ARROW_R) {
      EscKey = 'C';
    }
    else if(scancode == KEYCODE_ARROW_L) {
      EscKey = 'D';
    }

    if(EscKey != 0) {
      usb_kbd_put_queue(ANSI_ESC);
      usb_kbd_put_queue(EscKey);
    }
#endif
    return 0;
  }
  if(pressed == 2) {
    if(scancode != RepeatScanCode || ++repeat_delay < REPEAT_DELAY) {
      return 0;
    }
    repeat_delay=REPEAT_DELAY;
  }
  keycode=0;
  if((scancode>3) && (scancode<=0x1d)) { /* alpha numeric values */
    keycode=scancode-4 + 0x61;
    if(caps_lock)
      keycode&=~CAPITAL_MASK; /* switch to capital Letters */
    if(((modifier&(1<<LEFT_SHIFT))!=0)||((modifier&(1<<RIGHT_SHIFT))!=0)) {
      if(keycode & CAPITAL_MASK)
         keycode&=~CAPITAL_MASK; /* switch to capital Letters */
      else
         keycode|=CAPITAL_MASK; /* switch to non capital Letters */
    }
  }


  if((scancode>0x1d) && (scancode<0x3A)) {
    if(((modifier&(1<<LEFT_SHIFT))!=0)||((modifier&(1<<RIGHT_SHIFT))!=0))  /* shifted */
      keycode=usb_kbd_numkey_shifted[scancode-0x1e];
    else /* non shifted */
      keycode=usb_kbd_numkey[scancode-0x1e];
  }
  else if(scancode == KEYCODE_DEL) {
    keycode = 0x7f;
  }
#ifdef ANSI_SUPPORT
  else if(scancode >= KEYCODE_F1 && scancode <= KEYCODE_F4) {
  // F1..F4 translate to PF1..PF4 <esc>OP...
    EscKey = 'P' + scancode - F1;
  }
  else if(scancode == KEYCODE_NUMPAD_0) {
    EscKey = 'p';
  }
  else if(scancode >= KEYCODE_NUMPAD_1 && scancode <= KEYCODE_NUMPAD_9) {
  // Numeric keypad keys
    EscKey = 'q' + scancode - KEYCODE_NUMPAD_1;
  }
  else if(scancode == KEYCODE_NUMPAD_MINUS) {
    EscKey = 'm';
  }
  else if(scancode == KEYCODE_NUMPAD_PERIOD) {
    EscKey = 'n';
  }
  else if(scancode == KEYCODE_NUMPAD_ENTER) {
    EscKey = 'M';
  }
  else if(scancode == KEYCODE_ARROW_U) {
    EscKey = 'A';
  }
  else if(scancode == KEYCODE_ARROW_D) {
    EscKey = 'B';
  }
  else if(scancode == KEYCODE_ARROW_R) {
    EscKey = 'C';
  }
  else if(scancode == KEYCODE_ARROW_L) {
    EscKey = 'D';
  }
#else
  if(scancode == KEYCODE_NUMPAD_0) {
    keycode = '0';
  }
  else if(scancode >= KEYCODE_NUMPAD_1 && scancode <= KEYCODE_NUMPAD_9) {
  // Numeric keypad keys
    keycode = '1' + scancode - KEYCODE_NUMPAD_1;
  }
  else if(scancode == KEYCODE_NUMPAD_MINUS) {
    keycode = '-';
  }
  else if(scancode == KEYCODE_NUMPAD_PERIOD) {
    keycode = '.';
  }
  else if(scancode == KEYCODE_NUMPAD_ENTER) {
    keycode = '\r';
  }
#endif

  if(ctrl) {
    keycode = scancode - 0x3;
  }

  if(pressed==1) {
    if(scancode==KEYCODE_NUM_LOCK) {
      num_lock=~num_lock;
      return 1;
    }
    if(scancode==KEYCODE_CAPS_LOCK) {
      caps_lock=~caps_lock;
      return 1;
    }
    if(scancode==KEYCODE_SCROLL_LOCK) {
      scroll_lock=~scroll_lock;
      return 1;
    }
  }
  if(keycode != 0) {
    if(RepeatScanCode != scancode) {
    // New key pressed, save scan code and reset key repeat delay
      RepeatScanCode = scancode;
      repeat_delay = 0;
    }
    usb_kbd_put_queue(keycode);
  }
#ifdef ANSI_SUPPORT
  else {
    if(EscKey != 0) {
      usb_kbd_put_queue(ANSI_ESC);
      usb_kbd_put_queue('O');
      usb_kbd_put_queue(EscKey);
    }
  }
#endif
  return 0;
}

/* Interrupt service routines */
static int32_t usb_kb_irq(struct usb_device *dev, unsigned long pipe)
{
  int32_t i,res;
  uint8_t Modifiers = new_kb_pkt[0];
  bool KeyChange = false;

  res=0;
  if(dev->act_len!=8) {
    LOG("usb_hid Error %lX, len %d\n",dev->irq_status,dev->act_len);
    return 1;
  }

#ifdef VERBOSE_DEBUG_LOGGING
  for(i = 0; i < 8; i++) {
    if(old_kb_pkt[i] != new_kb_pkt[i]) {
      break;
    }
  }

  if(i < 8) {
    LOG_R("old: ");
    LOG_HEX(old_kb_pkt,8);
    LOG_R("new: ");
    LOG_HEX(new_kb_pkt,8);
  }
#endif

  switch(new_kb_pkt[0]) {
    case 0x0:   /* No combo key pressed */
      ctrl = 0;
      break;
    case 0x01:  /* Left Ctrl pressed */
    case 0x10:  /* Right Ctrl pressed */
      ctrl = 1;
      break;
  }

  if(kbd_raw) {
    for(i = 2; i < 8; i++) {
      if(old_kb_pkt[i] > 3 && memscan(&new_kb_pkt[2], old_kb_pkt[i], 6) == &new_kb_pkt[8]) {
        // Key Released
        usb_kbd_put_queue(0x8000 | old_kb_pkt[i]);
      }
      if(new_kb_pkt[i] > 3 && memscan(&old_kb_pkt[2], new_kb_pkt[i], 6) == &old_kb_pkt[8]) {
        // Key Pressed
        usb_kbd_put_queue(new_kb_pkt[i]);
      }
    }
  } else {
    for(i = 2; i < 8; i++) {
      if(old_kb_pkt[i] > 3 && memscan(&new_kb_pkt[2], old_kb_pkt[i], 6) == &new_kb_pkt[8]) {
        KeyChange = true;
        res|=usb_kbd_translate(old_kb_pkt[i],Modifiers,0);
      }
      if(new_kb_pkt[i] > 3 && memscan(&old_kb_pkt[2], new_kb_pkt[i], 6) == &old_kb_pkt[8]) {
        KeyChange = true;
        res|=usb_kbd_translate(new_kb_pkt[i],Modifiers,1);
      }
    }

    if(!KeyChange) {
      for(i = 7; i >= 2; i--) {
        if(new_kb_pkt[i] > 3 && new_kb_pkt[i] != KEYCODE_CAPS_LOCK && old_kb_pkt[i]==new_kb_pkt[i]) {
          // still pressed
          res |= usb_kbd_translate(new_kb_pkt[i],Modifiers,2);
        }
      }
    }
  }

  if(res==1) {
    usb_kbd_setled(dev);
  }
  memcpy(&old_kb_pkt[0],&new_kb_pkt[0], 8);
  return 1; /* install IRQ Handler again */
}

static int32_t usb_ms_irq(struct usb_device *dev, unsigned long pipe)
{
  int32_t i;

//  LOG("pipe %04X dlen %d\n", pipe, dev->act_len);

  for(i = 0; i < 8; i++) {
    if(old_ms_pkt[i] != new_ms_pkt[i]) {
      break;
    }
  }

  if(i < 8) {
    LOG_R("oldm: ");
    LOG_HEX(old_ms_pkt,8);
    LOG_R("newm: ");
    LOG_HEX(new_ms_pkt,8);
  }

  for(i = 0; i < 8; i++) {
    if(old_kb_pkt[i] != new_kb_pkt[i]) {
      break;
    }
  }

  if(i < 8) {
    LOG_R("oldk: ");
    LOG_HEX(old_kb_pkt,8);
    LOG_R("newk: ");
    LOG_HEX(new_kb_pkt,8);
  }

  memcpy(&old_ms_pkt[0],&new_ms_pkt[0], 8);
  return 1;
}

static int32_t usb_hid_irq(struct usb_device *dev, unsigned long pipe)
{
  if(dev->irq_status!=0) {
    LOG("usb_hid Error %lX, len %d\n",dev->irq_status,dev->act_len);
    return 1;
  }
//  if(dev->act_len != 8)
//    return usb_ms_irq(dev,pipe);
//  if(pipe == ms_pipe)
//    return usb_ms_irq(dev,pipe);
  if(pipe == kb_pipe)
    return usb_kb_irq(dev,pipe);
  return 1;
}

/* probes the USB device dev for keyboard type */
static int32_t usb_kb_probe(struct usb_device *dev, uint32_t ifnum)
{
  struct usb_interface_descriptor *iface;
  struct usb_endpoint_descriptor *ep;
  int32_t pipe,maxp;

  iface = &dev->config.if_desc[ifnum];
  if(iface->bNumEndpoints != 1) return 0;

  ep = &iface->ep_desc[0];

  if(!(ep->bEndpointAddress & 0x80)) return 0;
  if((ep->bmAttributes & 3) != 3) return 0;
  LOG("USB keyboard found set protocol...\n");
  /* ok, we found a USB Keyboard, install it */
  /* usb_kbd_get_hid_desc(dev); */
  usb_set_protocol(dev, iface->bInterfaceNumber, 0);
  LOG("USB keyboard found set idle...\n");
  usb_set_idle(dev, iface->bInterfaceNumber, REPEAT_RATE, 0);
  memset(&new_kb_pkt[0], 0, 8);
  memset(&old_kb_pkt[0], 0, 8);
  repeat_delay=0;
  kb_pipe = pipe = usb_rcvintpipe(dev, ep->bEndpointAddress);
  maxp = usb_maxpacket(dev, pipe);
  dev->irq_handle=usb_hid_irq;
  LOG("USB keyboard enable interrupt pipe...\n");
  usb_submit_int_msg(dev,pipe,&new_kb_pkt[0], maxp > 8 ? 8 : maxp,ep->bInterval);
  LOG("USB keyboard found\n");
  return 1;
}

/* probes the USB device dev for keyboard type */
static int32_t usb_ms_probe(struct usb_device *dev, uint32_t ifnum)
{
  struct usb_interface_descriptor *iface;
  struct usb_endpoint_descriptor *ep;
  int32_t pipe,maxp;

  iface = &dev->config.if_desc[ifnum];
  if(iface->bNumEndpoints != 1) return 0;

  ep = &iface->ep_desc[0];

  if(!(ep->bEndpointAddress & 0x80)) return 0;
  if((ep->bmAttributes & 3) != 3) return 0;
  LOG("USB mouse found set protocol...\n");
  /* ok, we found a USB mouse, install it */
  /* usb_kbd_get_hid_desc(dev); */
  usb_set_protocol(dev, iface->bInterfaceNumber, 0);
  LOG("USB mouse found set idle...\n");
//  usb_set_idle(dev, iface->bInterfaceNumber, REPEAT_RATE, 0);
  memset(&new_ms_pkt[0], 0, 8);
  memset(&old_ms_pkt[0], 0, 8);
  repeat_delay=0;
  ms_pipe = pipe = usb_rcvintpipe(dev, ep->bEndpointAddress);
  maxp = usb_maxpacket(dev, pipe);
  dev->irq_handle=usb_hid_irq;
  LOG("USB mouse enable interrupt pipe...\n");
  usb_submit_int_msg(dev,pipe,&new_ms_pkt[0], maxp > 8 ? 8 : maxp,ep->bInterval);
  LOG("USB mouse found\n");
  return 1;
}

static int32_t usb_hid_probe(struct usb_device *dev, uint32_t ifnum)
{
  struct usb_interface_descriptor *iface;
  iface = &dev->config.if_desc[ifnum];

  if(iface->bInterfaceClass != 3) return 0;
  if(iface->bInterfaceSubClass != 1) return 0;
  if(iface->bInterfaceProtocol == 1) return usb_kb_probe(dev, ifnum);
//  if(iface->bInterfaceProtocol == 2) return usb_ms_probe(dev, ifnum);

  return 0;
}

void usb_kbd_setraw(void) {
  kbd_raw = 1;
}

void usb_kbd_setcooked(void) {
  kbd_raw = 0;
}
