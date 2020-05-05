ERR = $(shell which clang++ > /dev/null;echo $$?)
ifeq "$(ERR)" "0"
	CXX = clang++
else
	CXX = g++
endif
CXXFLAGS = -std=c++17
LINKER = ld
LDLIBRARY = -ljsoncpp
SOURCES = bot.cpp
OBJECTS = bot.o
ETARGET = bot

AM = ../utils/amalgamate
AMFLAGS = -i ../utils/MahjongGB -i ../utils/MahjongGBCPP
TARGETDIR = ../build
STARGET = $(TARGETDIR)/fbot.cpp
RTARGET = $(TARGETDIR)/fbot

JUDGESOURCE = ../utils/judge/main.cpp
JUDGE = ./ju

TORCHPREFIX = /usr/local/libtorch
TORCHFLAGS = -isystem $(TORCHPREFIX)/include -isystem $(TORCHPREFIX)/include/torch/csrc/api/include -Wl,-rpath,$(TORCHPREFIX)/lib -Wl,--as-needed $(TORCHPREFIX)/lib/torch_cpu -Wl,--as-needed $(TORCHPREFIX)/lib/c10

RM = rm

.PHONY: all default clean amal judge release nn

all: default

judge: $(JUDGE)

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
	$(RM) $(OBJECTS)