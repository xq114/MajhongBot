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
ETARGET = $(TARGETDIR)/fbot
JUDGESOURCE = ../utils/judge/main.cpp
JUDGE = ../utils/judge/main
RM = rm

.PHONY: all default clean amal judge

all: default

judge: $(JUDGE)

default: $(ETARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(ETARGET): $(OBJECTS)
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