# How to create 48K ROMS


1. Create a folder inside ./roms

    ```
    cd roms
    mkdir myrom
    ```

2. Add a Makefile with the following contents:

    ```
    include $(CONFIG_ROM)

    LOCAL_BANKED_SRC_FILES := swap.c

    LOCAL_ROM_NAME := myrom
    LOCAL_SRC_FILES := myrom.c
    include $(BUILD_ROM_48K)
    ```

3. Put some stuff inside myrom.c and some stuff inside swap.c

NOTE: The contents of the sources in *LOCAL_BANKED_SRC_FILES* can be either code
or data. Symbols are linked in a way that can be referenced from *LOCAL_SRC_FILES* as
normal C code.


4. Run **make**

If everything goes well, your ROM will be created in:

```
./roms/myrom/out/rom/myrom.rom
```

48K ROMS will boot with the same page setup as 32K ROMs:

| Bank   | Slot |
|--------|------|
| 0 (0x0000-0x3FFF)     | BIOS |
| 1 (0x4000-0x7FFF)     | ROM  |
| 2 (0x8000-0xBFFF)     | ROM  |
| 3 (0xC000-0xFFFF)     | RAM  |

But they contain an additional page that can be switched in *bank 0*,
with the command *sys_set_rom()*. Running:

```
#include "sys.h"

sys_set_rom();
```

will result in the following page setup:

| Bank   | Slot |
|--------|------|
| 0 (0x0000-0x3FFF)     | ROM  (LOCAL_BANKED_SRC_FILES) |
| 1 (0x4000-0x7FFF)     | ROM  |
| 2 (0x8000-0xBFFF)     | ROM  |
| 3 (0xC000-0xFFFF)     | RAM  |

and running:

```
#include "sys.h"

sys_set_bios();
```

will return to the original map.

*NOTE: make sure interrupts are disabled while switching out the BIOS*
