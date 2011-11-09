/*
 * a3dcontrol: Acer Integrated 3D vision emitter activator.
 * Copyright (C) 2011 Stany MARCEL <stanypub@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <time.h>
#include <ctype.h>

#define VERSION		"0.2"
#define COPYRIGHT	"Copyright (C) 2011 Stany MARCEL <stanypub@gmail.com>"

char acmd1[] = { 0x51, 0x81, 0x59, 0xe7 };
char acmd2[] = { 0x51, 0x84, 0x03, 0xe0, 0x00, 0x01, 0x59 };
char acmd3[] = { 0x51, 0x84, 0x03, 0xe1, 0x00, 0x01, 0x58 };
char acmd4[] = { 0x51, 0x82, 0x01, 0xe1, 0x5d };

char aack1[] = { 0x82, 0x58, 0x01, 0xe5 };
char aack2[] = { 0x88, 0x02, 0x00, 0xe1, 0x01, 0x00, 0x01, 0x00, 0x01,
		0x54 };

char dcmd1[] = { 0x51, 0x84, 0x03, 0xe1, 0x00, 0x00, 0x59 };
char dcmd2[] = { 0x51, 0x82, 0x01, 0xe1, 0x5d };
char dack1[] = { 0x88, 0x02, 0x00, 0xe1, 0x01, 0x00, 0x01, 0x00, 0x00,
		0x55 };

char buf[256];

char zero = 0;
char eighty = 0x80;

/* Activation i2c messages */
struct i2c_msg msg_aw1[] = {
	{
		.addr  = 0x37,
		.flags = 0,
		.len   = sizeof(acmd1),
		.buf   = acmd1,
	},
};

struct i2c_msg msg_ar1[] = {
	{
		.addr  = 0x37,
		.flags = I2C_M_RD,
		.len   = sizeof(aack1) + 1,
		.buf   = buf,
	},
};

struct i2c_msg msg_aw2[] = {
	{
		.addr  = 0x37,
		.flags = 0,
		.len   = sizeof(acmd2),
		.buf   = acmd2,
	},
};

struct i2c_msg msg_aw3[] = {
	{
		.addr  = 0x37,
		.flags = 0,
		.len   = sizeof(acmd3),
		.buf   = acmd3,
	},
};

struct i2c_msg msg_aw4[] = {
	{
		.addr  = 0x37,
		.flags = 0,
		.len   = sizeof(acmd4),
		.buf   = acmd4,
	},
};

struct i2c_msg msg_ar2[] = {
	{
		.addr  = 0x37,
		.flags = I2C_M_RD,
		.len   = sizeof(aack2) + 1,
		.buf   = buf,
	},
};

struct i2c_msg refresh_msg[] = {
	{
		.addr  = 0x7d,
		.flags = 0,
		.len   = 1,
		.buf   = &zero,
	},
};

/* Modes i2c messages  */
struct i2c_msg start_msg[] = {
	{
		.addr  = 0x50,
		.flags = I2C_M_RD,
		.len   = 1,
		.buf   = buf,
	},
	{
		.addr  = 0x50,
		.flags = 0,
		.len   = 1,
		.buf   = &zero,
	},
	{
		.addr  = 0x50,
		.flags = I2C_M_RD,
		.len   = 256,
		.buf   = buf,
	},
};

struct i2c_msg edid_msg[] = {
	{
		.addr  = 0x50,
		.flags = 0,
		.len   = 1,
		.buf   = &eighty,
	},
	{
		.addr  = 0x50,
		.flags = I2C_M_RD,
		.len   = 256,
		.buf   = buf,
	},
};


/* Deactivation i2c messages */
struct i2c_msg msg_dw1[] = {
	{
		.addr  = 0x37,
		.flags = 0,
		.len   = sizeof(dcmd1),
		.buf   = dcmd1,
	},
};

struct i2c_msg msg_dw2[] = {
	{
		.addr  = 0x37,
		.flags = 0,
		.len   = sizeof(dcmd2),
		.buf   = dcmd2,
	},
};

struct i2c_msg msg_dr1[] = {
	{
		.addr  = 0x37,
		.flags = I2C_M_RD,
		.len   = sizeof(dack1) + 1,
		.buf   = buf,
	},
};



#define CHECK_IOCTL(FD, FL, M, STR) do {		\
	int r;						\
	if ((r = ioctl((FD), (FL), (M))) < 0) {		\
		perror(STR);				\
		close(FD);				\
		return r;				\
	} } while(0)

static int buf_neq(char * ref, char * read, int sref, int sread)
{
	int i;
	int size = sread;

	while ((*read == 0x6e) & (size > 0))
	{
		read++;
		size--;
	}

	if (size < sref)
	{
		return 1;

	}

	for (i = 0; i < sref; i++)
	{
		if (ref[i] != read[i])
		{
			return 1;
		}
	}
	return 0;
}


void us(unsigned int usec)
{
	unsigned int sec = 0;
	struct timespec ts;

	while (usec >= 1000000)
	{
		usec -= 1000000;
		sec  += 1;
	}
	ts.tv_sec  = sec;
	ts.tv_nsec = usec * 1000;
	nanosleep(&ts, 0);
}

int i2c_rw(int fd, struct i2c_msg * msg, int size, const char * text)
{
	struct i2c_rdwr_ioctl_data rdwr;

	rdwr.msgs     = msg;
	rdwr.nmsgs    = size / sizeof(struct i2c_msg);
	CHECK_IOCTL(fd, I2C_RDWR, &rdwr, text);
	return 0;
}

