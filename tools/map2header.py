#!/usr/bin/env python
#######
#
# Read json tile data from Tiled and generate a C header
#
# RetroDeluxe 2012
#
#  it expects
#    - N layers of background tiles that having a name containing the string
#        "tileset"
#    - N layers of objects
#
#  it generates
#    - a header file containing const byte arrays named using the name of the
#      file + the name of the layers.
#    - background tiles are enconded as an array (compressed or not)
#    - object layers are encoded as 4-uples
#             x,y coordinates {byte, byte}
#             bank {0,1,2}
#             source tile {0-255}
#
#  tile map dimensions are limited by the size of the decompression buffer,
#  also by the fact the coordinates are 8bit long.
#
#         - max number of screens in either x or y axis is 8
#         - max decompr buff size is considered to be 1 memory page (16Kb)
#
#             21 -  7 x 3 = 16128 bytes
#             16 -  4 x 4 = 12288 bytes
#              9 -  3 x 3 = 6912 bytes
#              4 -  2 x 2 = 3072 bytes
#
#  21 screens for a single map is quite big, if the compression is good
#   we can probaby fit 8 of those maps (Atlas) in a 32Kb rom; giving us 160 screens
#   which is a prety decent size.
#
#  TODO: It is possible to use Tiled to generate a full Atlas in one go.
#        Empty tiles are marked as zeros; this can be used to transform
#        from Atlas coordinates to map coordinates.
#

import json
import sys
import os
from optparse import OptionParser

class TileObject:
        """ Contains a 4x4 tile object
             described by the left-top position in the map and the
             left top tile number.
        """
        def __init__(self,x,y,tile):
            self.x = x;
            self.y = y;
            self.tile = (tile % 256) - 1;
            self.ct = 1

        def add(self,x,y,tile):
            """ check if new tile fits in this object """
            if self.is_full():
                return False
            if (x == self.x or x == self.x + 1) and (y == self.y or y == self.y + 1):
                self.ct+=1
                return True
            return False

        def is_full(self):
            """ if 4 items have already been added, we are full """
            if self.ct == 4:
                return True

        def dump(self):
            print ("{%s,%s,%s}" % (self.x, self.y, self.tile)),

class Tilemap:
        """ contains a tilemap, as used in the app """
        def __init__(self):
            pass

        def __setitem__(self,key,value):
            self.__dict__[key]=value

        def __getitem__(self,key):
            return self.__dict__[key]

