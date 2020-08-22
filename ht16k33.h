#ifndef HT16K33_H
#define HT16K33_H

#include <stdint.h>
#include "bstr.h"

//#define HT16K33_COL_CNT		16	
#define HT16K33_COL_CNT		8	
#define HT16K33_ROW_CNT		8

typedef struct ht16k33 {
	int	ht_busnr;
	int	ht_addr;

	bstr_t	*ht_filen;
	int	ht_fd;

	uint8_t	ht_cols[HT16K33_COL_CNT];	/* Bitmap */

} ht16k33_t;



ht16k33_t *ht16k33_init(int, int);
void ht16k33_uninit(ht16k33_t **);


#define HT16K33_LED_OFF		0
#define HT16K33_LED_ON		1
#define HT16K33_LED_TOGGLE	2

int ht16k33_clearleds(ht16k33_t *);
int ht16k33_setled(ht16k33_t *, int, int, int); /*  row, col, on or off */
int ht16k33_ledison(ht16k33_t *, int, int);
int ht16k33_displayleds(ht16k33_t *);
int ht16k33_printleds(ht16k33_t *);

#endif
