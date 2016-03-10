LUA=yes
CPPFLAGS:=$(shell pkg-config --cflags libcurl lua)
CXXFLAGS:=-Wall -pedantic -Wextra
LDLIBS:=$(shell pkg-config --libs libcurl lua)
