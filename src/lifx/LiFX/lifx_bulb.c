/*
 * lifx_bulb.c
 *
 *  Created on: Apr 19, 2015
 *      Author: Craig
 */

#include <stdint.h>
#include <gpio.h>
#include <hw_gpio.h>
#include <hw_memmap.h>

#include "lifx_bulb.h"

void delay_ms(uint32_t milliseconds);


void lifx_bulb_setup()
{
	/* Set the LED controlling pins' direction to output */
	GPIODirModeSet(GPIO_B_BASE, LED_PIN_WHITE|LED_PIN_BLUE|LED_PIN_GREEN|LED_PIN_RED, GPIO_DIR_MODE_OUT);
	/* Set the switch input pin direction to input */
	GPIODirModeSet(GPIO_D_BASE, GPIO_PIN_4, GPIO_DIR_MODE_IN);

//	GPIODirModeSet(GPIO_B_BASE, GPIO_PIN_1, GPIO_DIR_MODE_IN);
}

uint8_t sw_read()
{
	return (uint8_t)GPIOPinRead(GPIO_D_BASE, GPIO_PIN_4);
}

uint8_t led_status(uint8_t led_selection)
{
	return (uint8_t) GPIOPinRead(GPIO_B_BASE, led_selection);
}

void led_on(uint8_t led_selection)
{
	GPIOPinWrite(GPIO_B_BASE, led_selection, led_selection);
}
void led_off(uint8_t led_selection)
{
	GPIOPinWrite(GPIO_B_BASE, led_selection, ~led_selection);
}

/**
 * \brief Toggle the selcted LEDs ON or OFF
 */
void led_toggle(uint8_t led_selection)
{
	GPIOPinWrite(GPIO_B_BASE, led_selection, ~led_status(led_selection));
}

void led_panic()
{
	// set only white on
	led_on(LED_PIN_WHITE);
	led_off(LED_PIN_BLUE|LED_PIN_GREEN|LED_PIN_RED);

	while (1) {
		// toggle white and color on and off
		led_toggle(LED_PIN_WHITE|LED_PIN_BLUE|LED_PIN_GREEN|LED_PIN_RED);
		delay_ms(500);
	}
}

void led_test()
{

	led_on(LED_RED);
	delay_ms(500);
	led_on(LED_GREEN);
	delay_ms(500);
	led_on(LED_BLUE);
	delay_ms(500);
	led_on(LED_WHITE);
	delay_ms(500);

	delay_ms(3000);

	led_off(LED_RED);
	delay_ms(500);
	led_off(LED_GREEN);
	delay_ms(500);
	led_off(LED_BLUE);
	delay_ms(500);
	led_off(LED_WHITE);
	delay_ms(500);


	//FUN SEQUENCY


	delay_ms(3000);

	led_toggle(LED_RED);
	delay_ms(200);
	led_toggle(LED_ALL);
	delay_ms(200);
	led_toggle(LED_GREEN);
	delay_ms(200);
	led_toggle(LED_ALL);
	delay_ms(200);
	led_toggle(LED_BLUE);
	delay_ms(200);
	led_toggle(LED_ALL);
	delay_ms(200);
	led_toggle(LED_WHITE);
	delay_ms(200);

//    	delay_ms(3000);

	led_toggle(LED_RED);
	delay_ms(200);
	led_toggle(LED_ALL);
	delay_ms(200);
	led_toggle(LED_GREEN);
	delay_ms(200);
	led_toggle(LED_ALL);
	delay_ms(200);
	led_toggle(LED_BLUE);
	delay_ms(200);
	led_toggle(LED_ALL);
	delay_ms(200);
	led_toggle(LED_WHITE);
	delay_ms(200);
	led_toggle(LED_ALL);
	delay_ms(200);
	led_off(LED_ALL);

	delay_ms(3000);
}
