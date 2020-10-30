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
#include "usb_keys.h"
#include "pano_io.h"
#include "ff.h"
#include "vt100.h"
#include "hsvcolor.h"
#include "sincos.h"
#include "elf.h"

//#define LOG_TO_SERIAL
#define LOG_TO_BOTH
#define DEBUG_LOGGING
#define VERBOSE_DEBUG_LOGGING

#include "log.h"
#include "font.h"
#include "tiles.h"
#include "bootlogo.h"
#include "insertusb.h"
#include "palette.h"

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
uint16_t desktop_w = 320;
uint16_t desktop_h = 200;

bool ScreenDirty = 0;
bool DirectoryDirty = 1;

#define INDEX_NONE   0
#define INDEX_PREV   1  // Previous Screen
#define INDEX_NEXT   2  // Next Screen
#define INDEX_PARENT 3  // Parent Directory
#define INDEX_DIR    4  // Subdirectory
#define INDEX_FILE   5  // File

typedef struct Index_s {
  char Name[16];
  uint8_t Type;
} Index_t;

#define MENU_MAX 20
Index_t Menu[MENU_MAX];

int SelectedIndex = 0;
int TotalItems = 0;

extern void irq_mask(uint32_t mask);

void IdlePoll();

void HandleFunctionKey(int Function);
void HandleMouse();

typedef void (*Launcher_t)(void);
Launcher_t Launcher = (Launcher_t)0x0C000000;

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

// Common Tile Map used for Boot Logo & Insert USB graphics
void InitTilemap() {
  uint16_t i;
  for(i = 0; i < TILES_SIZE; i++) {
   uint32_t b = tilemap[i];
   BACKGROUND_IMG[i] = b;
  }
}

void ShowLogo(const uint16_t *map, uint16_t size, bool invert) {
  WaitVSync(1);
  VID_MODE = 0;

  InitTilemap();
  uint16_t MapWidth = 64 * 8;
  uint16_t MapHeight = (size / 64) * 8;

  uint16_t OffsetX = (VideoWidth-MapWidth)/2;
  uint16_t OffsetY = (VideoHeight-MapHeight)/2;

  BG0_BORDER = 0;
  BG1_BORDER = 0;
  BG1_SCR_X = OffsetX;
  BG1_SCR_Y = OffsetY;
  BG1_WIN_X = ((MapWidth-1 + OffsetX) << 16) | OffsetX;
  BG1_WIN_Y = ((MapHeight-1 + OffsetY) << 16) | OffsetY;

  uint16_t i;
  for(i = 0; i < size; i++) {
   uint32_t b = map[i];
   BACKGROUND_RAM[i] = b;
  }

  WaitVSync(1);
  PALETTE_RAM[0] = invert ? 0xffffff : 0x000000;
  PALETTE_RAM[1] = invert ? 0x000000 : 0xffffff;

  // Using Layer BG1 at 64x64
  VID_MODE = VID_MODE_BG1_EN | VID_MODE_BG1_SIZE(2) | VID_MODE_BG1_SCALE(0) | VID_MODE_BG1_PRIORITY(0);
  WaitVSync(0);
}

void InitPalette()
{
  uint16_t i;
  for(i = 0; i < PALETTE_SIZE; i++) {
   uint32_t c = stock_palette[i];
   PALETTE_RAM[i] = c;
  }
}

void MoveMouse() {
  SPR_OAM_ATTR0(0) = SPR_X(mouse_x) | SPR_Y(mouse_y) | SPR_MODE(0) | SPR_16_COLOR | SPR_SHAPE_SQUARE;
  SPR_OAM_ATTR1(0) = SPR_SIZE(1) | SPR_NAME(12) | SPR_PALETTE(14);
}

void vt100_putchar(char c) {
  if(c == '\n') {
    vt100_putc((uint8_t) '\r');
    vt100_putc((uint8_t) c);
  } else {
    vt100_putc((uint8_t) c);
  }
}

