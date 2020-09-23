#include <stdio.h>
#include <libgen.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "bstr.h"
#include "blog.h"
#include "ht16k33.h"
#include "digit.h"


#define I2C_BUSNR	1
#define I2C_ADDR	0x70


#if 0
int sliceof[50] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11, 11, 11, 10, 9
};
#endif

/*
int sliceof[30] = {
	0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 2, 2,
	2, 2, 3, 3, 4, 5,
	6, 7, 8, 9, 9, 10,
	10, 10, 11, 11, 11, 11 
};
*/

int sliceof[30] = {
	0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 2, 2,
	2, 2, 3, 3, 4, 5,
	6, 7, 8, 9, 9, 10,
	10, 11, 11, 6, 6, 7
};

int
main(int argc, char **argv)
{
	int		ret;
	char		*execn;
	ht16k33_t	*ht;
	int		i;
	int		t;
	int		u;
	int		v;
	struct timespec ts;
	uint8_t		l[12][64];
	time_t		prevdate;	
	time_t		now;	
	struct timeval	nowtv;
	int		slice;
	struct tm	*nowtm;

	ht = NULL;

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

	if(argc == 2 || !xstrcmp(argv[2], "clear")) {
		ht16k33_clearleds(ht);
		ht16k33_refreshleds(ht);
		goto end_label;
	}

	digit_init();

	memcpy(l, digit, 10 * 64);
	memcpy(&l[10][0], digit, 128);

	
	ht16k33_clearleds(ht);

	ts.tv_sec = 0;
	ts.tv_nsec = 1000000000 / 30;

	i = 0;
	time(&prevdate);

	t = 0;
	while(1) {
		nanosleep(&ts, NULL);

		ret = gettimeofday(&nowtv, NULL);
		if(ret != 0) {
			blogf("Couldn't get time");
			goto end_label;
		}

		nowtm = localtime(&nowtv.tv_sec);
		if(nowtm == NULL) {
			blogf("Couldn't get localtime");
			goto end_label;
		}

		i = nowtm->tm_sec % 10;

		slice = nowtv.tv_usec / (1000000 / 30);
		if(slice > 29)
			slice = 29;

		ht16k33_setleds(ht, l[i] + sliceof[slice] * 8);
		ht16k33_refreshleds(ht);

/*
		ht16k33_printleds(ht);
		printf("\n");
		fflush(stdout);
*/
	}


end_label:

	ht16k33_uninit(&ht);
	blog_uninit();

	return 0;
}
