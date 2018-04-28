/*
 * RetroDeLuxe Engine for MSX
 *
 * Copyright (C) 2018 Enric Martin Geijo (retrodeluxemsx@gmail.com)
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

/*
 * Trilo Tracker TMU to Header processor
 *
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <getopt.h>
#include <libgen.h>
#include <string.h>


#define VERSION "0.1"

struct tmu_header {
    uint8_t version:4;
    uint8_t chipset:4;
        #define CHIPSET_PSG_SCC     0
        #define CHIPSET_PSG_FM      1
        #define CHIPSET_SN_FM       3
        #define CHIPSET_PSG_SN_EPSG 8
        #define LABEL_LEN 32
    char name[LABEL_LEN];
    char author[LABEL_LEN];
    uint8_t speed;
    uint8_t seq_loop;
    uint8_t seq_lenght;
};

struct tmu_instrument_name {
        #define INSTR_NAME_LEN      16
    char name[INSTR_NAME_LEN];
};

struct tmu_instrument_macro_info {
    uint8_t lenght;
    uint8_t restart;
    uint8_t waveform;
};

struct tmu_instrument_macro {
    struct tmu_instrument_macro_info info;
        #define MACRO_DATA_LEN      32*4
    uint8_t data[MACRO_DATA_LEN];
};

struct tmu_instrument {
        #define MAX_INSTR   31
    struct tmu_instrument_name name[MAX_INSTR];
    struct tmu_instrument_macro macro[MAX_INSTR];
};

struct tmu_cmd {
    uint8_t note;
    uint8_t instr;
    uint8_t vol :4;
    uint8_t cmd :4;
    uint8_t param;
};

struct tmu_step {
        #define TMU_CHANNELS    8
    struct tmu_cmd channel[TMU_CHANNELS];
};

struct tmu_pattern {
        #define PATTERN_STEPS   64
    struct tmu_step step[64];
};

/* song data */

#define MAX_PATTERNS 128

struct tmu_header header;

#define SEQUENCE_LEN 256
uint8_t tmu_sequence[SEQUENCE_LEN];

struct tmu_instrument instruments;
struct tmu_pattern patterns[MAX_PATTERNS];

#define SIZE_WAVEFORMS 1024
uint8_t waveforms[SIZE_WAVEFORMS];

void usage(void)
{
  int i;

  printf("Usage: tmu2header [OPTION]... [file.tmu]\n"
         "Transforms a Trilo Tracker Music file into a MSX C header\n"
         " -h, --help           Print this help.\n"
         " -o, --output=FILE    output file,\n"
         "\n");
}

