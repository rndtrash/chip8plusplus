CXX = g++
CFLAGS = -O1 -std=c++14 $(shell sdl2-config --cflags) -lfmt $(shell sdl2-config --libs) -lGL

ifeq ($(DEBUG), YES)
    CFLAGS += -g
endif

.PHONY: all

all:
	mkdir -p bin
	$(CXX) src/main.cpp src/chip8.cpp $(CFLAGS) -o bin/chip8plusplus
