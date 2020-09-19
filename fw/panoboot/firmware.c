/*
 *  Pano_z80pack
 *
 *  Copyright (C) 2019  Skip Hansen
 *
 *  This file is derived from Verilogboy project:
 *  Copyright (C) 2019  Wenting Zhang <zephray@outlook.com>
 *
 *  This file is partially derived from PicoRV32 project:
 *  Copyright (C) 2017  Clifford Wolf <clifford@clifford.at>
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
#include "misc.h"
#include "usb.h"
#include "usb_gamepad.h"
#include "pano_io.h"
#include "ff.h"
#include "vt100.h"
#include "hsvcolor.h"
#include "sincos.h"

//#define LOG_TO_SERIAL
//#define LOG_TO_BOTH
//#define DEBUG_LOGGING
//#define VERBOSE_DEBUG_LOGGING
#include "log.h"
#include "font.h"
#include "logo.h"
#include "palette.h"

void InitFont()
{
  uint16_t x,y;
  for(y = (235*8); y < (256*8); y++) {
    for(x = 0; x < 8; x++) {
      uint32_t b = (((font_8x8[y-(236*8)])>>x) & 1) ? 1 : 0;
      FONT_RAM[(y*8)+(7-x)] = b;
    }
  }
}

void InitSprites()
{
  uint16_t i;
  for(i = 0; i < sizeof(spr_8x8); i++) {
    uint32_t b = spr_8x8[i];
    SPRITE_IMG[i] = b;
  }
  for(i = 0; i < SPR_OAM_COUNT; i++) {
    SPR_OAM_ATTR0(i) = SPR_DISABLE;
  }
}

void InitLogo()
{
  uint16_t i;
  for(i = 0; i < LOGO_TILES_SIZE; i++) {
    uint32_t b = logo_tiles[i];
    BACKGROUND_IMG[i] = b;
  }
  for(i = 0; i < LOGO_MAP_SIZE; i++) {
    uint32_t b = logo_map[i];
    BACKGROUND_RAM[i] = b;
  }
}

void InitPalette()
{
  uint16_t i;
  for(i = 0; i < PALETTE_SIZE; i++) {
    uint32_t c = stock_palette[i];
    PALETTE_RAM[i] = c;
  }
}

#define F1_KEY   1  // F1
#define F2_KEY   2  // F2
#define F3_KEY   3  // F3
#define F4_KEY   4  // F4
#define F5_KEY   5  // F5
#define F6_KEY   6  // F6
#define F7_KEY   7  // F7
#define F8_KEY   8  // F8
unsigned char gFunctionRequest;

unsigned char gMouseRequest;

unsigned char oldMouseButtons = 0;
unsigned char newMouseButtons = 0;

unsigned char mouseButtonsPressed = 0;
unsigned char mouseButtonsReleased = 0;
unsigned char mouseButtonsHeld = 0;

int newMouseOffsetX = 0;
int newMouseOffsetY = 0;

extern uint16_t VideoWidth;
extern uint16_t VideoHeight;

uint16_t mouse_x = 0;
uint16_t mouse_y = 0;
uint16_t desktop_x = 0;
uint16_t desktop_y = 40;
uint16_t desktop_w = 640;
uint16_t desktop_h = 400;

void IdlePoll();

void HandleFunctionKey(int Function);
void HandleMouse();

void MoveMouse() {
  SPR_OAM_ATTR0(0) = SPR_X(mouse_x) | SPR_Y(mouse_y) | SPR_MODE(0) | SPR_16_COLOR | SPR_SHAPE_SQUARE;
  SPR_OAM_ATTR1(0) = SPR_SIZE(1) | SPR_NAME(12) | SPR_PALETTE(14);
}

void main()
{
   FATFS FatFs;           /* File system object for each logical drive */
   DIR Dir;               /* Directory object */
   FILINFO Finfo;
   FRESULT res;
   const char root[] = "USB:/";
   char directory[18] = "";

   dly_tap = 0x03;

   leds = LED_RED;

   // Disable all video layers while initializing palette and font data
   VID_MODE = 0;

//   InitVideo(1024,768,75);
   InitVideo(800,600,60);
//   InitVideo(640,480,60);
//   InitVideo(320,480,60);

// Boot Logo shown while USB is initialized
   uint16_t OffsetX = (VideoWidth-512)/2;
   uint16_t OffsetY = (VideoHeight-192)/2;

   BG0_BORDER = 0;
   BG1_BORDER = 0;
   BG1_SCR_X = OffsetX;
   BG1_SCR_Y = OffsetY;
   BG1_WIN_X = ((512 + OffsetX) << 16) | OffsetX;
   BG1_WIN_Y = ((128 + OffsetY) << 16) | OffsetY;

   InitLogo();

   WaitVSync(1);
   PALETTE_RAM[0] = 0x000000;
   PALETTE_RAM[1] = 0xffffff;

   // Set up a 64x64 Tilemap for Boot Logo
   VID_MODE = VID_MODE_BG1_EN | VID_MODE_BG1_SIZE(2) | VID_MODE_BG1_SCALE(0) | VID_MODE_BG1_PRIORITY(0);
   WaitVSync(0);

   InitFont();
   InitSprites();

   vt100_init();
   usb_init();
   drv_usb_hid_init();
   usb_stor_scan(1);
