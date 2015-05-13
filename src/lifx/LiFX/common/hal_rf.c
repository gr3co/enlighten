//*****************************************************************************
//! @file       hal_rf.c
//! @brief      CC2538 radio interface. Supports
//!             \li CC2538EM
//!             \li CC2538/CC2591EM
//!             \li CC2538/CC2592EM
//!
//! Revised     $Date: 2014-01-23 09:56:58 +0100 (to, 23 jan 2014) $
//! Revision    $Revision: 11972 $
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
* @addtogroup hal_rf_api
* @{
******************************************************************************/


/******************************************************************************
* INCLUDES
*/
#include "hw_types.h"               // Using HWREG() macro
#include "hal_int.h"
#include "hal_rf.h"

#include "hw_memmap.h"              // Peripheral base definitions
#include "hw_flash_ctrl.h"          // DIECFGx register definitions
#include "hw_ints.h"                // Interrupt definitions
#include "hw_ana_regs.h"            // Register definitions
#include "hw_rfcore_ffsm.h"         // Register definitions
#include "hw_rfcore_sfr.h"          // Register definitions
#include "hw_rfcore_xreg.h"         // Register definitions
#include "hw_sys_ctrl.h"            // Register definitions
#include "hw_gpio.h"                // Register definitions
#include "hw_ioc.h"                 // Register definitions
#include "hw_cctest.h"              // Register definitions
#include "../common/hal_defs.h"


/******************************************************************************
* CONSTANTS AND DEFINES
*/
// Chip revision
#define REV_A
#define CHIPREVISION

// CC2538 RSSI Offset
#define RSSI_OFFSET                     73
// CC2538/CC2591 RSSI Offset
#define RSSI_OFFSET_LNA_CC2591_HIGHGAIN 79
#define RSSI_OFFSET_LNA_CC2591_LOWGAIN  69
// CC2538/CC2592 RSSI Offset
#define RSSI_OFFSET_LNA_CC2592_HIGHGAIN 85
#define RSSI_OFFSET_LNA_CC2592_LOWGAIN  81

// Various radio settings
#define AUTO_ACK                    RFCORE_XREG_FRMCTRL0_AUTOACK
#define AUTO_CRC                    RFCORE_XREG_FRMCTRL0_AUTOCRC

// RF interrupt flags
#define IRQ_TXDONE                  0x00000002
#define IRQ_RXPKTDONE               0x00000040

// Selected strobes
#define RFST                        RFCORE_SFR_RFST
#define ISRXON()                    st(HWREG(RFST) = 0x000000E3;)
#define ISTXON()                    st(HWREG(RFST) = 0x000000E9;)
#define ISTXONCCA()                 st(HWREG(RFST) = 0x000000EA;)
#define ISRFOFF()                   st(HWREG(RFST) = 0x000000EF;)
#define ISFLUSHRX()                 st(HWREG(RFST) = 0x000000ED;)
#define ISFLUSHTX()                 st(HWREG(RFST) = 0x000000EE;)

#define HAL_PA_LNA_INIT()
#define HAL_PA_LNA_RX_LGM()         st(HWREG(GPIO_D_BASE + (GPIO_O_DATA +     \
                                       (0x04 << 2))) = 0;)
#define HAL_PA_LNA_RX_HGM()         st(HWREG(GPIO_D_BASE + (GPIO_O_DATA +     \
                                       (0x04 << 2))) = 0x04;)

// TX power function arguments
#define HAL_RF_TXPOWER_22_DBM       22
#define HAL_RF_TXPOWER_20_DBM       20
#define HAL_RF_TXPOWER_16_DBM       16
#define HAL_RF_TXPOWER_13_DBM       13
#define HAL_RF_TXPOWER_7_DBM        7
#define HAL_RF_TXPOWER_4_DBM        4
#define HAL_RF_TXPOWER_3_DBM        3
#define HAL_RF_TXPOWER_0_DBM        0
#define HAL_RF_TXPOWER_MIN_3_DBM    (0x80|3)
#define HAL_RF_TXPOWER_MIN_9_DBM    (0x80|9)
#define HAL_RF_TXPOWER_MIN_15_DBM   (0x80|15)

