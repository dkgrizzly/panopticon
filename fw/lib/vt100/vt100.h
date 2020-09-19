/**
   This file is part of FORTMAX kernel.

   FORTMAX kernel is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   FORTMAX kernel is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with FORTMAX kernel.  If not, see <http://www.gnu.org/licenses/>.

   Copyright: Martin K. Schröder (info@fortmax.se) 2014
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define VT100_WIDTH  80
// The physical height is 30, but CP/M apps patched for z80pack such as
// Wordmaster and Wordstar assume a 80 x 24 terminal and don't work correctly
// on our 80 x 30 terminal

#define VT100_HEIGHT 50
#define START_OFFSET 0
//#define VT100_HEIGHT 24
//#define START_OFFSET 0

void vt100_init(void);
void vt100_putc(uint8_t ch);
void vt100_puts(const char *str);

#ifdef __cplusplus
}
#endif
