#!/bin/bash
if [ ! $1 ]; then
    echo "no C source file！"
else
    gcc -o kimi $1 ./libcjson.a -lcurl -lncurses
fi
