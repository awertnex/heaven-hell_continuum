# minecraft.c
**a 1:1 minecraft clone but in C with the C99 standard, as best as I could, with minimal library usage if any.\
It still builds fine without `-std=c99`\
eventually I will replace all minecraft resources with my own for copyright reasons.**

## dependencies
- [raylib](https://github.com/raysan5/raylib) by [raysan5](https://github.com/raysan5)

## requirements
- c compiler 'cc' or 'gcc' (or MSVC, the c/c++ compiler that comes with visual studio)
    - download `gcc` for windows from [sourceforge/gcc-win64](https://www.sourceforge.net/projects/gcc-win64/)
    - extract it and move the extracted folder somewhere like `Program Files`
    - add the path of the `bin` folder inside to the `path` environment variable

## build and run
#### linux (using cc):
```
git clone https://github.com/awertnex/minecraft.c.git
cd minecraft.c/
./build
```
if `./build` gives errors or is outdated, then:
```
cc build.c -o build -std=c99; ./build
```
the cflag `-std=c99` is optional but preferred.\
if build successful, run minecraft.c with `./minecraft_c`

#### windows (using file explorer):
- download the zip file (the green `<> Code` button at the top, then `Download ZIP`)
- extract, and run `minecraft_c.exe` inside
- if it gives errors or is outdated, run `build.exe` and then run `minecraft_c.exe`

**additional commands**
- build launcher with `./build launcher` and run with `./launcher`
    - the launcher creates a directory 'minecraft.c/' in your 'HOME' directory
    - all minecraft.c instances will be stored in 'minecraft.c/instances/'
    - the default path to 'minecraft.c/' can be changed from the file 'dir.c' in the function 'init_paths()' before building
- list available tests with `./build list`
    - this command searches the directory 'src/tests/' and loads all `.c` file names into an array to list
- build test [n] with `./build test [n]`
    - this command searches the directory 'src/tests/' and loads all `.c` file names into an array to execute by index [n]
- help `./build --help`
#### windows (using mingw/gcc):
```
git clone https://github.com/awertnex/minecraft.c.git
cd minecraft.c/
minecraft_c.exe
```
if `build.exe` gives errors or is outdated, then:
```
gcc build.c -std=c99; build.exe
```
the cflag `-std=c99` is optional but preferred.\
if build successful, run minecraft.c with `minecraft_c.exe`

**additional commands**
- build launcher with `build launcher` and run with `launcher.exe`
    - the launcher creates a directory 'minecraft.c/' in your 'appdata' folder
    - all minecraft.c instances will be stored in 'minecraft.c/instances/'
    - the default path to 'minecraft.c/' can be changed from the file 'dir.c' in the function 'init_paths()' before building
- list available tests with `build list`
    - this command searches the directory 'src/tests/' and loads all `.c` file names into an array to list
- build test [n] with `build test [n]`
    - this command searches the directory 'src/tests/' and loads all `.c` file names into an array to execute by index [n]
- help `build --help`
- - - -
