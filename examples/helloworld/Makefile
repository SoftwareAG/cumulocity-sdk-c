SRC_DIR:=.
BUILD_DIR:=build
SRC:=$(wildcard $(SRC_DIR)/*.cc)
OBJ:=$(addprefix $(BUILD_DIR)/,$(notdir $(SRC:.cc=.o)))

BIN_DIR:=bin
BIN:=c8yhelloworld-agent
CXXFLAGS:=-Wall -pedantic -Wextra -std=c++11\
		  -I$(C8Y_LIB_PATH)/include -MMD -O2 -DNDEBUG
LDFLAGS:=-O2 -s -flto -L../../lib
LDLIBS:=-lsera

.PHONY: all clean

all: $(BIN_DIR)/$(BIN)
	@:

$(BIN_DIR)/$(BIN): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -c -o $@

clean:
	@rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d $(BIN_DIR)/$(BIN)

-include $(OBJ:.o=.d)