//   drv_usb_gp_init();

   // Set interrupt mask to zero (enable all interrupts)
   // This is a PicoRV32 custom instruction
   asm(".word 0x0600000b");

   leds = LED_GREEN;

   WaitVSync(1);

   desktop_w = 640;
   desktop_h = 400;
   desktop_x = (VideoWidth - desktop_w)/2;
   desktop_y = (VideoHeight - desktop_h)/2;

   // Place the mouse at the center of the screen
   mouse_x = desktop_x + (desktop_w / 2);
   mouse_y = desktop_y + (desktop_h / 2);

   //MoveMouse();

   BG0_SCR_X = desktop_x;
   BG0_SCR_Y = desktop_y;
   BG0_WIN_X = ((desktop_w + desktop_x) << 16) | desktop_x;
   BG0_WIN_Y = ((desktop_h + desktop_y) << 16) | desktop_y;

   VID_MODE = 0
//       | VID_MODE_SPR_EN | VID_MODE_SPR_PRIORITY(0)
       | VID_MODE_BG0_EN | VID_MODE_BG0_SIZE(0) | VID_MODE_BG0_SCALE(0) | VID_MODE_BG0_PRIORITY(1)
   ;
   InitPalette();

   BG0_BORDER = 255;

   for(;;) {
      WaitVSync(1);

      //MoveMouse();
      //VID_HSYNCIRQ = 240;

      WaitVSync(0);

      IdlePoll();
//      if(usb_kbd_testc()) {
         //usb_kbd_getc() & 0x7f
//      }
   }

#if 0
   do {
      // Main loop
      res = f_mount(&FatFs, "", 1);
      if(res != FR_OK) {
         ELOG("Unable to mount filesystem: %d\n", (int)res);
         break;
      }

      LOG("Current directory: %s%s\n", root, directory);

      // First list all files
      res = f_opendir(&Dir, directory);
      if(res != FR_OK) {
         ELOG("Unable to open directory: %d\n", (int)res);
         break;
      }
      for(;;) {
         res = f_readdir(&Dir, &Finfo);
         if((res != FR_OK) || !Finfo.fname[0]) {
            break;
         }

         LOG_R("%-12s ", Finfo.fname);
         LOG_R("%7d ", Finfo.fsize);
         LOG_R("%c%c%c%c%c ",
                (Finfo.fattrib & AM_DIR) ? 'D' : '-',
                (Finfo.fattrib & AM_RDO) ? 'R' : '-',
                (Finfo.fattrib & AM_HID) ? 'H' : '-',
                (Finfo.fattrib & AM_SYS) ? 'S' : '-',
                (Finfo.fattrib & AM_ARC) ? 'A' : '-');
         LOG_R("%2d/%02d/%d %2d:%02d:%02d\n",
                (Finfo.fdate >> 5) & 0xf,
                (Finfo.fdate & 31),
                (Finfo.fdate >> 9) + 1980,
                (Finfo.ftime >> 11),
                (Finfo.ftime >> 5) & 0x3f);
      }
      f_closedir(&Dir);
   } while(false);
#endif

   leds = LED_BLUE;  // "blue screen of death"
   for(;;);
}

void FunctionKeyCB(unsigned char Function)
{
   VLOG("F%d key pressed\n",Function);
   gFunctionRequest = Function;
}

void MouseCB(unsigned char Buttons, int OffsetX, int OffsetY)
{
   newMouseButtons = Buttons;
   newMouseOffsetX = OffsetX;
   newMouseOffsetY = OffsetY;
   gMouseRequest = 1;
}

void HandleMouse()
{
   mouseButtonsHeld = oldMouseButtons & newMouseButtons;
   mouseButtonsPressed = (oldMouseButtons ^ newMouseButtons) & ~oldMouseButtons;
   mouseButtonsReleased = (oldMouseButtons ^ newMouseButtons) & oldMouseButtons;
   mouse_x += newMouseOffsetX;
   mouse_y += newMouseOffsetY;
   if(mouse_x < desktop_x)
     mouse_x = desktop_x;
   if(mouse_x > (desktop_x + desktop_w - 1))
     mouse_x = desktop_x + desktop_w - 1;
   if(mouse_y < desktop_y)
     mouse_y = desktop_y;
   if(mouse_y > (desktop_y + desktop_h - 1))
     mouse_y = desktop_y + desktop_h - 1;
}

void HandleFunctionKey(int Function)
{
   switch(Function) {
      case F1_KEY:
         mouse_x--;
         break;
      case F2_KEY:
         mouse_x++;
         break;
      case F3_KEY:
         mouse_y--;
         break;
      case F4_KEY:
         mouse_y++;
         break;
      case F5_KEY:
         break;
      case F6_KEY:
         break;
      case F7_KEY:
         break;
      case F8_KEY:
         break;
   }
}

void IdlePoll()
{
   usb_event_poll();
   if(gFunctionRequest != 0) {
      HandleFunctionKey(gFunctionRequest);
      gFunctionRequest = 0;
   }
   if(gMouseRequest != 0) {
      HandleMouse();
      gMouseRequest = 0;
   }
   if(gp_num_analogs >= 2) {
      newMouseOffsetX = gp_analog[0];
      newMouseOffsetY = gp_analog[1];
      HandleMouse();
   }
}
