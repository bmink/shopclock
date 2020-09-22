#include <stddef.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "ht16k33.h"
#include "blog.h"
#include "bint.h"

static int ht16k33_check_i2c_func(ht16k33_t *);
static int ht16k33_write_byte(ht16k33_t *, uint8_t);
static int ht16k33_write_word(ht16k33_t *, uint16_t);
static int ht16k33_write_reg(ht16k33_t *, uint16_t);
static int ht16k33_write_buf(ht16k33_t *, void *, size_t);

#define HT16K33_CMD_START_OSC		0x21
#define HT16K33_CMD_DISP_ON_BLINK_OFF	0x81
#define HT16K33_CMD_BRIGHTNESS_FULL	0xef

ht16k33_t *
ht16k33_init(int busnr, int addr)
{

	int		err;
	ht16k33_t	*ht;
	int		ret;

	err = 0;
	ht = NULL;

	ht = malloc(sizeof(ht16k33_t));
	if(ht == NULL) {
		blogf("Couldn't allocate ht");
		err = ENOMEM;
		goto end_label;
	}

	memset(ht, 0, sizeof(ht16k33_t));

	ht->ht_busnr = busnr;
	ht->ht_addr = addr;


	ht->ht_filen = binit();
	if(ht->ht_filen == NULL) {
		blogf("Couldn't allocate ht_filen");
		err = ENOMEM;
		goto end_label;
	}

	bprintf(ht->ht_filen, "/dev/i2c-%d", ht->ht_busnr);

	ret = open(bget(ht->ht_filen), O_RDWR);
	if(ret < 0) {
		blogf("Couldn't open file %s: %s", bget(ht->ht_filen),
		    strerror(errno));
		err = errno;
		goto end_label;
	} else
		ht->ht_fd = ret;

	/* Check if we fully support I2C */
	ret = ht16k33_check_i2c_func(ht);
	if(ret != 0) {
		blogf("Device does not support full I2C functionality");
		err = ENOEXEC;
		goto end_label;
	}


	/* Start the oscillator. */
	ret = ht16k33_write_byte(ht, HT16K33_CMD_START_OSC);
	if(ret != 0) {
		blogf("Couldn't start oscillator.");
		err = ret;
		goto end_label;
	}

	/* Display on, blink off. */
	ret = ht16k33_write_byte(ht, HT16K33_CMD_DISP_ON_BLINK_OFF);
	if(ret != 0) {
		blogf("Couldn't turn on display.");
		err = ret;
		goto end_label;
	}

	/* Full brightness. */
	ret = ht16k33_write_byte(ht, HT16K33_CMD_BRIGHTNESS_FULL);
	if(ret != 0) {
		blogf("Couldn't set brightness.");
		err = ret;
		goto end_label;
	}

end_label:

	if(err != 0 && ht) {
		ht16k33_uninit(&ht);
		ht = NULL;
	}

	return ht;
}


void
ht16k33_uninit(ht16k33_t **htp)
{
	if(htp == NULL || *htp == NULL)
		return;

	buninit(&(*htp)->ht_filen);

	free(*htp);
	*htp = NULL;
}


int
ht16k33_check_i2c_func(ht16k33_t *ht)
{
	unsigned long	funcs;
	int		ret;

	if(ht == NULL || !ht->ht_fd)
		return EINVAL;

	ret = ioctl(ht->ht_fd, I2C_FUNCS, &funcs);	
	if(ret != 0) {
		blogf("Couldn't ioctl device: %s", strerror(errno));
		return errno;
	}

	if(funcs & I2C_FUNC_I2C) {
		return 0;
	} else {
		return ENOEXEC;
	}
}


static int
ht16k33_write_byte(ht16k33_t *ht, uint8_t byte)
{
	return ht16k33_write_buf(ht, &byte, 1);
}

static int
ht16k33_write_word(ht16k33_t *ht, uint16_t word)
{
	return ht16k33_write_buf(ht, &word, 2);
}

