/*
 * This file is from u-boot project:
 *
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Note: Part of this code has been derived from linux
 *
 */
#ifndef _USB_H_
#define _USB_H_

#include <stdint.h>
#include <stdbool.h>

#define __LITTLE_ENDIAN
/* Configuration for slave device support */
#undef CONFIG_USB_KEYBOARD
#define CONFIG_USB_GAMEPAD
#define CONFIG_USB_STORAGE

#define cpu_to_le16(x) (x)
#define le16_to_cpu(x) (x)
#define cpu_to_le32(x) (x)
#define le32_to_cpu(x) (x)
#define le16_to_cpus(x)

#define cpu_to_be32(x) (((x & 0xff000000) >> 16) | ((x & 0xff0000) >> 8) | \
                     ((x & 0xff00) << 8) | ((x & 0xff) << 24))

#include "usb_defs.h"

#define USB_BUFSIZ   512
extern uint8_t usb_buf[USB_BUFSIZ];

/* Everything is aribtrary */
#define USB_ALTSETTINGALLOC      4
#define USB_MAXALTSETTING     128   /* Hard limit */

#define USB_MAX_DEVICE        5
#define USB_MAXCONFIG         1
#define USB_MAXINTERFACES     4   // DualShock 4 use 4 interfaces
#define USB_MAXENDPOINTS      2
#define USB_MAXCHILDREN       3  /* This is arbitrary */
#define USB_MAX_HUB           2

#define USB_CNTL_TIMEOUT 100 /* 100ms timeout */

/* String descriptor */
struct usb_string_descriptor {
   uint8_t  bLength;
   uint8_t  bDescriptorType;
   uint16_t wData[1];
} __attribute__ ((packed));

/* device request (setup) */
struct devrequest {
   uint8_t  requesttype;
   uint8_t  request;
   uint16_t value;
   uint16_t index;
   uint16_t length;
} __attribute__ ((packed));

/* All standard descriptors have these 2 fields in common */
struct usb_descriptor_header {
   uint8_t  bLength;
   uint8_t  bDescriptorType;
} __attribute__ ((packed));

/* Device descriptor */
struct usb_device_descriptor {
   uint8_t  bLength;
   uint8_t  bDescriptorType;
   uint16_t bcdUSB;
   uint8_t  bDeviceClass;
   uint8_t  bDeviceSubClass;
   uint8_t  bDeviceProtocol;
   uint8_t  bMaxPacketSize0;
   uint16_t idVendor;
   uint16_t idProduct;
   uint16_t bcdDevice;
   uint8_t  iManufacturer;
   uint8_t  iProduct;
   uint8_t  iSerialNumber;
   uint8_t  bNumConfigurations;
} __attribute__ ((packed));

/* Endpoint descriptor */
struct usb_endpoint_descriptor {
   uint8_t  bLength;
   uint8_t  bDescriptorType;
   uint8_t  bEndpointAddress;
   uint8_t  bmAttributes;
   uint16_t wMaxPacketSize;
   uint8_t  bInterval;
   uint8_t  bRefresh;
   uint8_t  bSynchAddress;
} __attribute__ ((packed)) __attribute__ ((aligned(2)));

/* Interface descriptor */
struct usb_interface_descriptor {
   uint8_t  bLength;
   uint8_t  bDescriptorType;
   uint8_t  bInterfaceNumber;
   uint8_t  bAlternateSetting;
   uint8_t  bNumEndpoints;
   uint8_t  bInterfaceClass;
   uint8_t  bInterfaceSubClass;
   uint8_t  bInterfaceProtocol;
   uint8_t  iInterface;

   uint8_t  no_of_ep;
   uint8_t  num_altsetting;
   uint8_t  act_altsetting;

   struct usb_endpoint_descriptor ep_desc[USB_MAXENDPOINTS];
} __attribute__ ((packed));


/* Configuration descriptor information.. */
struct usb_config_descriptor {
   uint8_t  bLength;
   uint8_t  bDescriptorType;
   uint16_t wTotalLength;
   uint8_t  bNumInterfaces;
   uint8_t  bConfigurationValue;
   uint8_t  iConfiguration;
   uint8_t  bmAttributes;
   uint8_t  MaxPower;

   uint8_t  no_of_if;   /* number of interfaces */
   struct usb_interface_descriptor if_desc[USB_MAXINTERFACES];
} __attribute__ ((packed));

/* HID descriptor */
struct usb_hid_descriptor {
   uint8_t  bLength;
   uint8_t  bDescriptorType;
   uint16_t  bcdHID;
   uint8_t   bCountryCode;
   uint8_t  bNumDescriptors;
   uint8_t   bReportDescriptorType;
   uint16_t wItemLength;
} __attribute__ ((packed));

