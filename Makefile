CXX			= g++
CXXFLAGS	= -std=c++17 -Wall -Wextra -Wno-write-strings -O2

I_DIR		= ./include

all: main

main: main.cc ./include/pd/pdargs.h
	$(CXX) $(CXXFLAGS) -I$(I_DIR) -o $@ $<

clean: clean_bin clean_obj

.PHONY: clean
