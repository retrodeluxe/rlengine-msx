#!/usr/bin/env python2.7
import json
import sys
import os
import ntpath
from optparse import OptionParser

class PaletteReader:
    """ reads a GIMP palette file in txt format """
    def __init__(self, filename):
        self.filename = filename;
        self.pal = []
        self.color = 0

    def read(self):
        file = open(self.filename)
        lines = file.readlines()
        for line in lines:
            # extract and transform to 9-rgb
            r = int(line[1:2], 16) / 2
            g = int(line[3:4], 16) / 2
            b = int(line[5:6], 16) / 2
            self.pal.append({'r': r, 'g' : g, 'b': b})
            self.color = self.color + 1

    def write(self, output):
        filename, file_extension = os.path.splitext(output)
        basepath = ntpath.basename(output)
        basename, base_extension = os.path.splitext(basepath)
        fout = open(filename + '.h', 'w+')

        print >>fout,("#ifndef __PALETTE_%s_H" % basename.upper())
        print >>fout,("#define __PALETTE_%s_H" % basename.upper())
        print >>fout,("const uint8_t %s_palette[] = {" % basename)
        for color in self.pal:
            print >>fout,("%d,%d,%d," % (color['r'], color['g'], color['b']))
        print >>fout,"};"
        print >>fout,"#endif"

if __name__ == '__main__':

    parser = OptionParser()
    parser.add_option("-s", "--source", dest="source", action="store", default=None,
                        help="Source file containing the palette")
    parser.add_option("-o", "--output", dest="output", action="store", default=None,
                        help="Basename of output header files containing the data")

    (opts, args) = parser.parse_args()
    if not opts.source:
        print ("required source")
        sys.exit(1)

    pal = PaletteReader(opts.source)
    pal.read()
    pal.write(opts.output)