class TileMapWriter:
        """ writes a tile map to set of C header files

        """
        def __init__(self,tilemap,rle,block):
            self.tilemap = tilemap
            self.rle = rle
            self.block = block
            self.block_dict = {}

        def _lookup_dict(self,block):
            key=str(block[0])+'_'+str(block[1])+'_'+str(block[2])+'_'+str(block[3])
            if self.block_dict.has_key(key):
                return self.block_dict[key]
            else:
                new_idx = len(self.block_dict)+1
                self.block_dict[key] = new_idx
                return new_idx

        def _replace_dict(self, buf):
            """ replace dict items """
            out_buf = []
            w = self.tilemap['width']
            h = self.tilemap['height']
            for i in range(0,h,2):
                for j in range(0,w,2):
                    block = [buf[i*w+j],buf[i*w+j+1],buf[(i+1)*w+j],buf[(i+1)*w+j+1]]
                    out_buf.append(self._lookup_dict(block) - 1) # need to adjust to zero
                    #print "BLOCK %s %s -- %s" % (i,j,self._lookup_dict(block))
            return out_buf

        def _compress_rle(self,buf):
            cnt = 0
            size = len(buf)
            buf_out = []
            while cnt < size:
                idx = cnt
                val = buf[idx]
                idx+=1
                while idx < size and idx - cnt < 127 and buf[idx] == val:
                    idx+=1
                if idx - cnt == 1:
                    while idx < size and idx - cnt < 127 and  \
                        (buf[idx] != buf[idx-1] or idx > 1 and buf[idx]!=buf[idx-2]):
                        idx+=1
                    while idx < size and buf[idx] == buf[idx-1]:
                        idx-=1
                    buf_out.append(cnt - idx)
                    for i in range(cnt,idx):
                        buf_out.append(buf[i])
                else:
                    buf_out.append(idx - cnt)
                    buf_out.append(val)
                cnt=idx
            return buf_out

        def _expand_block_keys(self):
            lo = []
            # careful, dicts are not sorted
            for idx in range(0,len(self.block_dict) + 1):
                for key in self.block_dict.keys():
                    if self.block_dict[key] == idx:
                        vals = key.rsplit('_')
                        a = int(vals[0])
                        b = int(vals[1])
                        c = int(vals[2])
                        d = int(vals[3])
                        lo.extend([a,b,c,d])
            return lo

        def mod_256(self, x):
            # tile indexes from Tiled are expressed as int + 1
            # we need to reference a single bank w.r.t 0
            return (x % 256)

        def compress_layer(self,layer):
            #print ("DEBUG: size before dict replace : %s" % len(layer))
            if self.block:
                cmpr1 = self._replace_dict(map(self.mod_256,layer))
                #print ("DEBUG: size after dict replace : %s" % len(cmpr1))
            else:
                cmpr1 = layer

            if self.rle:
                cmpr2 = self._compress_rle(cmpr1)
                #print ("DEBUG: size RLE compression : %s" % len(cmpr2))
            else:
                cmpr2 = cmpr1
            compr = []
            if self.block:
                #print ("DICT : %s =" % self._expand_block_keys())
                compr.extend(self._expand_block_keys())
                return (self._expand_block_keys(), cmpr1)

            #print ("MAP : %s = " % cmpr2)
            #compr.extend(cmpr2)
            return (None, compr1)

        def dump(self):
            object_types = {}
            special_properties = {}
            max_object_size = 1;
            for layer in self.tilemap['layers']:
                if 'objectgroup' in layer['type']:
                 ## Scan object types
                    for item in layer['objects']:
                        # find all types or names
                        _type = item['type']
                        if _type == '':
                            _type = item['name']
                        object_types[_type] = item['properties'].keys()
                        length = len(item['properties'].keys())
                        if length > max_object_size:
                            max_object_size = length
                        ## find special properties that require enums
                        for _property in item['properties'].keys():
                            value = item['properties'][_property]
                            if (not value.isdigit() and not value.replace('.','').isdigit()):
                                if not _property in special_properties:
                                    special_properties[_property] = {}
                                special_properties[_property][value.encode('ascii','ignore')] = '1'

            ## dump defines to identify each object type
            if len(object_types.keys()) > 0:
                print "\nenum object_type {"
                for key in object_types.keys():
                    print ("        %s, " % key.upper())
                print ("};\n")

            ## dump enums to identify certain string properties
            for key in special_properties:
                print ("\nenum object_property_%s {" % key.encode('ascii', 'ignore'))
                for _property in special_properties[key]:
                    print ("    %s_%s, " % (key.encode('ascii', 'ignore').upper(), _property.upper()))
                print ("};\n")

            ## dump object structures
            for key in object_types.keys():
                ## here keys and properties cannot be C keywords
                print ("struct map_object_%s {" % key)
                if len(object_types[key]) > 0:
                    for _property in object_types[key]:
                        ## TODO filter C Keywords
                        if _property == 'static':
                            _property = 'static_'
                        if _property in special_properties:
                            print ("     enum object_property_%s %s;" % (_property, _property))
                        else:
                            print ("     unsigned char %s;" % _property)
                else:
                    print ("     unsigned char dummy;")
                print ("};\n")

            if len(object_types.keys()) > 0:
                print ("union map_object {")
                for key in object_types.keys():
                    if key == "static":
                        _key = "static_"
                    else:
                        _key = key
                    print ("    struct map_object_%s %s;" % (key, _key))
                print ("};")

            print "struct map_object_item {"
            print ("    enum object_type type;")
            print ("    unsigned char x;")
            print ("    unsigned char y;")
            print ("    unsigned char w;")
            print ("    unsigned char h;")
            print ("    unsigned char visible;")
            if len(object_types.keys()) > 0:
                print ("    union map_object object;")
            print "};\n"

            # different type of dump depending of type
            for layer in self.tilemap['layers']:
                ## first dump tile layers
                if 'tilelayer' in layer['type']:
                    w = layer['width']
                    h = layer['height']
                    name = layer['name'].replace(' ','_')
                    print ("const unsigned char %s_w = %s;" % (name,w))
                    print ("const unsigned char %s_h = %s;" % (name,h))
                    if self.rle or self.block:
                        # if compressed, need additional size data
                        #
                        (_dict, compr_layer) = self.compress_layer(layer['data'])
                        print ("const unsigned char %s_dict_size = %s;" % (name, len(_dict)))
                        print ("const unsigned int %s_cmpr_size = %s;" % (name, len(compr_layer)))
                        print ("const unsigned char %s_cmpr_dict[] = {" % name)
                        for tile in _dict:
                            print ("%s," % tile ),
                        print "0 };"
                        print ("const unsigned char %s[] = {" % name)
                        for tile in compr_layer:
                            print ("%s," % tile ),
                        print "0 };"
                    else:
                        print ("const unsigned char %s[] = {" % name)
                        for tile in layer['data']:
                            val = (tile % 256)
                            print ("%s," % val),
                        print "0 };"
                elif 'objectgroup' in layer['type']:
                    count = 0
                    name = layer['name'].replace(' ','_')
                    print ("/* cast to map_object_item */")
                    print ("const unsigned char %s_nitems = %s;" % (name, len(layer['objects'])))
                    print ("const unsigned char %s[] = {" % name)
                    for item in layer['objects']:
                        print ("    /* object %s */" % count)
                        #print ("    {")
                        _type = item['type']
                        if _type == '':
                            _type = item['name']
                        print ("    %s, %s, %s, %s, %s, %s," % (_type.upper(),  item['x'] % 256,  item['y'] % 176,  item['width'], item['height'], 1 if item['visible'] else 0))
                        #print ("        { ");
                        #print ("        { .%s = { " % _type);
                        #print item['properties']
                        for _property in item['properties'].keys():
                            value = item['properties'][_property]
                            if _property in special_properties and not value.isdigit():
                                print ("    %s_%s, " % (_property.upper(), value.upper()))
                            elif not value.isdigit() and value.replace('.','').isdigit:
                                print ("    %s, " % (value.replace('.','')))
                            elif value.isdigit():
                                wrap = int(value) % 256
                                ## regular numeric value, wrapped to byte
                                print ("    %s, " % wrap)
                            else:
                                print ("    %s, " % value)
                        ## add at least one empty prop
                        if len(item['properties'].keys()) == 0:
                            print ("        0,")
                        ## add padding if needed
                        #padding = max_object_size - len(item['properties'].keys())
                        #for i in xrange(padding):
                        #    print ("            0,")
                        #print ("        },");
                        #print ("    },")
                        count = count + 1
                    print "};"

