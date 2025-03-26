#!/bin/bash

SOURCE="src/"
MAIN="minecraft_linux.c"
CHILDREN="chunking.c gui.c keymaps.c logic.c assets.c super_debugger.c"
CFLAGS="-Wall -Wextra"
LIBS="-lraylib -lm"
OUT="minecraft"
PLATFORM=""

tests=(
    "chunk_loader"
    "function_pointer"
)

case $1 in
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
        SOURCE="src/tests/"
        MAIN="${tests[0]}.c"
        CHILDREN=""
        OUT="test_${tests[0]}"
        ;;
        
    "2")
        SOURCE="src/tests/"
        MAIN="${tests[1]}.c"
        CHILDREN=""
        OUT="test_${tests[1]}"
        ;;
esac

pushd $SOURCE
time gcc $MAIN $CHILDREN $CFLAGS $LIBS -o $OUT$PLATFORM &&
    chmod 775 $OUT &&
    mv $OUT$PLATFORM ../
popd
