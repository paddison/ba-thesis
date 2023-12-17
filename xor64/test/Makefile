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
t_first_n: CFLAGS += -DXorMT
t_first_n: CXXFLAGS += -DXorMT
t_first_n: RNG := rng_generic_mt.o mt.o
t_first_n: mt.o rng_generic_mt.o jump.o t_first_n_mt.o poly_decomp.o gf2x_wrapper.o
	clang++ $(CXXFLAGS) $^ -o $@ $(NTL_FLAGS)


# in order to build jump.o, the minimal polynomial of the transition function 
# has to be calculated. Since this is expensive, it is done on compile time
jump.o: minpoly_out.h 

minpoly_out.h: minpoly

# builds the cpp file which calculates the minimal polynomial, creating the 
# header which contains it in form of a string
# the header is included in jump.c
minpoly: $(RNG) minpoly.o 
	echo $(RNG)
	echo "calculating minimal polynomial"
	clang++ $(CXXFLAGS) $^ $(RNG) -o ./src/$@ $(NTL_FLAGS)
	./src/$@
	rm ./src/$@

.PHONY: clean
clean:
	rm ../**/*.o
	rm ../src/minpoly_out.h
