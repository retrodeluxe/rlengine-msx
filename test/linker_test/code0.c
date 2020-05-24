//#include "gen/rom256_test.h"

#pragma bank 1

int function_in_code0() __banked
{
	int a;

	a = 5;
	a++;

	return a;
}

const char caca0[] = "test";
