//*****************************************************************************
//! @file       basic_rf.c
//! @brief      Basic RF library.
//!
//! Revised     $Date: 2014-01-23 09:55:53 +0100 (to, 23 jan 2014) $
//! Revision    $Revision: 11971 $
//
//  Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
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
* @addtogroup basic_rf_api
* @{
******************************************************************************/


/******************************************************************************
* INCLUDES
*/
#include "hal_int.h"
#include "hal_rf.h"
#include "basic_rf.h"
#ifdef __ICCARM__
#include "sys_ctrl.h"
#else
#include "hal_mcu.h"
#endif

/******************************************************************************
* CONSTANTS AND DEFINES
*/
// Packet and packet part lengths
#define PKT_LEN_MIC                         8
#define PKT_LEN_SEC                         PKT_LEN_UNSEC + PKT_LEN_MIC
#define PKT_LEN_AUTH                        8
#define PKT_LEN_ENCR                        24

// Packet overhead ((frame control field, sequence number, PAN ID,
// destination and source) + (footer))
// Note that the length byte itself is not included included in the packet length
#define BASIC_RF_PACKET_OVERHEAD_SIZE       ((2 + 1 + 2 + 2 + 2) + (2))
#define BASIC_RF_MAX_PAYLOAD_SIZE	        (127 - BASIC_RF_PACKET_OVERHEAD_SIZE - \
    BASIC_RF_AUX_HDR_LENGTH - BASIC_RF_LEN_MIC)
#define BASIC_RF_ACK_PACKET_SIZE	        5
#define BASIC_RF_FOOTER_SIZE                2
#define BASIC_RF_HDR_SIZE                   10

// The time it takes for the acknowledgment packet to be received after the
// data packet has been transmitted.
#define BASIC_RF_ACK_WAIT                   800   // milliseconds

// The length byte
#define BASIC_RF_PLD_LEN_MASK               0x7F

// Frame control field
#define BASIC_RF_FCF_NOACK                  0x8841
#define BASIC_RF_FCF_ACK                    0x8861
#define BASIC_RF_FCF_ACK_BM                 0x0020
#define BASIC_RF_FCF_BM                     (~BASIC_RF_FCF_ACK_BM)
#define BASIC_RF_SEC_ENABLED_FCF_BM         0x0008

// Frame control field LSB
#define BASIC_RF_FCF_NOACK_L                LO_UINT16(BASIC_RF_FCF_NOACK)
#define BASIC_RF_FCF_ACK_L                  LO_UINT16(BASIC_RF_FCF_ACK)
#define BASIC_RF_FCF_ACK_BM_L               LO_UINT16(BASIC_RF_FCF_ACK_BM)
#define BASIC_RF_FCF_BM_L                   LO_UINT16(BASIC_RF_FCF_BM)
#define BASIC_RF_SEC_ENABLED_FCF_BM_L       LO_UINT16(BASIC_RF_SEC_ENABLED_FCF_BM)

// Auxiliary Security header
#define BASIC_RF_AUX_HDR_LENGTH             5
#define BASIC_RF_LEN_AUTH                   BASIC_RF_PACKET_OVERHEAD_SIZE + \
    BASIC_RF_AUX_HDR_LENGTH - BASIC_RF_FOOTER_SIZE
#define BASIC_RF_SECURITY_M                 2
#define BASIC_RF_LEN_MIC                    8
#ifdef SECURITY_CCM
#undef BASIC_RF_HDR_SIZE
#define BASIC_RF_HDR_SIZE                   15
#endif

// Footer
#define BASIC_RF_CRC_OK_BM                  0x80

/******************************************************************************
* TYPEDEFS
*/
// The receive struct
typedef struct
{
  uint8 seqNumber;
  uint16 srcAddr;
  uint16 srcPanId;
  int8 length;
  uint8* pPayload;
  uint8 ackRequest;
  int8 rssi;
  volatile uint8 isReady;
  uint8 status;
} basicRfRxInfo_t;

