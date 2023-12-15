CXX_FLAGS := --std=c++11
all: minpoly.h

minpoly.h: xor64_minpoly

xor64_minpoly: xor64_rng_generic_64.o
	clang++ $(CXX_FLAGS) xor64_minpoly.cpp xor64_rng_generic_64.o -o xor64_minpoly -lntl -lgf2x -lgmp
	./xor64_minpoly
	rm xor64_minpoly

xor64_rng_generic_64.o:
