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
#include <stdbool.h>
#include "misc.h"
#include "i2c.h"

I2CContext_t S_AUDIO_I2C = {
  ((volatile uint32_t *)0x03000014),
  ((volatile uint32_t *)0x03000018)
};

I2CContext_t S_VGA_I2C = {
  ((volatile uint32_t *)0x03000010),
  ((volatile uint32_t *)0x03000014)
};

I2CContext_t *AUDIO_I2C = &S_AUDIO_I2C;
I2CContext_t *VGA_I2C = &S_VGA_I2C;

/* Reset to 1 by hardware
void i2c_init(I2CContext_t *pCtx) {
	*(pCtx->sda = 1;
	*(pCtx->scl = 1;
}*/

void i2c_start(I2CContext_t *pCtx) {
    // Start: negedge of DATA when CLK is high

    *(pCtx->sda) = 1;
    delay_loop(1);
    *(pCtx->scl) = 1;
    delay_loop(5);
    *(pCtx->sda) = 0;
    delay_loop(5);
    *(pCtx->scl) = 0;
    delay_loop(2);
}

void i2c_stop(I2CContext_t *pCtx) {
    // Stop: posedge of DATA when CLK is high

    *(pCtx->scl) = 0;
    *(pCtx->sda) = 0;
    delay_loop(4);
    *(pCtx->scl) = 1;
    delay_loop(5);
    *(pCtx->sda) = 1;
    delay_loop(4);
}

bool i2c_wait_ack(I2CContext_t *pCtx) {
    uint16_t err_count = 0;
    *(pCtx->sda) = 1; delay_loop(1);
    *(pCtx->scl) = 1; delay_loop(1);
    while(*(pCtx->sda)) {
        err_count ++;
        if (err_count > 500) {
            i2c_stop(pCtx);
            return false;
        }
    }
    *(pCtx->scl) = 0;
    return true;
}

void i2c_ack(I2CContext_t *pCtx) {
    *(pCtx->scl) = 0;

    *(pCtx->sda) = 0;
    delay_loop(2);
    *(pCtx->scl) = 1;
    delay_loop(2);
    *(pCtx->scl) = 0;
}

void i2c_nack(I2CContext_t *pCtx) {
    *(pCtx->scl) = 0;

    *(pCtx->sda) = 1;
    delay_loop(2);
    *(pCtx->scl) = 1;
    delay_loop(2);
    *(pCtx->scl) = 0;
}

bool i2c_send_byte(I2CContext_t *pCtx, uint8_t b) {

    *(pCtx->scl) = 0;
    for (int i = 0; i < 8; i++) {
        if (b & 0x80)
            *(pCtx->sda) = 1;
        else
            *(pCtx->sda) = 0;
        b <<= 1;
        delay_loop(2);
        *(pCtx->scl) = 1;
        delay_loop(2);
        *(pCtx->scl) = 0;
        delay_loop(2);
    }

    return i2c_wait_ack(pCtx);
}

uint8_t i2c_read_byte(I2CContext_t *pCtx) {
    uint8_t rx = 0;

    *(pCtx->sda) = 1;
    for (int i = 0; i < 8; i++) {
        *(pCtx->scl) = 0;
        delay_loop(2);
        *(pCtx->scl) = 1;
        rx <<= 1;
        rx |= *(pCtx->sda);
        delay_loop(1);
    }

    return rx;
}

void i2c_send_ack(I2CContext_t *pCtx, bool ack) {
    if (ack)
        i2c_ack(pCtx);
    else
        i2c_nack(pCtx);
}

bool i2c_read_reg(I2CContext_t *pCtx, uint8_t i2c_addr, uint8_t addr, uint8_t *data) {
    bool result;

    i2c_start(pCtx);
    if ((result = i2c_send_byte(pCtx, i2c_addr))) {
        i2c_send_byte(pCtx, addr);
        i2c_start(pCtx);
        i2c_send_byte(pCtx, i2c_addr | 0x1);
        *data = i2c_read_byte(pCtx);
        i2c_send_ack(pCtx, true);
    }
    i2c_stop(pCtx);

    return result;
}

bool i2c_write_reg(I2CContext_t *pCtx, uint8_t i2c_addr, uint8_t addr, uint8_t data) {
    bool result;

    i2c_start(pCtx);
    if ((result = i2c_send_byte(pCtx, i2c_addr))) {
        i2c_send_byte(pCtx, addr);
        i2c_send_byte(pCtx, data);
    }
    i2c_stop(pCtx);

    return result;
}
