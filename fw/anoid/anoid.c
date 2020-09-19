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
#include "hsvcolor.h"
#include "sincos.h"

// #define LOG_TO_SERIAL
//#define LOG_TO_BOTH
//#define DEBUG_LOGGING
//#define VERBOSE_DEBUG_LOGGING
#include "log.h"

extern uint16_t VideoWidth;
extern uint16_t VideoHeight;

uint16_t desktop_x = 0;
uint16_t desktop_y = 40;
uint16_t desktop_w = 640;
uint16_t desktop_h = 400;

void IdlePoll();
void LoadInitProg(void);

uint32_t timer_irq_count = 0;

uint32_t *irq_handler(uint32_t *regs, uint32_t irqs)
{
  if((irqs & (1<<0)) != 0) {
    timer_irq_count++;
    // ELOG("[TIMER-IRQ]");
  }

  if ((irqs & 6) != 0) {
    uint32_t pc = (regs[0] & 1) ? regs[0] - 3 : regs[0] - 4;
    uint32_t instr = *(uint16_t*)pc;

    if ((instr & 3) == 3)
      instr = instr | (*(uint16_t*)(pc + 2)) << 16;

    if (((instr & 3) != 3) != (regs[0] & 1)) {
      ELOG("Mismatch between q0 LSB and decoded instruction word! q0=0x%08x, instr=0x%08x\n", regs[0], instr);
      __asm__ volatile ("ebreak");
    }
  }

  if ((irqs & 6) != 0) {
    uint32_t pc = (regs[0] & 1) ? regs[0] - 3 : regs[0] - 4;
    uint32_t instr = *(uint16_t*)pc;

    if ((instr & 3) == 3)
      instr = instr | (*(uint16_t*)(pc + 2)) << 16;

    ELOG("\n");
    ELOG("------------------------------------------------------------\n");

    if ((irqs & 2) != 0) {
      if (instr == 0x00100073 || instr == 0x9002) {
        ELOG("EBREAK instruction at 0x%8x\n", pc);
      } else {
        if((instr & 3) == 3) {
          ELOG("Illegal Instruction at 0x%08x: 0x%08x\n", pc, instr);
        } else {
          ELOG("Illegal Instruction at 0x%08x: 0x%04x\n", pc, instr);
        }
      }
    }

    if ((irqs & 4) != 0) {
      if((instr & 3) == 3) {
        ELOG("Bus error in Instruction at 0x%08x: 0x%08x\n", pc, instr);
      } else {
        ELOG("Bus error in Instruction at 0x%08x: 0x%04x\n", pc, instr);
      }
    }

    for (int i = 0; i < 8; i++) {
      for (int k = 0; k < 4; k++) {
        int r = i + k*8;

        if(r == 0) {
          ELOG("pc  %8x", regs[r]);
        } else {
          ELOG("x%2d %8x", r, regs[r]);
        }
        if(k == 3) {
          ELOG("\n");
        } else {
          ELOG("    ");
        }
      }
    }

    leds = LED_BLUE;  // "blue screen of death"
    while(1);
  }

  if((irqs & (1<<3)) != 0) {
    ELOG("VSYNC\n");
  }
  if((irqs & (1<<4)) != 0) {
    ELOG("HSYNC %d\n", VID_SCANLINE);
  }
  return regs;
}

#define MAX_BALLS 4
#define MAX_PLAYERSIZE 10 // This is really upto 12 sprites because of end caps

typedef struct playfield_s {
  uint16_t x, y, width, height;
} playfield_t;
playfield_t playfield;

typedef struct player_s {
  uint16_t x, y;
  uint8_t width;
  uint8_t powerups;
  uint8_t alive;
} player_t;

player_t player;

typedef struct ball_s {
  uint16_t x, y;
  uint8_t alive;
} ball_t;

ball_t ball[MAX_BALLS] = {
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
};


// Powerups
#define PO_LASER        (1<<0)
#define PO_GROW         (1<<1)
#define PO_SHRINK       (1<<2)
#define PO_STICKY       (1<<3)
#define PO_SLOW         (1<<4)
#define PO_EXIT         (1<<5)
#define PO_SPLIT        (1<<6)
#define PO_1UP          (1<<7)

// 16x16 sprites
#define SO_MOUSE        (0<<2)
#define SO_BALL         (1<<2)
#define SO_PLAYER_LEFT  (2<<2)
#define SO_PLAYER_MID   (3<<2)
#define SO_PLAYER_RIGHT (4<<2)
#define SO_PLAYER_LASER (5<<2)

// 32x16 sprites
#define SO_POWERUP      (6<<2)

// Tiles
#define TO_BRICK_LEFT   (0<<2)
#define TO_BRICK_RIGHT  (1<<2)

#define TO_BACK_A       (4<<2)
#define TO_BACK_B       (5<<2)
#define TO_BACK_C       (6<<2)
#define TO_BACK_D       (7<<2)

