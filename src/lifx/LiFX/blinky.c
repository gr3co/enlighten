//*****************************************************************************
//! @file       blink_led.c
//! @brief      Blinks red and green LED's on the SmartRF06BB/EB
//!
//! Revised     $Date: 2014-02-13 14:56:58 +0100 (to, 13 feb 2014) $
//! Revision    $Revision: 29354 $

#include <stdint.h>
#include <gpio.h>
#include <hw_gpio.h>
#include <hw_memmap.h>
#include <hw_ioc.h>
#include <ioc.h>
#include <interrupt.h>
#include <sys_ctrl.h>
#include <systick.h>

#include "lifx_bulb.h"

//#include <basic_rf.h>

#define BULB_SELECT 4
//#define TEST_VALUES

/** 100kHz = 100,000Hz */
//#define FREQ_SYSTICK 100000
/** 500kHz = 500,000Hz */
#define FREQ_SYSTICK 500000
/** 800kHz = 800,000Hz */
//#define FREQ_SYSTICK 800000
/** 1MHz = 1,000,000Hz */
//#define FREQ_SYSTICK 1000000

#ifndef TEST_VALUES
	/* Normal Values */

#	if BULB_SELECT == 2
	#define FREQ_PILOT    (3400) // 3.3kHz
	#define FREQ_PREAMBLE (1800) // 1.8kHz
	#define FREQ_DEFAULT  (1500) // 1.5kHz
#	elif BULB_SELECT == 3
	#define FREQ_PILOT    (4200) //
	#define FREQ_PREAMBLE (2600) //
	#define FREQ_DEFAULT  (1500) // 1.5kHz
#	elif BULB_SELECT == 4
	#define FREQ_PILOT    (4200) //
	#define FREQ_PREAMBLE (2600) //
	#define FREQ_DEFAULT  (1500) // 1.5kHz
#	endif

	#define BIT_PERIOD    (FREQ_SYSTICK / 40) // 1/40 sec

#else
	/* Test Values*/

	#define FREQ_PILOT    (3300) // 3.3kHz
	#define FREQ_PREAMBLE (2400) // 2.4kHz
	#define FREQ_DEFAULT  (1500) // 1.5kHz

	//#define BIT_PERIOD    (FREQ_SYSTICK / 20) // 1/20 sec
	#define BIT_PERIOD    (FREQ_SYSTICK / 40) // 1/40 sec
#endif

/**
 * - Max systick is at 32MHz which is 32,000,000 times per second
 * - Current systick is set to 1MHz which is 1,000,000 per second
 * - To calculate the half period for 2kHz, we divide our systick
 *   frequency by 2kHz and divide by 2.
 *   So, 1,000,000/2,000 = 500. The half period is 500/2 = 250.
 *   General Equations:
 *   	period = 1,000,000/(freq_in_Hz)
 *   	period = 1,000,000 * time_duration
 *   	half_period = 1/2 * 1,000,000/(freq_in_Hz)
 *
 */
#define DEFAULT_HALF_PERIOD  (FREQ_SYSTICK / (2*FREQ_DEFAULT))  // 1/2 * systick_freq/default_freq
#define PREAMBLE_HALF_PERIOD (FREQ_SYSTICK / (2*FREQ_PREAMBLE)) // 1/2 * systick_freq/premable_freq
#define PILOT_HALF_PERIOD    (FREQ_SYSTICK / (2*FREQ_PILOT))    // 1/2 * systick_freq/pilot_freq

//*****************************************************************************
//
// Counter to count the number of interrupts that have been called.
//
//*****************************************************************************
volatile uint32_t systick = 0;

//*****************************************************************************
//
// The interrupt handler for the for Systick interrupt.
//
//*****************************************************************************
void
SysTickIntHandler(void)
{
    // Update the Systick interrupt counter.
    systick++;
}

//inline
uint32_t systick_get()
{
	return systick;
}