// TXPOWER values
#define CC2538_TXPOWER_7_DBM            0xFF
#define CC2538_TXPOWER_3_DBM            0xD5
#define CC2538_TXPOWER_0_DBM            0xB6
#define CC2538_TXPOWER_MIN_3_DBM        0xA1
#define CC2538_TXPOWER_MIN_9_DBM        0x72
#define CC2538_TXPOWER_MIN_15_DBM       0x42
// For CC2538/CC2591 combo
#define CC2538_CC2591_TXPOWER_20_DBM    0xE5
// For CC2538/CC2592 combo
#define CC2538_CC2592_TXPOWER_22_DBM    0xFF
#define CC2538_CC2592_TXPOWER_20_DBM    0xC5
#define CC2538_CC2592_TXPOWER_16_DBM    0x91
#define CC2538_CC2592_TXPOWER_13_DBM    0x72
#define CC2538_CC2592_TXPOWER_7_DBM     0x42
#define CC2538_CC2592_TXPOWER_4_DBM     0x24
#define CC2538_CC2592_TXPOWER_0_DBM     0x00


/******************************************************************************
* LOCAL VARIABLES
*/
static void (*pfISR)(void);
#ifdef INCLUDE_PA
static uint8 rssiOffset = RSSI_OFFSET_LNA_HIGHGAIN;
static unsigned char halRfEmModule = HAL_RF_CC2538_CC2592EM;
#else
static uint8 rssiOffset = RSSI_OFFSET;
static unsigned char halRfEmModule = HAL_RF_CC2538EM;
#endif


/******************************************************************************
* FUNCTION PROTOTYPES
*/
static void halRfIsr(void);
static void halRfPaLnaInit(void);


/******************************************************************************
* GLOBAL FUNCTIONS
*/
/**************************************************************************//**
* @brief    Power up, sets default tuning settings, enables autoack.
*
* @return   Returns SUCCESS (for interface compatibility)
******************************************************************************/
unsigned char halRfInit(void)
{
    //
    // Some of the below settings are indeed the reset value.
    //

    // Enable RF core clocks in active mode (not necessary on CC2538 PG1.0)
    HWREG(SYS_CTRL_RCGCRFC) = 1;

    // Enable auto ack and auto crc
    HWREG(RFCORE_XREG_FRMCTRL0) = (HWREG(RFCORE_XREG_FRMCTRL0) | (AUTO_ACK |  \
                                  AUTO_CRC));

    // Recommended RX settings
    HWREG(RFCORE_XREG_FRMFILT0) = 0x0D; // Enable frame filtering = 0x0D,
                                        // disable = 0x0C
    HWREG(RFCORE_XREG_AGCCTRL1) = 0x15;
    HWREG(RFCORE_XREG_FSCTRL)   = 0x5A;

    // Recommended TX settings (only those not already set for RX)
    HWREG(RFCORE_XREG_TXFILTCFG)= 0x09;
    HWREG(ANA_REGS_O_IVCTRL)    = 0x0B;
    HWREG(RFCORE_XREG_FRMCTRL1) = 0x00; // STXON does not affect RXENABLE[6]
    HWREG(RFCORE_XREG_MDMTEST1) = 0x08;
    HWREG(RFCORE_XREG_FSCAL1)   = 0x01;


    // Enable random generator
    // Not implemented

    if(halRfEmModule != HAL_RF_CC2538EM)
    {
        // Configure PA/LNA
        halRfPaLnaInit();
    }

    // Set RF interrupt priority to maximum
    IntPrioritySet(INT_RFCORERTX, 0);

    // Register halRfIsr() as RX interrupt function
    IntRegister(INT_RFCORERTX, &halRfIsr);

    // Enable RX interrupt
    halRfEnableRxInterrupt();

    return SUCCESS;
}


/**************************************************************************//**
* @brief    Function returns the chip ID.
*
* @return   Chip ID
******************************************************************************/
unsigned short halRfGetChipId(void)
{
    return HAL_RF_CHIP_ID_CC2538;
}


/**************************************************************************//**
* @brief    Function returns chip version.
*
* @return   Chip version (0 for CC2538 PG1, 2 for CC2538 PG2)
******************************************************************************/
unsigned char halRfGetChipVer(void)
{
    return ((HWREG(FLASH_CTRL_DIECFG2) >> 12) & 0x0F);
}


/**************************************************************************//**
* @brief    Function returns a random byte.
*
* @return   Random byte
******************************************************************************/
unsigned char halRfGetRandomByte(void)
{
    // Not implemented
    return 0;
}


/**************************************************************************//**
* @brief   Function returns RSSI offset.
*
* @return  RSSI offset
******************************************************************************/
unsigned char halRfGetRssiOffset(void)
{
    return rssiOffset;
}


