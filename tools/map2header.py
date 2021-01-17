#!/usr/bin/env python3
#
#   RetroDeLuxe Engine for MSX
#
#   Copyright (C) 2017 Enric Martin Geijo (retrodeluxemsx@gmail.com)
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


class MapObject:
    """
    Contains a Tiled Map object
    """

    def __init__(self, raw):
        self.w = raw['width']
        self.h = raw['height']
        self.x = raw['x']
        self.y = raw['y']
        self.name = raw['name'].replace(' ', '_')
        # self.id = raw['id']
        self.visible = raw['visible']
        self.type = raw['type']
        # self.rotation = raw['rotation']
        self.properties = {}
        if 'properties' in raw:
            self.properties = raw['properties']

    def dump(self):
        print("\tmapobject : [%s] [%s] [%s] (%s, %s)" % (self.id, self.name, self.type, self.x, self.y))


class TileLayer:
    """
    Contains a Tiled layer that contains tiles
    """

    def __init__(self, raw, seg_w, seg_h, compr, segment):
        self.w = raw['width']
        self.h = raw['height']
        self.x = raw['x']
        self.y = raw['y']
        self.name = raw['name']
        self.opacity = raw['opacity']
        self.visible = raw['visible']
        self.data = raw['data']
        self.seg_h = seg_h
        self.seg_w = seg_w
        self.compr = compr
        self.segment = segment
        if not self.segment:
            if self.compr == 'rle':
                self.mode = 1
            elif self.compr == 'block':
                self.mode = 2
            else:
                self.mode = 0
        else:
            if self.compr == 'rle':
                self.mode = 3
            elif self.compr == 'block':
                self.mode = 4
            else:
                self.mode = 5
        self.compress_all()

    def compress_all(self):
        # Compress RLE the whole map (cmpr ratio ~2)
        self.data_rle = self.compress_rle(self.data)

        # Split in rooms and compress each RLE (compr ratio ~2)
        self.data_rooms = self.room_split(self.data, self.seg_w, self.seg_h)
        self.data_rooms_rle = []
        for room in self.data_rooms:
            self.data_rooms_rle.append(self.compress_rle(room))

        # Compress the whole map using 4x4 blocks (compr ratio ~2 to 4)
        #   note: if the dictionary is bigger than 255 compression drops 50% because we need
        #   one additional byte to store each block
        self.block_dict = {}
        self.data_compr_4x4 = self.compress_4x4_dict(self.data, self.w, self.h)
        self.data_compr_4x4_dict = self.block_dict

        # Split in rooms and compress each using 4x4 blocks
        #   in most cases this is the most efficient compression, with compr ratio of ~4
        #   regardless of the size of the whole map.
        self.data_rooms_compr_4x4 = []
        self.data_rooms_compr_4x4_dict = []
        for block in self.data_rooms:
            self.block_dict = {}
            self.data_rooms_compr_4x4.append(self.compress_4x4_dict(block, self.seg_w, self.seg_h))
            self.data_rooms_compr_4x4_dict.append(self.block_dict)

    def room_split(self, buf_in, w, h):
        buf_out = []
        for by in range(0, int(self.h / h)):
            for bx in range(0, int(self.w / w)):
                offset = bx * w + by * h * self.w
                block = []
                for y in range(0, h):
                    for x in range(0, w):
                        block.append(buf_in[offset + x + y * self.w])
                buf_out.append(block)
        return buf_out

    def compress_rle(self, bufin):
        cnt = 0
        size = len(bufin)
        bufout = []
        while cnt < size:
            idx = cnt
            val = bufin[idx]
            idx += 1
            while idx < size and idx - cnt < 255 and bufin[idx] == val:
                idx += 1
            if idx - cnt == 1:
                while idx < size and idx - cnt < 255 and \
                        (bufin[idx] != bufin[idx - 1] or idx > 1
                         and bufin[idx] != bufin[idx - 2]):
                    idx += 1
                while idx < size and bufin[idx] == bufin[idx - 1]:
                    idx -= 1
                bufout.append(cnt - idx)
                for i in range(cnt, idx):
                    bufout.append(bufin[i])
            else:
                bufout.append(idx - cnt)
                bufout.append(val)
            cnt = idx
        return bufout

    def lookup_4x4_dict(self, block):
        key = str(block[0]) + '_' + str(block[1]) + '_' + str(block[2]) + '_' + str(block[3])
        if key in self.block_dict:
            return self.block_dict[key]
        else:
            new_idx = len(self.block_dict) + 1
            self.block_dict[key] = new_idx
            return new_idx

    def compress_4x4_dict(self, buf, w, h):
        bufout = []
        for i in range(0, h, 2):
            for j in range(0, w, 2):
                block = [buf[i * w + j], buf[i * w + j + 1],
                         buf[(i + 1) * w + j], buf[(i + 1) * w + j + 1]]
                bufout.append(self.lookup_4x4_dict(block) - 1)  # need to adjust to zero
        return bufout

    def expand_block_keys(self, dict_data):
        lo = []
        for idx in range(0, len(dict_data) + 1):
            for key in list(dict_data.keys()):
                if dict_data[key] == idx:
                    vals = key.rsplit('_')
                    a = int(vals[0])
                    b = int(vals[1])
                    c = int(vals[2])
                    d = int(vals[3])
                    lo.extend([a, b, c, d])
        return lo

    def dump(self):
        print(("tilelayer [%s] : uncompressed [%s] rle [%d]" %
               (self.name, len(self.data), len(self.data_rle))))

    def dump_as_c_header_accessor(self, target_file, basename):
        """
        Provide accessor for map segments (only makes sense in modes 3 and 4)
        """
        if self.mode == 3:
            print("FATAL: not implemented")
            exit(1)
        elif self.mode == 4:
            print(
                ("unsigned char *%s_%s_segment_dict[%s];" % (basename, self.name, len(self.data_rooms_compr_4x4_dict))),
                file=target_file)
            print(("unsigned char *%s_%s_segment[%s];" % (basename, self.name, len(self.data_rooms_compr_4x4))),
                  file=target_file)
            print(("void init_%s_tilelayers(void) {" % basename), file=target_file)
            room_cnt = 0
            for room_dict in self.data_rooms_compr_4x4_dict:
                print(("\t%s_%s_segment_dict[%s] = %s_%s_segment%s_dict;" % (
                basename, self.name, room_cnt, basename, self.name, room_cnt)), file=target_file)
                room_cnt += 1
            room_cnt = 0
            for room_data in self.data_rooms_compr_4x4:
                print(("\t%s_%s_segment[%s] = %s_%s_segment%s;" % (
                basename, self.name, room_cnt, basename, self.name, room_cnt)), file=target_file)
                room_cnt += 1
            print(("}"), file=target_file)
        else:
            pass
        pass

    def dump_as_c_header_no_data(self, file, basename):
        print(("extern const unsigned char %s_%s_w;" % (basename, self.name)), file=file)
        print(("extern const unsigned char %s_%s_h;" % (basename, self.name)), file=file)
        if self.mode == 1:
            # RLE
            print(("extern const unsigned int %s_rle[];" % self.name), file=file)
        elif self.mode == 2:
            # 4x4 Blocks
            print(("extern const unsigned int %s_cmpr_size;" % (self.name)), file=file)
            print(("extern const unsigned char %s_cmpr_dict[];" % self.name), file=file)
            print(("extern const unsigned int %s[];" % self.name), file=file)
        elif self.mode == 3:
            room_cnt = 0
            for room_data in self.data_rooms_rle:
                print(("extern const unsigned int segment%s_%s_rle[];" % (room_cnt, self.name)), file=file)
                room_cnt += 1
        elif self.mode == 4:
            room_cnt = 0
            for room_dict in self.data_rooms_compr_4x4_dict:
                print(("extern const unsigned char %s_%s_segment%s_dict[];" % (basename, self.name, room_cnt)),
                      file=file)
                room_cnt += 1
            room_cnt = 0
            for room_data in self.data_rooms_compr_4x4:
                print(("extern const unsigned char %s_%s_segment%s[];" % (basename, self.name, room_cnt)), file=file)
                room_cnt += 1
        else:
            print(("extern const unsigned char %s_%s[];" % (basename, self.name)), file=file)

    def dump_as_c_header(self, file, basename):
        """ Dump contents of tile layer as a C header.
            mode indicates compression:
            0 : uncompressed
            1 : rle
            2 : 4x4 block compression
            3 : split in rooms and compressed rle
            4 : split in rooms and compress using 4x4 blocks
        """
        total_size = 0
        print(("const unsigned char %s_%s_w = %s;" % (basename, self.name, self.w)), file=file)
        print(("const unsigned char %s_%s_h = %s;" % (basename, self.name, self.h)), file=file)
        if self.mode == 1:
            # RLE
            print(("const unsigned int %s_%s_rle_size = %s;" % (basename, self.name, len(self.data_rle))), file=file)
            print(("const unsigned int %s_%s_rle[] = {" % (basename, self.name)), file=file)
            total_size += len(self.data_rle)
            for tile in self.data_rle:
                print(("%s," % tile), end=' ', file=file)
            print("0 };", file=file)
        elif self.mode == 2:
            # 4x4 Blocks
            if len(self.data_compr_4x4_dict) > 256:
                print("FATAL: block compression dictionary too big")
                exit(1)
            print(("const unsigned int %s_%s_size = %s;" % (basename, self.name, len(self.data_compr_4x4))), file=file)
            print(("const unsigned char %s_%s_dict[] = {" % (basename, self.name)), end=' ', file=file)
            total_size += len(self.expand_block_keys(self.data_compr_4x4_dict))
            for tile in self.expand_block_keys(self.data_compr_4x4_dict):
                print(("%s," % tile), end=' ', file=file)
            print("0 };", file=file)
            print(("const unsigned char %s_%s[] = {" % (basename, self.name)), end=' ', file=file)
            total_size += len(self.data_compr_4x4) * 2
            for tile in self.data_compr_4x4:
                print(("%s," % tile), end=' ', file=file)
            print("0 };", file=file)
        elif self.mode == 3:
            room_cnt = 0
            for room_data in self.data_rooms_rle:
                print(("const unsigned char %s_%s_segment%s_rle_size = %s;" % (
                    basename, self.name, room_cnt, len(room_data))), file=file)
                print(("const unsigned char %s_%s_segment%s_rle[] = {" % (basename, self.name, room_cnt)), file=file)
                total_size += len(room_data)
                for tile in room_data:
                    print(("%s," % tile), end=' ', file=file)
                print("0 };", file=file)
                room_cnt += 1
        elif self.mode == 4:
            room_cnt = 0
            for room_dict in self.data_rooms_compr_4x4_dict:
                print(("const unsigned char %s_%s_segment%s_dict[] = {" % (basename, self.name, room_cnt)), end=' ',
                      file=file)
                total_size += len(self.expand_block_keys(room_dict))
                for tile in self.expand_block_keys(room_dict):
                    print(("%s," % tile), end=' ', file=file)
                print("0 };", file=file)
                room_cnt += 1
            room_cnt = 0
            for room_data in self.data_rooms_compr_4x4:
                print(("const unsigned char %s_%s_segment%s_size = %s;" % (
                    basename, self.name, room_cnt, len(room_data))), file=file)
                print(("const unsigned char %s_%s_segment%s[] = {" % (basename, self.name, room_cnt)), end=' ',
                      file=file)
                total_size += len(room_data)
                for tile in room_data:
                    print(("%s," % tile), end=' ', file=file)
                print("0 };", file=file)
                room_cnt += 1
        else:
            # uncompressed output (should not be allowed?)
            print(("const unsigned char %s_%s[] = {" % (basename, self.name)), file=file)
            total_size += len(self.data)
            for tile in self.data:
                val = (tile % 256)
                print(("%s," % val), end=' ', file=file)
            print("0 };", file=file)
        print(("// TOTAL_SIZE %s" % total_size), file=file)
        return total_size


