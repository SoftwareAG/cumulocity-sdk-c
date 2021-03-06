# Cumulocity C++ Library #

Cumulocity C++ Library is an asynchronous, event-driven networking library to ease your development for connecting to the Cumulocity cloud. The library uses Cumulocity's self invented SmartREST protocol for M2M communication which targets any device that are capable of running embedded Linux.

### Prerequisites ###

* *C++ 11*
* *Linux* >= 2.6.32
* *libcurl* >= 7.57.0
* *Lua* >= 5.1 (optional, for Lua support)

### How to build the library? ###

* Download the source code:

```
#!bash

git clone git@bitbucket.org:m2m/cumulocity-sdk-c.git
```

* Pull in all sub-module dependencies.
```
#!bash
git submodule init
git submodule update
```

* Define your init.mk with device specific CPPFLAGS, CXXFLAGS, LDFLAGS, LDLIBS (see common.mk for reference), and CXX if cross compiling.
* Copy the template *Makefile*:

```
#!bash

cp Makefile.template Makefile
```

* Customize the *Makefile* to your needs.
* Build the library in *debug* mode:

```
#!bash

make
```

* Starts developing your Cumulocity agent and link it to the library with **-lsera**.
* Build the library in *release* mode for production release:

```
#!bash

make release
```


### FAQ ###
* I encountered an error when building the library.  
  Try removing all intermediate build files first and start a new clean build:

```
#!bash

make clean
```
* How can I enable *Lua* support when building the library?  
  *Lua* support is disabled by default, to enable *Lua* support, add:

```
#!bash

SR_PLUGIN_LUA:=1
```
  to your init.mk.

* How can I contact Cumulocity in case I have questions?  
  You can reach us by email at Software AG Cumulocity IoT <Helpdesk-CumulocityIoT@softwareag.com>