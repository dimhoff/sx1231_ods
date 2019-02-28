/**
 * sx1231_raw.c - Command line utility to use libsx1231_ods
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
#include <strings.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include <errno.h>

#include <sx1231_ods.h>

#include "dehexify.h"

#define MAX_DATA_LEN (1024 * 1024)

/**
 * Reverse bit order in a byte
 */
static inline uint8_t reverse_byte(uint8_t b)
{
	b = ((b & 0xaa) >> 1) | ((b & 0x55) << 1);
	b = ((b & 0xcc) >> 2) | ((b & 0x33) << 2);
	b = ((b & 0xf0) >> 4) | ((b & 0x0f) << 4);

	return b;
}

void usage(const char *name)
{
	fprintf(stderr,
		"SX1231 Output Data Serializer - " VERSION "\n"
		"\n"
		"usage: %s [options]\n"
		"\n"
		"Options:\n"
		"  -d, --device=PATH         SPI device file to use (default: " DEFAULT_DEV_PATH ")\n"
		"  -f, --frequency=FREQ      Carrier frequency in MHz (default: 433.92 MHz)\n"
		"  -m, --modulation=MOD      Modulation scheme: OOK or FSK (default: OOK)\n"
		"  --fsk-deviation=FDEV      FSK frequency deviation in kHz (default: 5 kHz)\n"
		"                            Value should be in the range 1-130. Note that the\n"
		"                            actual max. deviation is clipped at about 135 ppm\n"
		"                            of the carrier frequency.\n"
		"  -r, --bit-rate=RATE       Bit rate in kbit/s (default: 9.6)\n"
		"  -p, --power=LEVEL         Set output power. Pout = -18 + LEVEL.\n"
		"                            0 < LEVEL < (31 (PA0) or 35 (PA1&PA2)).\n"
		"  --select-pa=(0|1)         Select power amplifier to use: 0=PA0 or 1=PA1&PA2\n"
#ifdef WITH_PA1_DEFAULT
		"                            Default: PA1&PA2\n"
#else
		"                            Default: PA0\n"
#endif
		"  --lsb-first               Send bytes LSB first\n"
		" -v                         Increase verbosity level, use multiple times\n"
		"                            for more logging\n"
		"  -h, --help                Print this help message\n"
		"\n"
		"Data is read from STDIN as a hexadecimal string of the bytes to send. Every line\n"
		"of input is send seperately.\n"
		, name);
}