class ObjectGroupLayer:
    """ Contains an object group layer
    """

    def __init__(self, raw):
        # self.w = raw['width']
        # self.h = raw['height']
        self.x = raw['x']
        self.y = raw['y']
        self.name = raw['name'].replace(' ', '_')
        self.opacity = raw['opacity']
        self.visible = raw['visible']
        # self.draworder = raw['draworder']
        self.raw_objects = raw['objects']
        self.objects = []
        for obj in self.raw_objects:
            self.objects.append(MapObject(obj))
        self.extract_properties()  # we run this globa

    def extract_properties(self):
        """
        Extract properties just for this Object Layer
        """
        self.object_properties = {}
        self.enum_properties = {}
        self.max_num_properties = {}
        for item in self.raw_objects:
            # find all types or names
            _type = item['type']
            if _type == '':
                _type = item['name']
            if 'properties' in item:
                # Add properties to dict avoiding repetitions.
                if _type in self.object_properties:
                    already_added = set(self.object_properties[_type])
                    new_items = set(item['properties'].keys())
                    to_be_added = new_items - already_added
                    self.object_properties[_type] = self.object_properties[_type] + list(to_be_added)
                else:
                    self.object_properties[_type] = list(item['properties'].keys())
                length = len(list(item['properties'].keys()))
                ## this works for a single layer not the whole thing.
                ## maybe replace padding with a special scan?
                # if length > self.max_num_properties[_type]:
                #    self.max_num_properties[_type] = length
                ## Find Properties that require enums
                for _property in list(item['properties'].keys()):
                    value = item['properties'][_property]
                    if (not value.isdigit() and not value.replace('.', '').isdigit()):
                        if not _property in self.enum_properties:
                            self.enum_properties[_property] = {}
                        self.enum_properties[_property][value.encode('ascii', 'ignore')] = '1'

    def dump(self):
        print(("object group : [%s] (%s, %s)" % (self.name, self.x, self.y)))
        for obj in self.objects:
            obj.dump()

    def dump_as_c_header_no_data(self, file, basename):
        count = 0
        # print >>file,("extern const unsigned char %s_%s_size;\n" % (basename, self.name))
        for item in self.raw_objects:
            print(("extern const unsigned char %s_%s_objs[];\n" % (basename, self.name)), end=' ', file=file)
            # count = count + 1

    def dump_as_c_header(self, file, basename, global_properties):
        # Need to dump all the objects in a room in a sequence --
        # map_room00_objs[] = {<OBJ1>, <OBJ2>, ... <TERMINATION>}
        # map_room01_objs[] = ...

        # in init just asign each room to a an array item.

        count = 0
        total_size = 0
        # print >>file,("const unsigned char %s_%s_size = %s;\n" % (basename, self.name, len(self.raw_objects)))
        print(("const unsigned char %s_%s_objs[] = {" % (basename, self.name)), end=' ', file=file)
        for item in self.raw_objects:
            # print >>file,("const unsigned char %s_%s_obj%s[] = {" % (basename, self.name, count)),
            _type = item['type']
            if _type == '':
                _type = item['name']
            # type is like MOVABLE, coordinates should be able to be negative up to -32
            print(("%s, %s, %s, %s, %s, %s," % (_type.upper(),
                                                item['x'] % 256, item['y'] % 176, item['width'],
                                                item['height'], 1 if item['visible'] else 0)), end=' ', file=file)
            total_size += 6
            for _property in global_properties[_type]:
                if 'properties' in item and _property in item['properties']:
                    value = item['properties'][_property]
                else:
                    value = "0"
                if _property in self.enum_properties and not value.isdigit():
                    ## this is like TYPE_TEMPLAR
                    print(("%s_%s," % (_property.upper(), value.upper())), end=' ', file=file)
                elif not value.isdigit() and value.replace('.', '').isdigit:
                    print(("%s," % (value.replace('.', ''))), end=' ', file=file)
                elif value.isdigit():
                    wrap = int(value) % 256
                    ## regular numeric value, wrapped to byte
                    print(("%s," % wrap), end=' ', file=file)
                else:
                    print(("%s," % value), end=' ', file=file)
            total_size += len(self.object_properties[_type])
            ## add padding if needed (should never be neeeded anymore)
            ## something else is still wrong over here...
            ## problem is that we need to add padding for each type - to ensure consistency -

            # padding = self.max_num_properties - len(self.object_properties[_type]) + 2
            # print padding
            # for i in xrange(padding):
            #     print >>file,("0,"),
            count = count + 1
        print(("255};"), file=file)
        return total_size

    def dump_structures(self):
        """ Dump data structures and definitions
            to interpret the data from C code.
        """
        if len(list(self.object_properties.keys())) > 0:
            print("\nenum object_type {")
            for key in list(self.object_properties.keys()):
                print(("    %s, " % key.upper()))
            print("};\n")

            ## enum_properties
            for key in self.enum_properties:
                print(("\nenum object_property_%s {" % key.encode('ascii', 'ignore')))
                for _property in self.enum_properties[key]:
                    print(("    %s_%s, " % (key.encode('ascii', 'ignore').upper(), _property.upper())))
                print("};\n")

    def dump_initializer(self):
        """ Generate initialization code
        """
        pass


