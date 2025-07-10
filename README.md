# Heaven/Hell Continuum

<h1 align="center">
  <img src="resources/logo/128x128.png" alt="Heaven-Hell Continuum">
</h1>

**a voxel game/engine written in C99, with minimal library usage if any.**

## Dependencies (already bundled)

- [glfw v3.5](https://github.com/glfw/glfw/releases) (headers modified)
- [glad v0.1.36](https://github.com/dav1dde/glad-web) (header modified)
- [stb_truetype.h v1.26](https://github.com/nothings/stb/blob/master/stb_truetype.h) (modified)
- [dejavu-fonts v2.37](https://github.com/dejavu-fonts/dejavu-fonts) (modified)
    - dejavu_sans_mono_ansi.ttf (subset: U+0000-00ff)
    - dejavu_sans_mono_ansi_bold.ttf (subset: U+0000-00ff)

### windows-specific:
- some mingw headers (modified)

## Download (unzip and launch)

Platform | Zip | Tar
--- | --- | ---
Linux (latest) | [v0.2.0-alpha_linux-x86_64.zip](https://github.com/awertnex/heaven-hell_continuum/releases/download/v0.2.0-alpha/heaven-hell_continuum_v0.2.0-alpha_linux-x86_64.zip) | [v0.2.0-alpha_linux-x86_64.tar.gz](https://github.com/awertnex/heaven-hell_continuum/releases/download/v0.2.0-alpha/heaven-hell_continuum_v0.2.0-alpha_linux-x86_64.tar.gz)
Windows (latest) | [v0.2.0-alpha_win64.zip](https://github.com/awertnex/heaven-hell_continuum/releases/download/v0.2.0-alpha/heaven-hell_continuum_v0.2.0-alpha_win64.zip)
[All Release Builds](https://github.com/awertnex/heaven-hell_continuum/blob/main/RELEASE_BUILDS.md)


## Build From Source

>**NOTES:**
>- for development build, turn off `RELEASE_BUILD` in `src/engine/h/logger.h` (set as `0`) to enable TRACE and DEBUG logging.
>- if build successful, you can place the built directory 'Heaven-Hell Continuum' anywhere you wish, that's the entire bundle.
>- the build tool will rebuild itself before building the project if you either modified its source or didn't build it in C99.

- - -

### for linux:

1. clone and build:

```bash
git clone --depth=1 https://github.com/awertnex/heaven-hell_continuum.git
cd heaven-hell_continuum/
./build
```

2. run:

```bash
./Heaven-Hell\ Continuum/hhc
```

### additional build commands:

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

- - -

### for windows (using mingw and gcc):

if you don't already have them (or any compiler that does the job):
- download `gcc`: [sourceforge/gcc-win64](https://www.sourceforge.net/projects/gcc-win64/)
- download `mingw`: [mingw-w64](https://www.mingw-w64.org/downloads/)

1. clone and build:

```command
git clone --depth=1 https://github.com/awertnex/heaven-hell_continuum.git
cd heaven-hell_continuum
./build.exe
```

2. run:

```command
`./Heaven-Hell\ Continuum/hhc.exe`.
```

### additional build commands:

- `./build.exe help`: show help
- `./build.exe engine`: build engine, and run with `./Heaven-Hell\ Continuum/engine.exe`
- `./build.exe launcher`: build launcher (for instance management), run with `./Heaven-Hell\ Continuum/launcher.exe`
    - all 'Heaven-Hell Continuum' instances will be stored in 'Heaven-Hell Continuum/instances/'
- `./build.exe list`: list all available options and tests
    - this command searches the folder './src/tests' and loads all `.c` file names into an array to list
- `./build.exe show`: show build command
- `./build.exe raw`: show build command, raw
- `./build.exe test [n]`: build test [n]
    - this command searches the folder './src/tests' and loads all `.c` file names into an array to execute by index [n]

- - -

## Build The Build Tool:

```command
gcc.exe build.c -o build.exe
```

## Contributing:
I'm currently not accepting contributions at this early stage, maybe when I have something more stable and actually usable

## Contributors:
- [sofia](https://github.com/EdgySofia666): alpha-testing on windows

