/* FTDI I2C monitor
 * Copyright (c) 2004 Nicolas Boichat
 * Copyright (c) 2011 Stany MARCEL
 *
 * Based on Nicolas Boichat work (http://www.boichat.ch/nicolas/ddcci/)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <stdio.h>
#include <unistd.h>
#include <ftdi.h>
#include <sys/time.h>

#define SIZE 512*10000

#define FTDI_CHECK(FUN, MSG, CTX) do {				\
		if ((FUN) < 0)					\
		{						\
			fprintf(stderr,				\
				"%s: %s\n",			\
				MSG,				\
				ftdi_get_error_string(&CTX));	\
			return -1;				\
		}                                               \
	} while (0)

int main(void)
{
	struct timeval lasttv;
	struct timeval newtv;
	struct ftdi_context    fc;
	unsigned char buf[SIZE + 1];
	int i, j;
	int old = -1;
	int c = 0;
	int sda = -1;
	int scl = -1;
	int nsda, nscl;
	int bitindex = 0;
	int byteindex = 0;
	int data = 0;
	int len;
	double us, usb;

	if (ftdi_init(&fc) < 0)
	{
		fprintf(stderr, "ftdi_init failed\n");
		return 1;
	}
	FTDI_CHECK(ftdi_usb_open(&fc, 0x0403, 0x6011), "USB OPEN", fc);
	FTDI_CHECK(ftdi_read_data_set_chunksize(&fc, 512), "SET CHUNK 512", fc);
	FTDI_CHECK(ftdi_set_bitmode(&fc, 0, BITMODE_BITBANG), "SET BITBANG MODE", fc);

	gettimeofday(&lasttv, NULL);
	for (i = 0; i < SIZE; ) {
		len = ((SIZE - i) < 512) ? SIZE - i : 512;
		FTDI_CHECK(len = ftdi_read_data(&fc, buf + i, len), "READ", fc); i += len;
	}
	gettimeofday(&newtv, NULL);
	us = (double)(newtv.tv_usec - lasttv.tv_usec) + (double)((newtv.tv_sec - lasttv.tv_sec)*1000000);
	usb = us/(SIZE);

        ftdi_usb_close(&fc);
        ftdi_deinit(&fc);

	buf[SIZE] = 0x00;

	j = 0;

	for (i = 0; i <= SIZE; i++) {
		if (old != buf[i]) {
			if (old != -1) {
				nsda = (old & 0x1) >> 0;
				nscl = (old & 0x2) >> 1;
				if ((scl == 1) && (nscl == 1)) { /* SCL high, SDA changing => start or stop */
					if ((sda == 1) && (nsda == 0)) {
						byteindex = 0;
						bitindex = 0;
						data = 0;
					}
					else if ((sda == 0) && (nsda == 1)) {
						printf("};\n");
					}
				}
				else if ((scl == 0) && (nscl == 1)) { /* SCL getting high => bit or ack received */
					if (bitindex == 8) { //ACK
						if (byteindex == 0) {
							printf("%.9f: @%02x %s: { ", usb*c / 1000000, data >> 1, ((data & 0x01)==0) ? "W": "R");
						}
						else {
							printf("0x%02x, ", data);

						}
						bitindex = 0;
						data = 0;
						byteindex++;
						j++;
						c = 0;
						/* if (nsda != 0) { */
						/* 	printf("!ACK "); */
						/* } */
					}
					else {
						data = (data << 1) | nsda;
						bitindex++;
					}
				}
				sda = nsda;
				scl = nscl;
			}
			old = buf[i];
		}
		c++;
	}

	return 0;
}
