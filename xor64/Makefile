CXX_FLAGS := --std=c++11
vpath %.c ./src
OBJECT_FILES := $(patsubst %.c, %.o, $(wildcard *.c))

all: 
	echo $(OBJECT_FILES)

minpoly_out.h: minpoly

xor64_minpoly: rng_generic_64.o
	clang++ $(CXX_FLAGS) xor64_minpoly.cpp xor64_rng_generic_64.o -o xor64_minpoly -lntl -lgf2x -lgmp
	./xor64_minpoly
	rm xor64_minpoly

rng_generic_64.o: src/rng_generic/rng_generic_64.c
