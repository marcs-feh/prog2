#!/usr/bin/env sh

cc=gcc
cflags='-std=c11 -Wall -Wextra -fPIC -O0'

set -eu

$cc $cflags -c prog2.c -o prog2.o &
$cc $cflags -c main.c -o main.o &
wait
$cc main.o prog2.o -o prog2.bin

