/**
 * spi.h - SPI helper functions
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
#ifndef __SPI_H__
#define __SPI_H__

/**
 * Read a single byte from SPI device
 *
 * @param fd	File descriptor of SPI device
 * @param addr	Address to read
 * @param data	Pointer to location to store read value
 *
 * @returns	0 on success
 */
int spi_read_reg(int fd, uint8_t addr, uint8_t *data);

/**
 * Burst read multiple bytes from SPI device
 *
 * @param fd	File descriptor of SPI device
 * @param addr	Address to start burst read
 * @param data	Pointer to buffer to store read data
 * @param len	Amount of bytes to read
 *
 * @returns	0 on success
 */
int spi_read_regs(int fd, uint8_t addr, uint8_t *data, size_t len);

/**
 * Write a single byte to SPI device
 *
 * @param fd	File descriptor of SPI device
 * @param addr	Target address to write
 * @param data	value to write
 *
 * @returns	0 on success
 */
int spi_write_reg(int fd, uint8_t addr, uint8_t data);

/**
 * Burst write multiple bytes to SPI device
 *
 * @param fd	File descriptor of SPI device
 * @param addr	Address to start burst write
 * @param data	Pointer to buffer containing data to write
 * @param len	Amount of bytes to write
 *
 * @returns	0 on success
 */
int spi_write_regs(int fd, uint8_t addr, const uint8_t *data, size_t len);

#endif // __SPI_H__
