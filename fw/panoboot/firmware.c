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
#include "pano_io.h"
#include "ff.h"
#include "vt100.h"
#include "hsvcolor.h"

// #define LOG_TO_SERIAL
#define LOG_TO_BOTH
// #define DEBUG_LOGGING
// #define VERBOSE_DEBUG_LOGGING
#include "log.h"

#define F1_KEY   1  // F1
#define F2_KEY   2  // F2
#define F3_KEY   3  // F3
#define F4_KEY   4  // F4
#define F5_KEY   5  // F5
#define F6_KEY   6  // F6
#define F7_KEY   7  // F7
#define F8_KEY   8  // F8
unsigned char gFunctionRequest;

void IdlePoll();
void LoadInitProg(void);
void HandleFunctionKey(int Function);

void irq_handler(uint32_t pc)
{
   ELOG("HARD FAULT PC = 0x%08x\n",pc);
   leds = LED_BLUE;  // "blue screen of death"
   while(1);
}

void movespr(int x, int y, int v) {
    SPR_OAM_ATTR0(0) = (v ? 0 : SPR_DISABLE) | SPR_X(x) | SPR_Y(y) | SPR_MODE(0) | SPR_16_COLOR | SPR_SHAPE_SQUARE;
    SPR_OAM_ATTR1(0) = SPR_SIZE(0) | SPR_NAME(0) | SPR_PALETTE(1);
    SPR_OAM_ATTR0(1) = SPR_DISABLE;
    SPR_OAM_ATTR1(1) = 0;
}

void main()
{
   FATFS FatFs;           /* File system object for each logical drive */
   DIR Dir;               /* Directory object */
   FILINFO Finfo;
   FRESULT res;
   const char root[] = "USB:/";
   char directory[18] = "";
   char DriveSave;
   uint8_t LastIoState = 0xff;
   uint32_t IoState;
   uint32_t Timeout;
   bool bWasHalted = false;
   HsvColor hsv = { 0, 255, 127 };
   RgbColor rgb = { 0, 0, 0 };
   uint16_t i;

   dly_tap = 0x03;

   // Set interrupt mask to zero (enable all interrupts)
   // This is a PicoRV32 custom instruction
   asm(".word 0x0600000b");

   leds = LED_RED;

   // Disable all video layers while initializing palette and font data
   VID_MODE = 0;

   InitPalette();
   InitFont();
   InitSprites();

   BG0_SCR_Y = 40;

   // Set up a 80x25 Text Console (Scaled to 640x400), 64x64 Tilemap (Scale2x), Sprite for Cursor
   VID_MODE =
       VID_MODE_SPR_EN | VID_MODE_SPR_PRIORITY(0) |
       VID_MODE_BG0_EN | VID_MODE_BG0_SIZE(0) | VID_MODE_BG0_SCALE(0) | VID_MODE_BG0_PRIORITY(1)
       | VID_MODE_BG1_EN | VID_MODE_BG1_SIZE(2) | VID_MODE_BG1_SCALE(3) | VID_MODE_BG1_PRIORITY(2)
   ;

   vt100_init();
   ALOG_R("\x1b[40m\x1b[H\x1b[2J");
   for(i = 0 ; i < 256; i++) {
     ALOG_R("\x1b[%d;%dH\x1b[48;5;%dm  ", ((i >> 4)<<1)+5, ((i & 0xf)<<1)+44, i);
     ALOG_R("\x1b[%d;%dH  ", ((i >> 4)<<1)+6, ((i & 0xf)<<1)+44, i);
   }
   ALOG_R("\x1b[40;37;1m");
   for(i = ' ' ; i < 256; i++) {
     ALOG_R("\x1b[%d;%dH%c", ((i >> 4)<<1)+5, ((i & 0xf)<<1)+9, i);
   }
   ALOG_R("\x1b[H");
   ALOG_R("\x1b[47;30;1m\x1b[1J Pano Logic G1 - ");
   ALOG_R("\x1b[38;5;240mP");
   ALOG_R("\x1b[38;5;241mi");
   ALOG_R("\x1b[38;5;242mc");
   ALOG_R("\x1b[38;5;243mo");
   ALOG_R("\x1b[38;5;244mR");
   ALOG_R("\x1b[38;5;245mV");
   ALOG_R("\x1b[38;5;246m3");
   ALOG_R("\x1b[38;5;247m2");
   ALOG_R("\x1b[38;5;248m @ ");
   ALOG_R("\x1b[38;5;249m2");
   ALOG_R("\x1b[38;5;250m5");
   ALOG_R("\x1b[38;5;251mM");
   ALOG_R("\x1b[38;5;252mH");
   ALOG_R("\x1b[38;5;253mz");
   ALOG_R("\x1b[47;30;1m - Compiled " __DATE__ " " __TIME__ "\x1b[1;76H\xee \xf8\n\x1b[0m\n");
   leds = LED_GREEN;

   for(;;)
   for(i=0; i<32768; i++) {
      hsv.h = i >> 5;
      rgb = HsvToRgb(hsv);
      for(int x = 0; x < 15; x++) {
      PALETTE_RAM[240+x] = (rgb.r << 16) | (rgb.g << 8) | (rgb.b);
      hsv.h += 8;
      rgb = HsvToRgb(hsv);
      }
      PALETTE_RAM[255] = (rgb.r << 16) | (rgb.g << 8) | (rgb.b);

      movespr((i & 63) + 8, ((i >> 6) & 63) + 8, i >> 8);
   }
   InitPalette();

   for(;;);

   usb_init();
   drv_usb_kbd_init();

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

   for( ; ; ) {
      IdlePoll();
      if(usb_kbd_testc()) {
         //usb_kbd_getc() & 0x7f
         //z80_con_status = 0xff;  // console input ready
      }
   }

   leds = LED_BLUE;  // "blue screen of death"
   while(1);
}

void FunctionKeyCB(unsigned char Function)
{
   VLOG("F%d key pressed\n",Function);
   gFunctionRequest = Function;
}

void HandleFunctionKey(int Function)
{
   switch(Function) {
      case F1_KEY:
         break;
      case F2_KEY:
         break;
      case F3_KEY:
         break;
      case F4_KEY:
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
}
