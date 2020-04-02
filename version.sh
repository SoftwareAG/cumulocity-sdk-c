#!/usr/bin/env sh
USAGE="Description: libsera release number versioner.

Usage:
$(basename $0) -h                -- Show this help.
$(basename $0) ask               -- Show current version.
$(basename $0) update <version>  -- Update version number to <version>."

if [ "$1" = "" -o "$1" = "-h" ]; then
    echo "$USAGE"
elif [ "$1" = "ask" ]; then
    echo -n "libsera: "
    grep 'REALNAME:=' Makefile | cut -c 21-
elif [ "$1" = "update" ]; then
    sed -i --follow-symlinks '/REALNAME:=/c\REALNAME:=$(SONAME).'"$2" Makefile.template
    sed -i --follow-symlinks "/PROJECT_NUMBER/c\PROJECT_NUMBER         = $2" Doxyfile
    sed -i --follow-symlinks "s/Version [0-9]\.[0-9][0-9]*/Version $2/" doc/title.tex
fi
exit 0