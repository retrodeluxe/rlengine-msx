/*
 * RetroDeLuxe Engine for MSX
 *
 * Copyright (C) 2020 Enric Martin Geijo (retrodeluxemsx@gmail.com)
 *
 * RDLEngine is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <png.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <getopt.h>
#include <libgen.h>
#include <string.h>

#define PALSIZE 16
#define MAX_SCR2_SIZE   6144

struct rgb {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct scr2 {
    uint8_t patrn;
    uint8_t color;
};

struct fbit {
    uint8_t color;
};

struct rgb tms9918_pal[PALSIZE]= {
    { 0  ,0  ,0  },             /* 0 Transparent                   */
    { 0  ,0  ,0  },             /* 1 Black           0    0    0   */
    { 33 ,200,66 },             /* 2 Medium green   33  200   66   */
    { 94 ,220,120},             /* 3 Light green    94  220  120   */
    { 84 ,85 ,237},             /* 4 Dark blue      84   85  237   */
    { 125,118,252},             /* 5 Light blue    125  118  252   */
    { 212,82 ,77 },             /* 6 Dark red      212   82   77   */
    { 66 ,235,245},             /* 7 Cyan           66  235  245   */
    { 252,85 ,84 },             /* 8 Medium red    252   85   84   */
    { 255,121,120},             /* 9 Light red     255  121  120   */
    { 212,193,84 },             /* A Dark yellow   212  193   84   */
    { 230,206,128},             /* B Light yellow  230  206  128   */
    { 33 ,176,59 },             /* C Dark green     33  176   59   */
    { 201,91 ,186},             /* D Magenta       201   91  186   */
    { 204,204,204},             /* E Gray          204  204  204   */
    { 255,255,255}              /* F White         255  255  255   */
};

struct png_header
{
	png_uint_32 width;
	png_uint_32 height;
	int bit_depth;
	int color_type;
	int interlace_method;
	int compression_method;
	int filter_method;
};

#define IMAGE_SIZE_B  (png_img.width * png_img.height / 8)
#define IMAGE_SIZE  (png_img.width * png_img.height / 2)

png_bytepp png_rows;
int rowbytes;
struct png_header png_img;      /* input image png header        */
struct rgb palette[PALSIZE];    /* target palette                */
struct scr2 *image_out_scr2;    /* image out in scr2 format      */
struct fbit *image_out_4bit;    /* image out in 4bit pal format  */
uint8_t *image_out_8bit;

char *input_file;               /* name of input file used to name data */

int rle_encode = 0;

uint16_t rgb_square_error(uint8_t clr, uint16_t x, uint16_t y)
{
    int16_t u0,u1,u2;
    uint8_t col = image_out_4bit[ x + y * png_img.width].color;

    u0 = (palette[col].r - palette[clr].r);
    u1 = (palette[col].g - palette[clr].g);
    u2 = (palette[col].b - palette[clr].b);
    return u0 * u0 + u1 * u1 + u2 * u2;
}


struct scr2 match_line(uint16_t x,  uint16_t y)
{
    int i;
    uint8_t c1, c2;
    uint16_t xx;
    uint8_t bp = 0, bc = 0;
    uint16_t cp, cs, bs = SHRT_MAX;
    uint16_t mc1, mc2;

    struct scr2 match;

    for (c1 = 1; c1 < 16; c1++) {
        for (c2 = c1 + 1; c2 < 16; c2++) {
            cs = 0; cp = 0;
            for (i = 0; i < 8; i++) {
                xx = ((x << 3) | i);

                mc1 = rgb_square_error(c1, xx, y);
                mc2 = rgb_square_error(c2, xx, y);

                cp = (cp << 1) | (mc1 > mc2);
                cs += (mc1 > mc2) ? mc2 : mc1;

                if (cs > bs) break;   /* error too big already */
            }
            if (cs < bs) {
                bs = cs;              /* best square error */
                bp = cp;              /* best pattern */
                bc = c2 * 16 + c1;    /* best color */
            }
        }
    }

    match.patrn = bp;
    match.color = bc;

    return match;
}

/**
 */
