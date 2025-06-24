# Heaven/Hell Continuum

<h1 align="center">
  <img src="resources/logo/128x128.png" alt="Heaven-Hell Continuum">
</h1>

**a voxel game/engine written in C99, with minimal library usage if any.**

## Dependencies (already bundled in download files)
### windows:
- [glfw](https://github.com/glfw/glfw/releases)
- [glew](https://github.com/nigels-com/glew/releases)
- [mingw (optional, for 'gcc' command)](https://www.mingw-w64.org/downloads/)
- [gcc (optional)](https://www.sourceforge.net/projects/gcc-win64/) (setup instructions found below)

### linux:
    - [glfw](https://github.com/glfw/glfw/releases)
    - [glew](https://github.com/nigels-com/glew/releases)


## Download
- [v0.1.4_win64.zip](https://github.com/awertnex/heaven-hell_continuum/releases/download/v0.1.4-alpha/heaven-hell_continuum-v0.1.4-win64.zip)
- [v0.1.4_linux-x86_64.tar.gz](https://github.com/awertnex/heaven-hell_continuum/releases/download/v0.1.4-alpha/heaven-hell_continuum-v0.1.4-linux-x86_64.tar.gz)
- [v0.1.4_linux-x86_64.zip](https://github.com/awertnex/heaven-hell_continuum/releases/download/v0.1.4-alpha/heaven-hell_continuum-v0.1.4-linux-x86_64.zip)
- unzip and launch

## All Release Builds:
- [Release Builds](https://github.com/awertnex/heaven-hell_continuum/blob/main/RELEASE_BUILDS.md)


## Build From Source

### For development build, turn off `RELEASE_BUILD` in `src/engine/h/logger.h` (set as `0`) before building.

### linux:

```bash
git clone --depth=1 https://github.com/awertnex/heaven-hell_continuum.git
cd heaven-hell_continuum/
./build
```
if build successful, run 'Heaven-Hell Continuum' with `./Heaven-Hell\ Continuum/hhc`\

### Build The Build Tool:
```bash
gcc build.c -std=c99 -o build # -std=c99 optional, but preferred
```

**additional build commands**
- `./build help`: show help
- `./build engine`: build engine, and run with `./Heaven-Hell\ Continuum/engine`
- `./build launcher`: build launcher (for instance management), run with `./Heaven-Hell\ Continuum/launcher`
    - all 'Heaven-Hell Continuum' instances will be stored in 'Heaven-Hell Continuum/instances/'
- `./build list`: list all available options and tests
    - this command searches the directory 'src/tests/' and loads all `.c` file names into an array to list
- `./build show`: show build command
- `./build raw`: show build command, raw
- `./build test [n]`: build test [n]
    - this command searches the directory 'src/tests/' and loads all `.c` file names into an array to execute by index [n]


### windows (using mingw and gcc):

**requirements**
- a c compiler (gcc from `mingw-w64`)
    - download `gcc` for windows from [sourceforge/gcc-win64](https://www.sourceforge.net/projects/gcc-win64/)
    - extract it and move the extracted folder somewhere like `C:\` (preferrably) or `C:\Program Files`
    - add the path of the `bin` folder inside to the `path` environment variable

```command
git clone --depth=1 https://github.com/awertnex/heaven-hell_continuum.git
cd heaven-hell_continuum
./build.exe
```
if build successful, run 'Heaven-Hell Continuum' with `./Heaven-Hell\ Continuum/hhc.exe`

### Build The Build Tool:
```command
gcc build.c -std=c99 -o build.exe
```

**additional build commands**
- `./build.exe help`: show help
- `./build.exe engine`: build engine, and run with `./Heaven-Hell\ Continuum/engine.exe`
- `./build.exe launcher`: build launcher (for instance management), run with `./Heaven-Hell\ Continuum/launcher.exe`
    - all 'Heaven-Hell Continuum' instances will be stored in 'Heaven-Hell Continuum/instances'
- `./build.exe list`: list all available options and tests
    - this command searches the folder './src/tests' and loads all `.c` file names into an array to list
- `./build.exe show`: show build command
- `./build.exe raw`: show build command, raw
- `./build.exe test [n]`: build test [n]
    - this command searches the folder './src/tests' and loads all `.c` file names into an array to execute by index [n]

## Contributors:
- [sofia](https://github.com/EdgySofia666): alpha-testing on windows

