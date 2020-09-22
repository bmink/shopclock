#ifndef HT16K33_H
#define HT16K33_H

#include <stdint.h>
#include "bstr.h"

#define HT16K33_ROW_CNT		16
#define HT16K33_COL_CNT		8	

#define HT16K33_BUFCMDSIZ		17

typedef struct ht16k33 {
	int	ht_busnr;
	int	ht_addr;

	bstr_t	*ht_filen;
	int	ht_fd;

	uint8_t	ht_bufcmd[HT16K33_BUFCMDSIZ];	/* First byte 0x00 (=command
						 * for "write display buffer
						 * starting at address 0,
						 * followed by the 16 bytes
						 * of the display buffer */

} ht16k33_t;



ht16k33_t *ht16k33_init(int, int);
void ht16k33_uninit(ht16k33_t **);



int ht16k33_clearleds(ht16k33_t *);
int ht16k33_setled(ht16k33_t *, int, int);
int ht16k33_unsetled(ht16k33_t *, int, int);
int ht16k33_toggleled(ht16k33_t *, int, int);
int ht16k33_ledison(ht16k33_t *, int, int);
int ht16k33_refreshleds(ht16k33_t *);
int ht16k33_printleds(ht16k33_t *);

#endif