void RebuildIndex(const char *directory, int InitialOffset) {
  DIR Dir;            /* Directory object */
  FILINFO Finfo;
  FRESULT res;
  int Entry = 0;
  int offset = InitialOffset;

  TotalItems = 0;

  if(offset > 0) {
    strcpy(Menu[Entry].Name, "--");
    Menu[Entry].Type = INDEX_PREV;
    offset--;
    Entry++;
  } else if(strlen(directory) > 0) {
    strcpy(Menu[Entry].Name, "..");
    Menu[Entry].Type = INDEX_PARENT;
    TotalItems++;
    Entry++;
  }

  // Rebuild Index
  res = f_opendir(&Dir, directory);
  if(res != FR_OK) {
    ELOG("Unable to open directory: %d\n", (int)res);
  } else {
    for(;;) {
      res = f_readdir(&Dir, &Finfo);
      if((res != FR_OK) || !Finfo.fname[0]) {
        break;
      }
      // Skip Hidden & System files
      if((Finfo.fattrib & AM_HID)) continue;
      if((Finfo.fattrib & AM_SYS)) continue;

      if((Finfo.fattrib & AM_DIR)) {
        if(offset) {
          offset--;
          TotalItems++;
          continue;
        }
        if(Entry < MENU_MAX) {
          strncpy(Menu[Entry].Name, Finfo.fname, 12);
          Menu[Entry].Name[12] = 0;
          Menu[Entry].Type = INDEX_DIR;
          Entry++;
        }
        TotalItems++;
      } else {
        if(!strstr(Finfo.fname, ".ELF")) continue;
        if(offset) {
          offset--;
          TotalItems++;
          continue;
        }
        if(Entry < MENU_MAX) {
          strncpy(Menu[Entry].Name, Finfo.fname, 12);
          Menu[Entry].Name[12] = 0;
          Menu[Entry].Type = INDEX_FILE;
          Entry++;
        }
        TotalItems++;
      }
    }
    f_closedir(&Dir);
    if(TotalItems - InitialOffset > MENU_MAX) {
      strcpy(Menu[MENU_MAX-1].Name, "++");
      Menu[MENU_MAX-1].Type = INDEX_NEXT;
    }
  }
  while(Entry < MENU_MAX) {
    Menu[Entry].Name[0] = 0;
    Menu[Entry].Type = INDEX_NONE;
    Entry++;
  }
}

