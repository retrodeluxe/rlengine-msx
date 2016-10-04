#ifndef _MSX_H_MAP
#define _MSX_H_MAP


void map_inflate(const byte * dict, const byte * in, byte * out, uint data_size,
		 byte w);
void map_inflate_screen(const byte * dict, const byte * in, byte * out, byte w, byte vpx, byte vpy);

#endif