class TiledMap:
    """ Contains Tiled Map Data """

    def __init__(self, raw_map):
        self.map_version = raw_map['version']
        self.map_orientation = raw_map['orientation']
        self.tile_w = raw_map['tilewidth']
        self.tile_h = raw_map['tileheight']
        self.map_w = raw_map['width']
        self.map_h = raw_map['height']
        self.raw_layers = raw_map['layers']
        self.segment_w = self.map_w
        self.segment_h = self.map_h
        self.compr = None
        self.segment = False
        #
        # Read custom properties from the map
        #
        if 'properties' in raw_map:
            if 'rl_compr' in raw_map['properties']:
                self.compr = raw_map['properties']['rl_compr'].lower()
            if 'rl_segment' in raw_map['properties']:
                self.segment = raw_map['properties']['rl_segment'].lower()
            if 'rl_seg_w' in raw_map['properties']:
                self.segment_w = int(raw_map['properties']['rl_seg_w'])
            if 'rl_seg_h' in raw_map['properties']:
                self.segment_h = int(raw_map['properties']['rl_seg_h'])
        if self.compr != 'rle' and self.compr != 'block':
            self.compr = None
        if self.segment != 'true':
            self.segment = False
        self.tile_layers = []
        self.objectgroup_layers = []
        self.process_layers()

    def __setitem__(self, key, value):
        self.__dict__[key] = value

    def __getitem__(self, key):
        return self.__dict__[key]

    def process_layers(self):
        for layer in self.raw_layers:
            if 'tilelayer' in layer['type']:
                self.tile_layers.append(TileLayer(layer, self.segment_w, self.segment_h, self.compr, self.segment))
            elif 'objectgroup' in layer['type']:
                self.objectgroup_layers.append(ObjectGroupLayer(layer))


