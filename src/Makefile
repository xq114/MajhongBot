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

TORCHPREFIX = /usr/local/src/libtorch
TORCHFLAGS = -isystem $(TORCHPREFIX)/include -isystem $(TORCHPREFIX)/include/torch/csrc/api/include -Wl,-rpath,$(TORCHPREFIX)/lib $(TORCHPREFIX)/lib/libtorch.so $(TORCHPREFIX)/lib/libc10.so -Wl,--no-as-needed,$(TORCHPREFIX)/lib/libtorch_cpu.so -Wl,--as-needed $(TORCHPREFIX)/lib/libc10.so -Wl,--no-as-needed,$(TORCHPREFIX)/lib/libtorch.so -Wl,--as-needed

RM = rm

.PHONY: all default clean amal jud release nn

all: default

jud: $(JUDGE)

default: $(ETARGET)

release: $(RTARGET)

nn: botnn

$(ETARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -g $< -o $@ $(LDLIBRARY)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(RTARGET): $(OBJECTS)
	mkdir -p $(TARGETDIR)
	$(CXX) $(OBJECTS) $(LDLIBRARY) -o $@

$(JUDGE): $(JUDGESOURCE)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDLIBRARY)

botnn: botnn.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(TORCHFLAGS)

amal: $(STARGET)

$(STARGET): $(SOURCES)
	$(AM) $(AMFLAGS) $(SOURCES) $(STARGET)

clean:
	$(RM) $(TARGETDIR)/*
	$(RM) $(OBJECTS) botnn