class TiledJsonReader:
        """ reads a json file saved from tiled"""
        def __init__(self, filename):
            self.filename = filename;
            self.data = open(filename)
            self.decoded = json.load(self.data)
            self.tilemap = Tilemap()

        def read(self):
            self.tilemap['version'] = self.decoded['version']
            self.tilemap['orientation'] = self.decoded['orientation']
            self.tilemap['tilewidth'] = self.decoded['tilewidth']
            self.tilemap['tileheight'] = self.decoded['tileheight']
            self.tilemap['width'] = self.decoded['width']
            self.tilemap['height'] = self.decoded['height']
            self.tilemap['tilesets'] = self.decoded['tilesets']
            self.tilemap['layers'] = self.decoded['layers']
            return self.tilemap


def dump_header(filename):
    name, ext = os.path.splitext(os.path.basename(filename))
    print ("#ifndef __MAP_DATA_%s" % name)
    print ("#define __MAP_DATA_%s" % name)

if __name__ == '__main__':

    parser = OptionParser()
    parser.add_option("-s", "--source", dest="source", action="store", default=None,
                        help="Source json file containing the tilemap")
    parser.add_option("-r", "--rle", dest="rle", action="store_true",
                       default=False, help="Compress tilemap data using RLE")
    parser.add_option("-b", "--block", dest="block", action="store_true",
                       default=False, help="Compress tilemap data using 4x4 block replacement dictionary")

    (opts, args) = parser.parse_args()
    if not opts.source:
        print "required source"
        sys.exit(1)

    dump_header(opts.source)
    reader = TiledJsonReader(opts.source)
    writer = TileMapWriter(reader.read(),opts.rle, opts.block)
    writer.dump()
    print "#endif"
