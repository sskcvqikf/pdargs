CXX			= g++
CXXFLAGS	= -std=c++17 -Wall -Wextra -O2
LNKFLAGS	= -std=c++17 -O2

I_DIR		= ./include

all: main

main: main.cc ./include/pd/pdargs.hh
	$(CXX) $(CXXFLAGS) -I$(I_DIR) -o $@ $<

clean: clean_bin clean_obj

.PHONY: clean
