#!/bin/sh

cd "$(dirname "$0")"

gcc \
game.c \
\
-std=c99 \
-O0 -g -o oko.run -lm -lcurses -D_POSIX_C_SOURCE


# core/*.c \
