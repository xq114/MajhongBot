ERR = $(shell which clang++ > /dev/null;echo $$?)
ifeq "$(ERR)" "0"
	CXX = clang++
else
	CXX = g++
endif
CXXFLAGS = -std=c++17
LINKER = ld
LDLIBRARY = -ljsoncpp
SOURCEDIR = .
SOURCES = $(SOURCEDIR)/bot.cpp
OBJECTS = $(SOURCEDIR)/bot.o
ETARGET = $(SOURCEDIR)/bot

AM = ../utils/amalgamate
AMFLAGS = -i ../utils/MahjongGB -i ../utils/MahjongGBCPP
TARGETDIR = ../build
STARGET = $(TARGETDIR)/fbot.cpp
RTARGET = $(TARGETDIR)/fbot

JUDGESOURCE = ../utils/judge/main.cpp
JUDGE = $(SOURCEDIR)/judge

NNTARGET = ./botnn_script
NNSOURCE = ./botnn_script.cpp

TORCHPREFIX = /usr/local/src/libtorch_cxx11
TORCHINCLUDES = -isystem $(TORCHPREFIX)/include -isystem $(TORCHPREFIX)/include/torch/csrc/api/include
ERR = $(shell which nvcc > /dev/null;echo $$?)
ifeq "$(ERR)" "0"
	CUDAPREFIX = /usr/local/cuda
	CUDAINCLUDES = -isystem $(CUDAPREFIX)/include
	TORCHLIBS = -Wl,-rpath,$(TORCHPREFIX)/lib:$(CUDAPREFIX)/lib64/stubs:$(CUDAPREFIX)/lib64 $(TORCHPREFIX)/lib/libtorch.so $(TORCHPREFIX)/lib/libc10.so $(CUDAPREFIX)/lib64/stubs/libcuda.so $(CUDAPREFIX)/lib64/libnvrtc.so $(CUDAPREFIX)/lib64/libnvToolsExt.so $(CUDAPREFIX)/lib64/libcudart.so $(TORCHPREFIX)/lib/libc10_cuda.so -Wl,--no-as-needed,$(TORCHPREFIX)/lib/libtorch_cpu.so -Wl,--as-needed -lpthread -Wl,--no-as-needed,$(TORCHPREFIX)/lib/libtorch_cuda.so -Wl,--as-needed $(TORCHPREFIX)/lib/libc10_cuda.so $(TORCHPREFIX)/lib/libc10.so $(CUDAPREFIX)/lib64/libcufft.so $(CUDAPREFIX)/lib64/libcurand.so -lcublas /usr/lib/x86_64-linux-gnu/libcudnn.so -Wl,--no-as-needed,$(TORCHPREFIX)/lib/libtorch.so -Wl,--as-needed $(CUDAPREFIX)/lib64/libnvToolsExt.so $(CUDAPREFIX)/lib64/libcudart.so
else
	TORCHLIBS = -Wl,-rpath,$(TORCHPREFIX)/lib $(TORCHPREFIX)/lib/libtorch.so $(TORCHPREFIX)/lib/libc10.so -Wl,--no-as-needed,$(TORCHPREFIX)/lib/libtorch_cpu.so -Wl,--as-needed -lpthread -Wl,--no-as-needed,$(TORCHPREFIX)/lib/libtorch.so -Wl,--as-needed $(TORCHPREFIX)/lib/libc10.so
endif

RM = rm

.PHONY: all default clean amal jud release nn

all: default

jud: $(JUDGE)

default: $(ETARGET)

release: $(RTARGET)

nn: $(NNTARGET)

$(ETARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -g $< -o $@ $(LDLIBRARY)

$(NNTARGET): $(NNSOURCE)
	$(CXX) $(CXXFLAGS) -g $< $(TORCHINCLUDES) $(CUDAINCLUDES) -o $@ $(LDLIBRARY) $(TORCHLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(RTARGET): $(OBJECTS)
	mkdir -p $(TARGETDIR)
	$(CXX) $(OBJECTS) $(LDLIBRARY) -o $@

$(JUDGE): $(JUDGESOURCE)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDLIBRARY)

amal: $(STARGET)

$(STARGET): $(SOURCES)
	$(AM) $(AMFLAGS) $(SOURCES) $(STARGET)

clean:
	$(RM) $(TARGETDIR)/*
	$(RM) $(OBJECTS) botnn