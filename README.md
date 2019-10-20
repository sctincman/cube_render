# Cube Renderer

Renders a GLTF scene onto the faces of a cube.

## Controls

* QWEASD - Move current camera
  * Q/E - Strafe left/right
  * A/D - Rotate left/right
  * W/S - Move forward/backwards
* TAB - Disabled/Enable lock on (locks camera to look at center, default enabled).
* P - Change projection between perspective/orthographic
* V - Change view between parent Cube scene and child GLTF scene

## Building

I use a garbage makefile so I could get this going quickly. It compiles for me, but YMMV unless you're on Arch Linux at this moment in time and have the same packages.

Needed dependencies:
* SDL2

Included dependencies:
* [tinygltf](https://github.com/syoyo/tinygltf) (MIT)

Build with `make`, executable will be located in current directory at `cube_render`.

Build a debug build with `make DEBUG=1`

Variables in the makefile are [mostly] conditionally defined so they can be overridden, for example if SDL2 lives somewhere else, this *should* work (not tested).

`make SDL_LIB="-L/somewhere/else -lGL -lGLEW -lSDL2 -Wl,-rpath=/somewhere/else" SDL_INCLUDE="-I/somewhere/else -DGL_GLEXT_PROTOTYPES"`