class TileMapWriter:
    """ writes a tile map to set of C header files
    """

    def __init__(self, tilemap, output):
        self.tilemap = tilemap
        self.output = output
        self.block_dict = {}
        filename, file_extension = os.path.splitext(self.output)
        self.filename = filename
        self.basepath = ntpath.basename(self.output)
        basename, base_extension = os.path.splitext(self.basepath)
        self.basename = basename

    def extract_properties(self):
        """
        Extract properties from all object layers
        """
        self.object_properties = {}
        self.enum_properties = {}
        self.max_num_properties = {}
        for object_layer in self.tilemap.objectgroup_layers:
            for item in object_layer.raw_objects:
                # find all types or names
                _type = item['type']
                if _type == '':
                    _type = item['name']
                if 'properties' in item:
                    # Add properties to dict avoiding repetitions.
                    if _type in self.object_properties:
                        already_added = set(self.object_properties[_type])
                        new_items = set(item['properties'].keys())
                        to_be_added = new_items - already_added
                        self.object_properties[_type] = self.object_properties[_type] + list(to_be_added)
                    else:
                        self.object_properties[_type] = list(item['properties'].keys())
                    length = len(list(item['properties'].keys()))
                    for _property in list(item['properties'].keys()):
                        value = item['properties'][_property]
                        if (not value.isdigit() and not value.replace('.', '').isdigit()):
                            if not _property in self.enum_properties:
                                self.enum_properties[_property] = {}
                            self.enum_properties[_property][value.encode('ascii', 'ignore')] = '1'

    def generate_headers(self):
        self.extract_properties()
        self.write_definitions()
        self.write_initialization()
        self.write_tilelayers()
        self.write_objectgroup_layers()
        self.write_grouping_header()

    def write_initialization(self):
        """
        write DATA segment vars for initialization code
        """
        filename, file_extension = os.path.splitext(self.output)
        basepath = ntpath.basename(self.output)
        basename, base_extension = os.path.splitext(basepath)

        fout = open(filename + '_init.h', 'w+')
        basename = basename + '_init'

        print(("#ifndef __MAP_INIT_H"), file=fout)
        print(("#define __MAP_INIT_H"), file=fout)

        #
        # Object Group layers
        #
        count = 0
        print(("unsigned char *%s_object_layer[%s];" % (self.basename, len(self.tilemap.objectgroup_layers))),
              file=fout)
        print(("/*\n * Initialization\n */"), file=fout)
        if count > 0:
            print(("unsigned char *%s_object[%s];" % (self.basename, count)), file=fout)
        print(("void main();\n"), file=fout)
        print(("void init() {\n main();\n}"), file=fout)
        print(("void init_%s_object_layers() {" % self.basename), file=fout)
        layer_count = 0
        for layer in self.tilemap.objectgroup_layers:
            name = layer.name.replace(' ', '_')
            print(("\t%s_object_layer[%s] = %s_%s_objs; " % (self.basename, layer_count, self.basename, name)),
                  file=fout)
            layer_count = layer_count + 1
        print(("}"), file=fout)
        for layer in self.tilemap.tile_layers:
            layer.dump_as_c_header_accessor(fout, self.basename)

        print(("#endif"), file=fout)

    def write_grouping_header(self):
        """
        write a header to group Layers
        """
        filename, file_extension = os.path.splitext(self.output)
        basepath = ntpath.basename(self.output)
        basename, base_extension = os.path.splitext(basepath)

        fout = open(filename + '.h', 'w+')

        print(("#ifndef __MAP_DATA_H"), file=fout)
        print(("#define __MAP_DATA_H"), file=fout)

        for layer in self.tilemap.tile_layers:
            header = basename + '_layer_' + layer.name + '.h'
            print(("#include \"%s\"" % header), file=fout)

        header = basename + '_layer_objects.h'
        print(("#include \"%s\"" % header), file=fout)

        header = basename + '_defs.h'
        print(("#include \"%s\"" % header), file=fout)
        print("#endif", file=fout)

    def write_definitions(self):
        """
        Write header file containing typedefs and structs
        """
        filename, file_extension = os.path.splitext(self.output)
        basepath = ntpath.basename(self.output)
        basename, base_extension = os.path.splitext(basepath)

        fout = open(filename + '_defs.h', 'w+')
        basename = basename + '_defs'

        print(("/* --- THIS FILE IS GENERATED, DO NOT EDIT --- */"), file=fout)
        print(("#ifndef __MAP_DATA_%s_H" % basename.upper()), file=fout)
        print(("#define __MAP_DATA_%s_H" % basename.upper()), file=fout)

        basename, base_extension = os.path.splitext(basepath)
        basename = basename + '_layer_objects'
        print(("#ifndef __MAP_DATA_%s_H" % basename.upper()), file=fout)
        for layer in self.tilemap.objectgroup_layers:
            layer.dump_as_c_header_no_data(fout, self.basename)
        print(("#endif"), file=fout)

        ## Accessor structures and initialization function for the map
        #
        for layer in self.tilemap.tile_layers:
            basename, base_extension = os.path.splitext(basepath)
            basename = basename + '_layer_' + layer.name
            print(("#ifndef __MAP_DATA_%s_H" % basename.upper()), file=fout)
            layer.dump_as_c_header_no_data(fout, self.basename)
            print(("#endif"), file=fout)

        if len(list(self.object_properties.keys())) > 0:
            print(("\nenum %s_object_type {" % self.basename), file=fout)
            for key in list(self.object_properties.keys()):
                print(("    %s, " % key.upper()), file=fout)
            print(("};\n"), file=fout)

            ## enum_properties
            for key in self.enum_properties:
                print(("\nenum %s_object_property_%s {" % (self.basename, key)), file=fout)
                for _property in self.enum_properties[key]:
                    print(("    %s_%s, " % (key.upper(), _property.decode('ascii').upper())), file=fout, flush=True)
                print(("};\n"), file=fout, flush=True)

        ## now additional structures and unions...
        for key in list(self.object_properties.keys()):
            ## here keys and properties cannot be C keywords
            print(("struct %s_object_%s {" % (self.basename, key)), file=fout)
            if len(self.object_properties[key]) > 0:
                for _property in self.object_properties[key]:
                    ## TODO filter C Keywords
                    if _property == 'static':
                        _property = 'static_'
                    if _property in self.enum_properties:
                        print(("     enum %s_object_property_%s %s;" % (self.basename, _property, _property)),
                              file=fout)
                    else:
                        print(("     unsigned char %s;" % _property), file=fout)
            else:
                print(("     unsigned char dummy;"), file=fout)
            print(("};\n"), file=fout)

        if len(list(self.object_properties.keys())) > 0:
            print(("union %s_object {" % self.basename), file=fout)
            for key in list(self.object_properties.keys()):
                if key == "static":
                    _key = "static_"
                else:
                    _key = key
                print(("    struct %s_object_%s %s;" % (self.basename, key, _key)), file=fout)
            print(("};"), file=fout)

        print(("struct %s_object_item {" % self.basename), file=fout)
        print(("    enum %s_object_type type;" % self.basename), file=fout)
        print(("    unsigned char x;"), file=fout)
        print(("    unsigned char y;"), file=fout)
        print(("    unsigned char w;"), file=fout)
        print(("    unsigned char h;"), file=fout)
        print(("    unsigned char visible;"), file=fout)
        if len(list(self.object_properties.keys())) > 0:
            print(("    union %s_object object;" % self.basename), file=fout)
        print("};\n", file=fout)

        print("#endif", file=fout)

    def write_tilelayers(self):
        """
        Write header file containing typedefs and structs
        """
        filename, file_extension = os.path.splitext(self.output)
        basepath = ntpath.basename(self.output)
        basename, base_extension = os.path.splitext(basepath)

        ## Fist write data
        ##
        for layer in self.tilemap.tile_layers:
            fout = open(filename + '_layer_' + layer.name + '.h', 'w+')
            basename = basename + '_layer_' + layer.name

            print(("#ifndef __MAP_DATA_%s_H" % basename.upper()), file=fout)
            print(("#define __MAP_DATA_%s_H" % basename.upper()), file=fout)

            size = layer.dump_as_c_header(fout, self.basename)
            if size > 8192:
                print("FATAL: map file bigger than 8k")
                exit(1)
            print("#endif", file=fout)

    def write_objectgroup_layers(self):
        """
        Write all object layers in a single output file
        """
        filename, file_extension = os.path.splitext(self.output)
        basepath = ntpath.basename(self.output)
        basename, base_extension = os.path.splitext(basepath)

        ## First write data
        #
        fout = open(filename + '_layer_objects.h', 'w+')
        basename = basename + '_layer_objects'

        print(("#ifndef __MAP_DATA_%s_H" % basename.upper()), file=fout)
        print(("#define __MAP_DATA_%s_H" % basename.upper()), file=fout)

        size = 0
        for layer in self.tilemap.objectgroup_layers:
            size += layer.dump_as_c_header(fout, self.basename, self.object_properties)
        if size > 8192:
            print("FATAL: map file bigger than 8k")
            exit(1)

        print("#endif", file=fout)


class TiledMapJsonReader:
    """ reads a json file saved from tiled"""

    def __init__(self, filename):
        self.filename = filename

    def read(self):
        self.data = open(self.filename)
        self.decoded = json.load(self.data)
        return TiledMap(self.decoded)


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

    reader = TiledMapJsonReader(opts.source)
    writer = TileMapWriter(reader.read(), opts.output)

    writer.generate_headers()
