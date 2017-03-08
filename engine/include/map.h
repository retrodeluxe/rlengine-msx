#ifndef _MSX_H_MAP
#define _MSX_H_MAP



#define map_inflate_screen(MAP, BUF, X, Y)	__map_inflate_screen(MAP ## _cmpr_dict, MAP, BUF, MAP ## _w, X, Y);

void map_inflate(const uint8_t * dict, const uint16_t * in, uint8_t * out, uint16_t data_size,
		 uint8_t w);
void __map_inflate_screen(const uint8_t * dict, const uint16_t * in, uint8_t * out, uint8_t w, uint8_t vpx, uint8_t vpy);

#endif
