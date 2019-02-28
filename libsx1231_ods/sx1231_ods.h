/**
 * sx1231_ods.h - SX1231 Output Data Serializer Library Header File
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
#ifndef __SX1231_H__
#define __SX1231_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "sx1231_ods_debug.h"
#include "sx1231_ods_error.h"

typedef struct {
	int fd;
	uint8_t fifo_thresh; /**< FifoLevel interrupt threshold */
} rf_dev_t;

int rf_open(rf_dev_t *dev, const char *spi_path);

void rf_close(rf_dev_t *dev);

enum {
	SX1231_MODULATION_FSK = 0,
	SX1231_MODULATION_OOK = 1
};

int rf_config(rf_dev_t *dev,
		float freq_mhz, float fdev_khz,
		int modulation, double data_rate_kbps);

int rf_set_pa(rf_dev_t *dev, uint8_t level, bool pa1_on);

int rf_send(rf_dev_t *dev, const uint8_t *data, size_t len);

#endif // __SX1231_H__
