#ifndef HT16K33_H
#define HT16K33_H

#include "bstr.h"

#define HT16K33_MAX_MEM	16

typedef struct ht16k33 {
	int	ht_busnr;
	int	ht_addr;

	bstr_t	*ht_filen;
	int	ht_fd;

	char	ht_mem[HT16K33_MAX_MEM];

} ht16k33_t;



ht16k33_t *ht16k33_init(int, int);
void ht16k33_uninit(ht16k33_t **);


#define HT16K33_LED_OFF	0
#define HT16K33_LED_ON	1

int ht16k33_clearleds(ht16k33_t *);
int ht16k33_getled(ht16k33_t *, int, int);
int ht16k33_setled(ht16k33_t *, int, int, int); /*  row, col, on or off */
int ht16k33_refreshleds(ht16k33_t *);

#endif
