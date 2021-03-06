/**
   Modified for use with Panologic devices by Skip Hansen 8/2019

   This file is derived from the avr-vt100 project:
   https://github.com/mkschreder/avr-vt100
   Copyright: Martin K. Schröder (info@fortmax.se) 2014

   This file is part of FORTMAX.

   FORTMAX is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   FORTMAX is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with FORTMAX.  If not, see <http://www.gnu.org/licenses/>.

   Copyright: Martin K. Schröder (info@fortmax.se) 2014

*/
#include <stdint.h>
#include <string.h>
#include "vt100.h"
#include "printf.h"
#include "pano_io.h"

// #define ECHO_CONSOLE_2_SERIAL

// #define DEBUG_LOGGING
// #define VERBOSE_DEBUG_LOGGING
// #define LOG_TO_SERIAL
#include "log.h"

#define abs(x) (x < 0 ? -x : x)
#define isdigit(x) (x >= '0' && x <= '9')

#define CURSOR_CHAR     0x16

#define KEY_ESC 0x1b
#define KEY_DEL 0x7f
#define KEY_BELL 0x07

#define STATE(NAME, TERM, EV, ARG) void NAME(struct vt100 *TERM, uint8_t EV, uint16_t ARG)

// states
enum {
   STATE_IDLE,
   STATE_ESCAPE,
   STATE_COMMAND
};

// events that are passed into states
enum {
   EV_CHAR = 1,
};

#define MAX_COMMAND_ARGS 4
static struct vt100 {
   union flags {
      uint8_t val;
      struct {
         // 0 = cursor remains on last column when it gets there
         // 1 = lines wrap after last column to next line
         uint8_t cursor_wrap : 1;
         uint8_t scroll_mode : 1;
         uint8_t origin_mode : 1;
      };
   } flags;

   //uint16_t screen_width, screen_height;
   // cursor position on the screen (0, 0) = top left corner.
   int16_t cursor_x, cursor_y;
   int16_t saved_cursor_x, saved_cursor_y; // used for cursor save restore
   int16_t scroll_start_row, scroll_end_row;
   // character width and height
   int8_t char_width, char_height;
   // colors used for rendering current characters
   uint8_t back_color, front_color;
   uint8_t inverse;
   // command arguments that get parsed as they appear in the terminal
   uint8_t narg; uint16_t args[MAX_COMMAND_ARGS];
   // current arg pointer (we use it for parsing)
   uint8_t carg;

   void (*state)(struct vt100 *term, uint8_t ev, uint16_t arg);
   void (*ret_state)(struct vt100 *term, uint8_t ev, uint16_t arg);

   uint32_t *VRam;
   uint32_t ScreenBuf[VT100_WIDTH * VT100_HEIGHT];
} term;


STATE(_st_idle, term, ev, arg);
STATE(_st_esc_sq_bracket, term, ev, arg);
STATE(_st_esc_question, term, ev, arg);
STATE(_st_esc_hash, term, ev, arg);

void _vt100_reset(void)
{
   int Len = VT100_HEIGHT * VT100_WIDTH;
   int Start = 0;
   uint32_t *p;
   uint32_t *cp;

   term.back_color = 0x0;
   term.front_color = 0x7;
   term.inverse = 0;
   term.cursor_x = term.cursor_y = term.saved_cursor_x = term.saved_cursor_y = 0;
   term.narg = 0;
   term.state = _st_idle;
   term.ret_state = 0;
   term.scroll_start_row = 0;
   term.scroll_end_row = VT100_HEIGHT; // outside of screen = whole screen scrollable
   term.flags.cursor_wrap = 0;
   term.flags.origin_mode = 0;
   term.VRam = (uint32_t *) BG0_TILES_ADR;

   p = term.VRam;
   cp = term.ScreenBuf;

   while(Len-- > 0) {
      *p++ = *cp++ = ' ';
   }

}

void _vt100_resetScroll(void)
{
   term.scroll_start_row = 0;
   term.scroll_end_row = VT100_HEIGHT;
}

