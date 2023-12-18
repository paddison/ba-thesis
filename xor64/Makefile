CXXFLAGS := -g -std=c++11 -I ./include -I ./src
CFLAGS := -g -I ./include -I ./src
ntl_flags := -lntl -lgmp -lgf2x

src := src
build := build
out := bin
inc := include

# object files needed for running the algorithm etc.
sources := gf2x_wrapper.cpp jump.c poly_decomp.c xor64.c
objects := $(patsubst %.c, $(build)/%.o, $(sources))
objects := $(patsubst %.cpp, $(build)/%.o, $(objects))

# object files for using normal xorshift rng
rng64 := rng_generic_64.c
rng64_obj := $(patsubst %.c, $(build)/%.o, $(rng64)) 

# object files for using tiny mersenne twister
rngtinymt := rng_generic_tinymt.c tinymt.c
rngtinymt_obj := $(patsubst %.c, $(build)/%.o, $(rngtinymt)) 

# object files for using mersenne twiser
rngmt := rng_generic_mt.c mt.c
rngmt_obj   := $(patsubst %.c, $(build)/%.o, $(rngmt))

# this tells make where all the source files, headers etc. are
vpath %.c src:src/lib/mt:src/lib/tinymt:test
vpath %.cpp src
vpath %.o build
vpath %.h include src

all: 
	echo $(rngtinymt_obj)
	echo $(rngmt_obj)
	echo $(objects)


# =====================================================================================
# Targets for building the library, one for each random number generator
# =====================================================================================

rng64: $(objects) $(rng64) $(build)/test.o
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)

rngmt: $(objects) $(rngmt) 
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)

rngtinymt: $(objects) $(rngtinymt) $(build)/test.o
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)


# =====================================================================================
# Building the tests for the application
# =====================================================================================

test: CXXFLAGS += -DTEST
test: CFLAGS += -DTEST
test: t_rng_first_n_tinymt t_rng_first_n_mt t_rng_first_n_64
	echo "test"

t_rng_first_n_64: $(rng64_obj) $(objects) $(build)/t_first_n_mt.o
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)

t_rng_first_n_tinymt: $(rngtinymt_obj) $(objects) $(build)/t_first_n_mt.o
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)

t_rng_first_n_mt: $(rngmt_obj) $(objects) $(build)/t_first_n_mt.o
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)


# =====================================================================================
# Building header files which contain the minimal polynomial of each respective rng
# =====================================================================================

# small helper function for building each headers
define build_header
	$(CXX) $(CXXFLAGS) $^ $(src)/minpoly.cpp -o $@ $(ntl_flags)
	./$@
	mv minpoly.h $(src)/$@.h
	rm $@ 
endef

headers: CXXFLAGS += -DCALC_MIN_POLY
headers: CFLAGS += -DCALC_MIN_POLY
headers: minpolymt.h minpoly64.h 

minpolymt.h: minpolymt 
minpoly64.h: minpoly64
minpolytinymt.h: minpolytinymt

minpolymt: $(rngmt_obj) 
	$(call build_header)

minpoly64: $(rng64_obj)
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

