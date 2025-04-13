<h1 align="center">
  <img src="resources/logo/128x128.png" alt="minecraft.c">
</h1>

**a 1:1 minecraft clone but in C with the C99 standard, as best as I could, with minimal library usage if any.\
It still builds fine without `-std=c99`**

## dependencies
- [raylib](https://github.com/raysan5/raylib) by [raysan5](https://github.com/raysan5)


## downloads
- [v0.1.4_linux-x86_64.tar.gz](https://github.com/awertnex/minecraft.c/releases/download/v0.1.4-alpha/minecraft_c-v0.1.4-linux-x86_64.tar.gz)
- [v0.1.4_linux-x86_64.zip](https://github.com/awertnex/minecraft.c/releases/download/v0.1.4-alpha/minecraft_c-v0.1.4-linux-x86_64.zip)
- [v0.1.4_win64.zip](https://github.com/awertnex/minecraft.c/releases/download/v0.1.4-alpha/minecraft_c-v0.1.4-win64.zip)
- unzip and launch


## build from source

### linux - debian-based:
```bash
git clone --depth=1 https://github.com/awertnex/minecraft.c.git
cd minecraft.c/
cc build.c -std=c99 -o build # -std=c99 optional, but preferred
./build
```
if build successful, run minecraft.c with `./minecraft_c`\

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


### windows (using mingw/gcc):

**requirements**
- a c compiler, 'cc' or 'gcc' (or MSVC, the visual studio c/c++ compiler)
    - download `gcc` for windows from [sourceforge/gcc-win64](https://www.sourceforge.net/projects/gcc-win64/)
    - extract it and move the extracted folder somewhere like `C:\` (preferrably) or `C:\Program Files`
    - add the path of the `bin` folder inside to the `path` environment variable

```command
git clone --depth=1 https://github.com/awertnex/minecraft.c.git
cd minecraft.c
gcc .\build.c -std=c99 -o build.exe
.\build.exe
.\minecraft_c.exe
```
if build successful, run minecraft.c with `minecraft_c.exe`\
cflag `-std=c99` optional, but preferred

**additional commands**
- build launcher with `.\build launcher` and run with `.\launcher.exe`
    - the launcher creates a folder 'minecraft.c' in your 'appdata' folder
    - all minecraft.c instances will be stored in 'minecraft.c\instances'
    - the default path to 'minecraft.c' can be changed from the file 'dir.c' in the function 'init_paths()' before building
- list available tests with `build list`
    - this command searches the folder '.\src\tests' and loads all `.c` file names into an array to list
- build test [n] with `build test [n]`
    - this command searches the folder '.\src\tests' and loads all `.c` file names into an array to execute by index [n]
- help `build --help`

## Contributors (alpha testing - windows)
- [sofia](https://github.com/EdgySofia666)