// clear screen from start_line to end_line (including end_line)
void _vt100_clearLines(struct vt100 *t, uint16_t start_line, uint16_t end_line)
{
   int Len = (end_line - start_line + 1) * VT100_WIDTH;
   int Start = start_line * VT100_WIDTH;
   uint32_t *p;
   uint32_t *cp;

   LOG("start_line: %d, end_line: %d\n",start_line,end_line);
   LOG("Start: %d, Len: %d\n",Start,Len);
   if(Len - Start > (VT100_WIDTH * VT100_HEIGHT) ) {
      // limit to the end of the screen
      Len = (VT100_WIDTH * VT100_HEIGHT) - Start;
   }

   p = &t->VRam[Start];
   cp = &t->ScreenBuf[Start];
   LOG("Start: %d, Len: %d\n",Start,Len);

   while(Len-- > 0) {
      uint32_t cattr = (t->inverse) ? ((t->back_color << 16) | (t->front_color << 8)) : ((t->front_color << 16) | (t->back_color << 8));
      *p++ = *cp++ = cattr | ' ';
   }
   if(cp > &t->ScreenBuf[VT100_WIDTH * VT100_HEIGHT]) {
      ELOG("Past end of ScreenBuf, cp 0x%x\n",(unsigned int) cp);
      for( ; ; );
   }
}

// clear line from cursor right/left
void _vt100_clearLine(struct vt100 *t)
{
   int Len;
   int Start;
   uint32_t *p;
   uint32_t *cp;

   if(t->narg == 0 || (t->narg == 1 && t->args[0] == 0)) {
   // clear to end of line (to \n or to edge?)
   // including cursor
      Start = (t->cursor_y * VT100_WIDTH) + t->cursor_x;
      Len = VT100_WIDTH - t->cursor_x;
   }
   else if(t->narg == 1 && t->args[0] == 1) {
   // clear from left to current cursor position
      Start = (t->cursor_y * VT100_WIDTH);
      Len = t->cursor_x;
   }
   else if(t->narg == 1 && t->args[0] == 2) {
   // clear whole current line
      Start = (t->cursor_y * VT100_WIDTH);
      Len = VT100_WIDTH;
   }
   p = &t->VRam[Start];
   cp = &t->ScreenBuf[Start];

   while(Len-- > 0) {
      uint32_t cattr = (t->inverse) ? ((t->back_color << 16) | (t->front_color << 8)) : ((t->front_color << 16) | (t->back_color << 8));
      *p++ = *cp++ = cattr | ' ';
   }
   if(cp > &t->ScreenBuf[VT100_WIDTH * VT100_HEIGHT]) {
      ELOG("Past end of ScreenBuf\n");
      for( ; ; );
   }
   t->state = _st_idle;
}

// scrolls the scroll region up (lines > 0) or down (lines < 0)
void _vt100_scroll(struct vt100 *t, int16_t lines)
{
   int Start;
   int Len;
   int i;
   uint32_t *cp;
   uint32_t *cp1;
   uint32_t *pFrom;
   uint32_t *pTo;
   int OffsetTop;

   if(!lines) return;

   OffsetTop = VT100_WIDTH * t->scroll_start_row;
   LOG("lines: %d OffsetTop: %d\n",lines,OffsetTop);

   // clearing of lines that we have scrolled up or down
   if(lines > 0) {
      Len = VT100_WIDTH * (VT100_HEIGHT - lines);
      LOG("Len: %d\n",Len);

      pTo = &t->VRam[OffsetTop];
      cp1 = &t->ScreenBuf[OffsetTop];
      cp = cp1 + (VT100_WIDTH * lines);

      i = Len;
      while(i-- > 0) {
         *pTo++ = *cp1++ = *cp++;
      }
      if(cp1 > &t->ScreenBuf[VT100_WIDTH * VT100_HEIGHT]) {
         ELOG("Past end of ScreenBuf\n");
         for( ; ; );
      }

      _vt100_clearLines(t,t->scroll_end_row-(lines-1)-1,t->scroll_end_row-1);
   }
   else if(lines < 0) {
      // update the scroll value (wraps around scroll_height)
      Len = VT100_WIDTH * (VT100_HEIGHT + lines);
      pTo = &t->VRam[OffsetTop];
      cp1 = &t->ScreenBuf[OffsetTop];
      cp = cp1 + (VT100_WIDTH * lines);

      i = Len;
      while(i-- > 0) {
         *pTo++ = *cp++ = *cp1++;
      }

      _vt100_clearLines(t,t->scroll_start_row, t->scroll_start_row - lines);
   }
}

