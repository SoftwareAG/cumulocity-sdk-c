CXXFLAGS=$(shell pkg-config --cflags lua) -I$(C8Y_LIB_PATH)/include -std=c++11 -pthread
bin/ex-06-lua: integrate.cc main.cc
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $^ -L$(C8Y_LIB_PATH)/lib -lsera -o $@
