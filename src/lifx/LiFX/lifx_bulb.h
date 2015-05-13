/*
 * lifx_bulb.h
 *
 *  Created on: Apr 19, 2015
 *      Author: Craig
 */

#ifndef LIFX_BULB_H_
#define LIFX_BULB_H_


/** Mapping of LED control to the Pins of Port B */
/*@{*/
#define LED_PIN_WHITE GPIO_PIN_2
#define LED_PIN_BLUE  GPIO_PIN_3
#define LED_PIN_GREEN GPIO_PIN_4
#define LED_PIN_RED   GPIO_PIN_5

#define LED_WHITE GPIO_PIN_2
#define LED_BLUE  GPIO_PIN_3
#define LED_GREEN GPIO_PIN_4
#define LED_RED   GPIO_PIN_5
#define LED_ALL (LED_WHITE|LED_BLUE|LED_GREEN|LED_RED)
/*@}*/

void lifx_bulb_setup();

uint8_t sw_read();

uint8_t led_status(uint8_t led_selection);
void    led_on(uint8_t led_selection);
void    led_off(uint8_t led_selection);
void    led_toggle(uint8_t led_selection);

void led_panic();
void led_test();

#endif /* LIFX_BULB_H_ */