/**************************************************************************//**
* @brief    Set RF channel in the 2.4GHz band. The Channel must be in the
*           range 11-26 (inclusive). 11=2405 MHz, channel spacing 5 MHz.
*
* @param    channel         Channel to set [11,26]
*
* @return   None
******************************************************************************/
void halRfSetChannel(unsigned char channel)
{
    HWREG(RFCORE_XREG_FREQCTRL) =
        (MIN_CHANNEL + (channel - MIN_CHANNEL) * CHANNEL_SPACING);
}


/**************************************************************************//**
* @brief    Function sets the device's short address
*
* @param    shortAddr       Chip short address
*
* @return   None
******************************************************************************/
void halRfSetShortAddr(unsigned short shortAddr)
{
    HWREG(RFCORE_FFSM_SHORT_ADDR0) = LO_UINT16(shortAddr);
    HWREG(RFCORE_FFSM_SHORT_ADDR1) = HI_UINT16(shortAddr);
}


/**************************************************************************//**
* @brief    Function sets the device's PAN ID.
*
* @param    panIdchannel    PAN ID to assign
*
* @return   None
******************************************************************************/
void halRfSetPanId(unsigned short panId)
{
    HWREG(RFCORE_FFSM_PAN_ID0) = LO_UINT16(panId);
    HWREG(RFCORE_FFSM_PAN_ID1) = HI_UINT16(panId);
}


/**************************************************************************//**
* @brief    Function sets the devices's TX power
*
* @param    power       Power level
*
* @return   SUCCSES or FAILED
******************************************************************************/
unsigned char halRfSetTxPower(unsigned char power)
{
    unsigned char n;

    if(halRfEmModule == HAL_RF_CC2538EM)
    {
        switch(power)
        {
        case HAL_RF_TXPOWER_7_DBM:      n = CC2538_TXPOWER_7_DBM;       break;
        case HAL_RF_TXPOWER_3_DBM:      n = CC2538_TXPOWER_3_DBM;       break;
        case HAL_RF_TXPOWER_0_DBM:      n = CC2538_TXPOWER_0_DBM;       break;
        case HAL_RF_TXPOWER_MIN_3_DBM:  n = CC2538_TXPOWER_MIN_3_DBM;   break;
        case HAL_RF_TXPOWER_MIN_9_DBM:  n = CC2538_TXPOWER_MIN_9_DBM;   break;
        case HAL_RF_TXPOWER_MIN_15_DBM: n = CC2538_TXPOWER_MIN_15_DBM;  break;
        default:
            return FAILED;
        }
    }
    else if(halRfEmModule == HAL_RF_CC2538_CC2591EM)
    {
        if(power != HAL_RF_TXPOWER_20_DBM)
        {
            return FAILED;
        }
        n = CC2538_CC2591_TXPOWER_20_DBM;

    }
    else if(halRfEmModule == HAL_RF_CC2538_CC2592EM)
    {
        switch(power)
        {
        case HAL_RF_TXPOWER_22_DBM: n = CC2538_CC2592_TXPOWER_22_DBM; break;
        case HAL_RF_TXPOWER_20_DBM: n = CC2538_CC2592_TXPOWER_20_DBM; break;
        case HAL_RF_TXPOWER_16_DBM: n = CC2538_CC2592_TXPOWER_16_DBM; break;
        case HAL_RF_TXPOWER_13_DBM: n = CC2538_CC2592_TXPOWER_13_DBM; break;
        case HAL_RF_TXPOWER_7_DBM:  n = CC2538_CC2592_TXPOWER_7_DBM;  break;
        case HAL_RF_TXPOWER_4_DBM:  n = CC2538_CC2592_TXPOWER_4_DBM;  break;
        case HAL_RF_TXPOWER_0_DBM:  n = CC2538_CC2592_TXPOWER_0_DBM;  break;
        default:
            return FAILED;
        }
    }
    else
    {
        return FAILED;
    }

    // Set TX power
    HWREG(RFCORE_XREG_TXPOWER) = n;

    return SUCCESS;
}


/**************************************************************************//**
* @brief    Set module. Enables to change between CC2538EM / CC2520-CC2592EM
*           support runtime. halRfInit must be called after running this
*           function.
*
* @param    uint8   emModule
*
* @return   none
******************************************************************************/
unsigned char halRfSetModule(unsigned char emModule)
{
    (void)halRfEmModule;

    switch(emModule) {
    case HAL_RF_CC2538EM:
    case HAL_RF_CC2538_CC2591EM:
    case HAL_RF_CC2538_CC2592EM:    break;
    default:                        return FAILED;
    }
    halRfEmModule = emModule;

    return SUCCESS;
}


