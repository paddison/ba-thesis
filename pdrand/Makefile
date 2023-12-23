# =====================================================================================
# Common Definitions
# =====================================================================================

# Compile Flags
#-----------------------------------------

include_dirs := -I ${HOME}/.local/include -I ./benchmark/util -I .
lib_dir := -L ${HOME}/.local/lib

CFLAGS := $(include_dirs) $(lib_dir) -pthread
opt_flag := -g3

# directory structure
#-----------------------------------------

build := build
out := bin
bench := benchmark

# object files needed for running the algorithm etc.
#-----------------------------------------

sources := prand48.c
objects := $(patsubst %.c, $(build)/%.o, $(sources))

# object files needed for benchmarks
bench_src := bench.c tools.c 
bench_obj := $(patsubst %.c, $(build)/%.o, $(bench_src))

test_src := prand48_tests.c
test_obj := $(patsubst %.c, $(build)/%.o, $(test_src))

# this tells make where all the source files, headers etc. are
#-----------------------------------------

vpath %.c benchmark/util benchmark
vpath %.o build

# =====================================================================================
# Default Targets
# =====================================================================================

all: prand 

prand: $(objects) 
	$(CC) $(CFLAGS) $^ -o $(out)/$@ 

# =====================================================================================
# Building the tests for the application
# =====================================================================================

# defining the test names
test: t_prand48

t_prand48: $(test_obj) 
	$(CC) $(CFLAGS) $^ -o $(out)/$@

# =====================================================================================
# Rules for building the benachmark executables
# =====================================================================================

benchmark: CC = mpicc
benchmark: CFLAGS += -I $(bench)
benchmark: opt_flag = -O3
benchmark: b_jump b_jump_vs_iter b_strong_scaling

b_jump: $(bench_obj) $(objects) $(build)/b_jump.o
	$(CC) $(CFLAGS) $^ -o $(out)/$@

b_jump_vs_iter: $(bench_obj) $(objects) $(build)/b_jump_vs_iter.o
	$(CC) $(CFLAGS) $^ -o $(out)/$@

b_strong_scaling: $(bench_obj) $(objects) $(build)/b_strong_scaling.o
	$(CC) $(CFLAGS) $^ -o $(out)/$@


# =====================================================================================
# Generic rules for building object files
# =====================================================================================

build/%.o: %.c
	$(CC) $(opt_flag) $(CFLAGS) -c $< -o $@

# =====================================================================================
# Cleaning the build directory
# =====================================================================================

.PHONY: clean
clean:
	rm build/*.o
	rm bin/*.csv

