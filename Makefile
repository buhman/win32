.SUFFIXES:
.INTERMEDIATE:
.SECONDARY:
.PHONY: all clean

CC = i686-w64-mingw32-gcc
CXX = i686-w64-mingw32-g++

CSTD += -std=gnu11
CXXSTD += -std=gnu++14

CFLAGS += -Wall -Werror -Wfatal-errors
CFLAGS += -Wno-error=unused-function
CFLAGS += -Wno-error=unused-variable
CFLAGS += -DUNICODE

LDFLAGS += -Wl,--subsystem,windows -mwindows -mconsole -municode -ld3d9 -ld3dx9 -lwinmm -ldinput8 -ldxguid

OPT = -Og

DEBUG = -g

DEPFLAGS = -MMD -MP

makefile_path := $(dir $(abspath $(firstword $(MAKEFILE_LIST))))

all: main

%.o: %.cpp
	$(CXX) $(CXXSTD) $(CFLAGS) $(OPT) $(DEBUG) $(DEPFLAGS) -MF ${<}.d -c $< -o $@

%.o: %.c
	$(CC) $(CSTD) $(CFLAGS) $(CXXFLAGS) $(OPT) $(DEBUG) $(DEPFLAGS) -MF ${<}.d -c $< -o $@

MAIN_OBJS = \
	main.o

main.exe: $(MAIN_OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

-include $(shell find $(makefile_path) -type f -name '*.d')

%: RCS/%,v
%: RCS/%
%: %,v
%: s.%
%: SCCS/s.%