// moves the cursor relative to current cursor position and scrolls the screen
void _vt100_move(struct vt100 *term, int16_t DeltaX, int16_t DeltaY)
{
   // calculate how many lines we need to move down or up if x movement goes outside screen
   int16_t new_x = DeltaX + term->cursor_x;

   if(new_x >= VT100_WIDTH) {
      if(term->flags.cursor_wrap) {
      // 1 = lines wrap after last column to next line
         DeltaY += new_x / VT100_WIDTH;
         term->cursor_x = new_x % VT100_WIDTH;
      }
      else {
      // 0 = cursor remains on last column when it gets there
         term->cursor_x = VT100_WIDTH - 1;
      }
   }
   else if(new_x < 0) {
      DeltaY += new_x / VT100_WIDTH - 1;
      term->cursor_x = VT100_WIDTH - (abs(new_x) % VT100_WIDTH) + 1;
   }
   else {
      term->cursor_x = new_x;
   }

   if(DeltaY) {
      int16_t new_y = term->cursor_y + DeltaY;
      int16_t to_scroll = 0;
      // bottom margin 39 marks last line as static on 40 line display
      // therefore, we would scroll when new cursor has moved to line 39
      // (or we could use new_y > VT100_HEIGHT here
      // NOTE: new_y >= term->scroll_end_row ## to_scroll = (new_y - term->scroll_end_row) +1
      if(new_y >= term->scroll_end_row) {
         //scroll = new_y / VT100_HEIGHT;
         //term->cursor_y = VT100_HEIGHT;
         to_scroll = (new_y - term->scroll_end_row) + 1;
         // place cursor back within the scroll region
         term->cursor_y = term->scroll_end_row - 1; //new_y - to_scroll;
         //scroll = new_y - term->bottom_margin;
         //term->cursor_y = term->bottom_margin;
      }
      else if(new_y < term->scroll_start_row) {
         to_scroll = (new_y - term->scroll_start_row);
         term->cursor_y = term->scroll_start_row; //new_y - to_scroll;
         //scroll = new_y / (term->bottom_margin - term->top_margin) - 1;
         //term->cursor_y = term->top_margin;
      }
      else {
         // otherwise we move as normal inside the screen
         term->cursor_y = new_y;
      }
      _vt100_scroll(term, to_scroll);
   }
}

void _vt100_removeCursor(struct vt100 *t)
{
}

void _vt100_drawCursor(struct vt100 *t)
{
}

// sends the character to the display and updates cursor position
void _vt100_putc(struct vt100 *t, uint8_t ch)
{
   // calculate current cursor position in the display ram
   int Offset = (t->cursor_y * VT100_WIDTH) + t->cursor_x;
   uint32_t cattr = (t->inverse) ? ((t->back_color << 16) | (t->front_color << 8)) : ((t->front_color << 16) | (t->back_color << 8));
   t->VRam[Offset] = cattr | (uint32_t) ch;
   t->ScreenBuf[Offset] = cattr | (uint32_t) ch;

   if(&t->ScreenBuf[Offset] >= &t->ScreenBuf[VT100_WIDTH * VT100_HEIGHT]) {
      ELOG("Past end of ScreenBuf\n");
      for( ; ; );
   }

   // move cursor right
   _vt100_move(t, 1, 0);
}

void vt100_puts(const char *str)
{
   while(*str) {
      vt100_putc(*str++);
   }
}

STATE(_st_command_arg, term, ev, arg){
   switch(ev) {
      case EV_CHAR:
         if(isdigit(arg)) { // a digit argument
            term->args[term->narg] = term->args[term->narg] * 10 + (arg - '0');
         }
         else if(arg == ';') { // separator
            term->narg++;
         }
         else { // no more arguments
            // go back to command state
            term->narg++;
            if(term->ret_state) {
               term->state = term->ret_state;
            }
            else {
               term->state = _st_idle;
            }
            // execute next state as well because we have already consumed a char!
            term->state(term, ev, arg);
         }
         break;
   }
}

