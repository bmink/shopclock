#include <stdio.h>
#include <libgen.h>
#include <errno.h>
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

	ht = ht16k33_init(I2C_BUSNR, I2C_ADDR);
	if(ht == NULL) {
		fprintf(stderr, "Could not initialize ht16k33\n");
		goto end_label;
	}

	ht16k33_printleds(ht);

end_label:
	blog_uninit();

	return 0;
}
