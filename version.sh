#!/usr/bin/env sh
USAGE="Description: libsera release number versioner.

Usage:
$(basename $0) -h         -- Show this help.
$(basename $0) ask        -- Show ntcagent and smsagent current versions.
$(basename $0) <version>  -- Update version number to <version>."

if [ "$1" == "" -o "$1" == "-h" ]; then
    echo "$USAGE"
elif [ "$1" == "ask" ]; then
    echo -n "libsera: "
    grep 'REALNAME:=' Makefile | cut -c 25-
else
    sed -i --follow-symlinks '/REALNAME:=/c\REALNAME:=$(LIBNAME).so.'"$1" Makefile
    sed -i --follow-symlinks "/PROJECT_NUMBER/c\PROJECT_NUMBER         = $1" Doxyfile
fi
