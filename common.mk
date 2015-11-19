CXXFLAGS:=-Wall -pedantic -Wextra\
		  $(shell pkg-config --cflags libcurl lua)
LDLIBS:=$(shell pkg-config --libs libcurl lua)
