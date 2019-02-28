/**
 * sx1231_rts.c - Somfy RTS implementation using libsx1231_ods
 *
 * Copyright (c) 2019, David Imhoff <dimhoff.devel@gmail.com>
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
#include "config.h"

#include "sx1231_ods.h"

#include <stdbool.h>
#include <time.h>

#define RTS_BITRATE		(1.655629139)
					// Bit rate at which to serialize data.
					// 1 basic RTS interval = 604 us.
#define RTS_INTER_FRAME_GAP_US  (30415)

#define bRts_MaxFrameSize_c	(23)	// length of frame buffer in bytes
#define bRts_PreambleSize_c	(9)	// offset of payload in frame buffer in bytes

// Array which holds the frame bits
// WARNING: LSB shifted out first!!!!!
uint8_t abRts_FrameArray[bRts_MaxFrameSize_c] = {
		0x01, 0xe1, 0xe1, 0xe1, 0xe1, 0xe1, 0xe1, 0xe1, // hardware sync
		0xfe, // software sync
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // payload
};

/**
 * Manchester encode data
 *
 * @param enc_data	pointer to the next byte to write in the transmit data
 *			buffer
 * @param b		Byte to encode
 *
 * @returns	Updated pointer to the next byte to write in the transmit data
 *		buffer
 */
static uint8_t *encode_rts(uint8_t *enc_data, uint8_t b) 
{
	uint8_t i;

	i=8;
	while (i > 0) {
		*enc_data = (*enc_data << 2);
		if (b & 0x80) {
			*enc_data |= 0x01;
		} else {
			*enc_data |= 0x02;
		}
		b <<= 1;

		if (i == 5) {
			enc_data++;
		}
		i--;
	}

	return (enc_data+1);
}

int sx1231_rts_init(rf_dev_t *sdev)
{
	return rf_config(sdev, 433.46, 0, SX1231_MODULATION_OOK, RTS_BITRATE);
}

int sx1231_rts_send(rf_dev_t *sdev, uint8_t data[7], bool long_press)
{
	int ret;
  	uint8_t *pbFrameHead;		// Pointer to frame Head 
	int frame_cnt;
	int i;

	if (long_press) {
		frame_cnt = 200;
	} else {
		frame_cnt = 4;
	}

	pbFrameHead = &abRts_FrameArray[bRts_PreambleSize_c];
	for (i = 0; i < 7; i++) {
		pbFrameHead = encode_rts(pbFrameHead, data[i]);
	}

	struct timespec ifg_time = {
		0, RTS_INTER_FRAME_GAP_US * 1000
	};
	for (i = 0; i < frame_cnt; i++) {
		ret =  rf_send(sdev, abRts_FrameArray, bRts_MaxFrameSize_c);
		if (ret != ERR_OK) {
			return ret;
		}

		// TODO: handle errors/interuptions...
		nanosleep(&ifg_time, NULL);
	}

	return ERR_OK;
}
