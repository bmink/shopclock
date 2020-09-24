#ifndef HT16K33_H
#define HT16K33_H

#include <stdint.h>
#include "bstr.h"

#define HT16K33_ROW_CNT		16
#define HT16K33_COL_CNT		8	


#define HT16K33_MODE_DEFAULT	0	/* Default mode 16x8 */
#define HT16K33_MODE_ADA_8X8	1	/* Adafruit 8x8 matrix mode. */
#define HT16K33_MODE_CNT	2

#define HT16K33_ROTATE_0	0	/* Default */
#define HT16K33_ROTATE_90	1
#define HT16K33_ROTATE_180	2
#define HT16K33_ROTATE_270	3
#define HT16K33_ROTATE_CNT	4

#define HT16K33_BRIGHTNESS_MAX	15

#define HT16K33_BUFCMDSIZ		17

typedef struct ht16k33 {
	int	ht_busnr;
	int	ht_addr;
	int	ht_mode;
	int	ht_rotate;

	int	ht_brightness;

	bstr_t	*ht_filen;
	int	ht_fd;

	uint8_t *ht_colbits;
	int	ht_rowcnt;
	int	ht_row_multiplier;

	uint8_t	ht_bufcmd[HT16K33_BUFCMDSIZ];	/* First byte 0x00 (=command
						 * for "write display buffer
						 * starting at address 0,
						 * followed by the 16 bytes
						 * of the display buffer */

} ht16k33_t;


ht16k33_t *ht16k33_init(int, int, int);
void ht16k33_uninit(ht16k33_t **);

/* Config functions. */
int ht16k33_setbrightness(ht16k33_t *, int);
int ht16k33_setrotate(ht16k33_t *, int);

/* Convenient LED functions */
int ht16k33_clearleds(ht16k33_t *);
int ht16k33_setled(ht16k33_t *, int, int);
int ht16k33_clearled(ht16k33_t *, int, int);
int ht16k33_toggleled(ht16k33_t *, int, int);
int ht16k33_ledison(ht16k33_t *, int, int);
int ht16k33_refreshleds(ht16k33_t *);
int ht16k33_printleds(ht16k33_t *);
int ht16k33_setleds(ht16k33_t *, uint8_t *);


/* Functions to directly write data to the HT16K33 */
int ht16k33_write_byte(ht16k33_t *, uint8_t);
int ht16k33_write_word(ht16k33_t *, uint16_t);
int ht16k33_write_reg(ht16k33_t *, uint16_t);
int ht16k33_write_buf(ht16k33_t *, void *, size_t);


#endif
