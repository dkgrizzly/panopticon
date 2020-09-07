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

#include "ff.h"
#include "pano_io.h"
#include "usb.h"
#include "vt100.h"
#include "misc.h"

// #define DEBUG_LOGGING
// #define LOG_TO_BOTH
// #define VERBOSE_DEBUG_LOGGING
#include "log.h"
#include "font.h"

void InitFont()
{
    for(int y = (236*8); y < (256*8); y++) {
        for(int x = 0; x < 8; x++) {
            uint32_t b = (((font_8x8[y-(236*8)])>>x) & 1) ? 1 : 0;
            FONT_RAM[(y*8)+(7-x)] = b;
        }
    }
}

void InitSprites()
{
    for(int y = 0; y < (20*8); y++) {
        for(int x = 0; x < 8; x++) {
            uint32_t b = (((font_8x8[y])>>x) & 1) ? 1 : 4;
            SPRITE_IMG[(y*8)+(7-x)] = b;
        }
    }
}

void InitPalette()
{
    PALETTE_RAM[0] = 0x000000;
    PALETTE_RAM[1] = 0x800000;
    PALETTE_RAM[2] = 0x008000;
    PALETTE_RAM[3] = 0x808000;
    PALETTE_RAM[4] = 0x000080;
    PALETTE_RAM[5] = 0x800080;
    PALETTE_RAM[6] = 0x008080;
    PALETTE_RAM[7] = 0xc0c0c0;
    PALETTE_RAM[8] = 0x808080;
    PALETTE_RAM[9] = 0xff0000;
    PALETTE_RAM[10] = 0x00ff00;
    PALETTE_RAM[11] = 0xffff00;
    PALETTE_RAM[12] = 0x0000ff;
    PALETTE_RAM[13] = 0xff00ff;
    PALETTE_RAM[14] = 0x00ffff;
    PALETTE_RAM[15] = 0xffffff;
    PALETTE_RAM[16] = 0x000000;
    PALETTE_RAM[17] = 0x00005f;
    PALETTE_RAM[18] = 0x000087;
    PALETTE_RAM[19] = 0x0000af;
    PALETTE_RAM[20] = 0x0000d7;
    PALETTE_RAM[21] = 0x0000ff;
    PALETTE_RAM[22] = 0x005f00;
    PALETTE_RAM[23] = 0x005f5f;
    PALETTE_RAM[24] = 0x005f87;
    PALETTE_RAM[25] = 0x005faf;
    PALETTE_RAM[26] = 0x005fd7;
    PALETTE_RAM[27] = 0x005fff;
    PALETTE_RAM[28] = 0x008700;
    PALETTE_RAM[29] = 0x00875f;
    PALETTE_RAM[30] = 0x008787;
    PALETTE_RAM[31] = 0x0087af;
    PALETTE_RAM[32] = 0x0087d7;
    PALETTE_RAM[33] = 0x0087ff;
    PALETTE_RAM[34] = 0x00af00;
    PALETTE_RAM[35] = 0x00af5f;
    PALETTE_RAM[36] = 0x00af87;
    PALETTE_RAM[37] = 0x00afaf;
    PALETTE_RAM[38] = 0x00afd7;
    PALETTE_RAM[39] = 0x00afff;
    PALETTE_RAM[40] = 0x00d700;
    PALETTE_RAM[41] = 0x00d75f;
    PALETTE_RAM[42] = 0x00d787;
    PALETTE_RAM[43] = 0x00d7af;
    PALETTE_RAM[44] = 0x00d7d7;
    PALETTE_RAM[45] = 0x00d7ff;
    PALETTE_RAM[46] = 0x00ff00;
    PALETTE_RAM[47] = 0x00ff5f;
    PALETTE_RAM[48] = 0x00ff87;
    PALETTE_RAM[49] = 0x00ffaf;
    PALETTE_RAM[50] = 0x00ffd7;
    PALETTE_RAM[51] = 0x00ffff;
    PALETTE_RAM[52] = 0x5f0000;
    PALETTE_RAM[53] = 0x5f005f;
    PALETTE_RAM[54] = 0x5f0087;
    PALETTE_RAM[55] = 0x5f00af;
    PALETTE_RAM[56] = 0x5f00d7;
    PALETTE_RAM[57] = 0x5f00ff;
    PALETTE_RAM[58] = 0x5f5f00;
    PALETTE_RAM[59] = 0x5f5f5f;
    PALETTE_RAM[60] = 0x5f5f87;
    PALETTE_RAM[61] = 0x5f5faf;
    PALETTE_RAM[62] = 0x5f5fd7;
    PALETTE_RAM[63] = 0x5f5fff;
    PALETTE_RAM[64] = 0x5f8700;
    PALETTE_RAM[65] = 0x5f875f;
    PALETTE_RAM[66] = 0x5f8787;
    PALETTE_RAM[67] = 0x5f87af;
    PALETTE_RAM[68] = 0x5f87d7;
    PALETTE_RAM[69] = 0x5f87ff;
    PALETTE_RAM[70] = 0x5faf00;
    PALETTE_RAM[71] = 0x5faf5f;
    PALETTE_RAM[72] = 0x5faf87;
    PALETTE_RAM[73] = 0x5fafaf;
    PALETTE_RAM[74] = 0x5fafd7;
    PALETTE_RAM[75] = 0x5fafff;
    PALETTE_RAM[76] = 0x5fd700;
    PALETTE_RAM[77] = 0x5fd75f;
    PALETTE_RAM[78] = 0x5fd787;
    PALETTE_RAM[79] = 0x5fd7af;
    PALETTE_RAM[80] = 0x5fd7d7;
    PALETTE_RAM[81] = 0x5fd7ff;
    PALETTE_RAM[82] = 0x5fff00;
    PALETTE_RAM[83] = 0x5fff5f;
    PALETTE_RAM[84] = 0x5fff87;
    PALETTE_RAM[85] = 0x5fffaf;
    PALETTE_RAM[86] = 0x5fffd7;
    PALETTE_RAM[87] = 0x5fffff;
    PALETTE_RAM[88] = 0x870000;
    PALETTE_RAM[89] = 0x87005f;
    PALETTE_RAM[90] = 0x870087;
    PALETTE_RAM[91] = 0x8700af;
    PALETTE_RAM[92] = 0x8700d7;
    PALETTE_RAM[93] = 0x8700ff;
    PALETTE_RAM[94] = 0x875f00;
    PALETTE_RAM[95] = 0x875f5f;
    PALETTE_RAM[96] = 0x875f87;
    PALETTE_RAM[97] = 0x875faf;
    PALETTE_RAM[98] = 0x875fd7;
    PALETTE_RAM[99] = 0x875fff;
    PALETTE_RAM[100] = 0x878700;
    PALETTE_RAM[101] = 0x87875f;
    PALETTE_RAM[102] = 0x878787;
    PALETTE_RAM[103] = 0x8787af;
    PALETTE_RAM[104] = 0x8787d7;
    PALETTE_RAM[105] = 0x8787ff;
    PALETTE_RAM[106] = 0x87af00;
    PALETTE_RAM[107] = 0x87af5f;
    PALETTE_RAM[108] = 0x87af87;
    PALETTE_RAM[109] = 0x87afaf;
    PALETTE_RAM[110] = 0x87afd7;
    PALETTE_RAM[111] = 0x87afff;
    PALETTE_RAM[112] = 0x87d700;
    PALETTE_RAM[113] = 0x87d75f;
    PALETTE_RAM[114] = 0x87d787;
    PALETTE_RAM[115] = 0x87d7af;
    PALETTE_RAM[116] = 0x87d7d7;
    PALETTE_RAM[117] = 0x87d7ff;
    PALETTE_RAM[118] = 0x87ff00;
    PALETTE_RAM[119] = 0x87ff5f;
    PALETTE_RAM[120] = 0x87ff87;
    PALETTE_RAM[121] = 0x87ffaf;
    PALETTE_RAM[122] = 0x87ffd7;
    PALETTE_RAM[123] = 0x87ffff;
    PALETTE_RAM[124] = 0xaf0000;
    PALETTE_RAM[125] = 0xaf005f;
    PALETTE_RAM[126] = 0xaf0087;
    PALETTE_RAM[127] = 0xaf00af;
    PALETTE_RAM[128] = 0xaf00d7;
    PALETTE_RAM[129] = 0xaf00ff;
    PALETTE_RAM[130] = 0xaf5f00;
    PALETTE_RAM[131] = 0xaf5f5f;
    PALETTE_RAM[132] = 0xaf5f87;
    PALETTE_RAM[133] = 0xaf5faf;
    PALETTE_RAM[134] = 0xaf5fd7;
    PALETTE_RAM[135] = 0xaf5fff;
    PALETTE_RAM[136] = 0xaf8700;
    PALETTE_RAM[137] = 0xaf875f;
    PALETTE_RAM[138] = 0xaf8787;
    PALETTE_RAM[139] = 0xaf87af;
    PALETTE_RAM[140] = 0xaf87d7;
    PALETTE_RAM[141] = 0xaf87ff;
    PALETTE_RAM[142] = 0xafaf00;
    PALETTE_RAM[143] = 0xafaf5f;
    PALETTE_RAM[144] = 0xafaf87;
    PALETTE_RAM[145] = 0xafafaf;
    PALETTE_RAM[146] = 0xafafd7;
    PALETTE_RAM[147] = 0xafafff;
    PALETTE_RAM[148] = 0xafd700;
    PALETTE_RAM[149] = 0xafd75f;
    PALETTE_RAM[150] = 0xafd787;
    PALETTE_RAM[151] = 0xafd7af;
    PALETTE_RAM[152] = 0xafd7d7;
    PALETTE_RAM[153] = 0xafd7ff;
    PALETTE_RAM[154] = 0xafff00;
    PALETTE_RAM[155] = 0xafff5f;
    PALETTE_RAM[156] = 0xafff87;
    PALETTE_RAM[157] = 0xafffaf;
    PALETTE_RAM[158] = 0xafffd7;
    PALETTE_RAM[159] = 0xafffff;
    PALETTE_RAM[160] = 0xd70000;
    PALETTE_RAM[161] = 0xd7005f;
    PALETTE_RAM[162] = 0xd70087;
    PALETTE_RAM[163] = 0xd700af;
    PALETTE_RAM[164] = 0xd700d7;
    PALETTE_RAM[165] = 0xd700ff;
    PALETTE_RAM[166] = 0xd75f00;
    PALETTE_RAM[167] = 0xd75f5f;
    PALETTE_RAM[168] = 0xd75f87;
    PALETTE_RAM[169] = 0xd75faf;
    PALETTE_RAM[170] = 0xd75fd7;
    PALETTE_RAM[171] = 0xd75fff;
    PALETTE_RAM[172] = 0xd78700;
    PALETTE_RAM[173] = 0xd7875f;
    PALETTE_RAM[174] = 0xd78787;
    PALETTE_RAM[175] = 0xd787af;
    PALETTE_RAM[176] = 0xd787d7;
    PALETTE_RAM[177] = 0xd787ff;
    PALETTE_RAM[178] = 0xd7af00;
    PALETTE_RAM[179] = 0xd7af5f;
    PALETTE_RAM[180] = 0xd7af87;
    PALETTE_RAM[181] = 0xd7afaf;
    PALETTE_RAM[182] = 0xd7afd7;
    PALETTE_RAM[183] = 0xd7afff;
    PALETTE_RAM[184] = 0xd7d700;
    PALETTE_RAM[185] = 0xd7d75f;
    PALETTE_RAM[186] = 0xd7d787;
    PALETTE_RAM[187] = 0xd7d7af;
    PALETTE_RAM[188] = 0xd7d7d7;
    PALETTE_RAM[189] = 0xd7d7ff;
    PALETTE_RAM[190] = 0xd7ff00;
    PALETTE_RAM[191] = 0xd7ff5f;
    PALETTE_RAM[192] = 0xd7ff87;
    PALETTE_RAM[193] = 0xd7ffaf;
    PALETTE_RAM[194] = 0xd7ffd7;
    PALETTE_RAM[195] = 0xd7ffff;
    PALETTE_RAM[196] = 0xff0000;
    PALETTE_RAM[197] = 0xff005f;
    PALETTE_RAM[198] = 0xff0087;
    PALETTE_RAM[199] = 0xff00af;
    PALETTE_RAM[200] = 0xff00d7;
    PALETTE_RAM[201] = 0xff00ff;
    PALETTE_RAM[202] = 0xff5f00;
    PALETTE_RAM[203] = 0xff5f5f;
    PALETTE_RAM[204] = 0xff5f87;
    PALETTE_RAM[205] = 0xff5faf;
    PALETTE_RAM[206] = 0xff5fd7;
    PALETTE_RAM[207] = 0xff5fff;
    PALETTE_RAM[208] = 0xff8700;
    PALETTE_RAM[209] = 0xff875f;
    PALETTE_RAM[210] = 0xff8787;
    PALETTE_RAM[211] = 0xff87af;
    PALETTE_RAM[212] = 0xff87d7;
    PALETTE_RAM[213] = 0xff87ff;
    PALETTE_RAM[214] = 0xffaf00;
    PALETTE_RAM[215] = 0xffaf5f;
    PALETTE_RAM[216] = 0xffaf87;
    PALETTE_RAM[217] = 0xffafaf;
    PALETTE_RAM[218] = 0xffafd7;
    PALETTE_RAM[219] = 0xffafff;
    PALETTE_RAM[220] = 0xffd700;
    PALETTE_RAM[221] = 0xffd75f;
    PALETTE_RAM[222] = 0xffd787;
    PALETTE_RAM[223] = 0xffd7af;
    PALETTE_RAM[224] = 0xffd7d7;
    PALETTE_RAM[225] = 0xffd7ff;
    PALETTE_RAM[226] = 0xffff00;
    PALETTE_RAM[227] = 0xffff5f;
    PALETTE_RAM[228] = 0xffff87;
    PALETTE_RAM[229] = 0xffffaf;
    PALETTE_RAM[230] = 0xffffd7;
    PALETTE_RAM[231] = 0xffffff;
    PALETTE_RAM[232] = 0x080808;
    PALETTE_RAM[233] = 0x121212;
    PALETTE_RAM[234] = 0x1c1c1c;
    PALETTE_RAM[235] = 0x262626;
    PALETTE_RAM[236] = 0x303030;
    PALETTE_RAM[237] = 0x3a3a3a;
    PALETTE_RAM[238] = 0x444444;
    PALETTE_RAM[239] = 0x4e4e4e;
    PALETTE_RAM[240] = 0x585858;
    PALETTE_RAM[241] = 0x626262;
    PALETTE_RAM[242] = 0x6c6c6c;
    PALETTE_RAM[243] = 0x767676;
    PALETTE_RAM[244] = 0x808080;
    PALETTE_RAM[245] = 0x8a8a8a;
    PALETTE_RAM[246] = 0x949494;
    PALETTE_RAM[247] = 0x9e9e9e;
    PALETTE_RAM[248] = 0xa8a8a8;
    PALETTE_RAM[249] = 0xb2b2b2;
    PALETTE_RAM[250] = 0xbcbcbc;
    PALETTE_RAM[251] = 0xc6c6c6;
    PALETTE_RAM[252] = 0xd0d0d0;
    PALETTE_RAM[253] = 0xdadada;
    PALETTE_RAM[254] = 0xe4e4e4;
    PALETTE_RAM[255] = 0xeeeeee;
}

void UartPutc(char c)
{
   uart0 = (uint32_t) c;
   uart1 = (uint32_t) c;
}

void LogPutc(char c,void *arg)
{
   int LogFlags = (int) arg;

   if(!(LogFlags & LOG_DISABLED)) {
      if(LogFlags & LOG_SERIAL) {
         UartPutc(c);
      }

      if(LogFlags & LOG_MONITOR) {
         PrintfPutc(c);
      }
   }
}

void PrintfPutc(char c)
{
   if(c == '\n') {
      vt100_putc((uint8_t) '\r');
      vt100_putc((uint8_t) c);
   }
   else {
      vt100_putc((uint8_t) c);
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

//
void DisplayString(const char *Msg,int Row,int Col)
{
   volatile uint32_t *p = (volatile uint32_t *) (BG0_TILES_ADR + (Row * VT100_WIDTH * sizeof(uint32_t)));
   const char *cp = Msg;
   while(*cp) {
      *p++ = *cp++;
   }
}

