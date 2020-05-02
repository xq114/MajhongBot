ERR = $(shell which g++ > /dev/null;echo $$?)
ifeq "$(ERR)" "0"
	CXX = g++
else
	CXX = clang++
endif
CXXFLAGS = -std=c++17
LDLIBRARY = -ljsoncpp
AM = ../utils/amalgamate
AMFLAGS = -i ../utils/MahjongGB -i ../utils/MahjongGBCPP
SOURCES = bot.cpp
OBJECTS = bot.o ../utils/MahjongGBCPP/MahjongGB.o
TARGETDIR = ../build
STARGET = $(TARGETDIR)/fbot.cpp
ETARGET = $(TARGETDIR)/fbot
JUDGESOURCE = ../utils/judge/main.cpp
JUDGE = ../utils/judge/main
RM = rm

.PHONY: all default clean amal judge mk

all: default

judge: $(JUDGE)

default: $(ETARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ -c

$(ETARGET): $(OBJECTS) mk
	mkdir -p $(TARGETDIR)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDLIBRARY)

$(JUDGE): $(JUDGESOURCE)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDLIBRARY)

amal: $(STARGET)

$(STARGET): $(SOURCES)
	$(AM) $(AMFLAGS) $(SOURCES) $(STARGET)

clean:
	$(RM) $(TARGETDIR)/*
	$(RM) $(OBJECTS)