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

#define HT16K33_CMD_START_OSC		0x21
#define HT16K33_CMD_DISP_ON_BLINK_OFF	0x81
#define HT16K33_CMD_BRIGHTNESS		0xe0


static uint8_t colbits_def[] = {
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};

/* Column addressing is a bit weird on Adafruit hardware... */
static uint8_t colbits_ada[] = {
	0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40
};


ht16k33_t *
ht16k33_init(int busnr, int addr, int mode)
{

	int		err;
	ht16k33_t	*ht;
	int		ret;

	err = 0;
	ht = NULL;

	if(mode >= HT16K33_MODE_CNT) {
		blogf("Wrong mode specified");
		err = EINVAL;
		goto end_label;
	}

	ht = malloc(sizeof(ht16k33_t));
	if(ht == NULL) {
		blogf("Couldn't allocate ht");
		err = ENOMEM;
		goto end_label;
	}

	memset(ht, 0, sizeof(ht16k33_t));

	ht->ht_busnr = busnr;
	ht->ht_addr = addr;

	ht->ht_mode = mode;
	switch(mode) {
	case HT16K33_MODE_ADA_8X8:
		ht->ht_colbits = colbits_ada;
		ht->ht_rowcnt = 8;
		ht->ht_row_multiplier = 2;
		break;
	default:
		ht->ht_colbits = colbits_def;
		ht->ht_rowcnt = HT16K33_ROW_CNT;
		ht->ht_row_multiplier = 1;
		break;
	}

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

	ret = ht16k33_setbrightness(ht, HT16K33_BRIGHTNESS_MAX);
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
ht16k33_setbrightness(ht16k33_t *ht, int brightness)
{
	int	ret;

	if(ht == NULL || !bint_betw(brightness, 0, BINT_INCL,
	    HT16K33_BRIGHTNESS_MAX, BINT_INCL))
		return EINVAL;

	/* Set brightness. */
	ret = ht16k33_write_byte(ht,
	    HT16K33_CMD_BRIGHTNESS | (uint8_t) brightness);
	if(ret != 0) {
		blogf("Couldn't send command byte.");
		return ret;
	}

	ht->ht_brightness = HT16K33_BRIGHTNESS_MAX;

	return 0;
}


int
ht16k33_setrotate(ht16k33_t *ht, int rotate)
{
	if(ht == NULL || !bint_betw(rotate, 0, BINT_INCL,
	    HT16K33_ROTATE_CNT, BINT_EXCL))
		return EINVAL;

	if(rotate != 0 && ht->ht_rowcnt != 8) {
		blogf("Only 8x8 modes support rotation");
		return EINVAL;
	}

	ht->ht_rotate = rotate;

	return 0;
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


int
ht16k33_write_byte(ht16k33_t *ht, uint8_t byte)
{
	return ht16k33_write_buf(ht, &byte, 1);
}

int
ht16k33_write_word(ht16k33_t *ht, uint16_t word)
{
	return ht16k33_write_buf(ht, &word, 2);
}

int
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


int
ht16k33_setled(ht16k33_t *ht, int row, int col)
{
	int	realrow;
	int	realcol;

	if(ht == NULL ||
	    !bint_betw(row, 0, BINT_INCL, ht->ht_rowcnt, BINT_EXCL) ||
	    !bint_betw(col, 0, BINT_INCL, HT16K33_COL_CNT, BINT_EXCL))
		return EINVAL;

	switch(ht->ht_rotate) {
	case HT16K33_ROTATE_90:
		realrow = col;
		realcol = HT16K33_COL_CNT - row - 1;
		break;
	case HT16K33_ROTATE_180:
		realrow = ht->ht_rowcnt - row - 1;
		realcol = HT16K33_COL_CNT - col - 1;
		break;
		break;
	case HT16K33_ROTATE_270:
		realrow = ht->ht_rowcnt - col - 1;
		realcol = row;
		break;
	default:
		realrow = row;
		realcol = col;
		break;
	}
	
	ht->ht_bufcmd[(realrow * ht->ht_row_multiplier) + 1] |=
	    ht->ht_colbits[realcol];

	return 0;
}


int
ht16k33_clearled(ht16k33_t *ht, int row, int col)
{
	int	realrow;
	int	realcol;

	if(ht == NULL ||
	    !bint_betw(row, 0, BINT_INCL, ht->ht_rowcnt, BINT_EXCL) ||
	    !bint_betw(col, 0, BINT_INCL, HT16K33_COL_CNT, BINT_EXCL))
		return EINVAL;

	switch(ht->ht_rotate) {
	case HT16K33_ROTATE_90:
		realrow = col;
		realcol = HT16K33_COL_CNT - row - 1;
		break;
	case HT16K33_ROTATE_180:
		realrow = ht->ht_rowcnt - row - 1;
		realcol = HT16K33_COL_CNT - col - 1;
		break;
		break;
	case HT16K33_ROTATE_270:
		realrow = ht->ht_rowcnt - col - 1;
		realcol = row;
		break;
	default:
		realrow = row;
		realcol = col;
		break;
	}

	ht->ht_bufcmd[(realrow * ht->ht_row_multiplier) + 1] &=
	    ~ht->ht_colbits[realcol];

	return 0;
}


int
ht16k33_toggleled(ht16k33_t *ht, int row, int col)
{
	int	realrow;
	int	realcol;

	if(ht == NULL ||
	    !bint_betw(row, 0, BINT_INCL, ht->ht_rowcnt, BINT_EXCL) ||
	    !bint_betw(col, 0, BINT_INCL, HT16K33_COL_CNT, BINT_EXCL))
		return EINVAL;

	switch(ht->ht_rotate) {
	case HT16K33_ROTATE_90:
		realrow = col;
		realcol = HT16K33_COL_CNT - row - 1;
		break;
	case HT16K33_ROTATE_180:
		realrow = ht->ht_rowcnt - row - 1;
		realcol = HT16K33_COL_CNT - col - 1;
		break;
		break;
	case HT16K33_ROTATE_270:
		realrow = ht->ht_rowcnt - col - 1;
		realcol = row;
		break;
	default:
		realrow = row;
		realcol = col;
		break;
	}

	ht->ht_bufcmd[(realrow * ht->ht_row_multiplier) + 1] ^=
	    ht->ht_colbits[realcol];

	return 0;
}


int
ht16k33_ledison(ht16k33_t *ht, int row, int col)
{
	int	realrow;
	int	realcol;

	if(ht == NULL ||
	    !bint_betw(row, 0, BINT_INCL, ht->ht_rowcnt, BINT_EXCL) ||
	    !bint_betw(col, 0, BINT_INCL, HT16K33_COL_CNT, BINT_EXCL)) {
		blogf("Invalid coordinates");
		return 0;
	}

	switch(ht->ht_rotate) {
	case HT16K33_ROTATE_90:
		realrow = col;
		realcol = HT16K33_COL_CNT - row - 1;
		break;
	case HT16K33_ROTATE_180:
		realrow = ht->ht_rowcnt - row - 1;
		realcol = HT16K33_COL_CNT - col - 1;
		break;
		break;
	case HT16K33_ROTATE_270:
		realrow = ht->ht_rowcnt - col - 1;
		realcol = row;
		break;
	default:
		realrow = row;
		realcol = col;
		break;
	}

	return ht->ht_bufcmd[(realrow * ht->ht_row_multiplier) + 1] &
	    ht->ht_colbits[realcol];
}


int
ht16k33_printleds(ht16k33_t *ht)
{
	int	row;
	int	col;

	if(ht == NULL)
		return EINVAL;

	for(row = 0; row < ht->ht_rowcnt; ++row) {
		for(col = 0; col < HT16K33_COL_CNT; ++col) {
			if(ht16k33_ledison(ht, row, col))
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


int
ht16k33_setleds(ht16k33_t *ht, uint8_t *leds)
{
	/* Sets all leds in a single call. The passed in int array must be
	 * of the correct size. (E.g. in ADA_8X8 mode, it must be 64 bytes. */

	int	row;
	int	col;
	uint8_t	*cur;

	if(ht == NULL || leds == NULL)
		return EINVAL;

	ht16k33_clearleds(ht);

	cur = leds;
	for(row = 0; row < ht->ht_rowcnt; ++row) {
		for(col = 0; col < HT16K33_COL_CNT; ++col) {
			if(*cur)
				ht16k33_setled(ht, row, col);
			++cur;
		}
	}

	return 0;
}