int rgb2msx_scr2_tiles()
{
    uint16_t x, y, j;
    uint16_t yy, qe;
    struct scr2 *dst = image_out_scr2;

    for (y = 0; y < (png_img.height + 7) >> 3; y++) {
        for (x = 0; x < (png_img.width + 7 ) >> 3; x++) {
            /* process 8x8 pixel block */
            for (j = 0; j < 8; j++) {
                yy = ((y << 3) | j);

                *dst++ = match_line(x, yy);
            }
        }
    }
    return 0;
}


uint8_t find_min_sqerr_color(struct rgb *col, struct rgb *pal)
{
    uint8_t i, best = PALSIZE;
    double er, eg, eb, se, least = INT_MAX;

    for (i = 0; i < PALSIZE; i++) {
        er = (col->b - pal[i].r) * 0.4;
        eg = (col->g - pal[i].g) * 0.75;
        eb = (col->r - pal[i].b) * 0.8;
        se = er * er + eg * eg + eb * eb;

        if (se < least) {
            best = i;
            least = se;
        }
    }

    return best;
}

/**
 * We assume the image is indexed 4bit using
 * an MSX1 palette
 */
int rgb2msx_palette()
{
    struct fbit *dst = image_out_4bit;
    int j;

    for (j = 0; j < png_img.height; j++) {
      int i;
      png_bytep row;
      row = png_rows[j];
      for (i = 0; i < rowbytes; i++) {
        png_byte pixel_pair;
	      pixel_pair = row[i];
        uint8_t right = pixel_pair & 0x0F;
        uint8_t left = (pixel_pair & 0xF0) >> 4;

        (dst++)->color = left;
        (dst++)->color = right;
      }
    }
    return 0;
}

int rgb2msx_8bit()
{
    uint8_t *dst = image_out_8bit;
    int j, count = 0;

    for (j = 0; j < png_img.height; j++) {
      int i;
      png_bytep row;
      row = png_rows[j];
      for (i = 0; i < rowbytes; i++) {
        png_byte pixel;
        count++;
	      pixel = row[i];
        *(dst++) = pixel;
      }
    }
    return 0;
}

void usage(void)
{
    int i;

    printf("Usage: png2header [OPTION]... [file.tga]\n"
           "Transform png graphic file into MSX C header\n"
           " -h, --help           Print this help.\n"
           " -f, --full (DEFAULT) full png to scr2/scr5 format conversion,\n"
           " -p, --palette        convert to msx1 palette only,\n"
           " -t, --type=TYPE      output format,\n"
           "                          TILE    : scr2/4 pattern & color C source,\n"
           "                          TILEH   : scr2/4 pattern & color H source (no data),\n"
           "                          SPRITE  : msx1 16x16 sprite planes C source,\n"
           "                          SPRITE2 : msx2 16x16 sprite planes C source (without OR mode),\n"
           "                          SPRITEH : msx1 16x16 sprite planes H source (no data),\n"
           "                          SPRITE2H : msx2 16x16 sprite planes H source (no data),\n"
           "                          SCR5    : scr5 bitmap C source,\n"
           "                          SCR5H   : scr5 bitmap H source (no data),\n"
           " -o, --output=FILE    output file,\n"
           "\n");
}


static int load_png_image(int fileidx, int argc, char **argv)
{
	FILE * fp;
	png_structp png_ptr;
	png_infop info_ptr;
	int j;

	if (argc - fileidx <= 0) {
		fprintf(stderr, "No input file specified\n");
		usage();
		return -1;
	}

	input_file = argv[fileidx];

	fp = fopen(input_file, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open %s file!\n",argv[1]);
		return -1;
	}
	png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		fprintf(stderr, "Cannot create PNG read structure\n");
		return -1;
	}
	info_ptr = png_create_info_struct (png_ptr);
	if (!png_ptr) {
		fprintf(stderr, "Cannot create PNG info structure\n");
	}
	png_init_io (png_ptr, fp);
	png_read_png (png_ptr, info_ptr, 0, 0);
	png_get_IHDR (png_ptr, info_ptr, &png_img.width, &png_img.height,
    &png_img.bit_depth, &png_img.color_type, &png_img.interlace_method,
    &png_img.compression_method, &png_img.filter_method);

	if (png_img.color_type != 3 /* indexed */) {
		fprintf(stderr, "Only PNG indexed color type is supported\n");
		return -1;
	}

	if (png_img.bit_depth != 4 && png_img.bit_depth != 8) {
		fprintf(stderr, "Only indexed PNG with 4bit or 8bit depth is supported, found depth %d\n",png_img.bit_depth);
		return -1;
	}

	png_rows = png_get_rows (png_ptr, info_ptr);
	rowbytes = png_get_rowbytes (png_ptr, info_ptr);

	fclose(fp);

  return 0;
}


