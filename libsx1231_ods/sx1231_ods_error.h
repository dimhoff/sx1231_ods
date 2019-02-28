/**
 * error.h - Error codes and error handeling helpers
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
#ifndef __ERROR_H__
#define __ERROR_H__

/************************* Helper Macros ************************************/
/**
 * Execute function and check return value
 *
 * Executes function X, and checks if it returned an error code. If an error
 * code is returned jump to label 'fail'. A variable err must be defined in
 * which the error code will be placed.
 */
#define TRY(X) \
	do { \
		err = X; \
		if (err != 0) goto fail;\
	} while(0)

/**
 * Prevent statement from changing errno
 *
 * Execute statement X, but restore errno to its previous value after
 * executing.
 */
#define SAVE_ERRNO(X) \
	do { \
		int tmp_errno = errno; \
		X; \
		errno = tmp_errno; \
	} while(0)


/************************* Error Macros *************************************/
#define ERROR_ERRNO_VALID(x) ((x & 0x40000000UL) != 0)
#define ERROR_CLASS(x) ((x & 0x0fff0000UL) >> 16)
#define ERROR_CODE(x) (x & 0xffffUL)

#define E(CLASS, CODE, FLAGS) ( \
		((FLAGS & 0x7) << 28) |  \
		((CLASS & 0xffff) << 16) | \
		(CODE & 0xffffUL) \
	)

// NOTE: Highest bit is reserved to allow error code to be returned as negative value

/************************* Error Flags **************************************/
#define ERR_FLAG_ERRNO_SET	0x1

/************************* Error Classes ************************************/
#define ERR_CLASS_GENERIC	0x0000
#define ERR_CLASS_SPI		0x0001
#define ERR_CLASS_RFM		0x0002


/************************* Error Codes **************************************/
#define ERR_OK			0

// Generic error's
#define ERR_UNSPEC		E(ERR_CLASS_GENERIC, 0x0001, 0)
#define ERR_INVAL		E(ERR_CLASS_GENERIC, 0x0002, 0)
#define ERR_RANGE		E(ERR_CLASS_GENERIC, 0x0003, 0)

// SPI errors
#define ERR_SPI_OPEN_DEV	E(ERR_CLASS_SPI, 0x0001, ERR_FLAG_ERRNO_SET)
#define ERR_SPI_IOCTL		E(ERR_CLASS_SPI, 0x0002, ERR_FLAG_ERRNO_SET)

// Class RF
#define ERR_RFM_CHIP_VERSION	E(ERR_CLASS_RFM, 0x0001, 0)
#define ERR_RFM_TX_OUT_OF_SYNC	E(ERR_CLASS_RFM, 0x0002, 0)

#endif // __ERROR_H__