enum {
   /* Maximum packet size; encoded as 0,1,2,3 = 8,16,32,64 */
   PACKET_SIZE_8   = 0,
   PACKET_SIZE_16  = 1,
   PACKET_SIZE_32  = 2,
   PACKET_SIZE_64  = 3,
};

struct usb_device {
   int32_t   devnum;        /* Device number on USB bus */
   int32_t   speed;         /* full/low/high */
   int8_t  mf[32];        /* manufacturer */
   int8_t  prod[40];      /* product */
   //int8_t   serial[32];    /* serial number */

   /* Maximum packet size; one of: PACKET_SIZE_* */
   int32_t maxpacketsize;
   /* one bit for each endpoint ([0] = IN, [1] = OUT) */
   uint32_t toggle[2];
   /* endpoint halts; one bit per endpoint # & direction;
    * [0] = IN, [1] = OUT
    */
   uint32_t halted[2];
   int32_t epmaxpacketin[16];     /* INput endpoint specific maximums */
   int32_t epmaxpacketout[16];    /* OUTput endpoint specific maximums */

   int32_t configno;        /* selected config number */
   struct usb_device_descriptor descriptor; /* Device Descriptor */
   struct usb_config_descriptor config; /* config descriptor */

   int32_t have_langid;     /* whether string_langid is valid yet */
   int32_t string_langid;      /* language ID for strings */
   int32_t (*irq_handle)(struct usb_device *dev, uint32_t pipe);
   uint32_t irq_status;
   void *privptr;

   /* HID specific information */
   struct usb_hid_descriptor hid_descriptor;

   /*
    * Child devices -  if this is a hub device
    * Each instance needs its own set of data structures.
    */
   uint32_t status;
   int32_t act_len;         /* transfered bytes */
   int32_t maxchild;        /* Number of ports if hub */
   int32_t portnr;
   struct usb_device *parent;
   struct usb_device *children[USB_MAXCHILDREN];
};

typedef struct block_dev_desc {
   int32_t      dev;     /* device number */
   uint8_t  target;     /* target SCSI ID */
   uint8_t  lun;     /* target LUN */
   uint8_t  type;    /* device type */
   uint8_t  removable;  /* removable device */
   uint32_t  lba;     /* number of blocks */
   uint32_t  blksz;      /* block size */
   int8_t     vendor [40+1]; /* IDE model, SCSI Vendor */
   int8_t     product[20+1]; /* IDE Serial no, SCSI product */
   int8_t     revision[8+1]; /* firmware revision */
   uint32_t  (*block_read)(int32_t dev,
                  uint32_t start,
                  uint32_t blkcnt,
                  void *buffer);
   uint32_t  (*block_write)(int32_t dev,
                   uint32_t start,
                   uint32_t blkcnt,
                   const void *buffer);
   void     *priv;      /* driver private struct pointer */
}block_dev_desc_t;

/**********************************************************************
 * this is how the lowlevel part communicate with the outer world
 */

int32_t usb_lowlevel_init(void);
int32_t usb_lowlevel_stop(void);
int32_t submit_bulk_msg(struct usb_device *dev, uint32_t pipe,
                    void *buffer, int32_t transfer_len,int32_t Timeout);
int32_t submit_control_msg(struct usb_device *dev, uint32_t pipe, void *buffer,
         int32_t transfer_len, struct devrequest *setup);
int32_t submit_int_msg(struct usb_device *dev, uint32_t pipe, void *buffer,
         int32_t transfer_len, int32_t interval);
void usb_event_poll(void);

/* Defines */
#define USB_UHCI_VEND_ID   0x8086
#define USB_UHCI_DEV_ID    0x7112

// USB Mass stoarge device
#define USB_MAX_STOR_DEV 1
block_dev_desc_t *usb_stor_get_dev(int32_t index);
int32_t usb_stor_scan(int32_t mode);
int32_t usb_stor_info(void);


// USB Keyboard & Mouse
int32_t drv_usb_hid_init(void);
extern uint8_t gCapsLockSwap;

int32_t usb_kbd_deregister(void);
int32_t usb_kbd_testc(void);
uint16_t usb_kbd_getc(void);
void usb_kbd_setraw(void);
void usb_kbd_setcooked(void);
int32_t usb_mouse_deregister(void);

// USB Game pad
int32_t drv_usb_gp_init(void);
int32_t usb_gp_deregister(void);

/* routines */
int32_t usb_init(void); /* initialize the USB Controller */
int32_t usb_stop(void); /* stop the USB Controller */

