/*
 *  pano_io.h
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
#ifndef _PANO_IO_H_
#define _PANO_IO_H_

#define BG0_WIDTH 64
//#define SCREEN_X    80
//#define SCREEN_Y    30

#define DLY_TAP_ADR              0x03000000
#define LEDS_ADR                 0x03000004
#define UART0_D_ADR              0x03000100
#define UART0_C_ADR              0x03000104
#define UART1_D_ADR              0x03000108
#define UART1_C_ADR              0x0300010C
#define IDT_ADR                  0x03000200
#define VID_MODE_ADR             0x08000000
#define VID_TIMING_FRONT_ADR     0x08000004
#define VID_TIMING_SYNC_ADR      0x08000008
#define VID_TIMING_BACK_ADR      0x0800000C
#define VID_TIMING_ACTIVE_ADR    0x08000010
#define VID_SCANLINE_ADR         0x08000014
#define BG0_SCR_X_ADR            0x08000020
#define BG0_SCR_Y_ADR            0x08000024
#define BG0_WIN_X_ADR            0x08000028
#define BG0_WIN_Y_ADR            0x0800002C
#define BG0_BORDER_ADR           0x08000030
#define BG1_SCR_X_ADR            0x08000040
#define BG1_SCR_Y_ADR            0x08000044
#define BG1_WIN_X_ADR            0x08000048
#define BG1_WIN_Y_ADR            0x0800004C
#define BG1_BORDER_ADR           0x08000050
#define BMP_SCR_X_ADR            0x08000060
#define BMP_SCR_Y_ADR            0x08000064
#define BMP_WIN_X_ADR            0x08000068
#define BMP_WIN_Y_ADR            0x0800006C
#define BMP_BORDER_ADR           0x08000070

#define PALETTE_ADR              0x08004000
#define SPR_OAM_ADR              0x08008000
#define SPR_MAP_ADR              0x08100000
#define BG0_TILES_ADR            0x08200000
#define BG0_MAP_ADR              0x08300000
#define BG1_TILES_ADR            0x08400000
#define BG1_MAP_ADR              0x08500000
#define BMP_RAM_ADR              0x08600000

#define VID_MODE                *((volatile uint32_t *)VID_MODE_ADR)

#define VID_MODE_SPR_EN          (1<<0)
#define VID_MODE_SPR_PRIORITY(n) ((n & 0x3)<<6)

#define VID_MODE_BG0_EN          (1<<8)
#define VID_MODE_BG0_SIZE(n)     ((n & 0x3)<<9)
#define VID_MODE_BG0_SCALE(n)    ((n & 0x7)<<11)
#define VID_MODE_BG0_PRIORITY(n) ((n & 0x3)<<14)

#define VID_MODE_BG1_EN          (1<<16)
#define VID_MODE_BG1_SIZE(n)     ((n & 0x3)<<17)
#define VID_MODE_BG1_SCALE(n)    ((n & 0x7)<<19)
#define VID_MODE_BG1_PRIORITY(n) ((n & 0x3)<<22)

#define VID_MODE_BMP_EN          (1<<24)
#define VID_MODE_BMP_SIZE(n)     ((n & 0x3)<<25)
#define VID_MODE_BMP_SCALE(n)    ((n & 0x7)<<27)
#define VID_MODE_BMP_PRIORITY(n) ((n & 0x3)<<30)

#define VID_PRIORITY_HIGHEST     0
#define VID_PRIORITY_HIGHER      1
#define VID_PRIORITY_LOWER       2
#define VID_PRIORITY_LOWEST      3

#define VID_TIMING_FRONT        *((volatile uint32_t *)(VID_TIMING_FRONT_ADR))
#define VID_TIMING_SYNC         *((volatile uint32_t *)(VID_TIMING_SYNC_ADR))
#define VID_TIMING_BACK         *((volatile uint32_t *)(VID_TIMING_BACK_ADR))
#define VID_TIMING_ACTIVE       *((volatile uint32_t *)(VID_TIMING_ACTIVE_ADR))
#define VID_TIMING_V(n)         ((n & 0xFFFF)<<16)
#define VID_TIMING_H(n)         (n & 0xFFFF)

#define VID_SCANLINE            *((volatile uint32_t *)(VID_SCANLINE_ADR))

// SPR_OAM_ATTR0
#define SPR_X(n)                 ((n & 0x7ff)<<0)
#define SPR_Y(n)                 ((n & 0x7ff)<<11)
#define SPR_ROTSCALE             (1<<22)
#define SPR_DOUBLE               (1<<23)
#define SPR_DISABLE              (1<<23)
#define SPR_MODE(n)              ((n & 0x3)<<24)
#define SPR_16_COLOR             (0<<26)
#define SPR_256_COLOR            (1<<26)
#define SPR_SHAPE_SQUARE         (0<<27)
#define SPR_SHAPE_TALL           (1<<27)
#define SPR_SHAPE_WIDE           (2<<27)

// SPR_OAM_ATTR1
#define SPR_FLIPX                (1<<0)
#define SPR_FLIPY                (1<<1)
#define SPR_SIZE(n)              ((n & 0x3)<<2)
#define SPR_NAME(n)              ((n & 0x3f)<<4)
#define SPR_PALETTE(n)           ((n & 0xf)<<16)

#define SPR_OAM_ATTR0(n)  *((volatile uint32_t *)(SPR_OAM_ADR + (n * 8)))
#define SPR_OAM_ATTR1(n)  *((volatile uint32_t *)(SPR_OAM_ADR + (n * 8) + 4))

#define BG0_SCR_X         *((volatile uint32_t *)BG0_SCR_X_ADR)
#define BG0_SCR_Y         *((volatile uint32_t *)BG0_SCR_Y_ADR)
#define BG0_WIN_X         *((volatile uint32_t *)BG0_WIN_X_ADR)
#define BG0_WIN_Y         *((volatile uint32_t *)BG0_WIN_Y_ADR)
#define BG0_BORDER         *((volatile uint32_t *)BG0_BORDER_ADR)

#define BG1_SCR_X         *((volatile uint32_t *)BG1_SCR_X_ADR)
#define BG1_SCR_Y         *((volatile uint32_t *)BG1_SCR_Y_ADR)
#define BG1_WIN_X         *((volatile uint32_t *)BG1_WIN_X_ADR)
#define BG1_WIN_Y         *((volatile uint32_t *)BG1_WIN_Y_ADR)
#define BG1_BORDER         *((volatile uint32_t *)BG1_BORDER_ADR)

#define BG1_TILE_NAME(n)   (n & 256)
#define BG1_TILE_FLIPX     (1 << 10)
#define BG1_TILE_FLIPY     (1 << 11)
#define BG1_TILE_PAL(n)    ((n & 16)<<12)

#define BMP_SCR_X         *((volatile uint32_t *)BMP_SCR_X_ADR)
#define BMP_SCR_Y         *((volatile uint32_t *)BMP_SCR_Y_ADR)
#define BMP_WIN_X         *((volatile uint32_t *)BMP_WIN_X_ADR)
#define BMP_WIN_Y         *((volatile uint32_t *)BMP_WIN_Y_ADR)
#define BMP_BORDER         *((volatile uint32_t *)BMP_BORDER_ADR)

#define PALETTE_RAM       ((volatile uint32_t *)PALETTE_ADR)
#define SPRITE_OAM        ((volatile uint32_t *)SPR_OAM_ADR)
#define SPRITE_IMG        ((volatile uint32_t *)SPR_MAP_ADR)
#define BACKGROUND_RAM    ((volatile uint32_t *)BG1_TILES_ADR)
#define BACKGROUND_IMG    ((volatile uint32_t *)BG1_MAP_ADR)
#define CHARACTER_RAM     ((volatile uint32_t *)BG0_TILES_ADR)
#define FONT_RAM          ((volatile uint32_t *)BG0_MAP_ADR)
#define BITMAP_RAM        ((volatile uint32_t *)BMP_RAM_ADR)

#define dly_tap           *((volatile uint32_t *)DLY_TAP_ADR)
#define leds              *((volatile uint32_t *)LEDS_ADR)
#define LED_RED            0x1
#define LED_GREEN          0x2
#define LED_BLUE           0x4


#define UART0_DAT         *((volatile uint32_t *)UART0_D_ADR)
#define UART0_CTL         *((volatile uint32_t *)UART0_C_ADR)
#define UART1_DAT         *((volatile uint32_t *)UART1_D_ADR)
#define UART1_CTL         *((volatile uint32_t *)UART1_C_ADR)

#define IDT_CONFIG        *((volatile uint32_t *)IDT_ADR)

#define IO_STAT_IDLE    0
#define IO_STAT_WRITE   1
#define IO_STAT_READ    2
#define IO_STAT_READY   3
#define IO_STATE_MASK   0x7
#define IO_STAT_HALTED  0x800000

#define BLACK           0
#define WHITE           0xffffff
#define GREEN           0x00ff00

#define ANSI_CLS        "\033[2J"

extern unsigned char gFunctionRequest;

void WaitVSync(int v);
void InitVideo(uint16_t x, uint16_t y, uint8_t hz);
void InitSprites();
void InitFont();
void InitLogo();
void InitPalette();
void UartPutc(char c);
void PrintfPutc(char c);
void FlushWriteCache(void);
void DisplayString(const char *Msg,int Row,int Col);

#endif // _PANO_IO_H_