// Tx state
typedef struct
{
  uint8 txSeqNumber;
  volatile uint8 ackReceived;
  uint8 receiveOn;
  uint32 frameCounter;
} basicRfTxState_t;


// Basic RF packet header (IEEE 802.15.4)
#if defined __ICC430__
#pragma pack(1)
#endif
typedef struct
{
  uint8   packetLength;
  uint8   fcf0;           // Frame control field LSB
  uint8   fcf1;           // Frame control field MSB
  uint8   seqNumber;
  uint16  panId;
  uint16  destAddr;
  uint16  srcAddr;
#ifdef SECURITY_CCM
  uint8   securityControl;
  uint8  frameCounter[4];
#endif
} basicRfPktHdr_t;
#if defined __ICC430__
#pragma pack()
#endif


/******************************************************************************
* LOCAL VARIABLES
*/
static basicRfRxInfo_t  rxi=      { 0xFF }; // Make sure sequence numbers are
static basicRfTxState_t txState=  { 0x00 }; // initialised and distinct.

static basicRfCfg_t* pConfig;
static uint8 txMpdu[BASIC_RF_MAX_PAYLOAD_SIZE+BASIC_RF_PACKET_OVERHEAD_SIZE+1];
static uint8 rxMpdu[128];

/******************************************************************************
* GLOBAL VARIABLES
*/


/******************************************************************************
* LOCAL FUNCTIONS
*/
/**************************************************************************//**
* @brief    Builds packet header according to IEEE 802.15.4 frame format
*
* @param    buffer          Pointer to buffer to write the header
* @param    destAddr        Destination short address
* @param    payloadLength   Length of higher layer payload
*
* @return   Returns  length of header
******************************************************************************/
static uint8 basicRfBuildHeader(uint8* buffer, uint16 destAddr, uint8 payloadLength)
{
  basicRfPktHdr_t *pHdr;
  uint16 fcf;

  pHdr= (basicRfPktHdr_t*)buffer;

  // Populate packet header
  pHdr->packetLength = payloadLength + BASIC_RF_PACKET_OVERHEAD_SIZE;
  fcf= pConfig->ackRequest ? BASIC_RF_FCF_ACK : BASIC_RF_FCF_NOACK;
  pHdr->fcf0 = LO_UINT16(fcf);
  pHdr->fcf1 = HI_UINT16(fcf);
  pHdr->seqNumber= txState.txSeqNumber;
  pHdr->panId= pConfig->panId;
  pHdr->destAddr= destAddr;
  pHdr->srcAddr= pConfig->myAddr;

#ifdef SECURITY_CCM

  // Add security to FCF, length and security header
  pHdr->fcf0 |= BASIC_RF_SEC_ENABLED_FCF_BM_L;
  pHdr->packetLength += PKT_LEN_MIC;
  pHdr->packetLength += BASIC_RF_AUX_HDR_LENGTH;

  pHdr->securityControl= SECURITY_CONTROL;
  pHdr->frameCounter[0]=   LO_UINT16(LO_UINT32(txState.frameCounter));
  pHdr->frameCounter[1]=   HI_UINT16(LO_UINT32(txState.frameCounter));
  pHdr->frameCounter[2]=   LO_UINT16(HI_UINT32(txState.frameCounter));
  pHdr->frameCounter[3]=   HI_UINT16(HI_UINT32(txState.frameCounter));

#endif

  // Make sure bytefields are network byte order
  UINT16_HTON(pHdr->panId);
  UINT16_HTON(pHdr->destAddr);
  UINT16_HTON(pHdr->srcAddr);

  return BASIC_RF_HDR_SIZE;
}


