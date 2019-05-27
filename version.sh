#!/usr/bin/env sh
USAGE="Description: libsera release number versioner.

Usage:
$(basename $0) -h         -- Show this help.
$(basename $0) ask        -- Show ntcagent and smsagent current versions.
$(basename $0) <version>  -- Update version number to <version>."

if [ "$1" = "" -o "$1" = "-h" ]; then
    echo "$USAGE"
elif [ "$1" = "ask" ]; then
    echo -n "libsera: "
    grep 'REALNAME:=' Makefile | cut -c 21-
else
    sed -i --follow-symlinks '/REALNAME:=/c\REALNAME:=$(SONAME).'"$1" Makefile
    sed -i --follow-symlinks "/PROJECT_NUMBER/c\PROJECT_NUMBER         = $1" Doxyfile
    sed -i --follow-symlinks "s/Version [0-9]\.[0-9][0-9]*/Version $1/" doc/title.tex
fi
