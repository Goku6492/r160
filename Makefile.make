CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O3
LIBS = -lcrypto

all: your_program

your_program: your_program.cpp
    $(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

.PHONY: clean
clean:
    rm -f your_program