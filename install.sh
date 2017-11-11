#!/bin/bash

if [ ! -f 'html' ]; then
    ./build.sh
fi

if [ -f 'html' ]; then
    if [ -d '/usr/local/bin/' ]; then
        cp 'html' '/usr/local/bin/'
    fi
fi

if [ -d '/usr/local/share/man/man1/' ]; then
    cp 'html.1' '/usr/local/share/man/man1/'
    mandb -q
fi
