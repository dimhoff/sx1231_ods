/**
 * debug.h - Debug logging functions
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
#ifndef __DEBUG_H__
#define __DEBUG_H__

extern unsigned int debug_level;

#define DBG_LVL_LOW 	0
#define DBG_LVL_MID	1
#define DBG_LVL_HIGH 	2
#define DBG_LVL_EXTREEM 3

#define DBG_EXEC(LVL, X) \
	do { \
		if (debug_level > LVL) \
			X; \
	} while(0)


#define DBG_PRINTF(LVL, ...) \
	do { \
		if (debug_level > LVL) \
			printf(__VA_ARGS__); \
	} while(0)

#define DBG_HEXDUMP(LVL, BUF, LEN) \
	do { \
		if (debug_level > LVL) { \
			size_t dbg_i; \
			for (dbg_i = 0; dbg_i < LEN; dbg_i++) { \
				printf("%02x ", (BUF)[dbg_i]); \
			} \
			putchar('\n'); \
		} \
	} while(0)

#endif // __DEBUG_H__
