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
#include "bfs.h"


#define I2C_BUSNR	1
#define I2C_ADDR	0x70
#define DIGIT_CNT	6


#if 0
int sliceof[50] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11, 11, 11, 10, 9
};
#endif

#if 1
int sliceof[30] = {
	0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 2, 2,
	2, 2, 3, 3, 4, 5,
	6, 7, 8, 9, 9, 10,
	10, 11, 11, 11, 11, 10
};
#endif

#if 0
int sliceof[30] = {
	0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 2, 2,
	2, 2, 3, 3, 4, 5,
	6, 7, 8, 9, 9, 10,
	10, 11, 11, 6, 6, 7
};
#endif

#define KILLFILE	".norun"

int do_digit_loop(int, char *);


int
main(int argc, char **argv)
{
	int		ret;
	char		*execn;
	int		digitidx;

	execn = basename(argv[0]);
	if(xstrempty(execn)) {
		fprintf(stderr, "Can't get executable name\n");
		goto end_label;
	}

	unlink(KILLFILE);

	for(digitidx = 0; digitidx < DIGIT_CNT; ++digitidx) {
		ret = fork();
		if(ret == 0) {
			/* Child */
			do_digit_loop(digitidx, execn);
		} else
		if(ret > 0) {
			/* Parent */
		} else
		if (ret < 0) {
			fprintf(stderr, "Could not fork: %s\n",
			    strerror(errno));
			goto end_label;
		}
	}

end_label:

	return 0;
}


int
do_digit_loop(int didx, char *execn)
{
	int		ret;
	ht16k33_t	*ht;
	int		num;
	struct timespec ts;
	uint8_t		l[12][64];
	time_t		prevdate;	
	struct timeval	nowtv;
	int		slice;
	struct tm	*nowtm;
	int		roll;
	int		maxnum;

	ht = NULL;

	ret = blog_init(execn);
	if(ret != 0) {
		fprintf(stderr, "Could not initialize logging: %s\n",
		    strerror(ret));
		goto end_label;
	}

	ht = ht16k33_init(I2C_BUSNR, I2C_ADDR + didx, HT16K33_MODE_ADA_8X8);
	if(ht == NULL) {
		fprintf(stderr, "Could not initialize ht16k33\n");
		goto end_label;
	}

/*
	if(argc == 2 || !xstrcmp(argv[2], "clear")) {
		ht16k33_clearleds(ht);
		ht16k33_refreshleds(ht);
		goto end_label;
	}
*/

	ret = ht16k33_setbrightness(ht, 1);
	if(ret != 0) {
		fprintf(stderr, "Could not set brightness.\n");
		goto end_label;
	}

	ret = ht16k33_setrotate(ht, HT16K33_ROTATE_270);
	if(ret != 0) {
		fprintf(stderr, "Could not set rotate.\n");
		goto end_label;
	}

	digit_init();

	/* Set up the bitmaps. */
	switch(didx) {
	case 0:
		/* Hour 1 */
		maxnum = 2;
		break;
	case 1:
		/* Hour 2 */
		maxnum = 9;
		break;
	case 2:
		/* Minute 1 */
		maxnum = 5;
		break;
	case 3:
		/* Minute 2 */
		maxnum = 9;
		break;
	case 4:
		/* Second 1 */
		maxnum = 5;
		break;
	case 5:
	default:
		/* Second 2 */
		maxnum = 9;
		break;
	}

	memcpy(l, digit, (maxnum + 1) * 64);
	memcpy(&l[maxnum + 1][0], digit, 128); /* Copy 0 and 1 at the end for
						 * rolling over. */
	
	ht16k33_clearleds(ht);

	ts.tv_sec = 0;
	ts.tv_nsec = 1000000000 / 30;

	time(&prevdate);

	while(1) {

		if(bfs_isfile(KILLFILE))
			break;

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

		roll = 0;

		switch(didx) {
		case 0:
			/* Hour 1 */
			num = nowtm->tm_hour / 10;
			if(nowtm->tm_sec == 59 && nowtm->tm_min == 59 &&
			    nowtm->tm_hour % 10 == 9)
				++roll;
			break;
		case 1:
			/* Hour 2 */
			num = nowtm->tm_hour % 10;
			if(nowtm->tm_sec == 59 && nowtm->tm_min == 59)
				++roll;
			break;
		case 2:
			/* Minute 1 */
			num = nowtm->tm_min / 10;
			if(nowtm->tm_sec == 59 && nowtm->tm_min % 10 == 9)
				++roll;
			break;
		case 3:
			/* Minute 2 */
			num = nowtm->tm_min % 10;
			if(nowtm->tm_sec == 59)
				++roll;
			break;
		case 4:
			/* Second 1 */
			num = nowtm->tm_sec / 10;
			if(nowtm->tm_sec % 10 == 9)
				++roll;
			break;
		case 5:
		default:
			/* Second 2 */
			num = nowtm->tm_sec % 10;
			++roll;
			break;
		}

		if(!roll) {
			slice = 0;
		} else {
			slice = nowtv.tv_usec / (1000000 / 30);
			if(slice > 29)
				slice = 29;
		}

		ht16k33_setleds(ht, l[num] + sliceof[slice] * 8);

/*
		if(slice < 15) {
			ht16k33_setled(ht, 2, 0);
			ht16k33_setled(ht, 5, 0);
		}
*/


		ht16k33_refreshleds(ht);

/*
		ht16k33_printleds(ht);
		printf("\n");
		fflush(stdout);
*/
	}


end_label:

	if(ht) {
		ht16k33_clearleds(ht);
		ht16k33_refreshleds(ht);

		ht16k33_uninit(&ht);
	}

	blog_uninit();

	return 0;
}
