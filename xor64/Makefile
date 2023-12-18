CXXFLAGS := -g -std=c++11 -I ./include -I ./src
CFLAGS := -g -I ./include -I ./src
ntl_flags := -lntl -lgmp -lgf2x

src := src
build := build
out := bin
inc := include

sources := $(wildcard $(src)/*.c*)
objects := $(patsubst $(src)/%.c, $(build)/%.o, $(sources))
objects := $(patsubst $(src)/%.cpp, $(build)/%.o, $(objects))

vpath %.c src:lib/mt:test
vpath %.cpp src
vpath %.o build
vpath %.h include src

all: headers rng64 rngmt

rng64: $(filter-out %minpoly.o %mt.o, $(objects))
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)

rngmt: $(filter-out %minpoly.o %rng_generic_64.o, $(objects)) $(build)/mt.o
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)

test: CXXFLAGS += -DTEST
test: CFLAGS += -DTEST
test: headers t_rng_generic_64
	echo "test"

t_rng_generic_64: $(build)/t_rng_generic_64.o $(filter %poly_decomp.o %gf2x_wrapper.o %jump.o %rng_generic_64.o, $(objects))
	$(CXX) $(CXXFLAGS) $^ -o $(out)/$@ $(ntl_flags)

headers: CXXFLAGS += -DCALC_MIN_POLY
headers: CFLAGS += -DCALC_MIN_POLY
headers: minpolymt.h minpoly64.h

minpolymt.h: minpolymt 
minpoly64.h: minpoly64

minpolymt: $(build)/mt.o
	$(CXX) $(CXXFLAGS) $^ $(src)/minpoly.cpp $(src)/rng_generic_mt.c -o $@ $(ntl_flags)
	./$@
	mv minpoly.h $(src)/$@.h
	rm $@ 

minpoly64: 
	$(CXX) $(CXXFLAGS) $(src)/minpoly.cpp $(src)/rng_generic_64.c -o $@ $(ntl_flags)
	./$@
	mv minpoly.h $(src)/$@.h
	rm $@ 

build/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm build/*.o