STATE(_st_esc_sq_bracket, term, ev, arg)
{
   switch(ev) {
      case EV_CHAR:
         if(isdigit(arg)) { // start of an argument
            term->ret_state = _st_esc_sq_bracket;
            _st_command_arg(term, ev, arg);
            term->state = _st_command_arg;
         }
         else if(arg == ';') { // arg separator.
            // skip. And also stay in the command state
         }
         else {
         // otherwise we execute the command and go back to idle
            _vt100_removeCursor(term);
            switch(arg) {
               case 'A': {
               // move cursor up (cursor stops at top margin)
                  int n = (term->narg > 0)?term->args[0]:1;
                  term->cursor_y -= n;
                  if(term->cursor_y < 0) term->cursor_y = 0;
                  term->state = _st_idle;
                  break;
               }
               case 'B': {
               // cursor down (cursor stops at bottom margin)
                  int n = (term->narg > 0)?term->args[0]:1;
                  term->cursor_y += n;
                  if(term->cursor_y > VT100_HEIGHT) term->cursor_y = VT100_HEIGHT;
                  term->state = _st_idle;
                  break;
               }
               case 'C': {
               // cursor right (cursor stops at right margin)
                  int n = (term->narg > 0)?term->args[0]:1;
                  term->cursor_x += n;
                  if(term->cursor_x > VT100_WIDTH) term->cursor_x = VT100_WIDTH;
                  term->state = _st_idle;
                  break;
               }
               case 'D': {
               // cursor left
                  int n = (term->narg > 0)?term->args[0]:1;
                  term->cursor_x -= n;
                  if(term->cursor_x < 0) term->cursor_x = 0;
                  term->state = _st_idle;
                  break;
               }
               case 'f':
               case 'H':
               // move cursor to position (default 0;0)
            // cursor stops at respective margins
                  term->cursor_x = (term->narg >= 1)?(term->args[1]-1):0;
                  term->cursor_y = (term->narg == 2)?(term->args[0]-1):0;
                  if(term->flags.origin_mode) {
                     term->cursor_y += term->scroll_start_row;
                     if(term->cursor_y >= term->scroll_end_row) {
                        term->cursor_y = term->scroll_end_row - 1;
                     }
                  }
                  if(term->cursor_x > VT100_WIDTH) term->cursor_x = VT100_WIDTH;
                  if(term->cursor_y > VT100_HEIGHT) term->cursor_y = VT100_HEIGHT;
                  term->state = _st_idle;
                  break;

               case 'J':
               // clear screen from cursor up or down
                  if(term->narg == 0 || (term->narg == 1 && term->args[0] == 0)) {
                     // clear down to the bottom of screen (including cursor)
                     _vt100_clearLines(term, term->cursor_y, VT100_HEIGHT-1);
                  }
                  else if(term->narg == 1 && term->args[0] == 1) {
                     // clear top of screen to current line (including cursor)
                     _vt100_clearLines(term, 0, term->cursor_y);
                  }
                  else if(term->narg == 1 && term->args[0] == 2) {
                     // clear whole screen
                     _vt100_clearLines(term, 0, VT100_HEIGHT-1);
                     // reset scroll value
                     _vt100_resetScroll();
                  }
                  term->state = _st_idle;
                  break;

               case 'K':
               // clear line from cursor right/left
                  _vt100_clearLine(term);
                  break;

               case 'L': // insert lines (args[0] = number of lines)
               case 'M': // delete lines (args[0] = number of lines)
                  term->state = _st_idle;
                  break;

               case 'P': {
               // delete characters args[0] or 1 in front of cursor
                  // TODO: this needs to correctly delete n chars
                  int n = ((term->narg > 0)?term->args[0]:1);
                  _vt100_move(term, -n, 0);
                  for(int c = 0; c < n; c++) {
                     _vt100_putc(term, ' ');
                  }
                  term->state = _st_idle;
                  break;
               }
               case 'c':
               // query device code
                  term->state = _st_idle;
                  break;

               case 'x':
                  term->state = _st_idle;
                  break;

               case 's': // save cursor pos
                     term->saved_cursor_x = term->cursor_x;
                     term->saved_cursor_y = term->cursor_y;
                     term->state = _st_idle;
                     break;

               case 'u': // restore cursor pos
                     term->cursor_x = term->saved_cursor_x;
                     term->cursor_y = term->saved_cursor_y;
                     //_vt100_moveCursor(term, term->saved_cursor_x, term->saved_cursor_y);
                     term->state = _st_idle;
                     break;

               case 'h':
               case 'l':
                     term->state = _st_idle;
                     break;

               case 'g':
                     term->state = _st_idle;
                     break;

               case 'm': // sets colors. Accepts up to 3 args
                     // [m means reset the colors to default
                     if(!term->narg) {
                        term->front_color = 0x7;
                        term->back_color = 0x0;
                        term->inverse = 0;
                     }
                     // 256-color
                     if((term->narg == 3) && (term->args[1] == 5)) {
                        if(term->args[0] == 38) {
                           term->front_color = term->args[2];
                           term->state = _st_idle;
                           break;
                        }
                        if(term->args[0] == 48) {
                           term->back_color = term->args[2];
                           term->state = _st_idle;
                           break;
                        }
                     }
                     while(term->narg) {
                        term->narg--;
                        int n = term->args[term->narg];
                        if(n == 0) { // all attributes off
                           term->front_color = 0x7;
                           term->back_color = 0x0;
                           term->inverse = 0;
                        }
                        else if(n == 1) { // bright
                           term->front_color |= 0x8;
                        }
                        else if(n == 7) { // inverse
                           term->inverse = 1;
                        }
                        else if(n >= 30 && n < 38) { // fg colors (8-color)
                           term->front_color = n-30;
                        }
                        else if(n == 39) { // default fg
                           term->front_color = 0x7;
                        }
                        else if(n >= 40 && n < 48) { // bg colors (8-color)
                           term->back_color = n-40;
                        }
                        else if(n == 49) { // default fg
                           term->back_color = 0x0;
                        }
                        else if(n >= 90 && n < 98) { // fg colors (16-color)
                           term->front_color = 8+n-90;
                        }
                        else if(n >= 100 && n < 108) { // bg colors (16-color)
                           term->back_color = 8+n-100;
                        }
                     }
                     term->state = _st_idle;
                     break;

               case '@': // Insert Characters
                  term->state = _st_idle;
                  break;

               case 'r': // Set scroll region (top and bottom margins)
                  // the top value is first row of scroll region
                  // the bottom value is the first row of static region after scroll
                  if(term->narg == 2 && term->args[0] < term->args[1]) {
                     // [1;40r means scroll region between 8 and 312
                     // bottom margin is 320 - (40 - 1) * 8 = 8 pix
                     term->scroll_start_row = term->args[0] - 1;
                     term->scroll_end_row = term->args[1] - 1;
                     LOG("Setting scroll region to %d/%d\n",
                         term->scroll_start_row,term->scroll_end_row);
                  }
                  else {
                     _vt100_resetScroll();
                  }
                  term->state = _st_idle;
                  break;

               case 'i': // Printing
               case 'y': // self test modes..
               case '=': // argument follows...
                  //term->state = _st_screen_mode;
                  term->state = _st_idle;
                  break;

               case '?': // '[?' escape mode
                  term->state = _st_esc_question;

                  break;
               default: // unknown sequence
                     term->state = _st_idle;
                     break;
            }
            _vt100_drawCursor(term);

            //term->state = _st_idle;
         } // else
         break;
      default: { // switch (ev)
            // for all other events restore normal mode
            term->state = _st_idle;
         }
   }
}