/**************************************************************************//**
* @brief    Builds mpdu (MAC header + payload) according to IEEE 802.15.4
*           frame format
*
* @param    destAddr        Destination short address
* @param    pPayload        Pointer to buffer with payload
* @param    payloadLength   Length of payload buffer
*
* @return   Returns length of mpdu
******************************************************************************/
static uint8 basicRfBuildMpdu(uint16 destAddr, uint8* pPayload, uint8 payloadLength)
{
  uint8 hdrLength, n;

  hdrLength = basicRfBuildHeader(txMpdu, destAddr, payloadLength);

  for(n=0;n<payloadLength;n++)
  {
    txMpdu[hdrLength+n] = pPayload[n];
  }

  return hdrLength + payloadLength; // total mpdu length
}


/**************************************************************************//**
* @brief    Interrupt service routine for received frame from radio
*           (either data or acknowlegdement)
*
*           rxi         File scope variable info extracted from the last
*                       incoming frame.
*           txState     File scope variable that keeps tx state info
*
* @return   None
******************************************************************************/
static void basicRfRxFrmDoneIsr(void)
{
  basicRfPktHdr_t *pHdr;
  uint8 *pStatusWord;
#ifdef SECURITY_CCM
  uint8 authStatus=0;
#endif

  // Map header to packet buffer
  pHdr= (basicRfPktHdr_t*)rxMpdu;

  // Clear interrupt and disable new RX frame done interrupt
  halRfDisableRxInterrupt();

  // Enable all other interrupt sources (enables interrupt nesting)
  halIntOn();

  // Read payload length.
  halRfReadRxBuf(&pHdr->packetLength,1);
  pHdr->packetLength &= BASIC_RF_PLD_LEN_MASK; // Ignore MSB

  // Is this an acknowledgment packet?
  // Only ack packets may be 5 bytes in total.
  if (pHdr->packetLength == BASIC_RF_ACK_PACKET_SIZE) {

    // Read the packet
    halRfReadRxBuf(&rxMpdu[1], pHdr->packetLength);

    // Make sure byte fields are changed from network to host byte order
    UINT16_NTOH(pHdr->panId);
    UINT16_NTOH(pHdr->destAddr);
    UINT16_NTOH(pHdr->srcAddr);
#ifdef SECURITY_CCM
    UINT32_NTOH(pHdr->frameCounter);
#endif

    rxi.ackRequest = !!(pHdr->fcf0 & BASIC_RF_FCF_ACK_BM_L);

    // Read the status word and check for CRC OK
    pStatusWord= rxMpdu + 4;

    // Indicate the successful ACK reception if CRC and sequence number OK
    if ((pStatusWord[1] & BASIC_RF_CRC_OK_BM) && (pHdr->seqNumber == txState.txSeqNumber)) {
      txState.ackReceived = TRUE;
    }
  }
  else
  {
    // It is data

    // It is assumed that the radio rejects packets with invalid length.
    // Subtract the number of bytes in the frame overhead to get actual payload.

    rxi.length = pHdr->packetLength - BASIC_RF_PACKET_OVERHEAD_SIZE;

#ifdef SECURITY_CCM
    rxi.length -= (BASIC_RF_AUX_HDR_LENGTH + BASIC_RF_LEN_MIC);
    authStatus = halRfReadRxBufSecure(&rxMpdu[1], pHdr->packetLength, rxi.length,
                                      BASIC_RF_LEN_AUTH, BASIC_RF_SECURITY_M);
#else
    halRfReadRxBuf(&rxMpdu[1], pHdr->packetLength);
#endif

    // Make sure byte fields are changed from network to host byte order
    UINT16_NTOH(pHdr->panId);
    UINT16_NTOH(pHdr->destAddr);
    UINT16_NTOH(pHdr->srcAddr);
#ifdef SECURITY_CCM
    UINT32_NTOH(pHdr->frameCounter);
#endif

    rxi.ackRequest = !!(pHdr->fcf0 & BASIC_RF_FCF_ACK_BM_L);

    // Read the source address
    rxi.srcAddr= pHdr->srcAddr;

    // Read the packet payload
    rxi.pPayload = rxMpdu + BASIC_RF_HDR_SIZE;

    // Read the FCS to get the RSSI and CRC
    pStatusWord= rxi.pPayload+rxi.length;
#ifdef SECURITY_CCM
    pStatusWord+= BASIC_RF_LEN_MIC;
#endif
    rxi.rssi = pStatusWord[0];

    // Notify the application about the received data packet if the CRC is OK
    // Throw packet if the previous packet had the same sequence number
    if( (pStatusWord[1] & BASIC_RF_CRC_OK_BM) && (rxi.seqNumber != pHdr->seqNumber) )
    {
      // If security is used check also that authentication passed
#ifdef SECURITY_CCM
      if( authStatus==SUCCESS )
      {
        if ( (pHdr->fcf0 & BASIC_RF_FCF_BM_L) ==
            (BASIC_RF_FCF_NOACK_L | BASIC_RF_SEC_ENABLED_FCF_BM_L))
        {
          rxi.isReady = TRUE;
        }
      }
#else
      if ( ((pHdr->fcf0 & (BASIC_RF_FCF_BM_L)) == BASIC_RF_FCF_NOACK_L) )
      {
        rxi.isReady = TRUE;
      }
      else
      {
        rxi.isReady = FALSE;
      }
#endif
    }
    else
    {
      rxi.isReady = FALSE;
    }
    rxi.seqNumber = pHdr->seqNumber;
  }

  // Enable RX frame done interrupt again
  halIntOff();
  halRfEnableRxInterrupt();
}


