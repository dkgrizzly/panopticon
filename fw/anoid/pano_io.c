/*
 *  pano_io.c
 *
 *  Copyright (C) 2019  Skip Hansen
 *
 *  Code derived from Z80SIM
 *  Copyright (C) 1987-2017 by Udo Munk
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
 * Copyright (C) 1987-2017 by Udo Munk
 *
 */

/*
 * This module contains the I/O handlers for hardware.
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "pano_io.h"
#include "usb.h"
#include "misc.h"

// #define DEBUG_LOGGING
// #define LOG_TO_BOTH
// #define VERBOSE_DEBUG_LOGGING
#include "log.h"
#include "font.h"
#include "palette.h"
#include "logo.h"

uint16_t VideoWidth = 640;
uint16_t VideoHeight = 480;
uint8_t VideoHz = 60;

void WaitVSync(int v)
{
  if(v) {
    while(VID_SCANLINE < VideoHeight) {
    // Do something other than busy wait, ideally.
    }
  } else {
    while(VID_SCANLINE >= VideoHeight) {
    // Do something other than busy wait, ideally.
    }
  }
}

void InitVideo(uint16_t x, uint16_t y, uint8_t hz)
{
  if((x == 1024) && (y == 768)) {
    // 1024x768 75Hz VSync, 60.022KHz HSync, 78.75MHz Pixel
    if(hz == 75) {
      IDT_CONFIG = 0x323B62;
      VID_TIMING_FRONT = VID_TIMING_H(16) | VID_TIMING_V(1);
      VID_TIMING_SYNC = VID_TIMING_H(96) | VID_TIMING_V(3);
      VID_TIMING_BACK = VID_TIMING_H(176) | VID_TIMING_V(28);
      VID_TIMING_ACTIVE = VID_TIMING_H(1024) | VID_TIMING_V(768);
      VideoWidth = 1024;
      VideoHeight = 768;
      VideoHz = 75;
      return;
    }

    // 1024x768 70Hz VSync, 56.475KHz HSync, 75MHz Pixel
    if(hz == 70) {
      IDT_CONFIG = 0x312197;
      VID_TIMING_FRONT = VID_TIMING_H(24) | VID_TIMING_V(3);
      VID_TIMING_SYNC = VID_TIMING_H(136) | VID_TIMING_V(6);
      VID_TIMING_BACK = VID_TIMING_H(144) | VID_TIMING_V(29);
      VID_TIMING_ACTIVE = VID_TIMING_H(1024) | VID_TIMING_V(768);
      VideoWidth = 1024;
      VideoHeight = 768;
      VideoHz = 70;
      return;
    }

    // 1024x768 60Hz VSync, 48.363KHz HSync, 65MHz Pixel
    IDT_CONFIG = 0x311C97;
    VID_TIMING_FRONT = VID_TIMING_H(24) | VID_TIMING_V(3);
    VID_TIMING_SYNC = VID_TIMING_H(136) | VID_TIMING_V(6);
    VID_TIMING_BACK = VID_TIMING_H(160) | VID_TIMING_V(29);
    VID_TIMING_ACTIVE = VID_TIMING_H(1024) | VID_TIMING_V(768);
    VideoWidth = 1024;
    VideoHeight = 768;
    VideoHz = 60;
    return;
  }
  if((x == 800) && (y == 600)) {
    // 800x600 85Hz VSync, 53.673KHz HSync, 56.25MHz Pixel
    if(hz == 85) {
      IDT_CONFIG = 0x312926;
      VID_TIMING_FRONT = VID_TIMING_H(32) | VID_TIMING_V(1);
      VID_TIMING_SYNC = VID_TIMING_H(64) | VID_TIMING_V(3);
      VID_TIMING_BACK = VID_TIMING_H(152) | VID_TIMING_V(27);
      VID_TIMING_ACTIVE = VID_TIMING_H(800) | VID_TIMING_V(600);
      VideoWidth = 800;
      VideoHeight = 600;
      VideoHz = 85;
      return;
    }
    // 800x600 75Hz VSync, 46.875KHz HSync, 49.50MHz Pixel
    if(hz == 75) {
      IDT_CONFIG = 0x312DB0;
      VID_TIMING_FRONT = VID_TIMING_H(16) | VID_TIMING_V(1);
      VID_TIMING_SYNC = VID_TIMING_H(80) | VID_TIMING_V(2);
      VID_TIMING_BACK = VID_TIMING_H(160) | VID_TIMING_V(21);
      VID_TIMING_ACTIVE = VID_TIMING_H(800) | VID_TIMING_V(600);
      VideoWidth = 800;
      VideoHeight = 600;
      VideoHz = 75;
      return;
    }
    // 800x600 72Hz VSync, 48.076KHz HSync, 50MHz Pixel
    if(hz == 72) {
      IDT_CONFIG = 0x311517;
      VID_TIMING_FRONT = VID_TIMING_H(56) | VID_TIMING_V(37);
      VID_TIMING_SYNC = VID_TIMING_H(120) | VID_TIMING_V(6);
      VID_TIMING_BACK = VID_TIMING_H(64) | VID_TIMING_V(23);
      VID_TIMING_ACTIVE = VID_TIMING_H(800) | VID_TIMING_V(600);
      VideoWidth = 800;
      VideoHeight = 600;
      VideoHz = 72;
      return;
    }
    // 800x600 56Hz VSync, 35.018KHz HSync, 38.1MHz Pixel
    if(hz == 56) {
      IDT_CONFIG = 0x33BAFB;
      VID_TIMING_FRONT = VID_TIMING_H(32) | VID_TIMING_V(37);
      VID_TIMING_SYNC = VID_TIMING_H(128) | VID_TIMING_V(6);
      VID_TIMING_BACK = VID_TIMING_H(128) | VID_TIMING_V(23);
      VID_TIMING_ACTIVE = VID_TIMING_H(800) | VID_TIMING_V(600);
      VideoWidth = 800;
      VideoHeight = 600;
      VideoHz = 56;
      return;
    }
    // 800x600 60Hz VSync, 37.878KHz HSync, 40MHz Pixel
    IDT_CONFIG = 0x311017;
    VID_TIMING_FRONT = VID_TIMING_H(40) | VID_TIMING_V(1);
    VID_TIMING_SYNC = VID_TIMING_H(128) | VID_TIMING_V(4);
    VID_TIMING_BACK = VID_TIMING_H(88) | VID_TIMING_V(23);
    VID_TIMING_ACTIVE = VID_TIMING_H(800) | VID_TIMING_V(600);
    VideoWidth = 800;
    VideoHeight = 600;
    VideoHz = 60;
    return;
  }
  if((x == 640) && (y == 480)) {
    if(hz == 85) {
      // 640x480 85Hz VSync, 31.778KHz HSync, 25.175MHz Pixel
      IDT_CONFIG = 0x348BF0;
      VID_TIMING_FRONT = VID_TIMING_H(16) | VID_TIMING_V(11);
      VID_TIMING_SYNC = VID_TIMING_H(96) | VID_TIMING_V(2);
      VID_TIMING_BACK = VID_TIMING_H(48) | VID_TIMING_V(31);
      VID_TIMING_ACTIVE = VID_TIMING_H(640) | VID_TIMING_V(480);
      VideoWidth = 640;
      VideoHeight = 480;
      VideoHz = 85;
      return;
    }
    if(hz == 75) {
      // 640x480 75Hz VSync, 39.375KHz HSync, 31.5MHz Pixel
      IDT_CONFIG = 0x311BB0;
      VID_TIMING_FRONT = VID_TIMING_H(16) | VID_TIMING_V(11);
      VID_TIMING_SYNC = VID_TIMING_H(96) | VID_TIMING_V(2);
      VID_TIMING_BACK = VID_TIMING_H(48) | VID_TIMING_V(32);
      VID_TIMING_ACTIVE = VID_TIMING_H(640) | VID_TIMING_V(480);
      VideoWidth = 640;
      VideoHeight = 480;
      VideoHz = 75;
      return;
    }
    if(hz == 72) {
      // 640x480 72Hz VSync, 37.860KHz HSync, 31.5MHz Pixel
      IDT_CONFIG = 0x311BB0;
      VID_TIMING_FRONT = VID_TIMING_H(24) | VID_TIMING_V(9);
      VID_TIMING_SYNC = VID_TIMING_H(40) | VID_TIMING_V(3);
      VID_TIMING_BACK = VID_TIMING_H(128) | VID_TIMING_V(28);
      VID_TIMING_ACTIVE = VID_TIMING_H(640) | VID_TIMING_V(480);
      VideoWidth = 640;
      VideoHeight = 480;
      VideoHz = 72;
      return;
    }
  }
  if((x == 512) && (y == 384)) {
    //if(hz == 60) {
      // Apple Macintosh Classic / LC Video
      // 512x384 60Hz VSync, 24.48KHz HSync, 15.67MHz Pixel
      IDT_CONFIG = 0x348BF0;
      VID_TIMING_FRONT = VID_TIMING_H(16) | VID_TIMING_V(1);
      VID_TIMING_SYNC = VID_TIMING_H(32) | VID_TIMING_V(3);
      VID_TIMING_BACK = VID_TIMING_H(80) | VID_TIMING_V(19);
      VID_TIMING_ACTIVE = VID_TIMING_H(512) | VID_TIMING_V(384);
      VideoWidth = 512;
      VideoHeight = 384;
      VideoHz = 60;
      return;
    //}
  }

  if((x == 320) && (y == 480)) {
    // 320x480 60Hz VSync, 31.47KHz HSync, 12.587MHz Pixel
    IDT_CONFIG = 0x3567F8;
    VID_TIMING_FRONT = VID_TIMING_H(8) | VID_TIMING_V(11);
    VID_TIMING_SYNC = VID_TIMING_H(48) | VID_TIMING_V(2);
    VID_TIMING_BACK = VID_TIMING_H(24) | VID_TIMING_V(31);
    VID_TIMING_ACTIVE = VID_TIMING_H(320) | VID_TIMING_V(480);
    VideoWidth = 320;
    VideoHeight = 480;
    VideoHz = 60;
    return;
  }

  // 640x480 60Hz VSync, 31.778KHz HSync, 25.175MHz Pixel
  IDT_CONFIG = 0x348BF0;
  VID_TIMING_FRONT = VID_TIMING_H(16) | VID_TIMING_V(11);
  VID_TIMING_SYNC = VID_TIMING_H(96) | VID_TIMING_V(2);
  VID_TIMING_BACK = VID_TIMING_H(48) | VID_TIMING_V(31);
  VID_TIMING_ACTIVE = VID_TIMING_H(640) | VID_TIMING_V(480);
  VideoWidth = 640;
  VideoHeight = 480;
  VideoHz = 60;
  return;
}

void InitFont()
{
    for(int y = (235*8); y < (256*8); y++) {
        for(int x = 0; x < 8; x++) {
            uint32_t b = (((font_8x8[y-(236*8)])>>x) & 1) ? 1 : 0;
            FONT_RAM[(y*8)+(7-x)] = b;
        }
    }
}

void InitSprites()
{
    for(int i = 0; i < sizeof(spr_8x8); i++) {
        uint32_t b = spr_8x8[i];
        SPRITE_IMG[i] = b;
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
    for(int i = 0; i < (sizeof(stock_palette)/sizeof(uint32_t)); i++) {
        uint32_t c = stock_palette[i];
        PALETTE_RAM[i] = c;
    }
}

void UartPutc(char c)
{
  if(!(UART0_CTL & 0x2)) {
    UART0_DAT = (uint32_t) c;
  }
  if(!(UART1_CTL & 0x2)) {
    UART1_DAT = (uint32_t) c;
  }
}

void PrintfPutc(char c)
{
  UartPutc(c);
}

void LogPutc(char c,void *arg)
{
   int LogFlags = (int) arg;

   if(!(LogFlags & LOG_DISABLED)) {
      if(LogFlags & LOG_SERIAL) {
         UartPutc(c);
      }
   }
}

#ifndef LOGGING_DISABLED
void LogHex(char *LogFlags,void *Data,int Len)
{
   int i;
   uint8_t *cp = (uint8_t *) Data;

   for(i = 0; i < Len; i++) {
      if(i != 0 && (i & 0xf) == 0) {
         _LOG(LogFlags,"\n");
      }
      else if(i != 0) {
         _LOG(LogFlags," ");
      }
      _LOG(LogFlags,"%02x",cp[i]);
   }
   if(((i - 1) & 0xf) != 0) {
      _LOG(LogFlags,"\n");
   }
}
#endif

void DisplayString(const char *Msg,int Row,int Col)
{
   volatile uint32_t *p = (volatile uint32_t *) (BG0_TILES_ADR + (Row * BG0_WIDTH * sizeof(uint32_t)));
   const char *cp = Msg;
   while(*cp) {
      *p++ = *cp++;
   }
}