STATE(_st_esc_question, term, ev, arg)
{
   // DEC mode commands
   switch(ev) {
      case EV_CHAR:
         if(isdigit(arg)) { // start of an argument
            term->ret_state = _st_esc_question;
            _st_command_arg(term, ev, arg);
            term->state = _st_command_arg;
         }
         else if(arg == ';') { // arg separator.
            // skip. And also stay in the command state
         }
         else {
            switch(arg) {
               case 'l':
                  // dec mode: OFF (arg[0] = function)
               case 'h': {
                     // dec mode: ON (arg[0] = function)
                     switch(term->args[0]) {
                        case 1: { // cursor keys mode
                              // h = esc 0 A for cursor up
                              // l = cursor keys send ansi commands
                              break;
                           }
                        case 2: { // ansi / vt52
                              // h = ansi mode
                              // l = vt52 mode
                              break;
                           }
                        case 3: {
                              // h = 132 chars per line
                              // l = 80 chars per line
                              break;
                           }
                        case 4: {
                              // h = smooth scroll
                              // l = jump scroll
                              break;
                           }
                        case 5: {
                              // h = black on white bg
                              // l = white on black bg
                              break;
                           }
                        case 6: {
                              // h = cursor relative to scroll region
                              // l = cursor independent of scroll region
                              term->flags.origin_mode = (arg == 'h')?1:0;
                              break;
                           }
                        case 7: {
                              // h = new line after last column
                              // l = cursor stays at the end of line
                              term->flags.cursor_wrap = (arg == 'h')?1:0;
                              break;
                           }
                        case 8: {
                              // h = keys will auto repeat
                              // l = keys do not auto repeat when held down
                              break;
                           }
                        case 9: {
                              // h = display interlaced
                              // l = display not interlaced
                              break;
                           }
                           // 10-38 - all quite DEC speciffic commands so omitted here
                     }
                     term->state = _st_idle;
                     break;
                  }
               case 'i': /* Printing */
               case 'n': /* Request printer status */
               default:
                  term->state = _st_idle;
                  break;
            }
            term->state = _st_idle;
         }
   }
}

