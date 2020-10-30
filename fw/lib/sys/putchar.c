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
#include "misc.h"

// #define DEBUG_LOGGING
// #define LOG_TO_BOTH
// #define VERBOSE_DEBUG_LOGGING
#include "log.h"

PutCharCallback_t PutCharCallback = (PutCharCallback_t)NULL;

void UartPutc(char c)
{
  if(!(UART0_CTL & 0x2)) {
    UART0_DAT = (uint32_t) c;
  }
  if(!(UART1_CTL & 0x2)) {
    UART1_DAT = (uint32_t) c;
  }
}

void LogPutc(char c,void *arg)
{
  int LogFlags = (int) arg;

  if(!(LogFlags & LOG_DISABLED)) {
    if(LogFlags & LOG_SERIAL) {
      UartPutc(c);
    }
    if((LogFlags & LOG_MONITOR) && (PutCharCallback != NULL)) {
      PutCharCallback(c);
    }
  }
}

void PrintfPutc(char c)
{
   UartPutc(c);
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