void dump_sprite_8x8_block(FILE *fd, struct fbit *base, uint8_t color)
{
    uint8_t i,j, enabled;
    uint8_t byte = 0;

    for (j = 0; j < 8; j++) {
        for (i = 0; i < 8; i++) {
            enabled = (base + i + j * png_img.width)->color == color ? 1 : 0;
            byte = byte << 1 | enabled;
        }
        fprintf(fd, "0x%2.2X,",byte);
    }
    fprintf(fd, "\n");
}

struct pattern_colors {
   uint8_t line_colors[16][8];
   uint8_t n_cols[16];
   uint8_t max_cols;
};


void dump_sprite2_8x8_block(FILE *fd, struct fbit *base, struct pattern_colors *colors, uint8_t cidx, uint8_t offset)
{
    uint8_t i,j, enabled;
    uint8_t byte = 0, color;

    for (j = 0; j < 8; j++) {
        byte = 0;
        if (cidx < colors->n_cols[j + offset]) {
          color = colors->line_colors[j + offset][cidx];
          for (i = 0; i < 8; i++) {
              enabled = (base + i + j * png_img.width)->color == color ? 1 : 0;
              byte = byte << 1 | enabled;
          }
        }
        fprintf(fd, "0x%2.2X,",byte);
    }
    fprintf(fd, "\n");
}

int block_8x8_has_color(struct fbit *base, uint8_t color)
{
    uint8_t i,j;

    for(j=0;j<8;j++) {
        for(i=0;i<8;i++) {
            if ((base + i + j * png_img.width)->color == color)
                return 1;
        }
    }
    return 0;
}

int pattern_has_color(struct fbit *idx, uint8_t color)
{
    return block_8x8_has_color(idx,color) |
           block_8x8_has_color(idx + 8 ,color) |
           block_8x8_has_color(idx + png_img.width * 8,color) |
           block_8x8_has_color(idx + 8 + png_img.width * 8 ,color);
    return 1;
}


int pattern_line_has_color(struct fbit *idx, uint8_t color)
{
  uint8_t i;
  for (i = 0; i < 16; i++) {
    if ((idx + i)->color == color)
      return 1;
  }
  return 0;
}


/**
 * Dump Sprite Mode 2 Data
 */