int load_tmu(char *file)
{
    FILE *tmu;
    uint16_t nr;

    tmu = fopen(file, "rb");
    if (tmu == NULL) {
        fprintf(stderr, "Cannot open %s file.\n", file);
        return -1;
    }

    /* read song header */
    nr = fread(&header, sizeof(struct tmu_header), 1, tmu);
    if (nr != 1) {
        fprintf(stderr, "Unable to read file header.\n");
        return -1;
    }

    printf("TMU2Header version %s\n", VERSION);
    printf("> -----------------------------------------\n");
    printf("> file : %s\n", file);
    printf("> version : %d\n", header.version);
    printf("> chipset : %d\n", header.chipset);
    printf("> song name : %s\n", header.name);
    printf("> song by   : %s\n", header.author);
    printf("> -----------------------------------------\n");
    printf("> speed : %d\n", header.speed);
    printf("> restart : %d\n", header.seq_loop);
    printf("> order lenght : %d\n", header.seq_lenght);
    printf("> order sequence : ");

    nr = fread(&tmu_sequence, sizeof(uint8_t), header.seq_lenght, tmu);
    if (nr != header.seq_lenght) {
        fprintf(stderr, "Unable to read file header.\n");
        return -1;
    }

    for (int i = 0; i < header.seq_lenght; i++) {
        printf(" %2d ", tmu_sequence[i]);
    }
    printf("\n");
    printf("> -----------------------------------------\n");

    /* read instrument names */
    nr = fread(instruments.name, sizeof(struct tmu_instrument_name), MAX_INSTR, tmu);
    if (nr != MAX_INSTR) {
        fprintf(stderr, "Unable to read instrument names.\n");
        return -1;
    }
    printf("> instruments : \n");
    for (int i = 0; i < MAX_INSTR; i++) {
         instruments.name[i].name[15] = '\0';
         printf("> %d - %s \n", i, instruments.name[i].name);
     }

    /* read instrument macros */
    for (int i = 0; i < MAX_INSTR; i++) {
        nr = fread(&instruments.macro[i].info, sizeof(struct tmu_instrument_macro_info), 1, tmu);
        if (nr != 1) {
            fprintf(stderr, "Unable to read instrument macros.\n");
            return -1;
        }
        printf("> instr %d, len: %d\n", i , instruments.macro[i].info.lenght);
        nr = fread(instruments.macro[i].data, sizeof(uint8_t), instruments.macro[i].info.lenght * 4 , tmu);
        if (nr != instruments.macro[i].info.lenght * 4) {
            fprintf(stderr, "Unable to read instrument macros.\n");
            return -1;
        }
    }

    /* read waveforms */
    if (header.chipset == CHIPSET_PSG_SCC || header.chipset == CHIPSET_SN_FM) {
        nr = fread(waveforms, sizeof(uint8_t), SIZE_WAVEFORMS, tmu);
        if (nr != SIZE_WAVEFORMS) {
            fprintf(stderr, "Unable to read waveforms.\n");
            return -1;
        }
    }

    /* read patterns */
    uint8_t pattern_number, *dest, c;
    uint16_t len, count;

    do {
        nr = fread(&pattern_number, sizeof(uint8_t), 1, tmu);
        if (nr != 1) {
            goto err;
        }

        printf("> pattern : %d\n", pattern_number);

        if (pattern_number < 128) {
            dest = (uint8_t *)&patterns[pattern_number];
            nr = fread(&len, sizeof(uint16_t), 1, tmu);
            if (nr != 1) {
                goto err;
            }
            printf("> lenght : %d\n", len);
            count = 0;

            do {
                nr = fread(&c, sizeof(uint8_t), 1, tmu);
                if (nr != 1) {
                    goto err;
                }
                //printf(" %2x ", c);

                len--;
                if (c > 0) {
                    *dest++ = c;
                    count++;
                } else {
                    /* process compressed empty areas */
                    nr = fread(&c, sizeof(uint8_t), 1, tmu);
                    if (nr != 1) {
                        goto err;
                    }
                    len--;
                    //printf("> compressed size : %d\n", c);
                    if (c == 0) {
                        //printf("> remaining lenght : %d\n", len);
                        printf("> realised count : %d\n", count);
                        break;
                    } else {
                        //printf("> uncompressing : %d\n", c);
                        while (c-- > 0) {
                            //printf(" 00 ");
                            *dest++ = 0;
                            count++;
                        }
                    }
                }
            } while (len > 0);
            //printf("\n");
        }
    } while (pattern_number != 255);

    printf("> -----------------------------------------\n");
    fclose(tmu);
    return 0;

err:
    fclose(tmu);
    fprintf(stderr, "Unable to read patters.\n");
    return -1;
}


int main(int argc, char **argv)
{
  char *output_file = NULL;
  char *input_file = NULL;
  int opt;

  #define OPT_HELP    'h'
  #define OPT_OUTPUT  'o'
  #define ALL_OPTIONS \
      { "help",   0, 0, OPT_HELP }, \
      { "output", 1, 0, OPT_OUTPUT }, \

  #define OPT_STR "ho:"

  static const struct option options[] = {
      ALL_OPTIONS
      { 0, 0, 0, 0},
  };
  static const char short_options[] = OPT_STR;

  while ((opt = getopt_long(argc, argv, short_options,
      options, 0)) != -1) {
      switch(opt) {
          case '?':
          case OPT_HELP:
              usage();
              return 0;
          case OPT_OUTPUT:
              output_file = optarg;
              break;
      default:
          break;
      }
  }

  if (output_file == NULL) {
      fprintf(stderr, "Need to specify output filename\n");
      usage();
      return -1;
  }

  if (argc - optind <= 0) {
      fprintf(stderr, "No input file specified\n");
      usage();
      return -1;
  }

  input_file = argv[optind];

  load_tmu(input_file);

  return 0;
}
