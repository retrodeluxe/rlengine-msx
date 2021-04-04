[![Generic badge](https://img.shields.io/badge/build-passing-green.svg)](https://shields.io/) [![Documentation Status](https://readthedocs.org/projects/retrodeluxe-engine-for-msx/badge/?version=latest)](https://retrodeluxe-engine-for-msx.readthedocs.io/en/latest/?badge=latest)

# RetroDeluxe Game Engine for MSX computers

RLE is a game engine targeted at MSX Computers, written in C and aimed at
making the development as easy as possible.

At the moment it provides:

* Build System support for 32K, 48K and up to 2048K ROMS
* Support for banked calls and automated bank switching allowing larger code bases
* Automatic compilation of assets (Maps, Tiles, Sprites) from common tools and formats (PNG, Tiled, etc.)
* Vortex Tracker II Music and SFX player (PT3, AFB)
* Different modes of compression for Map and Tile Data
* Several abstractions to ease development of complex games: Display Lists, Animations,  Collision Handlers, Fonts, etc.
* Early support for MSX2 (screen 5 blit, palette, sprites mode 2, and mouse)

RLE is a work in progress, but mature enough for development (check the tests folder for samples).

# Roadmap

 * Mass storage support (megaflashrom sd card), vfat and ext2 filesystems
 * Mode2 sprite assets using OR encoding
 * Arkos Tracker PSG player support
 * Sprite oclussion
 * Isometric blit

# Known issues

 * Code and Data split into ROM pages needs to be done manually in separate source files so that each compilation unit fits in 8KB

# Documentation

Also work in progress, but please check https://retrodeluxe-engine-for-msx.readthedocs.io/en/latest/

# How to Build

Currently supported platforms are MacOS and Ubuntu.

Just run 'make test' or 'make rom', this build run everything inside the test and roms folders.

You can also build individual roms from their root folder.