void systick_setup()
{
    //
    // Set the clocking to run directly from the external crystal/oscillator.
    // (no ext 32k osc, no internal osc)
    //
	/* external 32kHz - false,  internal - false ==> use external 0-32MHz */
    SysCtrlClockSet(false, false, SYS_CTRL_SYSDIV_32MHZ);
    // Set IO clock to the same as system clock
    SysCtrlIOClockSet(SYS_CTRL_SYSDIV_32MHZ);
    // Initialize the interrupt counter.
    systick = 0;

    //
    // Set up the period for the SysTick timer.  The SysTick timer period will
    // be equal to half the system clock, resulting in a period of 0.5 seconds.
    //
    //SysTickPeriodSet(SysCtrlClockGet()/2);

    /* 100kHz */
    //SysTickPeriodSet(SysCtrlClockGet()/100000);

    /* 1MHz */
    //SysTickPeriodSet(SysCtrlClockGet()/1000000);

    SysTickPeriodSet(SysCtrlClockGet()/FREQ_SYSTICK);


    SysTickIntDisable();
    // Use small interrupt map
    IntAltMapEnable();
    // The following call will result in a dynamic interrupt table being used.
    // The table resides in RAM.
    // Alternatively SysTickIntHandler can be statically registred in your
    // application.
    SysTickIntRegister(SysTickIntHandler);
    // Enable interrupts to the processor.
    IntMasterEnable();
    // Enable the SysTick Interrupt.
    SysTickIntEnable();
    // Enable SysTick.
    SysTickEnable();
}

/**
 * \brief Delay milliseconds
 */
void delay_ms(uint32_t milliseconds)
{
	uint32_t systick_counter_start;
	uint32_t local_systick;
	uint32_t ticks;

	/* grab current systick ASAP */
	systick_counter_start = systick_get();
	/* 1000 milliseconds in 1 second */
	ticks = ((SysCtrlClockGet()/SysTickPeriodGet()) / 1000) * milliseconds;

	/*  */
	while(1) {
		local_systick = systick_get();
		/* Check if time is up */
		if ((local_systick - systick_counter_start) >= ticks) {
			break;
		}
	}
}

void panic()
{
	led_panic();
}

/**
 * \brief Toggle LEDs ON and OFF
 */
void half_period_action()
{
	led_toggle(LED_WHITE);
}

void freqout_driver(uint32_t half_period)
{
	static uint32_t systick_counter_start = 0;
	static uint32_t prev_half_period = DEFAULT_HALF_PERIOD;
	uint32_t local_systick = systick_get();

	/* Check if half_period time is up */
	if ((local_systick - systick_counter_start) >= half_period) {
		/* reset the systick counter */
		systick_counter_start = local_systick;
		/* run half period action */
		half_period_action();
	}

	/* Detect when half_period changes */
	if (prev_half_period != half_period) {
		prev_half_period = half_period;
		/* do something when half_period changes */
	}
}

void freqout(uint32_t freq_half_period, uint32_t duration_period)
{
	uint32_t systick_counter_start;
	uint32_t local_systick;

	/* grab current systick ASAP */
	systick_counter_start = systick_get();

	/*  */
	while(1) {
		freqout_driver(freq_half_period);
		local_systick = systick_get();
		/* Check if time is up */
		if ((local_systick - systick_counter_start) >= duration_period) {
			break;
		}
	}
}

/**
 * \brief Sends 6bit data over VLC
 */
void VLC_send(uint8_t data)
{
	int num;

	/* Check that the data is only 6 bits wide */
	if (data & ~0x2F) {
		panic();
	}

	/* Preamble */
	freqout(PREAMBLE_HALF_PERIOD, BIT_PERIOD);
	/* Pilot Test */
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);

	/* Send each of the bits */
	for (num = 0; num < 6; num++) {
		freqout(
				(data&0x1) ? PILOT_HALF_PERIOD : DEFAULT_HALF_PERIOD,
				BIT_PERIOD
				);
		data >>= 1;
	}

	/* END Stop */
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
}

/**
 * \brief Sends 16bit data over VLC
 */
void VLC_send16(uint32_t data)
{
	int num;

	/* Check that the data is only 16 bits wide */
	if (data & ~0xFFFF) {
		panic();
	}

	/* Preamble */
	freqout(PREAMBLE_HALF_PERIOD, BIT_PERIOD);
	/* Pilot Test */
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);

	/* Send each of the bits */
	for (num = 0; num < 16; num++) {
		freqout(
				(data&0x1) ? PILOT_HALF_PERIOD : DEFAULT_HALF_PERIOD,
				BIT_PERIOD
				);
		data >>= 1;
	}

	/* END Stop */
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
}

void test()
{
	/* Preamble */
	freqout(PREAMBLE_HALF_PERIOD, (BIT_PERIOD*2)/3);
	/* Pilot Test */
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);

