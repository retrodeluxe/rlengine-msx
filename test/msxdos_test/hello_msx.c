/**
 *
 * Copyright (C) Retro DeLuxe 2013, All rights reserved.
 *
 */

void putchar(char c);
void puts(char *s);

int main(char** argv, int argc)
{
  puts("Hello MSXDOS\n");

  return 0;
}


void putchar(char c) {
    __asm
    ld		e,4(ix)
    ld		c,#2
    call	5
  __endasm;
}

void puts(char *s) {
	while (*s != 0) {
		putchar(*s);
		s++;
	}
}
