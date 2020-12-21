Engine API
==========

Sprite Module
-------------

The Sprite module provides a software abstraction to handle composite hardware
sprites consisting of several layers (colors) and multiple animation states.

It supports dynamic VRAM allocation and automatic calculation of the frames to
be displayed based on the current animation state of the sprite.

Up to 48 sprite pattern set definitions can be stored in RAM with 8 states
per sprite. The maximum allowed number of planes per frame depends on the sprite size,
due to the VDP limit of 4 sprites per row. This value varies from 1 (for 32x32 sprites)
to 3 (for 16x16 sprites).

The maximum number of animation steps per state is not defined, but the VDP sets a hard
limit of 2KB on the amount of individual patterns that can be allocated at any give time.
This amounts to a total 64 frames for 16x16 sprites.

.. code-block:: c

   #include "sprite.h".

.. c:autodoc:: ../../engine/include/sprite.h
.. c:autodoc:: ../../engine/spr.c

Tile Module
------------

.. c:autodoc:: ../../engine/include/tile.h
.. c:autodoc:: ../../engine/tile.c

Font Module
-----------

.. c:autodoc:: ../../engine/include/font.h
.. c:autodoc:: ../../engine/font.c

Display Object Module
---------------------

.. c:autodoc:: ../../engine/include/dpo.h
.. c:autodoc:: ../../engine/dpo.c

Animation Module
----------------

Physics Module
--------------
.. c:autodoc:: ../../engine/include/phys.h
.. c:autodoc:: ../../engine/phys.c


Vdp Module
----------

.. c:autodoc:: ../../engine/include/vdp.h
.. c:autodoc:: ../../engine/vdp.c

Timer Module
------------

.. c:autodoc:: ../../engine/include/timer.h
.. c:autodoc:: ../../engine/timer.c

System Module
-------------

.. c:autodoc:: ../../engine/include/sys.h
.. c:autodoc:: ../../engine/sys.c

Sound Effects Module
--------------------

.. c:autodoc:: ../../engine/include/sfx.h
.. c:autodoc:: ../../engine/sfx.c

Vortex Tracker Module
---------------------

.. c:autodoc:: ../../engine/include/pt3.h
.. c:autodoc:: ../../engine/pt3.c

Map Module
----------

.. c:autodoc:: ../../engine/include/map.h
.. c:autodoc:: ../../engine/map.c