void dump_sprite_file2(FILE *fd, int only_header)
{
  struct fbit *idx = image_out_4bit;
  uint8_t color, colcnt = 0, np = 0;
  uint16_t line, i;
  char *dataname, *filename, *path;
  struct pattern_colors pcolors[64];

  path = strdup(input_file);
  filename = basename(path);
  dataname = strdup(filename);
  dataname[strlen(dataname)-4]='\0';

  fprintf(fd, "#ifndef _GENERATED_SPRITES_H_%s\n", dataname);
  fprintf(fd, "#define _GENERATED_SPRITES_H_%s\n", dataname);

  if (only_header) {
          fprintf(fd, "extern const unsigned char %s_color[];", dataname);
          fprintf(fd, "extern const unsigned char %s[];\n", dataname);
          fprintf(fd, "#endif\n");
          return;
  }

  fprintf(fd, "const unsigned char %s_color[] = { ", dataname);

  do {
      // ignore transparent color for sprites
      for (color = 1; color < 16; color++) {
        for(line = 0; line < 16; line++) {
          if (pattern_line_has_color(idx + line * png_img.width, color)) {
              pcolors[np].line_colors[line][pcolors[np].n_cols[line]] = color;
              pcolors[np].n_cols[line] = pcolors[np].n_cols[line] + 1;
              if(pcolors[np].n_cols[line] > pcolors[np].max_cols)
                pcolors[np].max_cols = pcolors[np].n_cols[line];
          }
        }
      }
      // print now based on the colors
      for ( i= 0; i < pcolors[np].max_cols; i++) {
        for(line = 0; line < 16; line++) {
          color = 0;
          if (i < pcolors[np].n_cols[line])
            color = pcolors[np].line_colors[line][i];
          fprintf(fd, "%d,", color);
        }
      }

      // we move around the image in blocks of 16x16
      if (++colcnt > (png_img.width / 16) - 1) {
         colcnt = 0;
         idx += png_img.width * 15 + 16;
     } else {
         idx += 16;
     }
      np++;
  } while (idx < image_out_4bit + (png_img.width * png_img.height) - 1);

  fprintf(fd, "0 };\n");

  colcnt = 0; idx = image_out_4bit; np = 0;

  fprintf(fd, "const unsigned char %s[] = {\n", dataname);

  do {
      for ( i= 0; i < pcolors[np].max_cols; i++) {
        fprintf(fd, "/* ---- pattern: %d ---- */\n", np);
        dump_sprite2_8x8_block(fd, idx, &pcolors[np], i,  0);
        dump_sprite2_8x8_block(fd, idx + png_img.width * 8, &pcolors[np], i, 8);
        dump_sprite2_8x8_block(fd, idx + 8, &pcolors[np], i, 0);
        dump_sprite2_8x8_block(fd, idx + 8 + png_img.width * 8, &pcolors[np], i, 8);
      }
      /* move to the next block */
      if (++colcnt > (png_img.width / 16) - 1) {
         colcnt = 0;
         idx += png_img.width * 15 + 16;
      } else {
         idx += 16;
      }
      np++;
  } while (idx < image_out_4bit + (png_img.width * png_img.height) - 1);

  fprintf(fd, "0x00};\n");
  fprintf(fd, "#endif\n");

}


void dump_sprite_file(FILE *fd, int only_header)
{
    struct fbit *idx = image_out_4bit;
    uint8_t color, colcnt = 0, np = 0;
    char *dataname, *filename, *path;
    uint8_t used_colors[16], used_cnt = 0;

    path = strdup(input_file);
    filename = basename(path);
    dataname = strdup(filename);
    dataname[strlen(dataname)-4]='\0';

    fprintf(fd, "#ifndef _GENERATED_SPRITES_H_%s\n", dataname);
    fprintf(fd, "#define _GENERATED_SPRITES_H_%s\n", dataname);

    if (only_header) {
            fprintf(fd, "extern const unsigned char %s_color[];", dataname);
            fprintf(fd, "extern const unsigned char %s[];\n", dataname);
            fprintf(fd, "#endif\n");
            return;
    }

    fprintf(fd, "const unsigned char %s_color[] = { ", dataname);

    /* handle 8x8 sprites in single blocks */
    if (png_img.height == 8) {
      do {
          // ignore transparent color for sprites
          for (color = 1; color < 16; color++) {
              if (block_8x8_has_color(idx,color)) {
                  fprintf(fd, "%d,", color);
              }
          }
         idx += 8;
         np++;
      } while (idx < image_out_4bit + png_img.width - 1);
    } else {
      do {
          // ignore transparent color for sprites
          for (color = 1; color < 16; color++) {
              if (pattern_has_color(idx, color)) {
                  fprintf(fd, "%d,", color);
              }
          }
          if (++colcnt > (png_img.width / 16) - 1) {
             colcnt = 0;
             idx += png_img.width * 15 + 16;
         } else {
             idx += 16;
         }
          np++;
      } while (idx < image_out_4bit + (png_img.width * png_img.height) - 1);
    }

    fprintf(fd, "0 };\n");

    colcnt = 0; idx = image_out_4bit; np = 0;

    fprintf(fd, "const unsigned char %s[] = {\n", dataname);

    /* process 8x8 sprites in single blocks */
    if (png_img.height == 8) {
      do {
          // ignore transparent color for sprites
          for (color = 1; color < 16; color++) {
              if (pattern_has_color(idx, color)) {

                  fprintf(fd, "/* ---- pattern: %d color: %d ---- */\n", np, color);
                  dump_sprite_8x8_block(fd, idx, color);
              }
          }
          /* move to the next block */
          idx += 8;
          np++;
      } while (idx < image_out_4bit + png_img.width - 1);

    } else {
      do {
          // ignore transparent color for sprites
          for (color = 1; color < 16; color++) {
              if (pattern_has_color(idx, color)) {

                  fprintf(fd, "/* ---- pattern: %d color: %d ---- */\n", np, color);

                  dump_sprite_8x8_block(fd, idx, color);
                  dump_sprite_8x8_block(fd, idx + png_img.width * 8, color);
                  dump_sprite_8x8_block(fd, idx + 8, color);
                  dump_sprite_8x8_block(fd, idx + 8 + png_img.width * 8, color);
              }
          }
          /* move to the next block */
          if (++colcnt > (png_img.width / 16) - 1) {
             colcnt = 0;
             idx += png_img.width * 15 + 16;
          } else {
             idx += 16;
          }
          np++;
      } while (idx < image_out_4bit + (png_img.width * png_img.height) - 1);
    }

    fprintf(fd, "0x00};\n");
    fprintf(fd, "#endif\n");

}


