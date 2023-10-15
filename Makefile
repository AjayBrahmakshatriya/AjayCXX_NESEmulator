-include Makefile.inc

BASE_DIR=$(shell pwd)

BUILD_DIR?=$(BASE_DIR)/build
SRC_DIR=$(BASE_DIR)/src
INCLUDE_DIR=$(BASE_DIR)/include

BUILDIT_DIR?=$(BASE_DIR)/buildit

$(shell mkdir -p $(BUILD_DIR))


EXECUTABLES=$(BUILD_DIR)/nes_compiler


OBJS=$(BUILD_DIR)/driver.o $(BUILD_DIR)/emulator.o
INCLUDES=$(wildcard $(INCLUDE_DIR)/*.h)

.PHONY: all
all: executables


executables: $(EXECUTABLES)

CFLAGS=-O3 -Werror -pedantic $(shell make --no-print-directory -C $(BUILDIT_DIR) compile-flags) -I $(INCLUDE_DIR)
LDFLAGS=$(shell make --no-print-directory -C $(BUILDIT_DIR) linker-flags)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(INCLUDES)
	$(CXX) -o $@ $< $(CFLAGS) -c

$(BUILD_DIR)/nes_compiler: $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	- rm -rf $(BUILD_DIR)


