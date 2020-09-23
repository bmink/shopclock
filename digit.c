#include "digit.h"

uint8_t digit[10][64];

static char digitdef[10][64] = 
{

      // 01234567
	"  XXXX  "
	" XX  XX "
	" XX XXX "
	" XXXXXX "
	" XXX XX "
	" XX  XX "
	"  XXXX  "
        "        ",

      // 01234567
	"   XX   "
	"  XXX   "
	"   XX   "
	"   XX   "
	"   XX   "
	"   XX   "
	" XXXXXX "
	"        ",

      // 01234567
	"  XXXX  "
	" XX  XX "
	"     XX "
	"   XXX  "
	"  XX    "
	" XX  XX "
	" XXXXXX "
	"        ",


      // 01234567
        "  XXXX  "
        " XX  XX "
        "     XX " 
        "   XXX  " 
        "     XX "
        " XX  XX "
        "  XXXX  "
        "        ",

      // 01234567
        "   XXX  "
        "  XXXX  "
        " XX XX  "
        " XX XX  "
        " XXXXXX "
        "    XX  "
        "   XXXX "
	"        ",

      // 01234567
	" XXXXXX "
	" XX     "
	" XXXXX  "
	"     XX "
	"     XX "
	" XX  XX "
	"  XXXX  "
	"        ",

      // 01234567
	"   XXX  "
	"  XX    "
	" XX     "
	" XXXXX  "
	" XX  XX "
	" XX  XX "
	"  XXXX  "
	"        ",

      // 01234567
	" XXXXXX "
	" XX  XX "
	"     XX "
	"    XX  "
	"   XX   "
	"   XX   "
	"   XX   "
	"        ",

      // 01234567
	"  XXXX  "
	" XX  XX "
	" XX  XX "
	"  XXXX  "
	" XX  XX "
	" XX  XX "
	"  XXXX  "
	"        ",

      // 01234567
	"  XXXX  "
	" XX  XX "
	" XX  XX "
	"  XXXXX "
	"     XX "
	"    XX  "
	"  XXX   "
	"        "
};


void
digit_init(void)
{
	int	i;
	int	t;

	for(i = 0; i < 10; ++i) {
		for(t = 0; t < 64; ++t) {
			if(digitdef[i][t] == ' ')
				digit[i][t] = 0;
			else
				digit[i][t] = 1;
		}
	}
}