/******************************************************************************
* GLOBAL FUNCTIONS
*/
/**************************************************************************//**
* @brief    Initialise basic RF datastructures. Sets channel, short address and
*           PAN id in the chip and configures interrupt on packet reception
*
* @param    pRfConfig   Pointer to BASIC_RF_CONFIG struct. This struct must be
*                       allocated by higher layer.
*           txState     File scope variable that keeps tx state info.
*           rxi         File scope variable info extracted from the last
*                       incoming frame.
*
* @return   None
******************************************************************************/
uint8 basicRfInit(basicRfCfg_t* pRfConfig)
{
  if (halRfInit()==FAILED)
    return FAILED;

  halIntOff();

  // Set the protocol configuration
  pConfig = pRfConfig;
  rxi.pPayload   = NULL;

  txState.receiveOn = TRUE;
  txState.frameCounter = 0;

  // Set channel
  halRfSetChannel(pConfig->channel);

  // Write the short address and the PAN ID to the CC2520 RAM
  halRfSetShortAddr(pConfig->myAddr);
  halRfSetPanId(pConfig->panId);

  // if security is enabled, write key and nonce
#ifdef SECURITY_CCM
  basicRfSecurityInit(pConfig);
#endif

  // Set up receive interrupt (received data or acknowlegment)
  halRfRxInterruptConfig(basicRfRxFrmDoneIsr);

  halIntOn();

  return SUCCESS;
}


