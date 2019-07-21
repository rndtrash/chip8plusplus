CFLAGS = -O1 -std=c++14 -lfmt

ifeq ($(DEBUG), YES)
    CFLAGS += -g
endif

.PHONY: all

all:
	mkdir -p bin
	g++ src/main.cpp src/chip8.cpp $(CFLAGS) -o bin/chip8plusplus
