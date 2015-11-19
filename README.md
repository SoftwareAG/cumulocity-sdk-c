# Cumulocity C++ Library #

C++ Library for easy connecting embedded Linux powered devices to the Cumulocity cloud.

### Prerequisites ###

* *C++ 11*
* *Linux* >= 2.6.32
* *libcurl* >= 7.26.0
* *Lua* >= 5.0 (optional, for Lua support only)

### How to build the library? ###

* Download the source code:

```
#!bash

git clone --recursive git@bitbucket.org:m2m/cumulocity-sdk-c.git
```

* Define your init.mk with device specific CXXFLAGS, LDFLAGS, LDLIBS, and CXX if cross compiling.
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
* How can I contact Cumulocity in case I have questions?  
  You can reach us by email at support@cumulocity.com