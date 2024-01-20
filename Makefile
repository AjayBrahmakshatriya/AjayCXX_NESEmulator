-include Makefile.inc

BASE_DIR=$(shell pwd)

BUILD_DIR?=$(BASE_DIR)/build
SRC_DIR=$(BASE_DIR)/src
INCLUDE_DIR=$(BASE_DIR)/include

BUILDIT_DIR?=$(BASE_DIR)/buildit
RUNTIME_DIR?=$(BASE_DIR)/runtime

$(shell mkdir -p $(BUILD_DIR))
$(shell mkdir -p $(BUILD_DIR)/runtime)

EXECUTABLES=$(BUILD_DIR)/nes_compiler


OBJS=$(BUILD_DIR)/driver.o $(BUILD_DIR)/emulator.o $(BUILD_DIR)/pipeline.o $(BUILD_DIR)/loader.o

INCLUDES=$(wildcard $(INCLUDE_DIR)/*.h)
RUNTIME_INCLUDES=$(wildcard $(RUNTIME_DIR)/*.h)


RUNTIME_OBJS=$(BUILD_DIR)/runtime/runtime.o

.PHONY: all
all: executables $(BUILD_DIR)/libnes_runtime.so


executables: $(EXECUTABLES)

CFLAGS=-O3 -Werror -pedantic $(shell make --no-print-directory -C $(BUILDIT_DIR) compile-flags) -I $(INCLUDE_DIR) -fPIC
LDFLAGS=$(shell make --no-print-directory -C $(BUILDIT_DIR) linker-flags) -Wl,--rpath,$(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(INCLUDES)
	$(CXX) -o $@ $< $(CFLAGS) -c -I $(RUNTIME_DIR)

$(BUILD_DIR)/nes_compiler: $(OBJS) $(BUILD_DIR)/libnes_runtime.so
	$(CXX) -o $@ $(OBJS) $(LDFLAGS) -L $(BUILD_DIR)/ -lnes_runtime

$(BUILD_DIR)/libnes_runtime.so: $(RUNTIME_OBJS)
	$(CC) -o $@ $^ -shared -fPIC

$(BUILD_DIR)/runtime/%.o: $(RUNTIME_DIR)/%.c $(RUNTIME_INCLUDES)
	$(CC) -c -o $@ $< -O3 -I $(RUNTIME_DIR) -fPIC

clean:
	- rm -rf $(BUILD_DIR)


