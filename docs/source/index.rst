.. rdlEngine documentation master file, created by
   sphinx-quickstart on Sat Dec 12 00:03:19 2020.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

RetrodeLuxe Engine for MSX documentation
=====================================

.. toctree::
   :maxdepth: 2
   :caption: Contents:

Sprite Module
-------------

The Sprite module provides a software abstraction to handle composite hardware
sprites consisting of several layers (colors) and multiple animation states.

It supports dynamic VRAM allocation and automatic calculation of the frames to
be displayed based on the current animation state of the sprite.

It currently supports up to 48 sprite pattern set definitions and a maximum of 8 states
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


Tiles
-----

.. c:autodoc:: ../../engine/include/tile.h

Fonts
-----

.. c:autodoc:: ../../engine/include/font.h

Display Objects
---------------

.. c:autodoc:: ../../engine/include/dpo.h

Animation
---------


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
