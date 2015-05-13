//*****************************************************************************
//! @file       hal_int.c
//! @brief      Hardware Abstraction Layer interrupt control.
//!
//! Revised     $Date: 2014-01-10 11:55:54 +0100 (fr, 10 jan 2014) $
//! Revision    $Revision: 11712 $
//
//  Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
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
* @addtogroup hal_int_api
* @{
******************************************************************************/


/******************************************************************************
* INCLUDES
*/
#include "hal_types.h"
#include "hal_defs.h"
#include "hal_int.h"


/**************************************************************************//**
* @brief    Enable global interrupts.
*
* @return   None
******************************************************************************/
void halIntOn(void)
{
    HAL_INT_ON();
}


/**************************************************************************//**
* @brief    Turns global interrupts off
*
* @return   None
******************************************************************************/
void halIntOff(void)
{
    HAL_INT_OFF();
}


/**************************************************************************//**
* @brief    Turns global interrupts off and returns current interrupt state.
*           Should always be used together with halIntUnlock().
*
* @return   Returns the current interrupt state
******************************************************************************/
uint16 halIntLock(void)
{
    uint16 key;
    HAL_INT_LOCK(key);
    return(key);
}


/**************************************************************************//**
* @brief    Set interrupt state back to the state it had before calling
*           halIntLock(). Should always be used together with halIntLock().
*
* @param    key     Interrupt state when halIntLock() was called.
*
* @return   None
******************************************************************************/
void halIntUnlock(uint16 key)
{
    HAL_INT_UNLOCK(key);
}


/**************************************************************************//**
* Close the Doxygen group.
* @}
******************************************************************************/
