/**
 * spi.c - SPI helper functions
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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include "spi.h"
#include "sx1231_ods_error.h"
#include "sx1231_ods_debug.h"

/**
 * Execute a SPI transfer
 *
 * @param fd		File descriptor of SPI device
 * @param do_write	If True, perform a write operation. Else read.
 * @param addr		Register address at which to start operation
 * @param data		Buffer containing data to write or to store read data
 * @param len		Amount of bytes to read/write
 *
 * @returns	0 on success
 */
static int _spi_transfer(int fd, bool do_write, uint8_t addr,
				uint8_t *data, size_t len);

static int _spi_transfer(int fd, bool do_write, uint8_t addr,
				uint8_t *data, size_t len)
{
	struct spi_ioc_transfer	xfer[2];
	int err;

	if (addr & 0x80) {
		return ERR_INVAL;
	}

	memset(xfer, 0, sizeof(xfer));

	if (do_write) {
		addr |= 0x80;
	}

	// Send (rw // addr)
	xfer[0].tx_buf = (unsigned long) &addr;
	xfer[0].len = 1;

	// Read data
	if (do_write) {
		xfer[1].tx_buf = (unsigned long) data;
	} else {
		xfer[1].rx_buf = (unsigned long) data;
	}
	xfer[1].len = len;

	err = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);
	if (err < 0) {
		perror("SPI_IOC_MESSAGE");
		return ERR_SPI_IOCTL;
	}

	DBG_PRINTF(DBG_LVL_EXTREEM, "SPI %s @ 0x%02x:\n", do_write ? "WRITE" : "READ", addr & 0x7f);
	DBG_HEXDUMP(DBG_LVL_EXTREEM, data, len);

	return ERR_OK;
}

int spi_read_reg(int fd, uint8_t addr, uint8_t *data)
{
	return _spi_transfer(fd, false, addr, data, 1);
}

int spi_read_regs(int fd, uint8_t addr, uint8_t *data, size_t len)
{
	return _spi_transfer(fd, false, addr, data, len);
}

int spi_write_reg(int fd, uint8_t addr, uint8_t data)
{
	return _spi_transfer(fd, true, addr, &data, 1);
}

int spi_write_regs(int fd, uint8_t addr,
		      const uint8_t *data, size_t len)
{
	return _spi_transfer(fd, true, addr, (uint8_t *) data, len);
}
