#!/usr/bin/bash
echo "Prog : $1"
gcc  $1.c Mathutils.c -o  $1  -lSDL2 -lSDL2_image $(sdl2-config --cflags --libs ) -lm -Wall -Wextra
