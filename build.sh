#!/bin/bash

SOURCE="src/"
MAIN="minecraft_linux.c"
CHILDREN="chunking.c gui.c logic.c assets.c super_debugger.c"
CFLAGS="-Wall -Wextra"
LIBS="-lraylib -lm"
PLATFORM=""

tests=(
"chunk_loader"
)

case $1 in
    "list")
        local count=0
        echo "    TESTS:"
        for i in "${tests[@]}"; do
            printf "test %03g$count: ${tests[$i]}\n"
            let ++count
        done
        exit
        ;;

    "win")
        MAIN="minecraft_win.c"
        PLATFORM="_win.exe";
        ;;

    "0")
        MAIN="${tests[0]}.c"
        CHILDREN=""
        ;;
esac

pushd $SOURCE
time gcc $MAIN $CHILDREN $CFLAGS $LIBS -o minecraft$PLATFORM
if [ "$?" = 0 ]; then
    chmod 775 minecraft
    mv minecraft$PLATFORM ../
fi
popd
echo "Exit Code: $?"
