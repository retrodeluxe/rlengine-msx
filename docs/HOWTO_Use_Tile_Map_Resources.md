# How to use Tile Map resources

## Map resource files

The engine searches for maps in the *local resources* directory, which is
specified in the ROM Makefile:

```
LOCAL_RES_DIR := ./res
include $(BUILD_RESOURCES)
```

Maps should be placed in a subdirectory called **map**, and have *json*
extension. The only format supported as of now is **Tiled** *json* exports, with
some constrains that are explained below.

```
./rom/res/map/
./rom/res/map/map1.json
./rom/res/map/map2.json
...
```

## Generated Map Header files

Upon compilation, map resources are turned into header files that can be
included in the ROM code. Those can be found in the *generated* directory,
and have the same name as the resource they come from:

```
./rom/gen/map1.h
./rom/gen/map2.h
...
```

Map header files contain not only data, but also type definitions that simplify using
the resources from game code. More details on this further below.


## Defining Map resource files with Tiled

**Tiled Map Editor** is a popular tool, it can be downloaded [here](https://www.mapeditor.org).

In order to simplify the programming interface, the engine puts some constraints
on how Tiled maps should be created. Please remember to follow the guidelines
below.

### Layer types

Map2header only supports **tile layers** and **object layers**, but you can use
as many of them as you need.

#### Tile layers




#### Object Layers

When editing an object in Tiled, the properties panel on the left will shows a
list of Object properties, plus a list of Custom Properties. Of those, some are
automatically filled in when editing the map, others need to be filled in manually.

The engine will find and use the following Object properties, translating them
into data included in the header files:

| Property | Description  |
|:--------:|:------------:|
| id       |  unique id          |
| name     |  label as shown in the map editor           |
| type     |  user defined object type|
| x        |  position in world coordinates          |
| y        |  position in world coordinates      |
| width    |  width in pixels         |
| height   |  height in pixels       |
| visible  |     visibility flag      |

The **type** property is specially important, as each value used will define
a common set of custom properties used by all objects of that type.

If for instance, a type *SPRITE* is defined, the engine expects all objects of
this type to have the same set of custom properties.

When using the engine for the first time I recommend using types : **SPRITE, OBJECT and DYNTILE**

All Custom properties are compiled into the header files as well. Some recommended
custom properties are the following:

| Property | Description  |
|:--------:|:------------:|
| type     |  type of resource (e.g sprite name)        |
| color    |  color           |
| anim_type     |  type of animation |
| direction        |  direction of movement          |
| speed        |  speed      |

Using a proper list of custom properties can automate considerably the generation
of scenes within the game.

### Map properties

WIP