void printbuf(char* buf, int len)
{
	int i;

	for (i = 0; i < len; i++)
	{
		printf("%02x ", (unsigned char)buf[i]);
	}
	putchar('\n');
}


static int activate(int fd, unsigned long nb_refresh)
{
	unsigned long i;

	i2c_rw(fd, &start_msg[0], sizeof(start_msg), "START");
	printf("START: ");
	printbuf(buf, sizeof(buf));

	i2c_rw(fd, &edid_msg[0], sizeof(edid_msg), "EDID");
	printf("EDID : ");
	printbuf(buf, sizeof(buf));

	us(70000);

	do {
		do {
			i2c_rw(fd, &msg_aw1[0], sizeof(msg_aw1), "ACTIVATE W 1");
			us(50000);
			i2c_rw(fd, &msg_ar1[0], sizeof(msg_ar1), "ACTIVATE R 1");
			us(250000);
			printf("ACK1 : ");
			printbuf(buf, sizeof(aack1) + 1);
		}
		while (buf_neq(aack1, buf, sizeof(aack1), sizeof(aack1) + 1));

		i2c_rw(fd, &msg_aw2[0], 1, "ACTIVATE W 2");
		us(50000);

		do {
			i2c_rw(fd, &msg_aw3[0], sizeof(msg_aw3), "ACTIVATE W 3");
			us(50000);
			i2c_rw(fd, &msg_aw4[0], sizeof(msg_aw4), "ACTIVATE W 4");
			us(100000);
			i2c_rw(fd, &msg_ar2[0], sizeof(msg_ar2), "ACTIVATE R 2");
			us(50000);
			printf("ACK2 : ");
			printbuf(buf, sizeof(aack2) + 1);
			if (!buf_neq(aack1, buf, sizeof(aack1), sizeof(aack1) + 1))
			{
				buf[1] = 0xff;
				break ;
			}
		}
		while (buf_neq(aack2, buf, sizeof(aack2), sizeof(aack2) + 1));
	} while (buf_neq(aack2, buf, sizeof(aack2), sizeof(aack2) + 1));

	us(150000);

	i2c_rw(fd, &refresh_msg[0], sizeof(refresh_msg), "REFRESH");

	for (i = 0; i < nb_refresh; i++)
	{
		us(1000000);
		i2c_rw(fd, &refresh_msg[0], sizeof(refresh_msg), "REFRESH");
	}
	return 0;
}

static int deactivate(int fd)
{
	do {
		i2c_rw(fd, &msg_dw1[0], sizeof(msg_dw1), "DEACTIVATE W 1");
		us(50000);
		i2c_rw(fd, &msg_dw2[0], sizeof(msg_dw2), "DEACTIVATE W 2");
		us(100000);
		i2c_rw(fd, &msg_dr1[0], sizeof(msg_dr1), "DEACTIVATE R 1");
		us(50000);
		printf("ACK : ");
		printbuf(buf, sizeof(dack1) + 1);
	} while (buf_neq(dack1, buf, sizeof(dack1), sizeof(dack1) + 1));

	return 0;
}

static void help(void)
{
	puts("a3dcontrol [-d] [-r N] [-h] /dev/i2c-N\n"
	     "	Acer Integrated 3D vision emitter activator.\n"
	     "	Use DVI i2c ('modprobe i2c-dev' for nvidia proprietary driver).\n"
	     "	Require 3D DVI mode detected by screen.\n"
	     "\n"
	     "	-h	: Print this help and exit\n"
	     "	-v	: Print the version and exit\n"
	     "	-d	: Deactivate instead of Activate\n"
	     "	-r N	: Activation refresh number (defaut 0)");
}

static void version(void)
{
	printf("a3dcontrol v%s %s\n", VERSION, COPYRIGHT);
	puts("a3dcontrol comes with ABSOLUTELY NO WARRANTY."
	     " This is free software,\n"
	     "and you are welcome to redistribute it under"
	     " certain conditions.");
}


int main(int argc, char ** argv)
{
	int c;
	int fd;
	int dflag = 0;
	unsigned long r = 0;

	opterr = 0;

	while ((c = getopt (argc, argv, "hvdn:")) != -1)
	{
		switch (c)
		{
		case 'h':
			help();
			return 0;
		case 'v':
			version();
			return 0;
		case 'd':
			dflag = 1;
			break;
		case 'r':
			r = strtoul(optarg, 0, 10);
			break;
		case '?':
			if (optopt == 'c') {
				fprintf(stderr,
					"Option -%c requires an argument.\n",
					optopt);
			}
			else if (isprint(optopt)) {
				fprintf(stderr,
					"Unknown option `-%c'.\n",
					optopt);
			} else {
				fprintf(stderr,
					"Unknown option character `\\x%x'.\n",
					optopt);
			}
			help();
			return 1;
		default:
			abort();
		}
	}


	if (optind != argc -1)
	{
		help();
		return 1;
	}


	if ((fd = open(argv[optind], O_RDWR)) < 0)
	{
		perror(argv[1]);
		return 2;
	}

	if (dflag)
	{
		deactivate(fd);
	}
	else
	{
		activate(fd, r);
	}


	close(fd);
	return 0;
}
