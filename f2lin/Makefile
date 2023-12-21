# =====================================================================================
# Common Definitions
# =====================================================================================

# Compile Flags
#-----------------------------------------

include_dirs := -I ./include -I ./src -I ./src/rng_generic -I ${HOME}/.local/include

CXXFLAGS := -g -std=c++11 $(include_dirs) -L ${HOME}/.local/lib
CFLAGS := -g $(include_dirs) -L ${HOME}/.local/lib
ntl_flags := -lntl -lgmp -lgf2x

# directory structure
#-----------------------------------------

src := src
build := build
out := bin
lib := lib
inc := include
bench := benchmark

# object files needed for running the algorithm etc.
#-----------------------------------------

sources := gf2x_wrapper.cpp jump_ahead.c poly_decomp.c f2lin.c
objects := $(patsubst %.c, $(build)/%.o, $(sources))
objects := $(patsubst %.cpp, $(build)/%.o, $(objects))

# object files for random number generators
#-----------------------------------------

# object files for using normal xorshift rng
rng64 := rng_generic_64.c
rng64_obj := $(patsubst %.c, $(build)/%.o, $(rng64)) 

# object files for using tiny mersenne twister
rngtinymt := rng_generic_tinymt.c tinymt.c
rngtinymt_obj := $(patsubst %.c, $(build)/%.o, $(rngtinymt)) 

# object files for using mersenne twiser
rngmt := rng_generic_mt.c mt.c
rngmt_obj   := $(patsubst %.c, $(build)/%.o, $(rngmt))

# object files for using xoshiro256
rngxoshiro := rng_generic_xoshiro.c
rngxoshiro_obj   := $(patsubst %.c, $(build)/%.o, $(rngxoshiro))

# this tells make where all the source files, headers etc. are
#-----------------------------------------

vpath %.c src:src/rng_generic:src/lib/mt:src/lib/tinymt:test:benchmark
vpath %.cpp src
vpath %.o build
vpath %.h include src

# =====================================================================================
# Default Targets
# =====================================================================================

all: rng64test rngmttest rngtinymttest rngxoshirotest

rng64test: $(objects) $(rng64) $(build)/test.o
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)

rngmttest: $(objects) $(rngmt) $(build)/test.o
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)

rngtinymttest: $(objects) $(rngtinymt) $(build)/test.o
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)

rngxoshirotest: $(objects) $(rngxoshiro) $(build)/test.o
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)


# =====================================================================================
# Targets for building the library, one for each random number generator
# =====================================================================================

# note: this doesn't work yet, since i don't know how to build a static library
rng64: $(objects) $(rng64) 
	$(CXX) $(CXXFLAGS) $^ -o $(lib)/$@ $(ntl_flags)

rngmt: $(objects) $(rngmt) 
	$(CXX) $(CXXFLAGS) $^ -o $(lib)/$@ $(ntl_flags)

rngtinymt: $(objects) $(rngtinymt) 
	$(CXX) $(CXXFLAGS) $^ -o $(lib)/$@ $(ntl_flags)


# =====================================================================================
# Building the tests for the application
# =====================================================================================

# defining the test names
jump_ahead := $(build)/t_jump_ahead.o
verify_min_poly := $(build)/t_verify_min_poly.o
jump_ahead_algorithms := $(build)/t_jump_ahead_algorithms.o

test: CXXFLAGS += -DTEST
test: CFLAGS += -DTEST
test: t_verify_min_poly \
	  t_jump_ahead \
	  t_jump_ahead_algorithms;

# Test if jump == step for first n numbers
#-----------------------------------------

t_jump_ahead: t_jump_ahead_tinymt \
			  t_jump_ahead_mt \
			  t_jump_ahead_64 \
			  t_jump_ahead_xoshiro;

t_jump_ahead_64: $(rng64_obj) $(objects) $(jump_ahead)
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)

t_jump_ahead_tinymt: $(rngtinymt_obj) $(objects) $(jump_ahead)
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)

t_jump_ahead_mt: $(rngmt_obj) $(objects) $(jump_ahead)
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)

t_jump_ahead_xoshiro: $(rngxoshiro_obj) $(objects) $(jump_ahead)
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)

# Testing jump algorithms
#-----------------------------------------

t_jump_ahead_algorithms: t_jump_ahead_algorithms_tinymt \
						 t_jump_ahead_algorithms_mt \
						 t_jump_ahead_algorithms_64 \
						 t_jump_ahead_algorithms_xoshiro;

t_jump_ahead_algorithms_64: $(rng64_obj) $(objects) $(jump_ahead_algorithms)
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)

t_jump_ahead_algorithms_tinymt: $(rngtinymt_obj) $(objects) $(jump_ahead_algorithms)
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)

t_jump_ahead_algorithms_mt: $(rngmt_obj) $(objects) $(jump_ahead_algorithms)
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)

t_jump_ahead_algorithms_xoshiro: $(rngxoshiro_obj) $(objects) $(jump_ahead_algorithms)
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)


# Verifying minimal polynomials 
#-----------------------------------------

t_verify_min_poly: t_verify_min_poly_tinymt \
				   t_verify_min_poly_mt \
				   t_verify_min_poly_64 \
				   t_verify_min_poly_xoshiro;

t_verify_min_poly_64: $(rng64_obj) $(objects) $(verify_min_poly)
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)

t_verify_min_poly_tinymt: $(rngtinymt_obj) $(objects) $(verify_min_poly)
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)

t_verify_min_poly_mt: $(rngmt_obj) $(objects) $(verify_min_poly)
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)

t_verify_min_poly_xoshiro: $(rngxoshiro_obj) $(objects) $(verify_min_poly)
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)


# =====================================================================================
# Rules for building the benachmark executables
# =====================================================================================

benchmark: CFLAGS += -I $(bench)
benchmark: CXXFLAGS += -I $(bench)
benchmark: b_jump_ahead_algorithms_64

b_jump_ahead_algorithms_64: $(rng64_obj) $(objects) $(build)/tools.o $(build)/b_jump_ahead_algorithms.o
	$(CXX) $(CXXFLAGS) -O2  $^ -o $(out)/$@ $(ntl_flags)

# =====================================================================================
# Building header files which contain the minimal polynomial of each respective rng
# =====================================================================================

# small helper function for building each headers
define build_header
	$(CXX) $(CXXFLAGS) $^ $(src)/minpoly.cpp -o $@ $(ntl_flags)
	./$@
	mv minpoly.h $(src)/rng_generic/$@.h
	rm $@
endef

headers: CXXFLAGS += -DCALC_MIN_POLY
headers: CFLAGS += -DCALC_MIN_POLY
headers: minpolymt.h minpoly64.h minpolyxoshiro.h minpolytinymt.h

minpolymt.h: minpolymt 
minpoly64.h: minpoly64
minpolytinymt.h: minpolytinymt
minpolyxoshiro.h: minpolyxoshiro

minpolymt: $(rngmt_obj) 
	$(call build_header)

minpoly64: $(rng64_obj)
	$(call build_header)

minpolyxoshiro: $(rngxoshiro_obj)
	$(call build_header)

minpolytinymt: $(rngtinymt_obj)
	$(call build_header)


# =====================================================================================
# Generic rules for building object files
# =====================================================================================

build/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


# =====================================================================================
# Cleaning the build directory
# =====================================================================================

.PHONY: clean
clean:
	rm build/*.o

