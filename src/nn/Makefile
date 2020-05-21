ERR = $(shell which clang++ > /dev/null;echo $$?)
ifeq "$(ERR)" "0"
	CXX = clang++
else
	CXX = g++
endif
CXXFLAGS = -std=c++17 -Wall -Wextra -D_GLIBCXX_USE_CXX11_ABI=0 -DAT_PARALLEL_OPENMP=1 -fopenmp
SHARED = -fPIC -shared
RM = rm

TORCHPREFIX = /usr/local/src/libtorch
TORCHINCLUDES = -isystem $(TORCHPREFIX)/include -isystem $(TORCHPREFIX)/include/torch/csrc/api/include
ERR = $(shell which nvcc > /dev/null;echo $$?)
ifeq "$(ERR)" "0"
	CUDAPREFIX = /usr/local/cuda
	CUDAINCLUDES = -isystem $(CUDAPREFIX)/include
	TORCHLIBS = -Wl,-rpath,$(TORCHPREFIX)/lib:$(CUDAPREFIX)/lib64/stubs:$(CUDAPREFIX)/lib64 $(TORCHPREFIX)/lib/libtorch.so $(TORCHPREFIX)/lib/libc10.so $(CUDAPREFIX)/lib64/stubs/libcuda.so $(CUDAPREFIX)/lib64/libnvrtc.so $(CUDAPREFIX)/lib64/libnvToolsExt.so $(CUDAPREFIX)/lib64/libcudart.so $(TORCHPREFIX)/lib/libc10_cuda.so -Wl,--no-as-needed,$(TORCHPREFIX)/lib/libtorch_cpu.so -Wl,--as-needed -lpthread -Wl,--no-as-needed,$(TORCHPREFIX)/lib/libtorch_cuda.so -Wl,--as-needed $(TORCHPREFIX)/lib/libc10_cuda.so $(TORCHPREFIX)/lib/libc10.so $(CUDAPREFIX)/lib64/libcufft.so $(CUDAPREFIX)/lib64/libcurand.so -lcublas /usr/lib/x86_64-linux-gnu/libcudnn.so -Wl,--no-as-needed,$(TORCHPREFIX)/lib/libtorch.so -Wl,--as-needed $(CUDAPREFIX)/lib64/libnvToolsExt.so $(CUDAPREFIX)/lib64/libcudart.so
else
	TORCHLIBS = -Wl,-rpath,$(TORCHPREFIX)/lib $(TORCHPREFIX)/lib/libtorch.so $(TORCHPREFIX)/lib/libc10.so -Wl,--no-as-needed,$(TORCHPREFIX)/lib/libtorch_cpu.so -Wl,--as-needed -lpthread -Wl,--no-as-needed,$(TORCHPREFIX)/lib/libtorch.so -Wl,--as-needed $(TORCHPREFIX)/lib/libc10.so
endif

TARGET_LIB = txt2batch.so
HEADER_LIB = txt2batch.h statemachine.h
SOURCE_LIB = txt2batch.cpp statemachine.cpp

.PHONY:
	all default clean debug

all: default

default: $(TARGET_LIB)

$(TARGET_LIB): $(SOURCE_LIB) $(HEADER_LIB)
	$(CXX) $(CXXFLAGS) $(SHARED) $(SOURCE_LIB) $(TORCHINCLUDES) $(CUDAINCLUDES) -o $@ $(TORCHLIBS)

debug: batchtest

batchtest: $(SOURCE_LIB) $(HEADER_LIB) batchtest.cpp
	$(CXX) $(CXXFLAGS) -g $(SOURCE_LIB) batchtest.cpp $(TORCHINCLUDES) $(CUDAINCLUDES) -o $@ $(TORCHLIBS)

clean:
	$(RM) $(TARGET_LIB)