[![Generic badge](https://img.shields.io/badge/build-passing-green.svg)](https://shields.io/) [![Documentation Status](https://readthedocs.org/projects/retrodeluxe-engine-for-msx/badge/?version=latest)](https://retrodeluxe-engine-for-msx.readthedocs.io/en/latest/?badge=latest)

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

# Documentation

Also work in progress, but please check https://retrodeluxe-engine-for-msx.readthedocs.io/en/latest/

# How to Build

Currently supported platforms are MacOS and Ubuntu.

Just run 'make test' or 'make rom', this build run everything inside the test and roms folders.

You can also build individual roms from their root folder.