/**************************************************************************//**
* @brief    Function sets the gain mode. Only applicable for units with
*           CC2590/91. This function assumes that CC2538 GPIO pins have been
*           configured as GPIO output, for example, by using halRfInit();
*
* @param    gainMode    Gain mode
*
* @return   None
******************************************************************************/
void halRfSetGain(unsigned char gainMode)
{
    if (gainMode == HAL_RF_GAIN_LOW)
    {
        // Setting low gain mode
        HAL_PA_LNA_RX_LGM();
        if(halRfEmModule == HAL_RF_CC2538_CC2592EM)
        {
            rssiOffset = RSSI_OFFSET_LNA_CC2592_LOWGAIN;
        }
        else if(halRfEmModule == HAL_RF_CC2538_CC2591EM)
        {
            rssiOffset = RSSI_OFFSET_LNA_CC2591_LOWGAIN;
        }
    }
    else
    {
        // Setting high gain mode
        HAL_PA_LNA_RX_HGM();
        if(halRfEmModule == HAL_RF_CC2538_CC2592EM)
        {
            rssiOffset = RSSI_OFFSET_LNA_CC2592_HIGHGAIN;
        }
        else if(halRfEmModule == HAL_RF_CC2538_CC2591EM)
        {
            rssiOffset = RSSI_OFFSET_LNA_CC2591_HIGHGAIN;
        }
    }
}


/**************************************************************************//**
* @brief    Function writes \e length bytes to the TX FIFO. The function
*           flushes the TX FIFO before writing.
*
* @param    pData       Pointer to source buffer
* @param    length      Number of bytes to write
*
* @return   None
******************************************************************************/
void halRfWriteTxBuf(unsigned char* pData, unsigned char length)
{
    unsigned char i;

    // Making sure that the TX FIFO is empty.
    ISFLUSHTX();

    // Clearing TX done interrupt
    HWREG(RFCORE_SFR_RFIRQF1) &= ~IRQ_TXDONE;

    // Insert data
    for(i = 0; i < length; i++)
    {
        HWREG(RFCORE_SFR_RFDATA) = pData[i];
    }
}


/**************************************************************************//**
* @brief    Function appends \e length bytes to the TX FIFO, i.e. it does not
*           flush the TX FIFO before writing.
*
* @param    pData       Pointer to source buffer
* @param    length      Number of bytes to write
*
* @return   None
******************************************************************************/
void halRfAppendTxBuf(unsigned char* pData, unsigned char length)
{
    unsigned char i;

    // Insert data
    for(i = 0; i < length; i++)
    {
        HWREG(RFCORE_SFR_RFDATA) = pData[i];
    }
}


/**************************************************************************//**
* @brief    Function reads \e length bytes from the RX FIFO.
*
* @param    pData       Pointer to destination buffer
* @param    length      Number of bytes to read
*
* @return   None
******************************************************************************/
void halRfReadRxBuf(unsigned char* pData, unsigned char length)
{
    // Read data
    while (length > 0)
    {
        *pData++ = HWREG(RFCORE_SFR_RFDATA);
        length--;
    }
}


/**************************************************************************//**
* @brief    Function reads \e length bytes from the device's memory.
*
* @param    addr        Start address in memory
* @param    pData       Pointer to destination buffer
* @param    length      Number of bytes to read
*
* @return   Number of bytes read
******************************************************************************/
unsigned char halRfReadMemory(unsigned short addr, unsigned char* pData,
                              unsigned char length)
{
    return 0;
}


/**************************************************************************//**
* @brief    Function writes \e length bytes to the device's memory.
*
* @param    addr        Start address in memory
* @param    pData       Pointer to source buffer
* @param    length      Number of bytes to write
*
* @return   Number of bytes written
******************************************************************************/
unsigned char halRfWriteMemory(unsigned short addr, unsigned char* pData,
                               unsigned char length)
{
    return 0;
}


/**************************************************************************//**
* @brief    Transmit frame. Function returns when frame is sent.
*
* @return   SUCCESS or FAIL
******************************************************************************/
unsigned char halRfTransmit(void)
{
    // Sending
    ISTXON();

    // Waiting for transmission to finish
    while(!(HWREG(RFCORE_SFR_RFIRQF1) & IRQ_TXDONE) );

    // Clear TXDONE interrupt flag
    HWREG(RFCORE_SFR_RFIRQF1) = HWREG(RFCORE_SFR_RFIRQF1) & ~IRQ_TXDONE;

    return SUCCESS;
}


