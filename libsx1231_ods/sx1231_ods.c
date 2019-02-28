/**
 * sx1231.c - sx1231 interface functions
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "sx1231_enums.h"
#include "sx1231_ods.h"
#include "spi.h"

#define SX1231_FIFO_SIZE 66

#ifndef SX1231_FXOSC
# define SX1231_FXOSC (32.0 * 1e6)
#endif

#define SX1231_FSTEP (SX1231_FXOSC / 0x80000)

unsigned int debug_level = 0;

static int _reset(rf_dev_t *dev);
static int _sync_config(rf_dev_t *dev);
static int _switch_mode(rf_dev_t *dev, int mode);
static void _dump_status(rf_dev_t *dev);

int rf_open(rf_dev_t *dev, const char *spi_path)
{
	int err = ERR_UNSPEC;
	int fd;
	uint8_t val;

	fd = open(spi_path, O_RDWR);
	if (fd == -1) {
		return ERR_SPI_OPEN_DEV;
	}

	dev->fd = fd;

	// Check device version
	TRY(spi_read_reg(dev->fd, RegVersion, &val));
	if ((val & SX1231_VERSION_MASK) != SX1231_VERSION) {
		err = ERR_RFM_CHIP_VERSION;
		goto fail;
	}

	// Read config
	TRY(_sync_config(dev));

	return ERR_OK;
fail:
	SAVE_ERRNO(rf_close(dev));
	return err;
}

void rf_close(rf_dev_t *dev)
{
	close(dev->fd);
	dev->fd = -1;
}

int rf_config(rf_dev_t *dev,
		float freq_mhz, float fdev_khz,
		int modulation, double data_rate_kbps)
{
	int err = ERR_UNSPEC;

	// reset
	TRY(_reset(dev));

	// Program frequency configuration
	// TODO: range validation...
	uint32_t reg_freq = ((freq_mhz * 1e6) / SX1231_FSTEP);
	uint16_t reg_fdev = ((fdev_khz * 1e3) / SX1231_FSTEP);
	uint16_t reg_bitrate = SX1231_FXOSC / (data_rate_kbps * 1000);
	uint8_t buf[7];

	buf[0] = reg_bitrate >> 8;
	buf[1] = reg_bitrate;
	buf[2] = (reg_fdev >> 8) & 0x3f;
	buf[3] = reg_fdev;
	buf[4] = reg_freq >> 16;
	buf[5] = reg_freq >> 8;
	buf[6] = reg_freq;
	TRY(spi_write_regs(dev->fd, RegBitrateMsb, buf, 2 + 2 + 3));

	// Set modulation
	if (modulation == SX1231_MODULATION_OOK) {
		TRY(spi_write_reg(dev->fd, RegDataModul, 0x08));
	} else {
		TRY(spi_write_reg(dev->fd, RegDataModul, 0x00));
	}

	// Disable CLKOUT
	TRY(spi_write_reg(dev->fd, RegDioMapping2, 0x07));

	// Disable Preamble
	TRY(spi_write_reg(dev->fd, RegPreambleMsb, 0x00));
	TRY(spi_write_reg(dev->fd, RegPreambleLsb, 0x00));

	// Disable sync word
	TRY(spi_write_reg(dev->fd, RegSyncConfig, 0x18));

	// Set to unlimited packet mode
	// crcOn=false, dcFree=none, AddrFilt=none
	TRY(spi_write_reg(dev->fd, RegPacketConfig1, 0x00));
	TRY(spi_write_reg(dev->fd, RegPayloadLength, 0x00));

	// Start TX if FifoNotEmpty
	TRY(spi_write_reg(dev->fd, RegFifoThresh, 0x8f));

	// Configure PA
#ifdef WITH_PA1_DEFAULT
	TRY(rf_set_pa(dev, 0x1f, true));
#else
	TRY(rf_set_pa(dev, 0x1f, false));
#endif

	// TODO: generic: modulation shaping
	// TODO: transmitter: paRamp, over-current(OCP)
	// TODO: test: TcxoInputOn


	// Switch to standby mode
	TRY(_switch_mode(dev, OP_MODE_MODE_STDBY));

	err = ERR_OK;
fail:
	return err;
}

int rf_set_pa(rf_dev_t *dev, uint8_t level, bool pa1_on)
{
	uint8_t val = 0;
	if (pa1_on) {
		val |= 0x40;
		if (level > 0x1f) {
			val |= 0x20;
			level -= 4;
		}
	} else {
		val |= 0x80;
	}

	if (level > 0x1f) {
		return ERR_INVAL;
	}

	val |= level;

	return spi_write_reg(dev->fd, RegPaLevel, val);
}

int rf_send(rf_dev_t *dev, const uint8_t *data, size_t len)
{
	int err = ERR_UNSPEC;
	uint8_t send_len;
	uint8_t val;

	// Prefill Fifo
	send_len = (len <= SX1231_FIFO_SIZE) ? len : SX1231_FIFO_SIZE;
	TRY(spi_write_regs(dev->fd, RegFifo, data, send_len));
	data += send_len;
	len -= send_len;

	// Start TX
	TRY(_switch_mode(dev, OP_MODE_MODE_TX));

	while (len != 0) {
		// Wait till space in FIFO
		do {
			TRY(spi_read_reg(dev->fd, RegIrqFlags2, &val));
		} while (val & IRQ_FLAGS2_FIFOLEVEL);

		// Refill Fifo
		send_len = (len <= SX1231_FIFO_SIZE - dev->fifo_thresh) ? len : (SX1231_FIFO_SIZE - dev->fifo_thresh);
		TRY(spi_write_regs(dev->fd, RegFifo, data, send_len));
		data += send_len;
		len -= send_len;
	}

	// Wait till done
	do {
		TRY(spi_read_reg(dev->fd, RegIrqFlags2, &val));
	} while (! (val & IRQ_FLAGS2_PACKETSENT));

	TRY(_switch_mode(dev, OP_MODE_MODE_STDBY));

	return ERR_OK;
fail:
	return err;
}

static int _reset(rf_dev_t *dev)
{
	/* TODO:
	int err = ERR_UNSPEC;
	uint8_t val;
	*/

	return ERR_OK;
}

static int _sync_config(rf_dev_t *dev)
{
	int err = ERR_UNSPEC;

	TRY(spi_read_reg(dev->fd, RegFifoThresh, &dev->fifo_thresh));
	dev->fifo_thresh &= 0x7f;

	return ERR_OK;
fail:
	return err;
}

static int _switch_mode(rf_dev_t *dev, int mode)
{
	int err = ERR_UNSPEC;
	uint8_t val;

	assert((mode & ~0x1c) == 0);

	TRY(spi_write_reg(dev->fd, RegOpMode, mode));

	do {
		// TODO: add timeout
		TRY(spi_read_reg(dev->fd, RegIrqFlags1, &val));
	} while (! (val & IRQ_FLAGS1_MODEREADY));

	return ERR_OK;
fail:
	return err;
}

static void _dump_status(rf_dev_t *dev)
{
	uint8_t buf[2];

	spi_read_regs(dev->fd, RegIrqFlags1, buf, 2);
	printf("Interrupt Flags: %.2x %.2x\n", buf[0], buf[1]);
}

