#include <stddef.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include "ht16k3.h"
#include "blog.h"

ht16k3_t *
ht16k3_init(int busnr, int addr)
{

	int		err;
	ht16k3_t	*ht;

	err = 0;
	ht = NULL;

	ht = malloc(sizeof(ht16k3_t));
	if(ht == NULL) {
		blogf("Couldn't allocate ht");
		err = ENOMEM;
		goto end_label;
	}

	memset(ht, 0, sizeof(ht16k3_t));

	ht->ht_filen = binit();
	if(ht->ht_filen == NULL) {
		blogf("Couldn't allocate ht_filen");
		err = ENOMEM;
		goto end_label;
	}

	bprintf(ht->ht_filen, "/dev/i2c-%d", busnr);


/* USE I2C_FUNCS to see what is supported: https://stackoverflow.com/questions/9974592/i2c-slave-ioctl-purpose */

end_label:

	if(err != 0 && ht) {
		ht16k3_uninit(&ht);
		ht = NULL;
	}

	return ht;
}


void
ht16k3_uninit(ht16k3_t **htp)
{
	if(htp == NULL || *htp == NULL)
		return;

	buninit(&(*htp)->ht_filen);

	free(*htp);
	*htp = NULL;
}
