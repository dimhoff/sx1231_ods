/**
 * sx1231_enums.h - Defines for register addresses/masks
 *
 * Copyright (c) 2018, David Imhoff <dimhoff.devel@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the author nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __SX1231_ENUMS_H__
#define __SX1231_ENUMS_H__

#define SX1231_VERSION 0x20
#define SX1231_VERSION_MASK 0xF0

enum {
	RegFifo			= 0x00,
	RegOpMode		= 0x01,
	RegDataModul		= 0x02,
	RegBitrateMsb		= 0x03,
	RegBitrateLsb		= 0x04,
	RegFdevMsb		= 0x05,
	RegFdevLsb		= 0x06,
	RegFrfMsb		= 0x07,
	RegFrfMid		= 0x08,
	RegFrfLsb		= 0x09,
	RegOsc1			= 0x0A,
	RegAfcCtrl		= 0x0B,
	RegLowBat		= 0x0C,
	RegListen1		= 0x0D,
	RegListen2		= 0x0E,
	RegListen3		= 0x0F,
	RegVersion		= 0x10,
	RegPaLevel		= 0x11,
	RegPaRamp		= 0x12,
	RegOcp			= 0x13,
//	Reserved14		= 0x14,
//	Reserved15		= 0x15,
//	Reserved16		= 0x16,
//	Reserved17		= 0x17,
	RegLna			= 0x18,
	RegRxBw			= 0x19,
	RegAfcBw		= 0x1A,
	RegOokPeak		= 0x1B,
	RegOokAvg		= 0x1C,
	RegOokFix		= 0x1D,
	RegAfcFei		= 0x1E,
	RegAfcMsb		= 0x1F,
	RegAfcLsb		= 0x20,
	RegFeiMsb		= 0x21,
	RegFeiLsb		= 0x22,
	RegRssiConfig		= 0x23,
	RegRssiValue		= 0x24,
	RegDioMapping1		= 0x25,
	RegDioMapping2		= 0x26,
	RegIrqFlags1		= 0x27,
	RegIrqFlags2		= 0x28,
	RegRssiThresh		= 0x29,
	RegRxTimeout1		= 0x2A,
	RegRxTimeout2		= 0x2B,
	RegPreambleMsb		= 0x2C,
	RegPreambleLsb		= 0x2D,
	RegSyncConfig		= 0x2E,
	RegSyncValue		= 0x2F,
	RegPacketConfig1	= 0x37,
	RegPayloadLength	= 0x38,
	RegNodeAdrs		= 0x39,
	RegBroadcastAdrs	= 0x3A,
	RegAutoModes		= 0x3B,
	RegFifoThresh		= 0x3C,
	RegPacketConfig2	= 0x3D,
	RegAesKey		= 0x3E,
	RegTemp1		= 0x4E,
	RegTemp2		= 0x4F,
	RegTestLna		= 0x58,
	RegTestTcxo		= 0x59,
	RegTestllBw		= 0x5F,
	RegTestDagc		= 0x6F,
	RegTestAfc		= 0x71,
};

// RegOpMode
enum {
	OP_MODE_SEQUENCEROFF	= 0x80,
	OP_MODE_LISTENON	= 0x40,
	OP_MODE_LISTENABORT	= 0x20,
	OP_MODE_MODE_SLEEP	= 0x00,
	OP_MODE_MODE_STDBY	= (0x01 << 2),
	OP_MODE_MODE_FS		= (0x02 << 2),
	OP_MODE_MODE_TX		= (0x03 << 2),
	OP_MODE_MODE_RX		= (0x04 << 2),
};

// RegPacketConfig1
enum {
	PACKET_CONFIG1_PACKETFORMAT		= 0x80,
	PACKET_CONFIG1_DCFREE_NONE		= (0 << 5),
	PACKET_CONFIG1_DCFREE_MANCHESTER	= (1 << 5),
	PACKET_CONFIG1_DCFREE_WHITENING		= (2 << 5),
	PACKET_CONFIG1_CRCON			= 0x10,
	PACKET_CONFIG1_CRCAUTOCLEAROFF		= 0x80,
	PACKET_CONFIG1_ADDRESSFILTERING_NONE	= (0 << 1),
	PACKET_CONFIG1_ADDRESSFILTERING_NODE	= (1 << 1),
	PACKET_CONFIG1_ADDRESSFILTERING_NODE_BCAST = (2 << 1),
};

// RegIrqFlags1
enum {
        IRQ_FLAGS1_MODEREADY		= 0x80,
	IRQ_FLAGS1_RXREADY		= 0x40,
	IRQ_FLAGS1_TXREADY		= 0x20,
	IRQ_FLAGS1_PLLLOCK		= 0x10,
	IRQ_FLAGS1_RSSI			= 0x08,
	IRQ_FLAGS1_TIMEOUT		= 0x04,
	IRQ_FLAGS1_AUTOMODE		= 0x02,
	IRQ_FLAGS1_SYNCADDRESSMATCH 	= 0x01,
};

// RegIrqFlags2
enum {
	IRQ_FLAGS2_FIFOFULL	= 0x80,
	IRQ_FLAGS2_FIFONOTEMPTY = 0x40,
	IRQ_FLAGS2_FIFOLEVEL 	= 0x20,
	IRQ_FLAGS2_FIFOOVERRUN 	= 0x10,
	IRQ_FLAGS2_PACKETSENT 	= 0x08,
	IRQ_FLAGS2_PAYLOADREADY = 0x04,
	IRQ_FLAGS2_CRCOK 	= 0x02,
	IRQ_FLAGS2_LOWBAT 	= 0x01,
};

#endif // __SX1231_ENUMS_H__