//	/* Send each of the bits */
//	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
//	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
//	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
//	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
//	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
//	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
//
//	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
//	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
//	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
//	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
//	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
//	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
//
//	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
//	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
//	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
//	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);

#	if BULB_SELECT == 2
	/* Send each of the bits */

	/* AB */
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);

	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);

	/* ~AB */
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);

	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);

#	elif BULB_SELECT == 3
	/* Send each of the bits */

	/* CD */
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);

	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);

	/* ~CD */
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);

	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);

#	elif BULB_SELECT == 4
	/* Send each of the bits */

	/* EF */
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);

	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);

	/* ~EF */
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);

	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);

#	endif

//	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
//	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
//	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);

	/* END Stop */
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
}


void test_VLC_freqs()
{
	led_on(LED_RED);
	delay_ms(500);
	led_off(LED_RED);
	led_on(LED_GREEN);
	delay_ms(500);
	led_off(LED_GREEN);
	led_on(LED_BLUE);
	delay_ms(500);
	led_off(LED_BLUE);

//	led_on(LED_RED);
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
	freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
	//freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
	//freqout(PILOT_HALF_PERIOD, BIT_PERIOD);
//	led_off(LED_RED);


//	led_on(LED_GREEN);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	//freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	//freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
//	led_off(LED_GREEN);

//	led_on(LED_BLUE);
	freqout(PREAMBLE_HALF_PERIOD, BIT_PERIOD);
	freqout(PREAMBLE_HALF_PERIOD, BIT_PERIOD);
	//freqout(PREAMBLE_HALF_PERIOD, BIT_PERIOD);
	//freqout(PREAMBLE_HALF_PERIOD, BIT_PERIOD);
//	led_off(LED_BLUE);

	led_off(LED_WHITE);
}

void IR_Test()
{
	const uint32_t freq_ir = (FREQ_SYSTICK / (2*38000));
	//freqout_driver(freq_ir);
	//freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(freq_ir, BIT_PERIOD);
	//freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(freq_ir, BIT_PERIOD);
	//freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(freq_ir, BIT_PERIOD);
	//freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(freq_ir, BIT_PERIOD);
	//freqout(DEFAULT_HALF_PERIOD, BIT_PERIOD);
	freqout(freq_ir, BIT_PERIOD);
}

void find_pins()
{
	volatile uint8_t val1 = 0, v1t;
	volatile uint8_t val2 = 0, v2t;
	volatile uint8_t val3 = 0, v3t;
	volatile uint8_t val4 = 0, v4t;

	GPIODirModeSet(GPIO_B_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_6|GPIO_PIN_7, GPIO_DIR_MODE_IN);
	GPIODirModeSet(GPIO_A_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_DIR_MODE_IN);
	GPIODirModeSet(GPIO_C_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_DIR_MODE_IN);
	GPIODirModeSet(GPIO_D_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_DIR_MODE_IN);

	val1 = GPIOPinRead(GPIO_B_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_6|GPIO_PIN_7);
	val2 = GPIOPinRead(GPIO_A_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);
	val3 = GPIOPinRead(GPIO_C_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);
	val4 = GPIOPinRead(GPIO_D_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);

    while(1) {
    	v2t = GPIOPinRead(GPIO_A_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);
    	if (val2 != v2t) {
    		led_on(LED_RED);
    		val2 = v2t;
    	}

    	v3t = GPIOPinRead(GPIO_C_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);
		if (val3 != v3t) {
			led_on(LED_GREEN);
			val3 = v3t;
		}

		v4t = GPIOPinRead(GPIO_D_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);
		if (val4 != v4t) {
			led_on(LED_BLUE);
			val4 = v4t;
		}

		delay_ms(300);
		led_off(LED_ALL);
    }
}

	int main(void) {

	lifx_bulb_setup();
	systick_setup();

    while(1) {

    	if (sw_read()) {
    		/* Switch 1 - Test Mode */

    		test_VLC_freqs();
    		//IR_Test();
    	} else {
    		/* Switch 0 - Normal Mode */

    		test();

    		/* The following attempts to use VLC_send functions has resulted in
    		 * errors. This is possinbly due to the added overhead of the for loop
    		 * contained within them. */
    		//VLC_send6(0x2A);
    		//VLC_send16(0x5555);
    		//VLC_send16(BULB_SELECT | (BULB_SELECT << 4));
    	}
    }
}
