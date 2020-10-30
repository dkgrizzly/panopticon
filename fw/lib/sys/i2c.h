/*
 *  VerilogBoy
 *
 *  Copyright (C) 2019  Wenting Zhang <zephray@outlook.com>
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
#ifndef __I2C_H__
#define __I2C_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct I2CContext_s {
  volatile uint32_t *scl;
  volatile uint32_t *sda;
} I2CContext_t;

extern I2CContext_t *AUDIO_I2C;
extern I2CContext_t *VGA_I2C;

bool i2c_read_reg(I2CContext_t *pCtx, uint8_t i2c_addr, uint8_t addr, uint8_t *data);
bool i2c_write_reg(I2CContext_t *pCtx, uint8_t i2c_addr, uint8_t addr, uint8_t data);

#endif
