//#include "gen/rom256_test.h"

#pragma CODE_PAGE 3
#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "log.h"

extern char caca1[];

int function_in_code1() __banked
{
	int b;

	b = 5;
	b++;

	// this will not work because "code 1" is defined as const data in this page
	// when calling vpd_print_grp1 we switch to another page and the data is gone.
        vdp_print_grp1(10, 14, "code 1");

	// this is the right way to do it

	sys_ascii_set(4);
	vdp_print_grp1(10, 18, caca1);
	sys_ascii_restore();

	return b;
}