void main()
{
  FATFS FatFs;        /* File system object for each logical drive */
  FRESULT res;
  const char root[] = "USB:/";
  char directory[256] = "";
  int DirectoryOffset = 0;
  HsvColor hsv;
  RgbColor rgb;

  // LPDDR3 MIG
  dly_tap = 0x03;

  leds = LED_RED;

  // Disable all video layers while initializing palette and font data
  VID_MODE = 0;

//  InitVideo(1024,768,75);
  InitVideo(800,600,60);
//  InitVideo(640,480,60);
//  InitVideo(320,480,60);

  ShowLogo(BootLogo, BootLogoSize, 1);

  InitFont();
  InitSprites();

  vt100_init();
  PutCharCallback = &vt100_putchar;

  usb_init();
  drv_usb_hid_init();
  usb_stor_scan(1);
//  drv_usb_gp_init();
  usb_kbd_setraw();

  // Mask only the USB & Timer0 IRQ
  irq_mask(0xffffffe1);

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
//     | VID_MODE_SPR_EN | VID_MODE_SPR_PRIORITY(0)
     | VID_MODE_BG0_EN | VID_MODE_BG0_SIZE(0) | VID_MODE_BG0_SCALE(0) | VID_MODE_BG0_PRIORITY(1)
  ;
  InitPalette();

  // Main loop
  res = f_mount(&FatFs, "", 1);
  if(res != FR_OK) {
    //ALOG_R("\x1b[H\x1b[2J\x1b[25;29HPlease Insert USB Disk\n");
    ShowLogo(InsertUSB, InsertUSBSize, 0);
    for(;;);
  }

  hsv.h = 0;
  hsv.s = 255;
  hsv.v = 255;

  for(;;) {
    if(DirectoryDirty) {
      RebuildIndex(directory, DirectoryOffset);
      DirectoryDirty = 0;
      ScreenDirty = 1;
      SelectedIndex = 0;
    }

    WaitVSync(1);
    hsv.h++;
    rgb = HsvToRgb(hsv);
    PALETTE_RAM[255] = (rgb.r << 16) | (rgb.g << 8) | (rgb.b);

    if(ScreenDirty) {
      ALOG_R("\x1b[H\x1b[2J\x1b[38;5;15m");
      ALOG_R("Current Directory: %s%s\n", root, directory);
      if(TotalItems > 16) {
        ALOG_R("%i Items - Screen %i of %i\n\n", TotalItems, (DirectoryOffset / (MENU_MAX-1))+1, (TotalItems / (MENU_MAX-1))+1);
      } else {
        ALOG_R("%i Items\n\n", TotalItems);
      }

      for(int i = 0; i < MENU_MAX; i++) {
        switch(Menu[i].Type) {
          case INDEX_PREV:
            ALOG_R("  %2i: < Previous Page >\n", i+1);
            break;
          case INDEX_NEXT:
            ALOG_R("  %2i: < Next Page >\n", i+1);
            break;
          case INDEX_PARENT:
            ALOG_R("  %2i: < Parent Directory >\n", i+1);
            break;
          case INDEX_DIR:
            ALOG_R("  %2i: [\x1b[38;5;14m%s\x1b[38;5;15m]\n", i+1, Menu[i].Name);
            break;
          case INDEX_FILE:
            ALOG_R("  %2i: \x1b[38;5;10m%s\x1b[38;5;15m\n", i+1, Menu[i].Name);
            break;
        }
      }
      ALOG_R("\x1b[%i;1H\x1b[38;5;255m->", SelectedIndex+4, SelectedIndex);
      ScreenDirty = 0;
    }
    //MoveMouse();

    WaitVSync(0);

    IdlePoll();
    if(usb_kbd_testc()) {
      uint16_t c = usb_kbd_getc();
      switch(c) {
        case KEYCODE_ENTER:
          switch(Menu[SelectedIndex].Type) {
            case INDEX_PREV:
              DirectoryOffset -= (MENU_MAX-1);
              DirectoryDirty = 1;
              break;
            case INDEX_NEXT:
              DirectoryOffset += (MENU_MAX-1);
              DirectoryDirty = 1;
              break;
            case INDEX_PARENT: {
              char *slash = strrchr(directory, '/');
              if(slash != NULL) {
                slash[0] = 0;
              } else {
                directory[0] = 0;
              }
              DirectoryOffset = 0;
              DirectoryDirty = 1;
              break;
            }
            case INDEX_DIR:
              if(directory[0] != 0)
                strncat(directory, "/", sizeof(directory));
              strncat(directory, Menu[SelectedIndex].Name, sizeof(directory));
              DirectoryOffset = 0;
              DirectoryDirty = 1;
              break;
            case INDEX_FILE: {
              char pathname[256];
              elf_prog_t prog;
              if(directory[0] != 0) {
                strncpy(pathname, directory, sizeof(pathname));
                strncat(pathname, "/", sizeof(pathname));
                strncat(pathname, Menu[SelectedIndex].Name, sizeof(pathname));
              } else {
                strncpy(pathname, Menu[SelectedIndex].Name, sizeof(pathname));
              }
              ALOG_R("\x1b[%i;1H\x1b[38;5;15m", MENU_MAX+6);
              elf_load(pathname, &prog);
              irq_mask(0xffffffff);
              Launcher();
              break;
            }
          }
          break;
        case KEYCODE_ARROW_U:
          if(SelectedIndex > 0) {
            ALOG_R("\x1b[%i;1H\x1b[38;5;15m  ", SelectedIndex+4, SelectedIndex);
            SelectedIndex--;
            ALOG_R("\x1b[%i;1H\x1b[38;5;255m->", SelectedIndex+4, SelectedIndex);
          }
          break;
        case KEYCODE_ARROW_D:
          if((SelectedIndex < MENU_MAX) && (Menu[SelectedIndex+1].Type != INDEX_NONE)) {
            ALOG_R("\x1b[%i;1H\x1b[38;5;15m  ", SelectedIndex+4, SelectedIndex);
            SelectedIndex++;
            ALOG_R("\x1b[%i;1H\x1b[38;5;255m->", SelectedIndex+4, SelectedIndex);
          }
          break;
        case KEYCODE_F1:
          mouse_x--;
          break;
        case KEYCODE_F2:
          mouse_x++;
          break;
        case KEYCODE_F3:
          mouse_y--;
          break;
        case KEYCODE_F4:
          mouse_y++;
          break;
        case KEYCODE_F5:
          DirectoryDirty = 1;
          break;
        case KEYCODE_F6:
          break;
        case KEYCODE_F7:
          break;
        case KEYCODE_F8:
          break;
      }
    }
  }

  leds = LED_BLUE;  // "blue screen of death"
  for(;;);

  // TODO: Timeout, blank the screen, wait for "reset"
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

void IdlePoll()
{
  usb_event_poll();
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