// Color Palettes
// Palette Entry 0 is always transparent for sprites, but we can use 0,16,32,..224,240 as Text Colors

// Ball uses Palette Entries 13-15 of palette 1
#define PAL_BALL          1

// Player uses Palette Entries 9-15 of palettes 2-4
#define PAL_PLAYER_NORMAL 2
#define PAL_PLAYER_STICKY 3
#define PAL_PLAYER_LASER  4

// Bricks use Palette Entries 1-5 of palettes 2-15
#define PAL_BRICK_50      5
#define PAL_BRICK_60      6
#define PAL_BRICK_70      7
#define PAL_BRICK_80      8
#define PAL_BRICK_90      9
#define PAL_BRICK_100     10
#define PAL_BRICK_110     11
#define PAL_BRICK_120     12
#define PAL_BRICK_SILVER  13
#define PAL_BRICK_GOLD    14

// Level backgrounds use Palette Entries 8-15 of palettes 7-10
#define PAL_LEVEL(n)      ((n & 3) + 7)

void movesprites() {
    int i, s = 0, n, x, p;

    for(i = 0; i < MAX_BALLS; i++) {
      if(ball[i].alive) {
        SPR_OAM_ATTR0(s) = SPR_X(ball[i].x-8) | SPR_Y(ball[i].y-8) | SPR_MODE(0) | SPR_16_COLOR | SPR_SHAPE_SQUARE;
        SPR_OAM_ATTR1(s) = SPR_SIZE(1) | SPR_NAME(SO_BALL) | SPR_PALETTE(PAL_BALL);
      } else {
        SPR_OAM_ATTR0(s) = SPR_DISABLE;
      }
      s++;
    }

    if(player.powerups & PO_LASER) {
      p = PAL_PLAYER_LASER;
    } else if(player.powerups & PO_STICKY) {
      p = PAL_PLAYER_STICKY;
    } else {
      p = PAL_PLAYER_NORMAL;
    }

    // Player object is multiple 16x16 sprites to allow for powerup scenarios, and waste sprites :D
    x = player.x - (player.width * 8);
    for(i = 0; i < MAX_PLAYERSIZE+2; i++) {
      if(i == 0) {
        n = SO_PLAYER_LEFT;
      } else if(i <= (player.width)) {
        n = SO_PLAYER_MID;
      } else {
        n = SO_PLAYER_RIGHT;
      }

      if(i > (player.width + 2)) {
        SPR_OAM_ATTR0(s) = SPR_DISABLE;
      } else {
        SPR_OAM_ATTR0(s) = SPR_X(player.x) | SPR_Y(player.y) | SPR_MODE(0) | SPR_16_COLOR | SPR_SHAPE_SQUARE;
        SPR_OAM_ATTR1(s) = SPR_SIZE(1) | SPR_NAME(n) | SPR_PALETTE(p);
      }

      s++;
      x+=16;
    }


}

void main()
{
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

//   InitVideo(1024,768,75);
//   InitVideo(800,600,60);
   InitVideo(640,480,60);
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

   // USB Startup
   usb_init();
   drv_usb_hid_init();
//   drv_usb_gp_init();

   // Video Startup
   InitFont();
   InitSprites();

   WaitVSync(1);

   desktop_w = 512;
   desktop_h = 400;
   desktop_x = (VideoWidth - desktop_w)/2;
   desktop_y = (VideoHeight - desktop_h)/2;

   // Place the mouse at the center of the screen
   //mouse_x = desktop_x + (desktop_w / 2);
   //mouse_y = desktop_y + (desktop_h / 2);

   movesprites();

   // Set the "Text" BG0 layer positioning
   BG0_SCR_X = desktop_x;
   BG0_SCR_Y = desktop_y;
   BG0_WIN_X = ((desktop_w + desktop_x) << 16) | desktop_x;
   BG0_WIN_Y = ((desktop_h + desktop_y) << 16) | desktop_y;

   VID_MODE = 0
       | VID_MODE_SPR_EN | VID_MODE_SPR_PRIORITY(0)
       | VID_MODE_BG0_EN | VID_MODE_BG0_SIZE(1) | VID_MODE_BG0_SCALE(0) | VID_MODE_BG0_PRIORITY(1)
   ;
   InitPalette();

   leds = LED_GREEN;

   for(;;) {
      WaitVSync(1);

      movesprites();

      WaitVSync(0);

      IdlePoll();
   }

   leds = LED_BLUE;  // "blue screen of death"
   while(1);
}

void IdlePoll()
{
   usb_event_poll();
   if(gp_num_analogs >= 2) {
      int16_t px = player.x + gp_analog[0];
      if(px < (playfield.x + ((player.width+2) * 16/2)))
        px = (playfield.x + ((player.width+2) * 16/2));
      if(px > (playfield.x + playfield.width - ((player.width+2) * 16/2)))
        px = (playfield.x + playfield.width - ((player.width+2) * 16/2));
      player.x = px;
   }
}
