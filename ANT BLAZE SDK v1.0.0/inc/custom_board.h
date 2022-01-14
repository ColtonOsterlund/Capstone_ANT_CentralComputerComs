/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2017
All rights reserved.
*/

#ifndef CUSTOM_BOARD_H
#define CUSTOM_BOARD_H

#include "nrf_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

// LEDs definitions for D52 on ANT AA Battery Board
#define LEDS_NUMBER    4

// Active low leds
// Board does not define LED_START or LED_STOP since the LEDS are not on sequential pins
#define LED_A        24 //LED A
#define LED_B        31 //LED B
#define LED_C        17 //LED C
#define LED_D        20 //LED D

#define LEDS_ACTIVE_STATE 0

#define LEDS_LIST { LED_A, LED_B, LED_C, LED_D }

#define BSP_LED_0      LED_A
#define BSP_LED_1      LED_B
#define BSP_LED_2      LED_C
#define BSP_LED_3      LED_D

#define LEDS_INV_MASK  LEDS_MASK    // All LEDs are lit when GPIO is low

#define BUTTONS_NUMBER 3

// Board pull-up buttons
#define BUTTON_A     6  //BUTTON A
#define BUTTON_B     7  //BUTTON B
#define BUTTON_C     16 //BUTTON C
#define BUTTON_PULL  NRF_GPIO_PIN_PULLUP

#define BUTTONS_ACTIVE_STATE 0

#define BSP_BUTTON_0   BUTTON_A
#define BSP_BUTTON_1   BUTTON_B
#define BSP_BUTTON_2   BUTTON_C

#define BUTTONS_LIST { BUTTON_A, BUTTON_B, BUTTON_C }

// Battery board pull-up switches
#define SWITCH_1     12 // Switch 1
#define SWITCH_2     8  // Switch 2
#define SWITCH_3     15 // Switch 3
#define SWITCH_4     11 // Switch 4
#define SWITCH_5     14 // Switch 5
#define SWITCH_6     22 // Switch 6
#define SWITCH_7     23 // Switch 7
#define SWITCH_8     19 // Switch 8
#define SWITCH_PULL  NRF_GPIO_PIN_NOPULL

#define SWITCHES_NUMBER 8

#define BSP_SWITCH_0 SWITCH_1
#define BSP_SWITCH_1 SWITCH_2
#define BSP_SWITCH_2 SWITCH_3
#define BSP_SWITCH_3 SWITCH_4
#define BSP_SWITCH_4 SWITCH_5
#define BSP_SWITCH_5 SWITCH_6
#define BSP_SWITCH_6 SWITCH_7
#define BSP_SWITCH_7 SWITCH_8

#define BSP_SWITCH_0_MASK   (1<<BSP_SWITCH_0)
#define BSP_SWITCH_1_MASK   (1<<BSP_SWITCH_1)
#define BSP_SWITCH_2_MASK   (1<<BSP_SWITCH_2)
#define BSP_SWITCH_3_MASK   (1<<BSP_SWITCH_3)
#define BSP_SWITCH_4_MASK   (1<<BSP_SWITCH_4)
#define BSP_SWITCH_5_MASK   (1<<BSP_SWITCH_5)
#define BSP_SWITCH_6_MASK   (1<<BSP_SWITCH_6)
#define BSP_SWITCH_7_MASK   (1<<BSP_SWITCH_7)

#define SWITCHES_MASK (BSP_SWITCH_0_MASK | BSP_SWITCH_1_MASK | BSP_SWITCH_2_MASK | BSP_SWITCH_3_MASK | BSP_SWITCH_4_MASK | BSP_SWITCH_5_MASK | BSP_SWITCH_6_MASK | BSP_SWITCH_7_MASK)

// UART Pins. When using the UART, do not use SWITCH_1, BUTTON_C, LED_C
#define RX_PIN_NUMBER  (16UL)
#define TX_PIN_NUMBER  (17UL)
#define CTS_PIN_NUMBER UART_PIN_DISCONNECTED
#define RTS_PIN_NUMBER (12UL)

// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC      {.source        = NRF_CLOCK_LF_SRC_XTAL,            \
                                 .rc_ctiv       = 0,                                \
                                 .rc_temp_ctiv  = 0,                                \
                                 .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM}

#ifdef __cplusplus
}
#endif

#endif
