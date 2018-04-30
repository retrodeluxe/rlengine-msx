[![Generic badge](https://img.shields.io/badge/BUILD-FAILING_[REFACTORING_ONGOING]-RED.svg)](https://shields.io/)

# RetroDeluxe Game Engine for MSX1 computers

RLE is a game engine targeted at MSX1 Computers, written in C and aimed at
making the development as easy as possible.
È
At the moment it provides:

* Build System support for 32K, 48K and up to 2048K ROMS˜
* Automatic compilation of assets (Maps, Tiles, Sprites)
* Low level HAL libraries (VDP, PSG, System)
* High Level Helpers for Sprite and Tile Animation
* Different modes of compression for Map and Tile Data (RLE, Block)
* Basic Physics and Animation Support

RLE is a work in progress, and although it still lacks a music player, is mature
enough for development (see the tests folder for samples).

# Documentation (WIP)

[How To Create a 32K ROM](https://github.com/retrodeluxe/rlengine-msx1/blob/master/docs/HOWTO_Create_32K_ROM.md)

[How To Create a 48K ROM](https://github.com/retrodeluxe/rlengine-msx1/blob/master/docs/HOWTO_Create_48K_ROM.md)

[How To Use Tile Map resources](https://github.com/retrodeluxe/rlengine-msx1/blob/master/docs/HOWTO_Use_Tile_Map_Resources.md)


# How to Build

Just run make.

Currently supported platforms are MacOS (tested in Sierra) and Ubuntu (tested in 17.04)

Makefiles inside subdirectories will be found and built. Check the test directory for examples on how to write makefiles for individual ROMs.

# Creating assets

Maps can be created using Tiled (http://www.mapeditor.org/download.html, also available in ubuntu as package) and exporting to json format.

Some structure is assumed in the format of the map, please look at the samples in the test folder.

Sprites and Tiles can be created as images and exported to TGA format with GIMP (select no RLE, top down export options). Note that each pixel will be set to the closest color in the MSX1 palette. In case of SPRITES pixel by pixel matching is carried out, in case of TILES (Screen 2), each line of 8 pixels are taken together and matched to the best color combination by minimum squared error; note that depending on the input palette used this may not produce an accurate output, therefore; using an input palette close to the MSX palette and adjusting manually the pixels so that they are SCR2 compliant is recommended.
