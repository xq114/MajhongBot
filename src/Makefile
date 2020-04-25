CXX = g++
CXXFLAGS = -std=c++17
LDLIBRARY = -ljsoncpp
# PY = python
# AMAL = ../utils/amalgamate/amalgamate.py
AM = ../utils/amalgamate
AMFLAGS = -i ../utils/MahjongGB
# CONFIG = config.json
# DIR = .
SOURCES = bot.cpp
TARGETDIR = ../build
STARGET = $(TARGETDIR)/fbot.cpp
ETARGET = $(TARGETDIR)/fbot.exe
RM = rm

.PHONY: all default

all: default

default: $(ETARGET)

$(STARGET): $(SOURCES)
	$(AM) $(AMFLAGS) $(SOURCES) $(STARGET)

# $(STARGET): $(SOURCES) $(CONFIG)
# $(PY) $(AMAL) -c $(CONFIG) -s $(DIR)

$(ETARGET): $(STARGET)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDLIBRARY)

clean:
	$(RM) $(TARGETDIR)/*