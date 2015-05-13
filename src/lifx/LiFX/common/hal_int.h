/*******************************************************************************
*  Filename:        hal_int.h
*  Revised:         $Date: 2013-04-22 10:29:55 +0200 (ma, 22 apr 2013) $
*  Revision:        $Revision: 9898 $
*
*  Description:     Hardware Abstraction Layer interrupt control header file.
*
*
*  Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
*
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*******************************************************************************/


#ifndef HAL_INT_H
#define HAL_INT_H

/*******************************************************************************
* If building with a C++ compiler, make all of the definitions in this header
* have a C binding.
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * INCLUDES
 */
#include "hal_defs.h"
#include "hal_types.h"
#if defined(ewarm) || defined(__ICCARM__)
#include "hw_types.h"
#include "interrupt.h"
#endif


/*******************************************************************************
 * MACROS
 */
#if (defined __ICC430__) || defined(__MSP430__)

#include "intrinsics.h"

// Use the macros below to reduce function call overhead for common
// global interrupt control functions

#include "intrinsics.h"

#if (defined __ICC430__)
#define HAL_INT_ON(x)      st( __enable_interrupt(); )
#define HAL_INT_OFF(x)     st( __disable_interrupt(); )
#define HAL_INT_LOCK(x)    st( (x) = __get_interrupt_state(); \
                               __disable_interrupt(); )
#define HAL_INT_UNLOCK(x)  st( __set_interrupt_state(x); )
#endif

#if (defined __MSP430__)
#define HAL_INT_ON(x)      st( _enable_interrupts(); )
#define HAL_INT_OFF(x)     st( _disable_interrupts(); )
#define HAL_INT_LOCK(x)    st( (x) = _get_SR_register(); \
                               _disable_interrupts(); )
#define HAL_INT_UNLOCK(x)  st( _enable_interrupts(); /*_bis_SR_register(x);*/ )
#endif

#elif defined __ICC8051__

#define HAL_INT_ON(x)      st( EA = 1; )
#define HAL_INT_OFF(x)     st( EA = 0; )
#define HAL_INT_LOCK(x)    st( (x) = EA; EA = 0; )
#define HAL_INT_UNLOCK(x)  st( EA = (x); )

typedef unsigned short istate_t;

#elif defined DESKTOP

#define HAL_INT_ON()
#define HAL_INT_OFF()
#define HAL_INT_LOCK(x)    st ((x)= 1; )
#define HAL_INT_UNLOCK(x)

#elif defined __KEIL__

#define HAL_INT_ON(x)      st( EA = 1; )
#define HAL_INT_OFF(x)     st( EA = 0; )
#define HAL_INT_LOCK(x)    st( (x) = EA; EA = 0; )
#define HAL_INT_UNLOCK(x)  st( EA = (x); )

#elif (defined(ewarm) || defined(__ICCARM__))

#define HAL_INT_ON(x)      st( IntMasterEnable(); )
#define HAL_INT_OFF(x)     st( IntMasterDisable(); )
#define HAL_INT_LOCK(x)    st( (x) = !IntMasterDisable(); )
#define HAL_INT_UNLOCK(x)  st( if(x){ IntMasterEnable();} )

#else
#error "Unsupported compiler"
#endif


/*******************************************************************************
 * GLOBAL FUNCTIONS
 */
void   halIntOn(void);
void   halIntOff(void);
uint16 halIntLock(void);
void   halIntUnlock(uint16 key);


/*******************************************************************************
* Mark the end of the C bindings section for C++ compilers.
*******************************************************************************/
#ifdef  __cplusplus
}
#endif
#endif // #ifndef HAL_INT_H