STATE(_st_esc_left_br, term, ev, arg){
   switch(ev) {
      case EV_CHAR:
         switch(arg) {
            case 'A':
            case 'B':
               // translation map command?
            case '0':
            case 'O':
               // another translation map command?
               term->state = _st_idle;
               break;
            default:
               term->state = _st_idle;
         }
         //term->state = _st_idle;
   }
}

STATE(_st_esc_right_br, term, ev, arg)
{
   switch(ev) {
      case EV_CHAR:
         switch(arg) {
            case 'A':
            case 'B':
               // translation map command?
            case '0':
            case 'O':
               // another translation map command?
               term->state = _st_idle;
               break;
            default:
               term->state = _st_idle;
         }
         break;
   }
}

STATE(_st_esc_hash, term, ev, arg)
{
   switch(ev) {
      case EV_CHAR:
         switch(arg) {
            case '8': {
                  // self test: fill the screen with 'E'

                  term->state = _st_idle;
                  break;
               }
            default:
               term->state = _st_idle;
         }
   }
}

STATE(_st_escape, term, ev, arg)
{
#define CLEAR_ARGS \
            { term->narg = 0;\
            for(int c = 0; c < MAX_COMMAND_ARGS; c++)\
               term->args[c] = 0; }
   switch(ev) {
      case EV_CHAR:
         _vt100_removeCursor(term);
         switch(arg) {
            case '[': // command
               // prepare command state and switch to it
               CLEAR_ARGS;
               term->state = _st_esc_sq_bracket;
               break;

            case '(': /* ESC ( */
               CLEAR_ARGS;
               term->state = _st_esc_left_br;
               break;
            case ')': /* ESC ) */
               CLEAR_ARGS;
               term->state = _st_esc_right_br;
               break;
            case '#': // ESC #
               CLEAR_ARGS;
               term->state = _st_esc_hash;
               break;
            case 'P': //ESC P (DCS, Device Control String)
               term->state = _st_idle;
               break;
            case 'D': // moves cursor down one line and scrolls if necessary
               // move cursor down one line and scroll window if at bottom line
               _vt100_move(term, 0, 1);
               term->state = _st_idle;
               break;
            case 'M': // Cursor up
               // move cursor up one line and scroll window if at top line
               _vt100_move(term, 0, -1);
               term->state = _st_idle;
               break;
            case 'E': // next line
               // same as '\r\n'
               _vt100_move(term, 0, 1);
               term->cursor_x = 0;
               term->state = _st_idle;
               break;
            case '7': // Save attributes and cursor position
            case 's':
               term->saved_cursor_x = term->cursor_x;
               term->saved_cursor_y = term->cursor_y;
               term->state = _st_idle;
               break;
            case '8': // Restore them
            case 'u':
               term->cursor_x = term->saved_cursor_x;
               term->cursor_y = term->saved_cursor_y;
               term->state = _st_idle;
               break;
            case '=': // Keypad into applications mode
               term->state = _st_idle;
               break;
            case '>': // Keypad into numeric mode
               term->state = _st_idle;
               break;
            case 'Z': // Report terminal type
               // vt 100 response
               term->state = _st_idle;
               break;
            case 'c': // Reset terminal to initial state
               _vt100_reset();
               term->state = _st_idle;
               break;
            case 'H': // Set tab in current position
            case 'N': // G2 character set for next character only
            case 'O': // G3 "               "
            case '<': // Exit vt52 mode
               // ignore
               term->state = _st_idle;
               break;
            case KEY_ESC: { // marks start of next escape sequence
                  // stay in escape state
                  break;
               }
            default: { // unknown sequence - return to normal mode
                  term->state = _st_idle;
                  break;
               }
         }
         _vt100_drawCursor(term);
         break;
      default: {
            // for all other events restore normal mode
            term->state = _st_idle;
         }
   }
#undef CLEAR_ARGS
}

