# How to use Sprite Resources

## Sprite resource files

The engine searches for sprites in the local resources directory, which is specified in the ROM Makefile:

```
LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)
```

Sprites should be placed in a subdirectory called spr, and have tga extension. The only format supported as of now is TGA.

```
./rom/res/spr/
./rom/res/spr/bee1.tga
./rom/res/spr/rat.tga
...
```

## Format of Sprite Graphic files

Currently 3 sprite sizes are supported:

* 8x8
* 16x16
* 16x32

Graphic files must consist of a matrix of frames of WWxHH size, containing
either a single or several colors (up to 4), with a transparent background.

Upon defining a sprite pattern set to be used from code, it is possible to
specify the number of colors, the number of animation steps and the number of
directions of the sprite.

The ordering of the frames must be according to the sprite pattern definition to be used.

For instance:

```
sprite: bee1
planes: 1 (1 color)
directions: 2 (left and right)
animation steps: 2
```

The sprite frame matrix should be the following:

![bee1](https://raw.githubusercontent.com/retrodeluxe/rlengine-msx1/master/docs/bee1.png)

```
sprite: eggerland
planes: 2 (2 color)
directions: 4 (left, right, up, down)
animation steps: 3
```

![eggerland](https://raw.githubusercontent.com/retrodeluxe/rlengine-msx1/master/docs/eggerland.png)


## Generated Sprite Header files

Upon compilation, sprite resources are turned into header files that can be
included in the ROM code. Those can be found in the *generated* directory,
and have the same name as the resource they come from:

```
./rom/gen/bee1.h
./rom/gen/rat.h
./rom/gen/eggerland.h
```

## Accessing Sprite resources from C code

Upon including the rom resources header:

```
#include "gen/<ROM_NAME>.h"
```

sprite data is available for initialization.

```
spr_init();
```

First step is to define sprite patterns structures


```
struct spr_pattern_set bee_patt;
struct spr_pattern_set rat_patt;

SPR_DEFINE_PATTERN_SET(bee_patt, SPR_SIZE_16x16, 1, 2, 2, bee1);
SPR_DEFINE_PATTERN_SET(rat_patt, SPR_SIZE_16x16, 1, 2, 2, rat);
SPR_DEFINE_PATTERN_SET(egg_patt, SPR_SIZE_16x16, 2, 3, 4, eggerland);
```

Pattern sets need to be allocated into video memory, this is done by:

```
spr_valloc_pattern_set(&bee_patt);
spr_valloc_pattern_set(&rat_patt);
spr_valloc_pattern_set(&egg_patt);
```

Once the patterns are in video memory, they can be linked to a sprite:

```
struct spr_sprite_def eggspr;
struct spr_sprite_def monk;
SPR_DEFINE_SPRITE(bee[i], &bee_patt, 10, bee1_color);
SPR_DEFINE_SPRITE(rats[i], &rat_patt, 10, rat_color);
```

and the sprite manipulated using:

```
spr_set_pos()
spr_show()
spr_hide()
spr_animate()
```

WIP
