# minecraft.c
- - - -
**a 1:1 minecraft clone but in C, as best as I could, with minimal library usage.
eventually I will replace all minecraft resources with my own, for copyright reasons.**

## dependencies
- [raylib](https://github.com/raysan5/raylib) by [raysan5](https://github.com/raysan5)

## build and run
#### linux:
```
git clone https://github.com/awertnex/minecraft.c.git
cd minecraft.c
chmod 775 ./build.sh
./build.sh
```
and to run it:
```
./minecraft
```
additionally, following the command with a name (e.g. `./minecraft "minecraft.c 0.1.0"` creates a minecraft.c instance directory with that name next to the executable, and if the instance exists it will open it
