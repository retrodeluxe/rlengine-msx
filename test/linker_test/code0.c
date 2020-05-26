//#include "gen/rom256_test.h"

#pragma CODE_PAGE 2

int function_in_code0() __banked
{
	int a;

	a = 5;
	a++;

	return a;
}
