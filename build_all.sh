#!/usr/bin/env sh

make clean_all TARGET=linux

make TARGET=linux BUILD=release
make TARGET=linux BUILD=debug
make TARGET=netcom BUILD=release
make TARGET=netcom BUILD=debug 
