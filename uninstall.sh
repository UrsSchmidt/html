#!/bin/bash

if [ -f '/usr/local/bin/html' ]; then
    rm '/usr/local/bin/html'
fi

if [ -f '/usr/local/share/man/man1/html.1' ]; then
    rm '/usr/local/share/man/man1/html.1'
    mandb -q
fi