void dump_4bitbuffer_rle(struct fbit *buffer, FILE *file, int type)
{
  int16_t curr_byte, prev_byte, cnt = 0;
  uint8_t run_cnt = 0;
  struct fbit *p;

  prev_byte = -1;

  p = buffer;
  while (cnt < IMAGE_SIZE) {
    curr_byte = p->color << 4;
    p++;
    curr_byte |= p->color;
    p++;
    cnt++;
    if (cnt == IMAGE_SIZE)
            fprintf(file,"0x%2.2X};\n\n", curr_byte);
    else
            fprintf(file,"0x%2.2X,", curr_byte);
    if (curr_byte == prev_byte) {
      run_cnt = 0;
      while (cnt < IMAGE_SIZE) {
        curr_byte = p->color << 4;
        p++;
        curr_byte |= p->color;
        p++;
        cnt++;
        if (curr_byte == prev_byte) {
                run_cnt++;
                if (run_cnt == 255) {
                        fprintf(file,"0x%2.2X,", run_cnt);
                        prev_byte = -1;
                        break;
                }
        } else {
                fprintf(file,"0x%2.2X,", run_cnt);
                if (cnt == IMAGE_SIZE)
                        fprintf(file,"0x%2.2X};\n\n", curr_byte);
                else
                        fprintf(file,"0x%2.2X,", curr_byte);
                prev_byte = curr_byte;
                run_cnt = 0;
                break;
        }
      }
    } else {
            prev_byte = curr_byte;
    }
    if (cnt == IMAGE_SIZE) {
            if (run_cnt != 0)
                    fprintf(file,"0x%2.2X};\n\n", run_cnt);
            break;
    }
  }
}

void dump_buffer_rle(struct scr2 *buffer, FILE *file, int type)
{
        int16_t curr_byte, prev_byte, cnt = 0;
        uint8_t run_cnt = 0;
        struct scr2 *p;

        prev_byte = -1;

        p = buffer;
        while (cnt < IMAGE_SIZE_B) {
                if (type == 0)
                        curr_byte = (p++)->patrn;
                else
                        curr_byte = (p++)->color;
                cnt++;
                if (cnt == IMAGE_SIZE_B)
                        fprintf(file,"0x%2.2X};\n\n", curr_byte);
                else
                        fprintf(file,"0x%2.2X,", curr_byte);
                if (curr_byte == prev_byte) {
                        run_cnt = 0;
                        while (cnt < IMAGE_SIZE_B) {
                                if (type == 0)
                                        curr_byte = (p++)->patrn;
                                else
                                        curr_byte = (p++)->color;
                                cnt++;
                                if (curr_byte == prev_byte) {
                                        run_cnt++;
                                        if (run_cnt == 255) {
                                                fprintf(file,"0x%2.2X,", run_cnt);
                                                prev_byte = -1;
                                                break;
                                        }
                                } else {
                                        fprintf(file,"0x%2.2X,", run_cnt);
                                        if (cnt == IMAGE_SIZE_B)
                                                fprintf(file,"0x%2.2X};\n\n", curr_byte);
                                        else
                                                fprintf(file,"0x%2.2X,", curr_byte);
                                        prev_byte = curr_byte;
                                        run_cnt = 0;
                                        break;
                                }

                        }
                } else {
                        prev_byte = curr_byte;
                }
                if (cnt == IMAGE_SIZE_B) {
                        if (run_cnt != 0)
                                fprintf(file,"0x%2.2X};\n\n", run_cnt);
                        break;
                }
        }
}

