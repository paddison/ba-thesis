# =====================================================================================
# Common Definitions
# =====================================================================================

# Compile Flags
#-----------------------------------------

include_dirs := -I ${HOME}/.local/include -I .

CFLAGS := $(include_dirs) -pthread
opt_flag := -g3

# directory structure
#-----------------------------------------

build := build
out := bin
bench := bench
benchout := bin/bench

# object files needed for running the algorithm etc.
#-----------------------------------------

sources := prand48.c
objects := $(patsubst %.c, $(build)/%.o, $(sources))

# object files needed for benchmarks
bench_src := bench.c tools.c 
bench_obj := $(patsubst %.c, $(build)/%.o, $(bench_src))

test_src := prand48_tests.c
test_obj := $(patsubst %.c, $(build)/%.o, $(test_src))

benchmarks := b_iter_vs_jump b_strong_scaling

# this tells make where all the source files, headers etc. are
#-----------------------------------------

vpath %.c ../util bench
vpath %.o build

# =====================================================================================
# Default Targets
# =====================================================================================

all: prand 

prand: $(objects) $(out)
	$(CC) $(CFLAGS) $^ -o $(out)/$@ 

# =====================================================================================
# Building the tests for the application
# =====================================================================================

# defining the test names
test: t_prand48

t_prand48: $(test_obj) | $(out)
	$(CC) $(CFLAGS) $^ -o $(out)/$@

# =====================================================================================
# Rules for building the benachmark executables
# =====================================================================================

.SECONDEXPANSION:
benchmark: CC = mpicc
benchmark: CFLAGS += -I ../util
benchmark: opt_flag = -O3
benchmark: $(benchmarks) | $(benchout)
	mv $^ $|


$(benchmarks): $(bench_obj) $(objects) $(build)/$$@.o
	$(CC) $(CFLAGS) $^ -o $@


# =====================================================================================
# Generic rules for building object files
# =====================================================================================

build/%.o: %.c | $(build)
	$(CC) $(opt_flag) $(CFLAGS) -c $< -o $@

$(build):
	mkdir $(build)

# why is it not building this?
$(benchout): | $(out)
	mkdir $(benchout)

$(out):
	mkdir $(out)

# =====================================================================================
# Cleaning the build directory
# =====================================================================================

.PHONY: clean
clean:
	rm -r build
	rm -r bin

