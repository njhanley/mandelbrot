#!/bin/sh
mkdir -p bin/linux
gcc -o bin/linux/out main.c $(sdl2-config --cflags --libs)