int32_t usb_set_protocol(struct usb_device *dev, int32_t ifnum, int32_t protocol);
int32_t usb_set_idle(struct usb_device *dev, int32_t ifnum, int32_t duration,
         int32_t report_id);
struct usb_device *usb_get_dev_index(int32_t index);
int32_t usb_control_msg(struct usb_device *dev, uint32_t pipe,
         uint8_t request, uint8_t requesttype,
         uint16_t value, uint16_t index,
         void *data, uint16_t size, int32_t timeout);
int32_t usb_bulk_msg(struct usb_device *dev, uint32_t pipe,
         void *data, int32_t len, int32_t *actual_length, int32_t timeout);
int32_t usb_submit_int_msg(struct usb_device *dev, uint32_t pipe,
         void *buffer, int32_t transfer_len, int32_t interval);
void usb_disable_asynch(int32_t disable);
int32_t usb_maxpacket(struct usb_device *dev, uint32_t pipe);
//inline void wait_ms(uint32_t ms);
int32_t usb_get_configuration_no(struct usb_device *dev, uint8_t *buffer,
            int32_t cfgno);
int32_t usb_get_report(struct usb_device *dev, int32_t ifnum, uint8_t type,
         uint8_t id, void *buf, int32_t size);
int32_t usb_get_class_descriptor(struct usb_device *dev, int32_t ifnum,
         uint8_t type, uint8_t id, void *buf,
         int32_t size);
int32_t usb_clear_halt(struct usb_device *dev, int32_t pipe);
int32_t usb_string(struct usb_device *dev, int32_t index, int8_t *buf, uint32_t size);
int32_t usb_set_interface(struct usb_device *dev, int32_t interface, int32_t alternate);

/* big endian -> little endian conversion */
/* some CPUs are already little endian e.g. the ARM920T */
#define __swap_16(x) \
   ({ uint16_t x_ = (uint16_t)x; \
    (uint16_t)( \
      ((x_ & 0x00FFU) << 8) | ((x_ & 0xFF00U) >> 8)); \
   })
#define __swap_32(x) \
   ({ uint32_t x_ = (uint32_t)x; \
    (uint32_t)( \
      ((x_ & 0x000000FFUL) << 24) | \
      ((x_ & 0x0000FF00UL) <<  8) | \
      ((x_ & 0x00FF0000UL) >>  8) | \
      ((x_ & 0xFF000000UL) >> 24)); \
   })

#ifdef __LITTLE_ENDIAN
# define swap_16(x) (x)
# define swap_32(x) (x)
#else
# define swap_16(x) __swap_16(x)
# define swap_32(x) __swap_32(x)
#endif

/*
 * Calling this entity a "pipe" is glorifying it. A USB pipe
 * is something embarrassingly simple: it basically consists
 * of the following information:
 *  - device number (7 bits)
 *  - endpoint number (4 bits)
 *  - current Data0/1 state (1 bit)
 *  - direction (1 bit)
 *  - speed (2 bits)
 *  - max packet size (2 bits: 8, 16, 32 or 64)
 *  - pipe type (2 bits: control, interrupt, bulk, isochronous)
 *
 * That's 18 bits. Really. Nothing more. And the USB people have
 * documented these eighteen bits as some kind of glorious
 * virtual data structure.
 *
 * Let's not fall in that trap. We'll just encode it as a simple
 * uint32_t. The encoding is:
 *
 *  - max size:      bits 0-1 (00 = 8, 01 = 16, 10 = 32, 11 = 64)
 *  - direction:  bit 7    (0 = Host-to-Device [Out],
 *             (1 = Device-to-Host [In])
 *  - device:     bits 8-14
 *  - endpoint:      bits 15-18
 *  - Data0/1:    bit 19
 *  - speed:      bit 26      (0 = Full, 1 = Low Speed, 2 = High)
 *  - pipe type:  bits 30-31  (00 = isochronous, 01 = interrupt,
 *              10 = control, 11 = bulk)
 *
 * Why? Because it's arbitrary, and whatever encoding we select is really
 * up to us. This one happens to share a lot of bit positions with the UHCI
 * specification, so that much of the uhci driver can just mask the bits
 * appropriately.
 */
/* Create various pipes... */
#define create_pipe(dev,endpoint) \
      (((dev)->devnum << 8) | (endpoint << 15) | \
      ((dev)->speed << 26) | (dev)->maxpacketsize)
#define default_pipe(dev) ((dev)->speed << 26)

#define usb_sndctrlpipe(dev, endpoint) ((PIPE_CONTROL << 30) | \
                create_pipe(dev, endpoint))
#define usb_rcvctrlpipe(dev, endpoint) ((PIPE_CONTROL << 30) | \
                create_pipe(dev, endpoint) | \
                USB_DIR_IN)