void dump_tiles(struct scr2 *buffer, FILE *file, int only_header)
{
    struct scr2 *p;
    int bytectr=0, cnt;
    char *dataname, *filename, *path;

    path = strdup(input_file);
    filename = basename(path);
    dataname = strdup(filename);
    dataname[strlen(dataname)-4]='\0';

    fprintf(file,"#ifndef _GENERATED_TILESET_H_%s\n", dataname);
    fprintf(file,"#define _GENERATED_TILESET_H_%s\n", dataname);

    if (only_header) {
            fprintf(file,"extern const unsigned char %s_tile_w;\n", dataname);
            fprintf(file,"extern const unsigned char %s_tile_h;\n", dataname);
            fprintf(file,"extern const unsigned char %s_tile[];\n", dataname);
            fprintf(file,"extern const unsigned char %s_tile_color[];\n",dataname);
            return;
    }

    fprintf(file,"const unsigned char %s_tile_w = %d;\n", dataname, png_img.width / 8);
    fprintf(file,"const unsigned char %s_tile_h = %d;\n", dataname, png_img.height / 8);
    fprintf(file,"const unsigned char %s_tile[]={\n",dataname);

    if (rle_encode)
        dump_buffer_rle(buffer, file, 0);
    else {
            p = buffer;
            for (cnt = 0; cnt < (png_img.width * png_img.height / 8) ; p++, cnt++) {
                if(bytectr++ > 6) {
                    fprintf(file,"0x%2.2X,\n",p->patrn);
                    bytectr=0;
                } else {
                    fprintf(file,"0x%2.2X,",p->patrn);
                }
            }
            fprintf(file,"0x%2.2X};\n\n",p->patrn);
    }
    fprintf(file,"const unsigned char %s_tile_color[]={\n",dataname);

    if (rle_encode)
        dump_buffer_rle(buffer, file, 1);
    else {
            p = buffer;
            for (cnt = 0; cnt < (png_img.width * png_img.height / 8); p++, cnt++) {
                if(bytectr++ > 6) {
                    fprintf(file,"0x%2.2X,\n",p->color);
                    bytectr=0;
                } else {
                    fprintf(file,"0x%2.2X,",p->color);
                }
            }
            fprintf(file,"0x%2.2X};\n\n",p->color);
    }
}

void dump_bitmap(struct fbit *buffer, FILE *file, int only_header)
{
    struct fbit *p;
    int bytectr=0, cnt;
    char *dataname, *filename, *path;

    path = strdup(input_file);
    filename = basename(path);
    dataname = strdup(filename);
    dataname[strlen(dataname)-4]='\0';

    fprintf(file,"#ifndef _GENERATED_BITMAP_H_%s\n", dataname);
    fprintf(file,"#define _GENERATED_BITMAP_H_%s\n", dataname);

    if (only_header) {
            fprintf(file,"extern const unsigned int %s_bitmap_w;\n", dataname);
            fprintf(file,"extern const unsigned char %s_bitmap_h;\n", dataname);
            fprintf(file,"extern const unsigned char %s_bitmap[];\n", dataname);
            return;
    }

    fprintf(file,"const unsigned int %s_bitmap_w = %d;\n", dataname, png_img.width);
    fprintf(file,"const unsigned char %s_bitmap_h = %d;\n", dataname, png_img.height);
    fprintf(file,"const unsigned char %s_bitmap[]={\n",dataname);

    if (rle_encode)
        dump_4bitbuffer_rle(buffer, file, 0);
    else {
            uint8_t scr5pix;
            p = buffer;
            for (cnt = 0; cnt < (png_img.width * png_img.height)/2 ; p++, cnt++) {
                scr5pix = p->color << 4;
                p++;
                scr5pix |= p->color;
                if(bytectr++ > 6) {
                    fprintf(file,"0x%2.2X,\n",scr5pix);
                    bytectr=0;
                } else {
                    fprintf(file,"0x%2.2X,",scr5pix);
                }
            }
            fprintf(file,"0x%2.2X};\n\n",scr5pix);
    }
}

