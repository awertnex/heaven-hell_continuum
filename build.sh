#!/bin/bash

set -e

DIR="$HOME/programming/minecraft.c/"
SOURCE="src/"
MAIN="main.c"
CHILDREN="logger.c launcher.c assets.c chunking.c dir.c gui.c keymaps.c logic.c super_debugger.c"
CFLAGS="-Wall -Wextra -ggdb -Wpedantic"
LIBS="-lraylib -lm"
OUT="minecraft"
EXTENSION=""

LAUNCHER_MAIN="launcher/launcher.c"
LAUNCHER_CHILDREN="logger.c keymaps.c"
LAUNCHER_LIBS="-lraylib"
LAUNCHER_OUT="launcher"

tests=(
    "chunk_loader"
    "function_pointer"
)

if [[ "$1" ]]; then
    case $1 in
        "edit")
            nv ./build.sh
            exit
            ;;

        "list")
            count=1
            echo "launcher: 0"
            echo "    TESTS:"
            for i in "${tests[@]}"; do
                printf "test $count: $i\n"
                let ++count
            done
            exit
            ;;

        "win")
            MAIN="main_win.c"
            EXTENSION=".exe";
            ;;

        "0")
            echo "Building minecraft.c Launcher.."
            MAIN=$LAUNCHER_MAIN
            CHILDREN=$LAUNCHER_CHILDREN
            LIBS=$LAUNCHER_LIBS
            OUT=$LAUNCHER_OUT
            ;;

        "1")
            echo "Building test 001: ${tests[0]}.."
            SOURCE="src/tests/"
            MAIN="${tests[0]}.c"
            CHILDREN=""
            OUT="test_${tests[0]}"
            ;;

        "2")
            echo "Building test 002: ${tests[1]}.."
            SOURCE="src/tests/"
            MAIN="${tests[1]}.c"
            CHILDREN=""
            OUT="test_${tests[1]}"
            ;;
        *)
            echo "Invalid '$1'"
            echo "Try: './build.sh list' to list all items, or './build.sh' to build 'main.c'"
            exit
            ;;
    esac
fi

pushd $SOURCE
time cc $MAIN $CHILDREN $CFLAGS $LIBS -o $OUT$EXTENSION &&
    chmod 775 $OUT &&
    mv $OUT$EXTENSION $DIR
popd
