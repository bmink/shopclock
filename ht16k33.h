#ifndef HT16K33_H
#define HT16K33_H

#include <stdint.h>
#include "bstr.h"

#define HT16K33_ROW_CNT		16
#define HT16K33_COL_CNT		8	

#define HT16K33_MODE_DEFAULT	0	/* Default mode 16x8 */
#define HT16K33_MODE_ADA_8X8	1	/* Adafruit 8x8 matrix mode. */
#define HT16K33_MODE_CNT	2

#define HT16K33_BUFCMDSIZ		17

typedef struct ht16k33 {
	int	ht_busnr;
	int	ht_addr;
	int	ht_mode;

	bstr_t	*ht_filen;
	int	ht_fd;

	uint8_t	ht_bufcmd[HT16K33_BUFCMDSIZ];	/* First byte 0x00 (=command
						 * for "write display buffer
						 * starting at address 0,
						 * followed by the 16 bytes
						 * of the display buffer */

} ht16k33_t;



ht16k33_t *ht16k33_init(int, int, int);
void ht16k33_uninit(ht16k33_t **);

/* Convenient LED functions */
int ht16k33_clearleds(ht16k33_t *);
int ht16k33_setled(ht16k33_t *, int, int);
int ht16k33_unsetled(ht16k33_t *, int, int);
int ht16k33_toggleled(ht16k33_t *, int, int);
int ht16k33_ledison(ht16k33_t *, int, int);
int ht16k33_refreshleds(ht16k33_t *);
int ht16k33_printleds(ht16k33_t *);


/* Functions to directly write data to the HT16K33 */
int ht16k33_write_byte(ht16k33_t *, uint8_t);
int ht16k33_write_word(ht16k33_t *, uint16_t);
int ht16k33_write_reg(ht16k33_t *, uint16_t);
int ht16k33_write_buf(ht16k33_t *, void *, size_t);




#endif
