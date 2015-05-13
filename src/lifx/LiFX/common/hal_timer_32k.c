//*****************************************************************************
//! @file       hal_timer_32k.c
//! @brief      32 kHz timer HAL implementation for CC2538.
//!
//! Revised     $Date: 2012-11-22 10:58:20 +0100 (to, 22 nov 2012) $
//! Revision    $Revision: 8825 $
//
//  Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************/


/**************************************************************************//**
* @addtogroup hal_timer_32k_api
* @{
******************************************************************************/


/******************************************************************************
* INCLUDES
*/
#include "hal_types.h"
#include "hal_defs.h"
#include "hal_timer_32k.h"

#include "hw_types.h"           // Using tBoolean
#include "hw_ints.h"            // Access to SM timer interrupt vector offset
#include "interrupt.h"          // Access to driverlib interrupt fns
#include "sleepmode.h"          // Access to driverlib sleepmode fns


/******************************************************************************
* DEFINES
*/


/******************************************************************************
* LOCAL VARIABLES
*/
static unsigned long ulHalTimer32kInterval;
static void (*pFptr)(void);


/******************************************************************************
* FUNCTION PROTOTYPES
*/
static void halTimer32kIsr(void);


/******************************************************************************
* GLOBAL FUNCTIONS
*/
/**************************************************************************//**
* @brief    Set up sleep mode timer to generate interrupt every \c cycles
*           32768 Hz. Use halTimer32kIntConnect() to connect an ISR to the
*           interrupt and halTimer32kIntEnable() to enable timer interrupts.
*
* @param    cycles      Number of cycles between every timer interrupt.
*
* @return   None
******************************************************************************/
void halTimer32kInit(uint16 cycles)
{
    // Store cycle count
    ulHalTimer32kInterval = cycles;

    // Disable sleep mode timer interrupts
    IntDisable(INT_SMTIM);

    // Set compare value
    SleepModeTimerCompareSet((SleepModeTimerCountGet() + cycles));
}


/**************************************************************************//**
* @brief    Restart 32 kHz timer. Timer interval is set using halTimer32kInit().
*
* @return   None
******************************************************************************/
void halTimer32kRestart(void)
{
    // Set sleep mode timer compare value
    SleepModeTimerCompareSet((SleepModeTimerCountGet()+ulHalTimer32kInterval));

    // Clear pending interrupts
    IntPendClear(INT_SMTIM);
}


/**************************************************************************//**
* @brief    Connect interrupt service routine to 32 kHz timer interrupt.
*
* @param    pFnHandle   Void function pointer to interrupt service routine
*
* @return   None
******************************************************************************/
void halTimer32kIntConnect(void (*pFnHandle)(void))
{
    tBoolean intDisabled = IntMasterDisable();
    SleepModeIntRegister(&halTimer32kIsr);  // Register function and
    IntDisable(INT_SMTIM);                  // disable SMTIM interrupts
    pFptr = pFnHandle;                      // Set custom ISR
    if(!intDisabled) IntMasterEnable();
}


/**************************************************************************//**
* @brief    Function enables 32 kHz timer interrupt.
*
* @return   None
******************************************************************************/
void halTimer32kIntEnable(void)
{
    // Clear pending interrupts
    IntPendClear(INT_SMTIM);

    // Enable interrupt
    IntEnable(INT_SMTIM);
}


/**************************************************************************//**
* @brief    Function disables 32 kHz timer interrupt.
*
* @return   None
******************************************************************************/
void halTimer32kIntDisable(void)
{
    // Disable sleep mode timer interrupts
    IntDisable(INT_SMTIM);
}


/******************************************************************************
* LOCAL FUNCTIONS
*/
/**************************************************************************//**
* @brief    Timer interrupt service routine. Sets new compare value and
*           executes user's custom ISR if any.
*
* @return   None
******************************************************************************/
static void halTimer32kIsr(void)
{
    // Set new compare value
    SleepModeTimerCompareSet((SleepModeTimerCountGet()+ulHalTimer32kInterval));

    // Run custom ISR
    if(pFptr) {
        (pFptr)();
    }
    // Not necessary to clear interrupt flag
}


/**************************************************************************//**
* Close the Doxygen group.
* @}
******************************************************************************/
