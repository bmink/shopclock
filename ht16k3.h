#ifndef HT16K3_H
#define HT16K3_H

#define HT16K3_MAX_MEM	16

typedef struct ht16k3 {

	char	ht_mem[HT16K3_MAX_MEM];

} ht16k3_t;


ht16k3_t *ht16k3_init(void);
void ht16k3_uninit(ht16k3_t **);


#define HT16K3_LED_OFF	0
#define HT16K3_LED_ON	1

/*  row, col, on or off */
int ht16k3_clearleds(ht16k3_t *);
int ht16k3_getled(ht16k3_t *, int, int);
int ht16k3_chled(ht16k3_t *, int, int, int);
int ht16k3_refreshleds(ht16k3_t *);

#endif
