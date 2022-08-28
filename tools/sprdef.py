#!/usr/bin/env python3
#
#   RetroDeLuxe Engine for MSX
#
#   Copyright (C) 2022 Enric Martin Geijo (retrodeluxemsx@gmail.com)
#
#   RDLEngine is free software: you can redistribute it and/or modify it under
#   the terms of the GNU General Public License as published by the Free
#   Software Foundation, version 2.
#
#   This program is distributed in the hope that it will be useful, but WITHOUT
#   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
#   details.
#
#   You should have received a copy of the GNU General Public License along with
#   this program; If not, see <http://www.gnu.org/licenses/>.
#
import json
import ntpath
import os
import sys
from optparse import OptionParser


class SpriteDefs:
    """ contains sprit definitions data """

    def __init__(self, raw_defs):
        self.defs = raw_defs
        # TODO: validate JSON

class SpriteDefWriter:
    """ writes a sprite definition header """

    def __init__(self, defs, output):
        self.output = output
        self.defs = defs
        filename, file_extension = os.path.splitext(self.output)
        self.filename = filename
        self.basepath = ntpath.basename(self.output)
        basename, base_extension = os.path.splitext(self.basepath)
        self.basename = basename

    def generate_headers(self):
        """
        write down header data
        """
        filename, file_extension = os.path.splitext(self.output)
        basepath = ntpath.basename(self.output)
        basename, base_extension = os.path.splitext(basepath)

        fout = open(filename + '.h', 'w+')
        #basename = basename

        print(("#ifndef __SPR_INIT_H"), file=fout)
        print(("#define __SPR_INIT_H"), file=fout)

        ## Enumerate pattern names 
        print(("enum spr_patterns_t {"), file=fout)
        for spr in self.defs.defs["sprites"]:
            print(("\t%s," % spr["pattern_name"]), file=fout)
        print(("};"), file=fout)

        # Add preamble in case this file is included in the main module
        print(("void main();\n"), file=fout)
        print(("void init_spr() {\n main();\n}"), file=fout)

        # define constants for the states for each def
        for spr in self.defs.defs["sprites"]:
            print(("const uint8_t %s_states[] = {" % spr["data"]), file=fout, end="")
            for state in spr["states"]:
                print(("%s," % state["frames"]), file=fout, end="")
            print(("};"), file=fout)

        # Add sprite defs initiliazation function
        print(("void spr_load_defs() {"), file=fout)
        for spr in self.defs.defs["sprites"]:
            print(("\tSPR_DEFINE_PATTERN_SET(%s, SPR_SIZE_%s, %s, %s, %s_states, %s);" % (spr["pattern_name"],
                                                                                 spr["size"],
                                                                                 spr["planes"],
                                                                                 len(spr["states"]),
                                                                                 spr["data"],
                                                                                 spr["data"])), file=fout)
        print(("}"), file=fout)

        print(("#endif"), file=fout)

class SpriteDefJsonReader:
    """ reads a sprite def json file"""

    def __init__(self, filename):
        self.filename = filename

    def read(self):
        self.data = open(self.filename)
        self.decoded = json.load(self.data)
        return SpriteDefs(self.decoded)

if __name__ == '__main__':

    parser = OptionParser()
    parser.add_option("-s", "--source", dest="source", action="store", default=None,
                      help="Source json file containing the tilemap")
    parser.add_option("-o", "--output", dest="output", action="store", default=None,
                      help="Basename of output header files containing the data")

    (opts, args) = parser.parse_args()
    if not opts.source:
        print("required source")
        sys.exit(1)

    reader = SpriteDefJsonReader(opts.source)
    writer = SpriteDefWriter(reader.read(), opts.output)
    writer.generate_headers()