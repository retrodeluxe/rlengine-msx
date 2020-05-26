//#include "gen/rom256_test.h"

#pragma CODE_PAGE 3
#include "msx.h"
#include "sys.h"
#include "vdp.h"
#include "log.h"
int function_in_code1() __banked
{
	int b;

	b = 5;
	b++;

        vdp_print_grp1(10, 14, "code 1");

	return b;
}
