ERR = $(shell which clang++ > /dev/null;echo $$?)
ifeq "$(ERR)" "0"
	CXX = clang++
else
	CXX = g++
endif
CXXFLAGS = -std=c++17 -Wall -Wextra -fPIC
RM = rm

TORCHPREFIX = /usr/local/src/libtorch
TORCHFLAGS = -isystem $(TORCHPREFIX)/include -isystem $(TORCHPREFIX)/include/torch/csrc/api/include -Wl,-rpath,$(TORCHPREFIX)/lib $(TORCHPREFIX)/lib/libtorch.so $(TORCHPREFIX)/lib/libc10.so -Wl,--no-as-needed,$(TORCHPREFIX)/lib/libtorch_cpu.so -Wl,--as-needed $(TORCHPREFIX)/lib/libc10.so -Wl,--no-as-needed,$(TORCHPREFIX)/lib/libtorch.so -Wl,--as-needed

TARGET_LIB = txt2batch.so
HEADER_LIB = txt2batch.h
SOURCE_LIB = txt2batch.cpp

.PHONY:
	all default clean

all: default

default: $(TARGET_LIB)

$(TARGET_LIB): $(SOURCE_LIB) $(HEADER_LIB)
	$(CXX) $(CXXFLAGS) -shared $< -o $@ $(TORCHFLAGS)

clean:
	$(RM) $(TARGET_LIB)