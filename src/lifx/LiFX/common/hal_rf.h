//*****************************************************************************
//! @file       hal_rf.h
//! @brief      HAL radio interface header file
//!
//! Revised     $Date: 2014-01-23 09:57:27 +0100 (to, 23 jan 2014) $
//! Revision    $Revision: 11974 $
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
#ifndef HAL_RF_H
#define HAL_RF_H


/******************************************************************************
* If building with a C++ compiler, make all of the definitions in this header
* have a C binding.
******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif


/******************************************************************************
* INCLUDES
*/
#if chip==2538
#include "hw_types.h"
#endif
#include <hal_types.h>


/******************************************************************************
* TYPEDEFS
*/


/******************************************************************************
* CONSTANTS AND DEFINES
*/
// Chip ID's
#define HAL_RF_CHIP_ID_CC1100               0x00
#define HAL_RF_CHIP_ID_CC1110               0x01
#define HAL_RF_CHIP_ID_CC1111               0x11
#define HAL_RF_CHIP_ID_CC2420               0x02
#define HAL_RF_CHIP_ID_CC2500               0x80
#define HAL_RF_CHIP_ID_CC2510               0x81
#define HAL_RF_CHIP_ID_CC2511               0x91
#define HAL_RF_CHIP_ID_CC2550               0x82
#define HAL_RF_CHIP_ID_CC2520               0x84
#define HAL_RF_CHIP_ID_CC2430               0x85
#define HAL_RF_CHIP_ID_CC2431               0x89
#define HAL_RF_CHIP_ID_CC2530               0xA5
#define HAL_RF_CHIP_ID_CC2531               0xB5
#define HAL_RF_CHIP_ID_CC2533               0x95
#define HAL_RF_CHIP_ID_CC2540               0x8D
#define HAL_RF_CHIP_ID_CC2541               0x41
#define HAL_RF_CHIP_ID_CC2538               0xB964

#define HAL_RF_CC2538EM                     0
#define HAL_RF_CC2538_CC2591EM              1
#define HAL_RF_CC2538_CC2592EM              2

// CC2590/91 gain modes
#define HAL_RF_GAIN_LOW                     0
#define HAL_RF_GAIN_HIGH                    1

// IEEE 802.15.4 defined constants (2.4 GHz logical channels)
#define MIN_CHANNEL 				        11    //!< Min. channel (2405 MHz)
#define MAX_CHANNEL                         26    //!< Max. channel (2480 MHz)
#define CHANNEL_SPACING                     5     //!< Channel spacing in MHz


/******************************************************************************
* GLOBAL FUNCTIONS
*/
// Generic RF interface
uint8 halRfInit(void);
uint8 halRfSetTxPower(uint8 power);
uint8 halRfTransmit(void);
void  halRfSetGain(uint8 gainMode);     // With CC2590/91 only
uint8 halRfSetModule(uint8 emModule);   // with/without CC2590?

uint16 halRfGetChipId(void);
uint8 halRfGetChipVer(void);
uint8 halRfGetRandomByte(void);
uint8 halRfGetRssiOffset(void);

void  halRfWriteTxBuf(uint8* pData, uint8 length);
void  halRfAppendTxBuf(uint8* pData, uint8 length);
void  halRfReadRxBuf(uint8* pData, uint8 length);
void  halRfWaitTransceiverReady(void);
uint8 halRfReadMemory(uint16 addr, uint8* pData, uint8 length);
uint8 halRfWriteMemory(uint16 addr, uint8* pData, uint8 length);

void  halRfReceiveOn(void);
void  halRfReceiveOff(void);
void  halRfDisableRxInterrupt(void);
void  halRfEnableRxInterrupt(void);
void  halRfRxInterruptConfig(ISR_FUNC_PTR pfISR);

// IEEE 802.15.4 specific interface
void  halRfSetChannel(uint8 channel);
void  halRfSetShortAddr(uint16 shortAddr);
void  halRfSetPanId(uint16 PanId);


/******************************************************************************
* Mark the end of the C bindings section for C++ compilers.
******************************************************************************/
#ifdef  __cplusplus
}
#endif
#endif // #ifndef HAL_RF_H
