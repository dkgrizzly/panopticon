#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "misc.h"
#include "pano_io.h"

//#define LOG_TO_SERIAL
//#define LOG_TO_BOTH
//#define DEBUG_LOGGING
//#define VERBOSE_DEBUG_LOGGING
#include "log.h"

uint32_t timer_irq_count = 0;
uint8_t vsync_irq_count = 0;

uint32_t *irq_handler(uint32_t *regs, uint32_t irqs)
{
  leds = LED_BLUE;  // "blue screen of death"

  if((irqs & (1<<4)) != 0) {
    uint16_t sl = VID_SCANLINE;
    VID_HSYNCIRQ = (sl & 0x7f0) + 0x10;
    BG0_BORDER = sl >> 4;
    //ELOG("HSYNC %d\n", VID_SCANLINE);
  }
  if((irqs & (1<<3)) != 0) {
    VID_HSYNCIRQ = 0;
    vsync_irq_count++;
    //ELOG("VSYNC\n");
  }
  if((irqs & (1<<0)) != 0) {
    timer_irq_count++;
    // ELOG("[TIMER-IRQ]");
  }

  if ((irqs & 6) != 0) {
#if 0
    uint32_t pc = (regs[0] & 1) ? regs[0] - 3 : regs[0] - 4;
    uint32_t instr = *(uint16_t*)pc;

    if ((instr & 3) == 3)
      instr = instr | (*(uint16_t*)(pc + 2)) << 16;

    if (((instr & 3) != 3) != (regs[0] & 1)) {
      ELOG("Mismatch between q0 LSB and decoded instruction word! q0=0x%08x, instr=0x%08x\n", regs[0], instr);
      __asm__ volatile ("ebreak");
    }

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
#endif
    while(1);
  }

  leds = LED_RED;  // "blue screen of death"
  return regs;
}
