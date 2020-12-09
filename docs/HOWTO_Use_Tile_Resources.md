# How to use Tile Resources

## Tile resource files

The engine searches for maps in the *local resources* directory, which is
specified in the ROM Makefile:

```
LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)
```

Tile sets should be placed in a subdirectory called tile, and have tga extension. The only format supported as of now is TGA.

```
./rom/res/tile/
./rom/res/tile/maptiles.tga
./rom/res/tile/portal.tga
...
```

## Format of Tile Set Graphic files

Tile set graphic files must have the following format:

* TGA without compression
* RGB Color
* Alpha channel
* The size of the image must be multiple of 8 pixels and smaller than 16Kb
    * Valid sizes are, for instance 256x64, 128x128, 512x32...

Files with the format can be exported from GIMP:

![SaveDialog](https://raw.githubusercontent.com/retrodeluxe/rlengine-msx1/master/docs/tgasave.png)

To ensure the palette is correct it is recommended to switch the image to indexed mode with an optimum 16 color palette while working on it, then switch back to RGB and add an Alpha channel before exporting.

![Indexed](https://raw.githubusercontent.com/retrodeluxe/rlengine-msx1/master/docs/indexedColor.png)



## Generated Tile Header files

Upon compilation, tile resources are turned into header files that can be
included in the ROM code. Those can be found in the *generated* directory,
and have the same name as the resource they come from:

```
./rom/gen/maptiles.h
./rom/gen/portal.h
```

## Accessing Tile resources from C code

Upon including the rom resources header:

```
#include "gen/<ROM_NAME>.h"
```

tile data is available for initialisation.

You can use the macro _INIT_TILE_SET_ to assign a tile resource to a tile_set structure.

```
TileSet my_tileset;

INIT_TILE_SET(my_tileset, <tile_resource_name>);
```

A tile_set structure can be copied to the VRAM pattern table by using:

```
tile_set_to_vram(&my_tileset, 0);
```

At this point, the tile patterns are available to be used by writting to the name table.

```
vdp_poke(VRAM_BASE_NAME, <tile number>);
```

This way of using tiles is suitable for drawing maps or displaying static images. Also see the section about dynamic tiles to draw animated objects based on tiles.
