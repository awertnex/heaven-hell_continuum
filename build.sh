#!/bin/bash

SOURCE='src/'
MAIN='minecraft_linux.c'
CHILDREN='chunking.c gui.c logic.c assets.c super_debugger.c'
CFLAGS='-Wall -Wextra'
LIBS='-lraylib -lm'
PLATFORM=''

if [ "$1" = "win" ]; then
	MAIN='minecraft_win.c'
	PLATFORM='_win.exe';
fi

pushd $SOURCE
time gcc $MAIN $CHILDREN $CFLAGS $LIBS -o minecraft$PLATFORM
if [ "$?" = 0 ]; then
    chmod 775 minecraft
    mv minecraft$PLATFORM ../
fi
popd
echo "Exit Code: $?"