/**************************************************************************//**
* @brief    Turn receiver on.
*
* @return   None
******************************************************************************/
void halRfReceiveOn(void)
{
    // Make sure the RX FIFO is empty and enter RX
    ISFLUSHRX();
    ISRXON();
}


/**************************************************************************//**
* @brief    Turn receiver off.
*
* @return   None
******************************************************************************/
void halRfReceiveOff(void)
{
    // Disable RX and make sure the RX FIFO is empty.
    ISRFOFF();
    ISFLUSHRX();
}


#ifndef MRFI
/**************************************************************************//**
* @brief    Clear and disable RX interrupt.
*
* @return   None
******************************************************************************/
void halRfDisableRxInterrupt(void)
{
    // disable RXPKTDONE interrupt
    HWREG(RFCORE_XREG_RFIRQM0) &= (~BV(6));
    // disable general RF interrupts
    IntDisable(INT_RFCORERTX);
}


/**************************************************************************//**
* @brief    Enable RX interrupt.
*
* @return   None
******************************************************************************/
void halRfEnableRxInterrupt(void)
{
    // enable RXPKTDONE interrupt
    HWREG(RFCORE_XREG_RFIRQM0) |= BV(6);

    // enable general RF interrupts
    IntEnable(INT_RFCORERTX);
}


/**************************************************************************//**
* @brief    Configure RX interrupt.
*
* @return   None
******************************************************************************/
void halRfRxInterruptConfig(void (*pf)(void))
{
    unsigned short s;
    HAL_INT_LOCK(s);
    pfISR= pf;
    HAL_INT_UNLOCK(s);
}
#endif


/**************************************************************************//**
* @brief    Function waits until the transceiver is ready (SFD inactive).
*
* @return   None
******************************************************************************/
void halRfWaitTransceiverReady(void)
{
    // Wait for SFD not active and TX_Active not active
    while (HWREG(RFCORE_XREG_FSMSTAT1) & (BV(1) | BV(5) ));
}


/**************************************************************************//**
* LOCAL FUNCTIONS
*/
/**************************************************************************//**
* @brief    This function initializes the CC2538 to control the CC2592 PA/LNA
*           signals. CC2538 GPIO connected to CC2592 HGM is configured as high.
*           CC2538 RX(TX) active status signals are mappe to CC2592 EN(PAEN)
*           signals.
*
* @return   None
******************************************************************************/
static void halRfPaLnaInit(void)
{
    // Configure CC2538 PD2 (CC2592 HGM) as GPIO output
    HWREG(GPIO_D_BASE + GPIO_O_DIR)   |= (0x04);
    HWREG(GPIO_D_BASE + GPIO_O_AFSEL) &= (0x04);
    HWREG(IOC_PD2_OVER) = 0;

    // Set CC2592 to HGM
    halRfSetGain(HAL_RF_GAIN_HIGH);

    // Use CC2538 RF status signals to control CC2592 LNAEN and PAEN.
    // CC2538 PC2 is connected to CC2592 LNAEN
    // CC2538 PC3 is connected to CC2592 PAEN
    HWREG(RFCORE_XREG_RFC_OBS_CTRL0) = 0x11; // rfc_obs_sig0 = rx_active
    HWREG(CCTEST_OBSSEL2)            = 0x80; // rfc_obs_sig0 => PC2
    HWREG(RFCORE_XREG_RFC_OBS_CTRL1) = 0x10; // rfc_obs_sig1 = tx_active
    HWREG(CCTEST_OBSSEL3)            = 0x81; // rfc_obs_sig1 => PC3
}


#ifndef MRFI
/**************************************************************************//**
* @brief    Interrupt service routine that handles RFPKTDONE interrupt.
*
* @return   None
******************************************************************************/
static void halRfIsr(void)
{
    unsigned short s;
    HAL_INT_LOCK(s);

    if(HWREG(RFCORE_SFR_RFIRQF0) & IRQ_RXPKTDONE)
    {
        if(pfISR)
        {
            // Execute the custom ISR
            (*pfISR)();
        }

        // Clear general RF interrupt flag
        IntPendClear(INT_RFCORERTX);

        // Clear RXPKTDONE interrupt
        HWREG(RFCORE_SFR_RFIRQF0) = HWREG(RFCORE_SFR_RFIRQF0) & ~IRQ_RXPKTDONE;
    }

    HAL_INT_UNLOCK(s);
}
#endif


/**************************************************************************//**
* Close the Doxygen group.
* @}
******************************************************************************/
