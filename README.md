[![Generic badge](https://img.shields.io/badge/build-passing-green.svg)](https://shields.io/)

# RetroDeluxe Game Engine for MSX1 computers

RLE is a game engine targeted at MSX1 Computers, written in C and aimed at
making the development as easy as possible.

At the moment it provides:

* Build System support for 32K, 48K and up to 2048K ROMS
* Automatic compilation of assets (Maps, Tiles, Sprites)
* Low level HAL libraries (VDP, PSG, System)
* Vortex Tracker II Music Player
* High Level Helpers for Sprite and Tile Animation
* Different modes of compression for Map and Tile Data (RLE, Block)
* Basic Physics and Animation Support

RLE is a work in progress, but mature enough for development (see the tests folder for samples).

# How to Build

Just run make.

Currently supported platforms are MacOS (tested in Sierra) and Ubuntu (tested in 17.04)

Makefiles inside subdirectories will be found and built. Check the test directory for examples on how to write makefiles for individual ROMs.


# Documentation (WIP)

[How To Create a 32K ROM](https://github.com/retrodeluxe/rlengine-msx1/blob/master/docs/HOWTO_Create_32K_ROM.md)

[How To Create a 48K ROM](https://github.com/retrodeluxe/rlengine-msx1/blob/master/docs/HOWTO_Create_48K_ROM.md)

[How To Create a 2M ROM](https://github.com/retrodeluxe/rlengine-msx1/blob/master/docs/HOWTO_Create_2M_ROM.md)

[Tile Resources](https://github.com/retrodeluxe/rlengine-msx1/blob/master/docs/HOWTO_Use_Tile_Resources.md)

[Sprite Resources](https://github.com/retrodeluxe/rlengine-msx1/blob/master/docs/HOWTO_Use_Sprite_Resources.md)

[Dynamic Tiles]

[Tile Map resources](https://github.com/retrodeluxe/rlengine-msx1/blob/master/docs/HOWTO_Use_Tile_Map_Resources.md)

[Display Lists]

[Full API]