#define usb_sndisocpipe(dev, endpoint) ((PIPE_ISOCHRONOUS << 30) | \
                create_pipe(dev, endpoint))
#define usb_rcvisocpipe(dev, endpoint) ((PIPE_ISOCHRONOUS << 30) | \
                create_pipe(dev, endpoint) | \
                USB_DIR_IN)
#define usb_sndbulkpipe(dev, endpoint) ((PIPE_BULK << 30) | \
                create_pipe(dev, endpoint))
#define usb_rcvbulkpipe(dev, endpoint) ((PIPE_BULK << 30) | \
                create_pipe(dev, endpoint) | \
                USB_DIR_IN)
#define usb_sndintpipe(dev, endpoint)  ((PIPE_INTERRUPT << 30) | \
                create_pipe(dev, endpoint))
#define usb_rcvintpipe(dev, endpoint)  ((PIPE_INTERRUPT << 30) | \
                create_pipe(dev, endpoint) | \
                USB_DIR_IN)
#define usb_snddefctrl(dev)      ((PIPE_CONTROL << 30) | \
                default_pipe(dev))
#define usb_rcvdefctrl(dev)      ((PIPE_CONTROL << 30) | \
                default_pipe(dev) | \
                USB_DIR_IN)

/* The D0/D1 toggle bits */
#define usb_gettoggle(dev, ep, out) (((dev)->toggle[out] >> ep) & 1)
#define usb_dotoggle(dev, ep, out)  ((dev)->toggle[out] ^= (1 << ep))
#define usb_settoggle(dev, ep, out, bit) ((dev)->toggle[out] = \
                  ((dev)->toggle[out] & \
                   ~(1 << ep)) | ((bit) << ep))

/* Endpoint halt control/status */
#define usb_endpoint_out(ep_dir) (((ep_dir >> 7) & 1) ^ 1)
#define usb_endpoint_halt(dev, ep, out) ((dev)->halted[out] |= (1 << (ep)))
#define usb_endpoint_running(dev, ep, out) ((dev)->halted[out] &= ~(1 << (ep)))
#define usb_endpoint_halted(dev, ep, out) ((dev)->halted[out] & (1 << (ep)))

#define usb_packetid(pipe) (((pipe) & USB_DIR_IN) ? USB_PID_IN : \
             USB_PID_OUT)

#define usb_pipeout(pipe)  ((((pipe) >> 7) & 1) ^ 1)
#define usb_pipein(pipe)   (((pipe) >> 7) & 1)
#define usb_pipedevice(pipe)  (((pipe) >> 8) & 0x7f)
#define usb_pipe_endpdev(pipe)   (((pipe) >> 8) & 0x7ff)
#define usb_pipeendpoint(pipe)   (((pipe) >> 15) & 0xf)
#define usb_pipedata(pipe) (((pipe) >> 19) & 1)
#define usb_pipespeed(pipe)   (((pipe) >> 26) & 3)
#define usb_pipeslow(pipe) (usb_pipespeed(pipe) == USB_SPEED_LOW)
#define usb_pipetype(pipe) (((pipe) >> 30) & 3)
#define usb_pipeisoc(pipe) (usb_pipetype((pipe)) == PIPE_ISOCHRONOUS)
#define usb_pipeint(pipe)  (usb_pipetype((pipe)) == PIPE_INTERRUPT)
#define usb_pipecontrol(pipe) (usb_pipetype((pipe)) == PIPE_CONTROL)
#define usb_pipebulk(pipe) (usb_pipetype((pipe)) == PIPE_BULK)


/*************************************************************************
 * Hub Stuff
 */
struct usb_port_status {
   uint16_t wPortStatus;
   uint16_t wPortChange;
} __attribute__ ((packed));

struct usb_hub_status {
   uint16_t wHubStatus;
   uint16_t wHubChange;
} __attribute__ ((packed));


/* Hub descriptor */
struct usb_hub_descriptor {
   uint8_t  bLength;
   uint8_t  bDescriptorType;
   uint8_t  bNbrPorts;
   uint16_t wHubCharacteristics;
   uint8_t  bPwrOn2PwrGood;
   uint8_t  bHubContrCurrent;
   uint8_t  DeviceRemovable[(USB_MAXCHILDREN+1+7)/8];
   uint8_t  PortPowerCtrlMask[(USB_MAXCHILDREN+1+7)/8];
   /* DeviceRemovable and PortPwrCtrlMask want to be variable-length
      bitmaps that hold max 255 entries. (bit0 is ignored) */
} __attribute__ ((packed));


struct usb_hub_device {
   struct usb_device *pusb_dev;
   struct usb_hub_descriptor desc;
};

#endif /*_USB_H_ */
