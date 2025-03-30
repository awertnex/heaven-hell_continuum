#!/bin/bash

set -e

DIR="$HOME/programming/minecraft.c/"
SOURCE="src/"
MAIN="minecraft_linux.c"
CHILDREN="logger.c assets.c chunking.c dir.c gui.c keymaps.c launcher.c logic.c super_debugger.c"
CFLAGS="-Wall -Wextra -ggdb"
LIBS="-lraylib -lm"
OUT="minecraft"
PLATFORM=""

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
            echo "    TESTS:"
            for i in "${tests[@]}"; do
                printf "test $count: $i\n"
                let ++count
            done
            exit
            ;;

        "win")
            MAIN="minecraft_win.c"
            PLATFORM="_win.exe";
            ;;

        "1")
            echo "Building test 001: ${tests[0]}"
            SOURCE="src/tests/"
            MAIN="${tests[0]}.c"
            CHILDREN=""
            OUT="test_${tests[0]}"
            ;;

        "2")
            echo "Building test 002: ${tests[1]}"
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
time gcc $MAIN $CHILDREN $CFLAGS $LIBS -o $OUT$PLATFORM &&
    chmod 775 $OUT &&
    mv $OUT$PLATFORM $DIR
popd
