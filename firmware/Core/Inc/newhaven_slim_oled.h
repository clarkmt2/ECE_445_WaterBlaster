/*
 * Newhaven Display Slim OLED Driver
 * ---------------------------------
 *
 * Base SPI Code by Saurabh Baxi, Applications Engineer at Newhaven Display
 *   International, Inc.
 * Modifications and extensions by Tom Honaker.
 *
 *
 *
 * This is a comprehensive driver for running Newhaven's slim OLED character
 * displays from just about any STM32 MCU with enough free pins to connect to
 * one. The interface being used is SPI, but it's bit-banged on any two pins
 * so there's no requirement to use hardware SPI support.
 *
 * Check the datasheet for your display to wire it to the MCU in SPI mode.
 *
 * In order to save on precious memory, the library uses very little global
 * RAM. Variables are mainly declared inside functions so memory use is far
 * more local than global.
 *
 * The driver was designed and tested against a Newhaven NHD-0420CW-AB3 slim
 * OLED display module connected to a Nucleo-F070RB. The library should work
 * with few changes with any Newhaven slim OLED module and any STM32 MCU,
 * although some unused functions might need to be cut if memory space is at
 * a premium.
 *
 *
 *
 * IMPORTANT NOTE ON STRINGS/CHARS
 *
 * C/C++ strings end in nulls (0x00), but to a LCD or OLED display, character
 * code 0x00 is an actual character. When sending text to the display, remember
 * to NOT send a 0x00 unless displaying character #0 is the desired result!
 *
 *
 *
 * Software License Agreement (BSD License)
 *
 * Copyright (c) 2015-2017 by Newhaven Display International, Inc.
 * Copyright (c) 2017-2019 by Tom Honaker.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */



#ifndef NHD_OLED_DRIVER_H
#define NHD_OLED_DRIVER_H


#include <stdint.h>



// NOTE: Replace this line with the root HAL header for your specific MCU.
#include "stm32f4xx_hal.h"



// Pin Designations
//
// NOTE: EDIT THESE TO SUIT YOUR NEEDS. These pins don't have to be connected
// to an actual SPI module, and in fact it's probably best if they're not.
// The library will bit-bang SPI to the display, and thus doesn't need SPI
// hardware on the MCU.

// Data pin number     __________ - Change this as required.
#define OLED_MOSI_Pin  GPIO_PIN_7

// Data port                _____ - Change this as required.
#define OLED_MOSI_GPIO_Port GPIOA

// Clock pin number    __________ - Change this as required.
#define OLED_CLK_Pin   GPIO_PIN_5

// Clock port               _____ - Change this as required.
#define OLED_CLK_GPIO_Port  GPIOA



void NHD_OLED_begin();
void NHD_OLED_sendCommand(uint8_t command);
void NHD_OLED_sendData(uint8_t data);
void NHD_OLED_setupPins();
void NHD_OLED_setupInit();
void NHD_OLED_displayControl(uint8_t display, uint8_t cursor, uint8_t block);
void NHD_OLED_displayOn();
void NHD_OLED_displayOff();
void NHD_OLED_textClear();
void NHD_OLED_cursorHome();
void NHD_OLED_cursorMoveToRow(uint8_t rowNumber);
void NHD_OLED_textClearRow(uint8_t rowNumber);
void NHD_OLED_shift(uint8_t dc, uint8_t rl);
void NHD_OLED_cursorPos(uint8_t row, uint8_t column);
void NHD_OLED_print_len(char *text, uint8_t len);
void NHD_OLED_print_char(char text);
void NHD_OLED_print_len_pos(char *text, uint8_t len, uint8_t r, uint8_t c);
void NHD_OLED_print_pos(char text, uint8_t r, uint8_t c);
void NHD_OLED_textPrintCentered(char *text, uint8_t length, uint8_t row);
void NHD_OLED_textPrintRightJustified(char *text, uint8_t length, uint8_t row);
void NHD_OLED_textSweep(char *text, uint8_t length, uint8_t row, char leftSweepChar, char rightSweepChar, uint8_t timeDelay);
void output();

// SPI Bit-Bang - This procedure shouldn't be called directly.
void NHD_OLED_SPIBitBang(uint8_t data, uint8_t isCommand);


#endif



/*
 * End of file!
 */
