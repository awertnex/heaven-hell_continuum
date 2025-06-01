<h1 align="center">
  <img src="resources/logo/128x128.png" alt="minecraft.c">
</h1>

**a 1:1 minecraft clone but in C99, with minimal library usage if any.**

## dependencies
- [raylib](https://github.com/raysan5/raylib) by [raysan5](https://github.com/raysan5)


## downloads
- [v0.1.4_linux-x86_64.tar.gz](https://github.com/awertnex/minecraft.c/releases/download/v0.1.4-alpha/minecraft_c-v0.1.4-linux-x86_64.tar.gz)
- [v0.1.4_linux-x86_64.zip](https://github.com/awertnex/minecraft.c/releases/download/v0.1.4-alpha/minecraft_c-v0.1.4-linux-x86_64.zip)
- [v0.1.4_win64.zip](https://github.com/awertnex/minecraft.c/releases/download/v0.1.4-alpha/minecraft_c-v0.1.4-win64.zip)
- unzip and launch


## build from source

### Important note: for development build, RELEASE_BUILD in `src/engine/h/logger.h` should be `0` before building.

### linux - debian-based:

```bash
git clone --depth=1 https://github.com/awertnex/minecraft.c.git
cd minecraft.c/
cc src/build.c -std=c99 -o build # -std=c99 optional, but preferred
./build
```
if build successful, run minecraft.c with `./bin/minecraft_c`\

**additional commands**
- help `./build -h` or `./build --help`
- build launcher with `./build launcher` and run with `./launcher`
    - the launcher creates a directory 'minecraft.c/' in your 'HOME' directory
    - all minecraft.c instances will be stored in 'minecraft.c/instances/'
    - the default path to 'minecraft.c/' can be changed from the file 'dir.c' in the function 'init_paths()' before building
- list available tests with `./build -l` or `./build --list`
    - this command searches the directory 'src/tests/' and loads all `.c` file names into an array to list
- show build command with `./build -v` or `./build --verbose`
- append <arg> to build command with `./build -a <arg>`
- build test [n] with `./build test [n]`
    - this command searches the directory 'src/tests/' and loads all `.c` file names into an array to execute by index [n]


### windows (using mingw/gcc):

**requirements**
- a c compiler, 'cc' or 'gcc' (or MSVC, the visual studio c/c++ compiler)
    - download `gcc` for windows from [sourceforge/gcc-win64](https://www.sourceforge.net/projects/gcc-win64/)
    - extract it and move the extracted folder somewhere like `C:\` (preferrably) or `C:\Program Files`
    - add the path of the `bin` folder inside to the `path` environment variable

```command
git clone --depth=1 https://github.com/awertnex/minecraft.c.git
cd minecraft.c
gcc .\src\build.c -std=c99 -o build.exe
.\build.exe
```
if build successful, run minecraft.c with `.\bin\minecraft_c.exe`\
cflag `-std=c99` optional, but preferred

**additional commands**
- help `.\build.exe -h` or `.\build.exe --help`
- build launcher with `.\build.exe launcher` and run with `.\launcher.exe`
    - the launcher creates a folder 'minecraft.c' in your 'appdata' folder
    - all minecraft.c instances will be stored in 'minecraft.c\instances'
    - the default path to 'minecraft.c' can be changed from the file 'dir.c' in the function 'init_paths()' before building
- list available tests with `.\build.exe -l` or `.\build.exe --list`
    - this command searches the folder '.\src\tests' and loads all `.c` file names into an array to list
- show build command with `.\build.exe -v` or `.\build.exe --verbose`
- append <arg> to build command with `.\build.exe -a <arg>`
- build test [n] with `.\build.exe test [n]`
    - this command searches the folder '.\src\tests' and loads all `.c` file names into an array to execute by index [n]

## Contributors (alpha testing - windows)
- [sofia](https://github.com/EdgySofia666)