/**************************************************************************//**
* @brief    Send packet
*
* @param    destAddr    Destination short address
* @param    pPayload    Pointer to payload buffer. This buffer must be
*                       allocated by higher layer.
* @param    length      Length of payload
*           txState     File scope variable that keeps tx state info
*           mpdu        File scope variable. Buffer for the frame to send
*
* @return   Returns SUCCESS or FAILED
******************************************************************************/
uint8 basicRfSendPacket(uint16 destAddr, uint8* pPayload, uint8 length)
{
  uint8 mpduLength;
  uint8 status;

  // Turn on receiver if its not on
  if(!txState.receiveOn) {
    halRfReceiveOn();
  }

  // Check packet length
  length = MIN(length, BASIC_RF_MAX_PAYLOAD_SIZE);

  // Wait until the transceiver is idle
  halRfWaitTransceiverReady();

  // Turn off RX frame done interrupt to avoid interference on the SPI interface
  halRfDisableRxInterrupt();

  mpduLength = basicRfBuildMpdu(destAddr, pPayload, length);

#ifdef SECURITY_CCM
  halRfWriteTxBufSecure(txMpdu, mpduLength, length, BASIC_RF_LEN_AUTH, BASIC_RF_SECURITY_M);
  txState.frameCounter++;     // Increment frame counter field
#else
  halRfWriteTxBuf(txMpdu, mpduLength);
#endif

  // Turn on RX frame done interrupt for ACK reception
  halRfEnableRxInterrupt();

  // Send frame with CCA. return FAILED if not successful
  if(halRfTransmit() != SUCCESS) {
    status = FAILED;
  }

  // Wait for the acknowledge to be received, if any
  if (pConfig->ackRequest) {
    txState.ackReceived = FALSE;

    // We'll enter RX automatically, so just wait until we can be sure that the ack reception should have finished
#ifdef __ICCARM__

    // The timeout consists of a 12-symbol turnaround time, the ack packet duration, and a small margin
    // TODO: Improve solution!
    SysCtrlDelay((uint32)((SysCtrlClockGet() / 1000000) * BASIC_RF_ACK_WAIT));
#else
    halMcuWaitUs(BASIC_RF_ACK_WAIT);
#endif

    // If an acknowledgment has been received (by RxFrmDoneIsr), the ackReceived flag should be set
    status = txState.ackReceived ? SUCCESS : FAILED;

  } else {
    status = SUCCESS;
  }

  // Turn off the receiver if it should not continue to be enabled
  if (!txState.receiveOn) {
    halRfReceiveOff();
  }

  if(status == SUCCESS) {
    txState.txSeqNumber++;
  }

#ifdef SECURITY_CCM
  halRfIncNonceTx();          // Increment nonce value
#endif

  return status;
}


/**************************************************************************//**
* @brief    Check if a new packet is ready to be read by next higher layer
*
* @param    none
*
* @return   uint8 - TRUE if a packet is ready to be read by higher layer
******************************************************************************/
uint8 basicRfPacketIsReady(void)
{
  return rxi.isReady;
}


/**************************************************************************//**
* @brief    Copies the payload of the last incoming packet into a buffer
*
* @param    pRxData     Pointer to data buffer to fill. This buffer must be
*                       allocated by higher layer.
* @param    len         Number of bytes to read in to buffer
* @param    pRssi       Pointer to variable holding packet RSSI. NULL if RSSI
*                       is not to be stored.
*           rxi         File scope variable holding the information of the last
*                       incoming packet
*
* @return   uint8 - Number of bytes actually copied into buffer
******************************************************************************/
uint8 basicRfReceive(uint8* pRxData, uint8 len, int16* pRssi)
{
  uint8 chunkSize, i;

  halIntOff();

  // Critical region start
  // TODO: Copy data using DMA?
  chunkSize = MIN(rxi.length, len);
  for(i = 0; i < chunkSize; i++) {
    *pRxData++ = rxi.pPayload[i];
  }

  if(pRssi != NULL) {
    *pRssi = rxi.rssi - halRfGetRssiOffset();
  }
  rxi.isReady = FALSE;

  // Critical region end
  halIntOn();

  return chunkSize;
}


/**************************************************************************//**
* @brief    Function copies the payload of the last incoming packet into a
*           buffer.
*
* @return   int8 - RSSI value
******************************************************************************/
int8 basicRfGetRssi(void)
{
  return rxi.rssi - halRfGetRssiOffset();
}

/**************************************************************************//**
* @brief    Turns on receiver on radio
*
*           txState     File scope variable
*
* @return   None
******************************************************************************/
void basicRfReceiveOn(void)
{
  txState.receiveOn = TRUE;
  halRfReceiveOn();
}


/**************************************************************************//**
* @brief    Turns off receiver on radio
*
*           txState     File scope variable
*
* @return   None
******************************************************************************/
void basicRfReceiveOff(void)
{
  txState.receiveOn = FALSE;
  halRfReceiveOff();
}


/**************************************************************************//**
* Close the Doxygen group.
* @}
******************************************************************************/
