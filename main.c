#include <stdio.h>
#include <libgen.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include "bstr.h"
#include "blog.h"
#include "ht16k33.h"


#define I2C_BUSNR	1
#define I2C_ADDR	0x70


int
main(int argc, char **argv)
{
	int		ret;
	char		*execn;
	ht16k33_t	*ht;
	int		i;
	int		t;
	struct timespec ts;

	execn = basename(argv[0]);
	if(xstrempty(execn)) {
		fprintf(stderr, "Can't get executable name\n");
		goto end_label;
	}

	ret = blog_init(execn);
	if(ret != 0) {
		fprintf(stderr, "Could not initialize logging: %s\n",
		    strerror(ret));
		goto end_label;
	}

	ht = ht16k33_init(I2C_BUSNR, I2C_ADDR, HT16K33_MODE_ADA_8X8);
	if(ht == NULL) {
		fprintf(stderr, "Could not initialize ht16k33\n");
		goto end_label;
	}

	
	//ht16k33_setled(ht, 0, 0, HT16K33_LED_ON);
	//ht16k33_setled(ht, 0, 1, HT16K33_LED_ON);
	//ht16k33_printleds(ht);
	//ht16k33_printleds(ht);

	ht16k33_clearleds(ht);
	ht16k33_refreshleds(ht);

	ts.tv_sec = 0;
	ts.tv_nsec = 5000000;

	while(1) {
		for(i = 0; i < 8; ++i) {
			for(t = 0; t < 8; ++t) {
				ht16k33_toggleled(ht, i, t);
				ht16k33_refreshleds(ht);

/*
				printf("\n\n");
				ht16k33_printleds(ht);
*/
				
				nanosleep(&ts, NULL);
			}
		}
	}


end_label:
	blog_uninit();

	return 0;
}
