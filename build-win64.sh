#!/bin/sh
mkdir -p bin/win64
x86_64-w64-mingw32-gcc -o bin/win64/out.exe main.c $(x86_64-w64-mingw32-sdl2-config --cflags --libs)
cp /usr/x86_64-w64-mingw32/bin/SDL2.dll bin/win64/