STATE(_st_idle, term, ev, arg)
{
   switch(ev) {
      case EV_CHAR: {
            switch(arg) {
               case 5: // AnswerBack for vt100's
                  break;
               case '\n': { // new line
                     _vt100_move(term, 0, 1);
                     term->cursor_x = 0;
                     //_vt100_moveCursor(term, 0, term->cursor_y + 1);
                     // do scrolling here!
                     break;
                  }
               case '\r': { // carrage return (0x0d)
                     term->cursor_x = 0;
                     //_vt100_move(term, 0, 1);
                     //_vt100_moveCursor(term, 0, term->cursor_y);
                     break;
                  }
               case '\b': { // backspace 0x08
                     _vt100_move(term, -1, 0);
                     // backspace does not delete the character! Only moves cursor!
                     //ili9340_drawChar(term->cursor_x * term->char_width,
                     // term->cursor_y * term->char_height, ' ');
                     break;
                  }
               case KEY_DEL: { // del - delete character under cursor
                     // Problem: with current implementation, we can't move the rest of line
                     // to the left as is the proper behavior of the delete character
                     // fill the current position with background color
                     _vt100_putc(term, ' ');
                     _vt100_move(term, -1, 0);
                     //_vt100_clearChar(term, term->cursor_x, term->cursor_y);
                     break;
                  }
               case '\t': { // tab
                     // tab fills characters on the line until we reach a multiple of tab_stop
                     int tab_stop = 4;
                     int to_put = tab_stop - (term->cursor_x % tab_stop);
                     while(to_put--) _vt100_putc(term, ' ');
                     break;
                  }
               case KEY_BELL: { // bell is sent by bash for ex. when doing tab completion
                     // sound the speaker bell?
                     // skip
                     break;
                  }
               case KEY_ESC: // escape
                  term->state = _st_escape;
                  break;

               default: {
                     _vt100_putc(term, arg);
                     break;
                  }
            }
            _vt100_drawCursor(term);
            break;
         }
      default: {}
   }
}

void vt100_init()
{
   _vt100_reset();
}

void UartPutc(char c);

void vt100_putc(uint8_t c)
{
#ifdef ECHO_CONSOLE_2_SERIAL
   if(c >= 0x20 && c < 0x7f) {
      UartPutc(c);
   }
   else if(c == '\r' || c == '\n') {
      UartPutc(c);
   }

#endif

#ifdef VERBOSE_DEBUG_LOGGING
   static int Pos = 0;
   if(c < 0x20 || c > 0x7f) {
      Pos += LOG_R("[%02x]",c);
   }
   else {
      Pos += LOG_R("%c",c);
   }
   if(c == '\n' || Pos >= (80-4)) {
      Pos = 0;
      LOG_R("\n");
   }
#endif
   term.state(&term, EV_CHAR, 0x0000 | c);
}

