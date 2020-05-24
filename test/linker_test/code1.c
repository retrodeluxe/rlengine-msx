//#include "gen/rom256_test.h"

#pragma bank 2

int function_in_code1() __banked
{
	int b;

	b = 5;
	b++;

	return b;
}

const char caca1[] = "test";
