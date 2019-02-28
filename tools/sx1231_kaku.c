/**
 * sx1231_kaku.c - Klik Aan-Klik Uit implementation for SX1231 RF Transmitter
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
#include "version.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>

#include "sx1231_ods.h"


#define ENCODED_BITS_PER_IVAL   (8)		// Use 8 bits to encode 1 basic interval T
#define ENCODED_BITRATE         (1000.0 * ENCODED_BITS_PER_IVAL * 1.0 / 300.0)
						// One basic interval T=~300, use 8 encoded
						// bits per interval, value in kbit/s

#define KAKU_DATA_SYMBOLS_PER_FRAME  (32)	// 32 data bits per frame
#define KAKU_IVALS_PER_SYMBOL   (7)		// Amount of basic intervals, T, per data bit
#define KAKU_FRAME_SIZE         (4 * 8 * KAKU_T_PER_SYMBOL)
						// Amount of data bits per KaKu frame

#define KAKU_PREAMBLE_IVALS     (9)		// Amount of basic intervals preamble
#define KAKU_DATA_IVALS         (KAKU_IVALS_PER_SYMBOL * KAKU_DATA_SYMBOLS_PER_FRAME)
						// Amount of basic intervals preamble
#define KAKU_END_IVALS          (1)		// Amount of basic intervals for stop bit

#define KAKU_FRAME_IVALS        (KAKU_PREAMBLE_IVALS + KAKU_DATA_IVALS + KAKU_END_IVALS)
						// Amount of basic intervals for full frame

#define KAKU_FRAME_REPEAT       (4)		// Number of times frame is repeated
#define KAKU_INTER_FRAME_GAP_US (7700)		// Time in us between frame repeats



/**
 * Perform KAKU PWM encoding on data
 *
 * Takes one byte and encodes it using the KAKU PWM encoding. One bit is
 * encoded into 7 byte. A byte should be transmitted in 275 us.
 *
 * @param enc_data	pointer to the next byte to write in the transmit data
 *			buffer
 * @param b		Byte to encode
 *
 * @returns	Updated pointer to the next byte to write in the transmit data
 *		buffer
 */
static uint8_t *encode_kaku(uint8_t *enc_data, uint8_t b) 
{
	int i;

	i=8;
	while (i > 0) {
		if (b & 0x80) {
			enc_data[0] = 0xff;
			enc_data[1] = 0x00;
			enc_data[2] = 0x00;
			enc_data[3] = 0x00;
			enc_data[4] = 0x00;
			enc_data[5] = 0xff;
			enc_data[6] = 0x00;
		} else {
			enc_data[0] = 0xff;
			enc_data[1] = 0x00;
			enc_data[2] = 0xff;
			enc_data[3] = 0x00;
			enc_data[4] = 0x00;
			enc_data[5] = 0x00;
			enc_data[6] = 0x00;
		}
		b <<= 1;

		enc_data += 7;
		i--;
	}

	return enc_data;
}

/**
 * Send a frame using KAKU
 *
 * This function encodes the data in 'payload' according to the KAKU protocol,
 * pre-/appends the preamble/Inter-frame gap, and sends out the frame 4 times
 * using OOK modulation on 433.9 MHz.
 *
 * @param dev		Device handle
 * @param data		The 4-bytes frame data
 */
int kaku_send(rf_dev_t *dev, uint8_t data[4])
{
	int ret;
	uint8_t frame_buf[KAKU_FRAME_IVALS];
  	uint8_t *frame_head = frame_buf;

	memset(frame_buf, 0, sizeof(frame_buf));

	// Preamble
	*frame_head = 0xff;
	frame_head += KAKU_PREAMBLE_IVALS;

	// Data
	frame_head = encode_kaku(frame_head, data[0]);
	frame_head = encode_kaku(frame_head, data[1]);
	frame_head = encode_kaku(frame_head, data[2]);
	frame_head = encode_kaku(frame_head, data[3]);

	// Stop bit
	*frame_head = 0xff;

	for (int i=0; i < KAKU_FRAME_REPEAT; i++) {
		ret = rf_send(dev, frame_buf, sizeof(frame_buf));
		if (ret != ERR_OK) {
			return ret;
		}

		// Inter Frame Gap
		struct timespec ifg_time = {
			0, KAKU_INTER_FRAME_GAP_US * 1000
		};
		nanosleep(&ifg_time, NULL);
	}

	return ERR_OK;
}

void usage(const char *name)
{
	fprintf(stderr,
			//TODO: description + VERSION
		"usage: %s [options] <address> <unit> <on|off>\n"
		"\n"
		"Options:\n"
		" -d <path>	Path to serial device file\n"
		" -h		Print this help message\n"
		"\n"
		"Arguments:\n"
		"address: The hexadecimal address of the remote\n"
		"unit: The unit number(0-15) of a multi channel remote\n"
		"on|off: the action to perform\n"
		, name);
}

int main(int argc, char *argv[])
{
	int opt;
	const char *dev_path = DEFAULT_DEV_PATH;
	rf_dev_t dev;
	unsigned char kaku_data[4];
	enum { ButtonOn, ButtonOff } button;
	char *tmp;
	uint32_t addr;
	int unit;
	int ret;

	while ((opt = getopt(argc, argv, "d:h")) != -1) {
		switch (opt) {
		case 'd':
			dev_path = optarg;
			break;
		case 'h':
			usage(argv[0]);
			exit(EXIT_SUCCESS);
			break;
		default: /* '?' */
			usage(argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	
	if (argc - optind != 3) {
		fprintf(stderr, "Incorrect amount of arguments\n");
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	addr = strtol(argv[optind], &tmp, 16);
	if (*tmp != '\0') {
		fprintf(stderr, "Unparsable characters in address argument\n");
		exit(EXIT_FAILURE);
	}
	optind++;

	unit = strtol(argv[optind], &tmp, 0);
	if (*tmp != '\0') {
		fprintf(stderr, "Unparsable characters in unit argument\n");
		exit(EXIT_FAILURE);
	}
	if (unit > 0xf || unit < 0) {
		fprintf(stderr, "Unit number out of range(0-15)\n");
		exit(EXIT_FAILURE);
	}
	optind++;

	if (strcmp(argv[optind], "on") == 0) {
		button = ButtonOn;
	} else if (strcmp(argv[optind], "off") == 0) {
		button = ButtonOff;
	} else {
		fprintf(stderr, "Unknown direction argument\n");
		exit(EXIT_FAILURE);
	}

	// Open SX1231
	if (rf_open(&dev, dev_path) != 0) {
		exit(EXIT_FAILURE);
	}

	ret = rf_config(&dev, 433.92, 0, SX1231_MODULATION_OOK, ENCODED_BITRATE);
	if (ret != ERR_OK) {
		fprintf(stderr, "ERROR: Failed configuring module: %d\n", ret);
		rf_close(&dev);
		exit(EXIT_FAILURE);
	}

	// encode KAKU frame data
	kaku_data[0] = addr >> 18;
	kaku_data[1] = addr >> 10;
	kaku_data[2] = addr >> 2;
	kaku_data[3] = (addr << 6) & 0xc0;
	if (button == ButtonOn) {
		kaku_data[3] |= 0x10;
	} else {
		kaku_data[3] &= ~0x10;
	}
	kaku_data[3] = (kaku_data[3] & 0xF0) | (unit & 0x0F);

	// send frame
	ret = kaku_send(&dev, kaku_data);

	rf_close(&dev);

	if (ret != ERR_OK) {
		fprintf(stderr, "ERROR: Failed sending command: %d\n", ret);
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
