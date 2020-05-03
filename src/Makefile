ERR = $(shell which clang++ > /dev/null;echo $$?)
ifeq "$(ERR)" "0"
	CXX = clang++
else
	CXX = g++
endif
CXXFLAGS = -std=c++17
LINKER = ld
LDLIBRARY = -ljsoncpp
AM = ../utils/amalgamate
AMFLAGS = -i ../utils/MahjongGB -i ../utils/MahjongGBCPP
SOURCES = bot.cpp
OBJECTS = bot.o
TARGETDIR = ../build
STARGET = $(TARGETDIR)/fbot.cpp
RTARGET = $(TARGETDIR)/fbot
ETARGET = bot
JUDGESOURCE = ../utils/judge/main.cpp
JUDGE = ./ju
RM = rm

.PHONY: all default clean amal judge release

all: default

judge: $(JUDGE)

default: $(ETARGET)

release: $(RTARGET)

$(ETARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -g $< -o $@ $(LDLIBRARY)

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
	$(RM) $(OBJECTS)