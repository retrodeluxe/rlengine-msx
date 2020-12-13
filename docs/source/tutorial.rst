Getting Started
===============

Creating a simple ROM
--------------------

Let's start by creating a sub-folder inside the ``roms`` directory

.. code-block:: shell

    cd roms
    mkdir myrom


and adding a ``Makefile`` with the following contents:

.. code-block:: make

    include $(CONFIG_ROM)

    LOCAL_ROM_NAME := myrom
    LOCAL_SRC_FILES := myrom.c
    include $(BUILD_ROM_32K)


For it to compile, we need to add a source file ``myrom.c``

.. code-block:: c

    #include "msx.h"
    #include "sys.h"
    #include "vdp.h"

    void main()
    {
      vdp_set_mode(MODE_GRP1);
      vdp_set_color(COLOR_WHITE, COLOR_BLACK);
      vdp_clear(0);

      vdp_puts(10, 10, "Hello MSX");

      for(;;);
    }

Now we just need to run:

.. code-block:: shell

    make myrom

If everything goes well, a file ``myrom.rom`` will be created inside ``myrom/out/rom``
and it can be run with openmsx

.. code-block:: shell

    openmsx myrom.rom


Adding game assets
-------------------

Let's now add the following asset to ``myrom``

.. image:: bee1.png

.. code-block:: make

    LOCAL_RES_DIR := ./res
    include $(BUILD_RESOURCES)
  


Animating Sprites
------------------

Displaying a Map
----------------


Handling basic collisions
-------------------------