static int
ht16k33_write_buf(ht16k33_t *ht, void *buf, size_t siz)
{
	/*
	 * See:
	 * https://www.kernel.org/doc/Documentation/i2c/dev-interface
	 * https://gist.github.com/JamesDunne/9b7fbedb74c22ccc833059623f47beb7
	 */

	struct i2c_msg			msg;
	struct i2c_rdwr_ioctl_data	iodat;
	int				ret;

	if(ht == NULL || buf == NULL || siz == 0)
		return EINVAL;

	msg.addr = ht->ht_addr;
	msg.flags = 0;
        msg.len = siz;
        msg.buf = buf;

	iodat.msgs = &msg;
	iodat.nmsgs = 1;

	ret = ioctl(ht->ht_fd, I2C_RDWR, &iodat);
	if(ret < 0) {
		blogf("Couldn't communicate with device: %s", strerror(errno));
		return ret;
	}

	return 0;
}

int
ht16k33_clearleds(ht16k33_t *ht)
{
	if(ht == NULL)
		return EINVAL;

	memset(ht->ht_bufcmd, 0, HT16K33_BUFCMDSIZ);

	return 0;
}

static uint8_t colbit_def[HT16K33_COL_CNT] = {
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};

static uint8_t colbit_ada[HT16K33_COL_CNT] = {
	0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40
};


int
ht16k33_setled(ht16k33_t *ht, int row, int col)
{
	if(ht == NULL ||
	    !bint_betw(row, 0, BINT_INCL, HT16K33_ROW_CNT, BINT_EXCL) ||
	    !bint_betw(col, 0, BINT_INCL, HT16K33_COL_CNT, BINT_EXCL))
		return EINVAL;

	ht->ht_bufcmd[row + 1] |= colbit_ada[col];

	return 0;
}


int
ht16k33_clearled(ht16k33_t *ht, int row, int col)
{
	if(ht == NULL ||
	    !bint_betw(row, 0, BINT_INCL, HT16K33_ROW_CNT, BINT_EXCL) ||
	    !bint_betw(col, 0, BINT_INCL, HT16K33_COL_CNT, BINT_EXCL))
		return EINVAL;

	ht->ht_bufcmd[row + 1] &= ~colbit_ada[col];

	return 0;
}


int
ht16k33_toggleled(ht16k33_t *ht, int row, int col)
{
	if(ht == NULL ||
	    !bint_betw(row, 0, BINT_INCL, HT16K33_ROW_CNT, BINT_EXCL) ||
	    !bint_betw(col, 0, BINT_INCL, HT16K33_COL_CNT, BINT_EXCL))
		return EINVAL;

	ht->ht_bufcmd[row + 1] ^= colbit_ada[col];

	return 0;
}


int
ht16k33_ledison(ht16k33_t *ht, int row, int col)
{
	if(ht == NULL ||
	    !bint_betw(row, 0, BINT_INCL, HT16K33_ROW_CNT, BINT_EXCL) ||
	    !bint_betw(col, 0, BINT_INCL, HT16K33_COL_CNT, BINT_EXCL)) {
		blogf("Invalid coordinates");
		return 0;
	}

	return ht->ht_bufcmd[row + 1] & colbit_ada[col];
}


int
ht16k33_printleds(ht16k33_t *ht)
{
	int	x;
	int	y;

	if(ht == NULL)
		return EINVAL;

	for(y = 0; y < HT16K33_ROW_CNT; ++y) {
		for(x = 0; x < HT16K33_COL_CNT; ++x) {
			if(ht16k33_ledison(ht, x, y))
				printf("#");
			else
				printf(".");
		}
		printf("\n");
	}

	return 0;
}


int
ht16k33_refreshleds(ht16k33_t *ht)
{
	if(ht == NULL)
		return EINVAL;

	return ht16k33_write_buf(ht, ht->ht_bufcmd, HT16K33_BUFCMDSIZ);
}
