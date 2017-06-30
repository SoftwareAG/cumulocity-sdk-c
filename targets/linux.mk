SR_PLUGIN_LUA:=1
CXX:=/usr/bin/g++
CPPFLAGS:=$(shell pkg-config --cflags libcurl lua5.2)
CXXFLAGS:=-Wall -pedantic -Wextra
LDLIBS:=$(shell pkg-config --libs libcurl lua5.2)