void dump_bitmap_8bit(uint8_t *buffer, FILE *file, int only_header)
{
    uint8_t *p;
    int bytectr=0, cnt;
    char *dataname, *filename, *path;

    path = strdup(input_file);
    filename = basename(path);
    dataname = strdup(filename);
    dataname[strlen(dataname)-4]='\0';

    fprintf(file,"#ifndef _GENERATED_BITMAP_H_%s\n", dataname);
    fprintf(file,"#define _GENERATED_BITMAP_H_%s\n", dataname);

    if (only_header) {
            fprintf(file,"extern const unsigned int %s_bitmap_w;\n", dataname);
            fprintf(file,"extern const unsigned char %s_bitmap_h;\n", dataname);
            fprintf(file,"extern const unsigned char %s_bitmap[];\n", dataname);
            return;
    }
    fprintf(file,"const unsigned int %s_bitmap_w = %d;\n", dataname, png_img.width);
    fprintf(file,"const unsigned char %s_bitmap_h = %d;\n", dataname, png_img.height);
    fprintf(file,"const unsigned char %s_bitmap[]={\n",dataname);
    //if (rle_encode)
    //    dump_4bitbuffer_rle(buffer, file, 0);
    //else {
            uint8_t pix;
            p = buffer;
            for (cnt = 0; cnt < png_img.width * png_img.height ; p++, cnt++) {
                pix = *p;
                if(bytectr++ > 6) {
                    fprintf(file,"0x%2.2X,\n",pix);
                    bytectr=0;
                } else {
                    fprintf(file,"0x%2.2X,",pix);
                }
            }
            fprintf(file,"0x%2.2X};\n\n",pix); // I think this is wrong
    //}
}


void dump_tile_file(FILE *fd, int only_header)
{

    dump_tiles(image_out_scr2, fd, only_header);

    fprintf(fd,"#endif\n");
}

void dump_bitmap_file(FILE *fd, int only_header)
{
  if (png_img.bit_depth == 4) {
    dump_bitmap(image_out_4bit, fd, only_header);
  } else if (png_img.bit_depth == 8) {
    dump_bitmap_8bit(image_out_8bit, fd, only_header);
  }
  fprintf(fd,"#endif\n");
}

int generate_header(char *outfile, char *type)
{
	int do_tile = 0;
	int do_sprite = 0;
  int do_sprite2 = 0;
	int do_only_header = 0;
	int do_scr5 = 0;
	FILE *file;

	if (!strcmp(type, "TILE")) {
		do_tile = 1;
	} else if (!strcmp(type, "SPRITE")) {
		do_sprite = 1;
  } else if (!strcmp(type, "SPRITE2")) {
		do_sprite2 = 1;
	} else if (!strcmp(type, "TILEH")) {
		do_tile = 1;
		do_only_header = 1;
	} else if (!strcmp(type, "SPRITEH")) {
		do_sprite = 1;
		do_only_header = 1;
  } else if (!strcmp(type, "SPRITE2H")) {
		do_sprite2 = 1;
		do_only_header = 1;
	} else if (!strcmp(type, "SCR5")) {
		do_scr5 = 1;
	} else if (!strcmp(type, "SCR5H")) {
		do_scr5 = 1;
		do_only_header = 1;
	} else {
		printf("Unsupported output type.\n");
		usage();
		return -1;
	}

	file = fopen(outfile,"w");
	if (file == NULL) {
		fprintf(stderr, "Cannot open %s file for writing\n",outfile);
		return -1;
	}

	if (do_sprite)
		dump_sprite_file(file, do_only_header);
  else if (do_sprite2)
    dump_sprite_file2(file, do_only_header);
	else if (do_tile)
		dump_tile_file(file, do_only_header);
	else if (do_scr5)
		dump_bitmap_file(file, do_only_header);

	fclose(file);
	return 0;
}

