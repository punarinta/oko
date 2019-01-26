#!/bin/sh

cd "$(dirname "$0")"

gcc \
game.c \
core/*.c \
\
-std=c99 \
-O0 -g -o oko.run -lm -lncursesw -D_POSIX_C_SOURCE
