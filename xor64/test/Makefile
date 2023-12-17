# ----------------------
#  Variable Definitions
# ----------------------

CC := clang
CXX := clang++
CXXFLAGS := -g --std=c++11
CFLAGS := -g
NTL_FLAGS := -lntl -lgf2x -lgmp
# default rng
#RNG := rng_generic_64.o
# prerequisites for building the application, without any rngs.

# -------------------------------
#  Directories with source files 
# ------------------------------- 

vpath %.c ../src:../src/rng_generic:../lib/mt
vpath %.cpp ../src
vpath %.h ../src

# ---------
#  Targets 
# ---------

# testing
t_rng_generic_64: CXXFLAGS += -DF_NAME="../src/minpoly_out.h"
t_rng_generic_64: rng_generic_64.o jump.o t_rng_generic_64.o poly_decomp.o gf2x_wrapper.o
	clang++ $(CXXFLAGS) $^ -o $@ $(NTL_FLAGS)

t_first_n: CFLAGS += -DXorMT
t_first_n: CXXFLAGS += -DXorMT
t_first_n: mt.o rng_generic_mt.o jump.o t_first_n_mt.o poly_decomp.o gf2x_wrapper.o
	clang++ $(CXXFLAGS) $^ -o $@ $(NTL_FLAGS)

t_rng_generic_mt: CFLAGS += -DXorMT
t_rng_generic_mt: CXXFLAGS += -DXorMT
t_rng_generic_mt: mt.o rng_generic_mt.o t_rng_generic_mt.o
	clang++ $(CXXFLAGS) $^ -o $@ $(NTL_FLAGS)

# in order to build jump.o, the minimal polynomial of the transition function 
# has to be calculated. Since this is expensive, it is done on compile time
jump.o: minpoly_out.h 

minpoly_out.h: minpoly

# builds the cpp file which calculates the minimal polynomial, creating the 
# header which contains it in form of a string
# the header is included in jump.c
minpoly:  mt.o rng_generic_64.o minpoly.o
	echo "calculating minimal polynomial"
	clang++ $(CXXFLAGS) $^  -o $@ $(NTL_FLAGS)
	./$@
	rm ./$@

.PHONY: clean
clean:
	rm ../**/*.o
	rm ../src/minpoly_out.h
