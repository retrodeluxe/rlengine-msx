# TODO and BUGS

## build system
- add rules for each test/rom

## engine
- vdp: fix vdp_fastcopy_nametable for msx1
- displ: add helpers for display lists
- music: trilo-tracker psg replayer
- sfx: trilo-tracker sfx replayer
- sys: fix time sync problems (clock runs slow)
- ~~vdp: add helpers to avoid using constants to access name tables, etc.~~
- spr: fix spr_animate to support 8 directions and up/down animation
- event: add module for event driven user input
- mem: some memory management
- event: keyboard entry
- event: mouse support (https://www.msx.org/wiki/Mouse/Trackball#Direct_usage_of_mouse)
- sys: use threads instead of IRQs?
- sys: add timers with callbacks

## Tests / Samples
- ~~add dyntile test~~
- game_test:
   - fix build
   - use to ascii8 ROM type
   - split map into pages
   - ...
- map_test: add rest of map compression options
- phys_test: sprite colisions, monk death

## Tools

##### Map2Header - Tiled Map compiler
- ~~If dictionary is bigger than 256, raise overflow error~~
- ~~Add compressed size for the tile Layers~~
- Support multiple tile layers?
- add variables for segment size (w,h)
- add number of objects per room variables
- document the interface
- add support for non segmented maps, and RLE compression
- port to python3

##### Tmu2Header - TriloTracker compiler
- ~~Load TMU song~~
- Process
- Generate header

##### Tga2Header - Graphic Assets compiler
- Add PNG support
- ~~Document better process to export TGA from GIMP~~
