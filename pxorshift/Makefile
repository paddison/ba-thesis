BUILD_DIR := build
SRC_DIR := src
TEST_DIR := test
BENCH_DIR :=benchmark

CFLAGS := -Wall -g
CXXFLAGS := -std=c++11 -Wall -g
CC := clang 
CXX := clang++

# rules for application
all: polynomials

# rules for tests
test: t_polynomials t_tools

polynomials: polynomials.o rng_generic_xor64.o gray.o
	$(CXX) $(BUILD_DIR)/polynomials.o $(BUILD_DIR)/gray.o $(BUILD_DIR)/rng_generic_xor64.o -o out/polynomials -lntl -lgmp -lgf2x -lc --verbose 

# rules for tests
t_polynomials: t_polynomials.o polynomials.o rng_generic_xor64.o gray.o
	$(CXX) $(BUILD_DIR)/t_polynomials.o $(BUILD_DIR)/polynomials.o $(BUILD_DIR)/gray.o $(BUILD_DIR)/rng_generic_xor64.o -o out/t_polynomials -lntl -lgmp -lgf2x -lc --verbose 

t_jump_xor64: polynomials_xor64.o rng_generic_xor64.o gray.o t_jump_xor64.o
	$(CXX) $(BUILD_DIR)/t_jump_xor64.o $(BUILD_DIR)/polynomials_xor64.o $(BUILD_DIR)/gray.o $(BUILD_DIR)/rng_generic_xor64.o -o out/t_jump_xor64 -lntl -lgmp -lgf2x -lc --verbose 

t_jump_sfmt: polynomials_sfmt.o rng_generic_sfmt.o gray.o sfmt.o t_jump_sfmt.o
	$(CXX) $(BUILD_DIR)/t_jump_sfmt.o $(BUILD_DIR)/polynomials_sfmt.o $(BUILD_DIR)/gray.o $(BUILD_DIR)/rng_generic_sfmt.o $(BUILD_DIR)/sfmt.o -o out/t_jump_sfmt -lntl -lgmp -lgf2x -lc --verbose 
 
t_jump_mt: polynomials_mt.o rng_generic_mt.o gray.o mt.o t_jump_mt.o
	$(CXX) $(BUILD_DIR)/t_jump_mt.o $(BUILD_DIR)/polynomials_mt.o $(BUILD_DIR)/gray.o $(BUILD_DIR)/rng_generic_mt.o $(BUILD_DIR)/mt.o -o out/t_jump_mt -lntl -lgmp -lgf2x -lc --verbose 

# rules for benchmarks
benchmark: b_flint b_polynomials b_algorithms

b_flint: b_flint.o rng_generic_xor64.o tools.o
	$(CC) $(BUILD_DIR)/b_flint.o $(BUILD_DIR)/rng_generic_xor64.o $(BUILD_DIR)/tools.o -o out/b_flint -lflint

b_polynomials: b_polynomials.o rng_generic_xor64.o gray.o tools.o
	$(CXX) $(CXXFLAGS) $(BUILD_DIR)/b_polynomials.o $(BUILD_DIR)/rng_generic_xor64.o $(BUILD_DIR)/gray.o $(BUILD_DIR)/tools.o -o out/b_polynomials -lntl -lgmp -lgf2x

b_algorithms: b_algorithms.o rng_generic_xor64.o gray.o tools.o
	$(CXX) $(BUILD_DIR)/b_algorithms.o $(BUILD_DIR)/rng_generic_xor64.o $(BUILD_DIR)/gray.o $(BUILD_DIR)/tools.o -o out/b_algorithms -lntl -lgmp -lgf2x

# rules for object files
# c object files
rng_generic_xor64.o:
	$(CC) $(CFLAGS) -c $(SRC_DIR)/rng_generic_xor64.c -o $(BUILD_DIR)/rng_generic_xor64.o

rng_generic_sfmt.o:
	$(CC) $(CFLAGS) -c $(SRC_DIR)/rng_generic_sfmt.c -o $(BUILD_DIR)/rng_generic_sfmt.o

rng_generic_mt.o:
	$(CC) $(CFLAGS) -c $(SRC_DIR)/rng_generic_mt.c -o $(BUILD_DIR)/rng_generic_mt.o


gray.o:
	$(CC) $(CFLAGS) -c $(SRC_DIR)/gray.c -o $(BUILD_DIR)/gray.o

# cpp object files
polynomials_xor64.o:
	$(CXX) $(CXXFLAGS) -c -std=c++11 $(SRC_DIR)/polynomials.cpp -o $(BUILD_DIR)/polynomials_xor64.o

polynomials_sfmt.o:
	$(CXX) $(CXXFLAGS) -DSFMT -c -std=c++11 $(SRC_DIR)/polynomials.cpp -o $(BUILD_DIR)/polynomials_sfmt.o

polynomials_mt.o:
	$(CXX) $(CXXFLAGS) -DMT64 -c -std=c++11 $(SRC_DIR)/polynomials.cpp -o $(BUILD_DIR)/polynomials_mt.o
# rules for test object files
t_polynomials.o:
	$(CXX) $(CXXFLAGS) -c -std=c++11 $(TEST_DIR)/t_polynomials.cpp -o $(BUILD_DIR)/t_polynomials.o

t_jump_xor64.o:
	$(CXX) $(CXXFLAGS) -c $(TEST_DIR)/t_jump.cpp -o $(BUILD_DIR)/t_jump_xor64.o

t_jump_sfmt.o:
	$(CXX) $(CXXFLAGS) -DSFMT -c $(TEST_DIR)/t_jump.cpp -o $(BUILD_DIR)/t_jump_sfmt.o

t_jump_mt.o:
	$(CXX) $(CXXFLAGS) -DMT64 -c $(TEST_DIR)/t_jump.cpp -o $(BUILD_DIR)/t_jump_mt.o

# rules for benchmark object files
b_flint.o:
	$(CC) $(CFLAGS) -c $(BENCH_DIR)/b_flint.c -o $(BUILD_DIR)/b_flint.o

b_polynomials.o:
	$(CXX) $(CXXFLAGS) -c $(BENCH_DIR)/b_polynomials.cpp -o $(BUILD_DIR)/b_polynomials.o

b_algorithms.o:
	$(CXX) $(CXXFLAGS) -c $(BENCH_DIR)/b_algorithms.cpp -o $(BUILD_DIR)/b_algorithms.o

tools.o:
	$(CC) $(CFLAGS) -c $(BENCH_DIR)/tools.c -o $(BUILD_DIR)/tools.o

# build mersenne twister
sfmt.o:
	$(CC) $(CFLAGS) -DSFMT_MEXP=19937 -c lib/sfmt/SFMT.c -o $(BUILD_DIR)/sfmt.o 

mt.o:
	$(CC) $(CFLAGS) -c lib/mt/mt.c -o $(BUILD_DIR)/mt.o 

clean:
	rm $(BUILD_DIR)/*.o
