CXXFLAGS = -std=c++11 -I ./include
CFLAGS = -I ./include
ntl_flags = -lntl -lgmp -lgf2x

vpath %.c src:src/rng_generic:lib/mt
vpath %.cpp src
vpath %.o build

sources := $(wildcard src/*.c)
objects := $(patsubst src/%.c, build/%.o, $(sources))
minpoly := build/minpoly.o



all:
	echo $(filter %rng_generic_64.o,$(objects))

rng64: jump64.o gf2x_wrapper.o xor64.o poly_decomp.o rng_generic_64.o

jumpmt.o: minpolymt
minpolymt: $(minpoly) $(filter %rng_generic_mt.o, $(objects)) build/mt.o
	echo "hello"

jump64.o: minpoly64
minpoly64: $(minpoly) $(filter %rng_generic_64.o,$(objects))
	$(CXX) $^ -o $@ $(ntl_flags)


build/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o build/$@

.PHONY: clean
clean:
	rm build/*.o

