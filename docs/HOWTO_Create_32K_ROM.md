# How to create 32K ROMS

1. Create a folder inside ./roms

    ```
    cd roms
    mkdir myrom
    ```

2. Add a Makefile with the following contents:

    ```
    include $(CONFIG_ROM)

    LOCAL_ROM_NAME := myrom
    LOCAL_SRC_FILES := myrom.c
    include $(BUILD_ROM_32K)
    ```

3. Put some stuff inside myrom.c
4. Run **make**

If everything goes well, your ROM will be created in:

```
./roms/myrom/out/rom/myrom.rom
```