int main(int argc, char *argv[])
{
	const char *dev_path = DEFAULT_DEV_PATH;
	rf_dev_t dev;

	float freq = 433.92;
	float fdev = 5;
	int modulation = SX1231_MODULATION_OOK;
	float bit_rate = 4.8;
#ifdef WITH_PA1_DEFAULT
	bool use_pa1 = true;
#else
	bool use_pa1 = false;
#endif
	uint8_t pa_level = 0x1f;
	uint8_t *data;
	int data_len;
	bool lsb_first = false;

	int ret;
	int retval = EXIT_SUCCESS;

	// Option parsing
	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			{ "device",            required_argument,  0, 'd' },
			{ "frequency",         required_argument,  0, 'f' },
			{ "modulation",        required_argument,  0, 'm' },
			{ "fsk-deviation",     required_argument,  0,  0  },
			{ "bit-rate",          required_argument,  0, 'r' },
			{ "power",             required_argument,  0, 'p' },
			{ "select-pa",         required_argument,  0,  0  },
			{ "lsb-first",         no_argument,        0,  0  },
			{ "help",              no_argument,        0, 'h' },
			{ 0, 0, 0, 0 }
		};
		int c;
		char *endp;

		c = getopt_long(argc, argv, "d:f:m:p:r:vh",
				long_options, &option_index);
		if (c == -1)
			break;

		if (c == 0) {
			const char *optname = long_options[option_index].name;

			if (strcmp(optname, "fsk-deviation") == 0) {
				fdev = strtof(optarg, &endp);
				if (endp == NULL || *endp != '\0') {
					fprintf(stderr, "Frequency deviation"
						"not a valid number\n");
					exit(EXIT_FAILURE);
				}
				if (fdev < 1 || fdev > 130) {
					fprintf(stderr,
						"Frequency deviation out of "
						"range (1 < fdev < 130)\n");
					exit(EXIT_FAILURE);
				}
			} else if (strcmp(optname, "select-pa") == 0) {
				if (strlen(optarg) != 1) {
					fprintf(stderr, "select-pa argument "
							"must be '0' or '1'\n");
					exit(EXIT_FAILURE);
				}
				if (optarg[0] == '0') {
					use_pa1 = false;
				} else if (optarg[0] == '1') {
					use_pa1 = true;
				} else {
					fprintf(stderr, "select-pa argument "
							"must be '0' or '1'\n");
					exit(EXIT_FAILURE);
				}
			} else if (strcmp(optname, "lsb-first") == 0) {
				lsb_first = true;
			}
		} else {
			switch (c) {
			case 'd':
				dev_path = optarg;
				break;
			case 'f':
				freq = strtof(optarg, &endp);
				if (endp == NULL || *endp != '\0') {
					fprintf(stderr, "Carrier frequency "
						"not a valid number\n");
					exit(EXIT_FAILURE);
				}
				if (freq > 960 || freq < 240) {
					fprintf(stderr,
						"Carrier frequency out of "
						"range (240 < freq < 960)\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 'm':
				if (strcasecmp(optarg, "OOK") == 0) {
					modulation = SX1231_MODULATION_OOK;
				} else if (strcasecmp(optarg, "FSK") == 0) {
					modulation = SX1231_MODULATION_FSK;
				} else {
					fprintf(stderr, "Invalid modulation "
						"type\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 'p':
				ret = strtol(optarg, &endp, 0);
				if (endp == NULL || *endp != '\0') {
					fprintf(stderr, "PA Level "
						"not a valid number\n");
					exit(EXIT_FAILURE);
				}
				if (ret < 0 || ret > (0x1f + 4)) {
					fprintf(stderr,
						"PA Level out of "
						"range (0 < pa_level < 35)\n");
					exit(EXIT_FAILURE);
				}
				pa_level = ret;
				break;
			case 'r':
				bit_rate = strtof(optarg, &endp);
				if (endp == NULL || *endp != '\0') {
					fprintf(stderr, "Bit rate "
						"not a valid number\n");
					exit(EXIT_FAILURE);
				}
				if (bit_rate < 0.123 || bit_rate > 50) {
					fprintf(stderr,
						"Bit rate out of "
						"range (0.123 < freq < 50)\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 'v':
				debug_level++;
				break;
			case 'h':
				usage(argv[0]);
				exit(EXIT_SUCCESS);
				break;
			case '?':
				fprintf(stderr, "Unknown option: %c\n", optopt);
				usage(argv[0]);
				exit(EXIT_FAILURE);
				break;
			default:
				fprintf(stderr, "illegal option %c\n", c);
				usage(argv[0]);
				exit(EXIT_FAILURE);
				break;
			}
		}
	}

	// Argument parsing
	if (argc - optind != 0) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	// Open device
	if (rf_open(&dev, dev_path) != 0) {
		fprintf(stderr, "Failed to open device\n");
		exit(EXIT_FAILURE);
	}

	// Configure device
	ret = rf_config(&dev, freq, fdev, modulation, bit_rate);
	if (ret != ERR_OK) {
		fprintf(stderr, "Failed configuring module: %d\n", ret);
		rf_close(&dev);
		exit(EXIT_FAILURE);
	}

	ret = rf_set_pa(&dev, pa_level, use_pa1);
	if (ret != ERR_OK) {
		fprintf(stderr, "Failed configure PA: %d\n", ret);
		rf_close(&dev);
		exit(EXIT_FAILURE);
	}

	char *inp_data = NULL;
	size_t inp_data_alloc_len = 0;
	size_t inp_data_len = 0;
	while (!feof(stdin)) {
		if (inp_data_alloc_len != 0) {
			free(inp_data);
			inp_data = NULL;
			inp_data_alloc_len = 0;
		}

		// Read input data from input
		ssize_t rlen;
		
		rlen = getline(&inp_data, &inp_data_alloc_len, stdin);
		if (rlen < 0) {
			if (errno != 0) {
				perror("ERROR: Failed to read from standard input");
				retval = EXIT_FAILURE;
			}
			break;
		} else if (rlen == 0 || rlen == 1) {
			continue;
		}
		inp_data_len = rlen - 1;
		if (inp_data[inp_data_len] == '\n') {
			inp_data[inp_data_len] = '\0';
		}

		// Check input
		if (inp_data_len & 1) {
			fprintf(stderr, "ERROR: Data must consist of a even amount of bytes\n");
			continue;
		}
		data_len = inp_data_len / 2;
		if (data_len > MAX_DATA_LEN) {
			fprintf(stderr, "ERROR: Data can not be longer than %u bytes\n", MAX_DATA_LEN);
			continue;
		}

		// Dehexify input data
		data = (uint8_t *) calloc(data_len, sizeof(uint8_t));
		if (data == NULL) {
			fprintf(stderr, "ERROR: Unable to allocate data memory\n");
			free(inp_data);
			retval = EXIT_FAILURE;
			break;
		}
		if (dehexify(inp_data, data_len, data) != 0) {
			fprintf(stderr, "ERROR: Unable to dehexify data\n");
			free(data);
			continue;
		}

		// Reverse bytes if LSB first
		if (lsb_first) {
			for (int i=0; i < data_len; i++) {
				data[i] = reverse_byte(data[i]);
			}
		}

		// Send bits
		ret = rf_send(&dev, data, data_len);

		free(data);

		if (ret == ERR_OK) {
			fprintf(stderr, "OK\n");
		} else {
			fprintf(stderr, "ERROR: Failed sending command: %d\n", ret);
		}
	}

	if (inp_data_alloc_len != 0) {
		free(inp_data);
		inp_data = NULL;
		inp_data_alloc_len = 0;
	}
	rf_close(&dev);
	return retval;
}