#define OPT_HELP    'h'
#define OPT_FULL    'f'
#define OPT_PALETTE 'p'
#define OPT_TYPE    't'
#define OPT_OUTPUT  'o'
#define OPT_RLE 'z'
#define ALL_OPTIONS \
	{ "help",   0, 0, OPT_HELP }, \
	{ "full",   0, 0, OPT_FULL }, \
	{ "palette", 0, 0, OPT_PALETTE }, \
	{ "type",   1, 0, OPT_TYPE }, \
	{ "output", 1, 0, OPT_OUTPUT }, \
	{ "rle", 0, 0, OPT_RLE }, \

#define OPT_STR "hfpt:o:z"

static const struct option options[] = {
					ALL_OPTIONS
					{ 0, 0, 0, 0},
					};
static const char short_options[] = OPT_STR;

int main(int argc, char **argv)
{
	int do_full = 1;
	int do_palette = 0;
	int fileidx;
	int result = 0;
	char *type = 0;
	char *outfile = 0;
	int opt;

	while ((opt = getopt_long(argc, argv,
			short_options, options, 0)) != -1) {
	switch(opt) {
		case '?':
		case OPT_HELP:
			usage();
			return 0;
		case OPT_FULL:
			do_full = 1;
			break;
		case OPT_PALETTE:
			do_full = 0;
			do_palette = 1;
			break;
		case OPT_TYPE:
			type = optarg;
			break;
		case OPT_OUTPUT:
			outfile = optarg;
			break;
		case OPT_RLE:
			rle_encode = 1;
			break;
		default:
			break;
		}
	}

	fileidx = optind;

	if (type == 0 || outfile == 0) {
		fprintf(stderr, "Need to specify output filename and type\n");
		usage();
		return -1;
	}

	if (!do_palette && !strcmp(type,"SPRITE")) {
		fprintf(stderr, "output type SPRITE requires palette only processing\n");
		usage();
		return -1;
	}

	int i;
	for (i=0; i<PALSIZE; i++) {
		palette[i].r = tms9918_pal[i].r;
		palette[i].g = tms9918_pal[i].g;
		palette[i].b = tms9918_pal[i].b;
	}

	result = load_png_image(fileidx, argc, argv);

	if (!strcmp(type,"TILE") && ((png_img.width / 8 * png_img.height > 2048) ||
		(png_img.width % 8 != 0 || png_img.height % 8 != 0))) {
			fprintf(stderr, "When generating TILE output, \
				input file size must have width and heigth multiple of 8 and be smaller than 256x64 pixels (16Kb).\n");
			return -1;
	}

	/** process image **/
  int imagesize = png_img.width * png_img.height;
  if (png_img.bit_depth == 4) {
    image_out_4bit = malloc(imagesize * sizeof(struct fbit));
    image_out_scr2 = malloc(MAX_SCR2_SIZE * sizeof(struct scr2));
  } else if (png_img.bit_depth == 8) {
    image_out_8bit = malloc(imagesize * sizeof(uint8_t));
  }

	// if source is rgb and output is scr2/scr4
	if ((result == 0) && do_full) {
    if (png_img.bit_depth == 4) {
      result = rgb2msx_palette();
      result = rgb2msx_scr2_tiles();
   } else if (png_img.bit_depth == 8) {
      result = rgb2msx_8bit();
   }
 }
  // sprites
	if ((result == 0) && do_palette && png_img.bit_depth == 4) {
	 	result = rgb2msx_palette();
	}

	if (result == 0) {
		result = generate_header(outfile, type);
	}

	fflush(stdout);
	fflush(stderr);
	return result;
}
