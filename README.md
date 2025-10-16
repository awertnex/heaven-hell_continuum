# Heaven/Hell Continuum

<h1 align="center">
  <img src="resources/logo/128x128.png" alt="Heaven-Hell Continuum">
</h1>

**a voxel game/engine in C99, with minimal dependencies if any.**


## Download (unzip and launch)

Format | Linux (latest) | Windows (latest)
--- | --- | ---
tar | [v0.2.0-beta-linux-x86_64.tar.gz](https://github.com/awertnex/heaven-hell_continuum/releases/download/v0.2.0-beta/heaven-hell_continuum-v0.2.0-beta-linux-x86_64.tar.gz)
zip | [v0.2.0-beta-linux-x86_64.zip](https://github.com/awertnex/heaven-hell_continuum/releases/download/v0.2.0-beta/heaven-hell_continuum-v0.2.0-beta-linux-x86_64.zip) | not yet available
[All Release Builds](https://github.com/awertnex/heaven-hell_continuum/blob/main/RELEASE_BUILDS.md)


## Dependencies (already bundled)

- [glfw v3.4](https://github.com/glfw/glfw/releases) (headers modified)
- [glad v0.1.36](https://github.com/dav1dde/glad-web) (header modified)
- [stb_image.h v2.30](https://github.com/nothings/stb/blob/master/stb_image.h) (modified)
- [stb_image_write.h v1.26](https://github.com/nothings/stb/blob/master/stb_image_write.h) (modified, unused)
- [stb_truetype.h v1.26](https://github.com/nothings/stb/blob/master/stb_truetype.h) (modified)
- [dejavu-fonts v2.37](https://github.com/dejavu-fonts/dejavu-fonts) (modified)
    - dejavu_sans_ansi.ttf (subset: U+0000-00ff)
    - dejavu_sans_bold_ansi.ttf (subset: U+0000-00ff)
    - dejavu_sans_mono_ansi.ttf (subset: U+0000-00ff)
    - dejavu_sans_mono_bold_ansi.ttf (subset: U+0000-00ff)

### windows-specific:
- [w64devkit v2.4.0](https://github.com/skeeto/w64devkit) (not necessary for runtime, just for building from source)


## Build From Source

>**NOTES:**
>- for development build, turn off `RELEASE_BUILD` in `src/engine/h/logger.h` (set as `0`), enables TRACE and DEBUG logging.
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
- (I broke this command) `./build engine`: build engine, and run with `./Heaven-Hell\ Continuum/engine`
- (I broke this command) `./build launcher`: build launcher (for instance management), run with `./Heaven-Hell\ Continuum/launcher`
    - all 'Heaven-Hell Continuum' instances will be stored in 'Heaven-Hell Continuum/instances/'
- `./build list`: list all available options and tests
    - this command searches the directory 'src/tests/' and loads all `.c` file names into an array to list
- `./build show`: show build command
- `./build raw`: show build command, raw
- (I broke this command) `./build test [n]`: build test [n]
    - this command searches the directory 'src/tests/' and loads all `.c` file names into an array to execute by index [n]

- - -

### for windows (using any C compiler, a suggestion is "gcc" from "mingw"): (it is very broken)

if you don't already have a C compiler:
- [w64devkit v2.4.0](https://github.com/skeeto/w64devkit) (includes gcc toolchain)

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
- (I broke this command) `./build.exe engine`: build engine, and run with `./Heaven-Hell\ Continuum/engine.exe`
- (I broke this command) `./build.exe launcher`: build launcher (for instance management), run with `./Heaven-Hell\ Continuum/launcher.exe`
    - all 'Heaven-Hell Continuum' instances will be stored in 'Heaven-Hell Continuum/instances/'
- `./build.exe list`: list all available options and tests
    - this command searches the folder './src/tests' and loads all `.c` file names into an array to list
- `./build.exe show`: show build command
- `./build.exe raw`: show build command, raw
- (I broke this command) `./build.exe test [n]`: build test [n]
    - this command searches the folder './src/tests' and loads all `.c` file names into an array to execute by index [n]

- - -

## Build The Build Tool:

```command
gcc.exe build.c -o build.exe
```

## Contributing:
currently not accepting contributions since the project is in an early stage, maybe when I have something stable and actually usable

## Contributors:
- [sofia](https://github.com/EdgySofia666): alpha-testing on windows

