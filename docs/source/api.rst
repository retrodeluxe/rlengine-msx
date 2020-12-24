Engine API
==========

General definitions
-------------------

The MSX header contains several useful definitions used by other modules.

.. code-block:: c

   #include "msx.h".

.. c:autodoc:: ../../engine/include/msx.h

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

The Tile module provides a software abstraction to handle a set of tiles defined
either as a :c:type:`TileSet` or a :c:type:`TileObject`.

A :c:type:`TileSet` can be defined as a **static** set of tiles and behave as a container for
tile pattern and color definitions. But alternatively can also be defined as **dynamic** and contain
structural information for animation in a similar way as Sprites do (states and frames).

A dynamic :c:type:`TileSet` can be used as a pattern into a :c:type:`TileObject` to
display animated tiles on screen.

.. code-block:: c

   #include "tile.h".

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

The Physics Module provides an abstraction to define what objects in a scene
generate collisions and to allow for detecting and handling those collisions
during animation.

``TileObjects``, ``TileSets`` and individual tiles can be used to configure
collision events that can then be detected during the animation of a
``DisplayObject``.

.. code-block:: c

   #include "phys.h".

.. c:autodoc:: ../../engine/include/phys.h
.. c:autodoc:: ../../engine/phys.c


Vdp Module
----------

The Vdp module provides functions to configure the Vdp and transfer data from
RAM to VRAM.

.. code-block:: c

   #include "vdp.h"

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
