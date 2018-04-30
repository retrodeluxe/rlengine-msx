# How to create ASCII8 MegaROMS (Up to 2Mbytes)


1. Create a folder inside ./roms

    ```
    cd roms
    mkdir myrom
    ```

2. Add a Makefile with the following contents:

    ```
    include $(CONFIG_ROM)

    LOCAL_ROM_NUM_PAGES := <N>
    LOCAL_PAGE_1_SRC_FILES := swap1.c
    LOCAL_PAGE_2_SRC_FILES := swap2.c
    ...
    LOCAL_PAGE_<N>_SRC_FILES := swap<N>.c

    LOCAL_ROM_NAME := myrom
    LOCAL_SRC_FILES := myrom.c
    include $(BUILD_ROM_ASCII8)
    ```

3. Put some stuff inside myrom.c and some stuff inside the swap<X>.c files

NOTE: The contents of the sources in *LOCAL_PAGE_X_SRC_FILES* can be either code
or data. Symbols are linked in a way that can be referenced from *LOCAL_SRC_FILES* as
normal C code.

The Maximum code or data size of each one of the *LOCAL_PAGE_X_SRC_FILES* is limited
to the ASCII8 Mapper page size of 8KBytes.

4. Run **make**

If everything goes well, your ROM will be created in:

```
./roms/myrom/out/rom/myrom.rom
```

256K ROMS will boot with the same page setup as 32K ROMs, but with an ASCII8
mapper initialised as follows:

| Bank  | Slot | ASCII8 Bank | Page |
|-------|------|-------------|
| 0     | BIOS |             | |
| 1     | ROM  |  0 (0x4000-0x5FFF) |   0
|       | ROM  |  1 (0x6000-0x7FFF) | 1
| 2     | ROM  |  2 (0x8000-0x9FFF)| 2
|       | ROM  |  3 (0xA000-0xBFFF) | 3
| 3     | RAM  |             | | |


Paged code is linked to be used **only in ASCII8 bank 3**. It can be acessed by
switching the corresponding page with the command *sys_set_ascii_page3(X)*.

Note that ASCII8 pages 0,1,2,3 are used by *LOCAL_SRC_FILES* therefore, the first
page used by *LOCAL_PAGE_1_SRC_FILES* is **page number 4**.

The following command:

```
#include "sys.h"

sys_set_ascii_page3(4);
```

will result in the following page setup:

| Bank  | Slot | ASCII8 Bank | Page |
|-------|------|-------------|
| 0     | BIOS |             | |
| 1     | ROM  |  0 (0x4000-0x5FFF) |   0
|       | ROM  |  1 (0x6000-0x7FFF) | 1
| 2     | ROM  |  2 (0x8000-0x9FFF)| 2
|       | ROM  |  3 (0xA000-0xBFFF) | 4
| 3     | RAM  |             | | |


*It is possible to define up to 251 **LOCAL_PAGE_X_SRC_FILES**, but management
of the contents of each is up to the developer. Which means that currently the
engine provides no mechanism for automatic switching of the pages and special
care needs to be taken to switch to the proper page before accessing the data
or running the code placed in there.*
