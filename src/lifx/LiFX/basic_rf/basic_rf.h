//*****************************************************************************
//! @file       basic_rf.s
//! @brief      Basic RF library header file.
//!
//!             The "Basic RF" library contains simple functions for packet
//!             transmission and reception with the IEEE 802.15.4 compliant
//!             radio devices. The intention of this library is to demonstrate
//!             how the radio devices are operated, and not to provide a
//!             complete and fully-functional packet protocol. The protocol
//!             uses 802.15.4 MAC compliant data and acknowledgment packets,
//!             however it contains only a small subset of  the 802.15.4
//!             standard:
//!             - Association, scanning nor beacons are not implemented
//!             - No defined coordinator/device roles (peer-to-peer, all nodes
//!               are equal)
//!             - Waits for the channel to become ready, but does not check CCA
//!               twice (802.15.4 CSMA-CA)
//!             - Does not retransmit packets
//!
//!             INSTRUCTIONS:
//!             Startup:
//!             1. Create a basicRfCfg_t structure, and initialize the members:
//!             2. Call basicRfInit() to initialize the packet protocol.
//!
//!             Transmission:
//!             1. Create a buffer with the payload to send
//!             2. Call basicRfSendPacket()
//!
//!             Reception:
//!             1. Check if a packet is ready to be received by highger layer
//!                with basicRfPacketIsReady()
//!             2. Call basicRfReceive() to receive the packet by higher layer
//!
//!             FRAME FORMATS:
//!             Data packets (without security):
//!             [Preambles (4)][SFD (1)][Length (1)][Frame control field (2)]
//!             [Sequence number (1)][PAN ID (2)][Dest. address (2)][Source address (2)]
//!             [Payload (Length - 2+1+2+2+2)][Frame check sequence (2)]
//!
//!             Acknowledgment packets:
//!             [Preambles (4)][SFD (1)][Length = 5 (1)][Frame control field (2)]
//!             [Sequence number (1)][Frame check sequence (2)]
//!
//! Revised     $Date: 2012-11-21 16:28:30 +0100 (on, 21 nov 2012) $
//! Revision    $Revision: 8816 $
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
#ifndef __BASIC_RF_H__
#define __BASIC_RF_H__


/******************************************************************************
* INCLUDES
*/
#include "../common/hal_defs.h"
#include "../common/hal_types.h"


/******************************************************************************
* TYPEDEFS
*/

typedef struct {
    uint16 myAddr;
    uint16 panId;
    uint8 channel;
    uint8 ackRequest;
    #ifdef SECURITY_CCM
    uint8* securityKey;
    uint8* securityNonce;
    #endif
} basicRfCfg_t;


/******************************************************************************
* GLOBAL FUNCTIONS
*/
uint8 basicRfInit(basicRfCfg_t* pRfConfig);
uint8 basicRfSendPacket(uint16 destAddr, uint8* pPayload, uint8 length);
uint8 basicRfPacketIsReady(void);
int8   basicRfGetRssi(void);
uint8 basicRfReceive(uint8* pRxData, uint8 len, int16* pRssi);
void basicRfReceiveOn(void);
void basicRfReceiveOff(void);


#endif // #ifdef __BASIC_RF_H__
