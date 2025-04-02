# minecraft.c
**a 1:1 minecraft clone but in C, as best as I could, with minimal library usage, if any.
eventually I will replace all minecraft resources with my own, for copyright reasons.**

## dependencies
- [raylib](https://github.com/raysan5/raylib) by [raysan5](https://github.com/raysan5)

## requirements
- c compiler 'cc' or 'gcc'

## build and run
#### for both linux and windows:
```
git clone https://github.com/awertnex/minecraft.c.git
cd minecraft.c/
cc src/build.c -o build -std=c99
./build
```
if build successful, run minecraft.c with `./minecraft_c`

- - - -
**additional commands**
- build launcher with `./build launcher` and run with `./launcher`
    - the launcher creates a directory 'minecraft.c/' in your 'HOME' directory on linux or in your 'appdata' folder on windows
    - all minecraft.c instances will be stored in 'minecraft.c/instances'
    - the default path to 'minecraft.c/' can be changed from the file 'dir.c' in the function 'init_paths()' before building
- list available tests with `./build list`, build test [n] with `./build test [n]`
- help `./build --help`
