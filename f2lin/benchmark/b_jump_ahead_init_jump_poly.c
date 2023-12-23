/*
 * The goal of this benchmark is to measure the initialization routines of the jump 
 * algorithm. 
 * Important here are the initialization of the decomposition polynomial, and the 
 * calculation of the jump polynomial.
 */
#include <stdio.h>
#include <stdlib.h>
#include "gf2x_wrapper.h"
#include "mpi.h"
#include "tools.h"
#include "bench.h"
#include "poly_decomp.h"
#include "poly_rand.h"

#define N_DEG 7
#define N_BENCH 2

/* This implementation is exactly copied from "jump_ahead.c" */
static GF2X* init_jump_poly(const GF2X* min_poly, const size_t jump_size) {
    GF2X* jump_poly = GF2X_zero_init();
    GF2X* x = GF2X_zero_init();
    GF2X_SetCoeff(x, 1, 1);
    GF2XModulus* minimal_poly_mod = GF2XModulus_zero_init();

    GF2XModulus_build(minimal_poly_mod, min_poly);
    GF2X_PowerMod(jump_poly, x, jump_size, minimal_poly_mod);
    
    GF2X_zero_destroy(x);
    GF2XModulus_destroy(minimal_poly_mod);

    return jump_poly;
}

static
void compute_jump_poly(size_t deg, size_t jump_size, size_t iterations, size_t repetitions) {
    F2LinBMPI bmpis[N_BENCH];
    GF2X* min_poly = f2lin_poly_rand_init(deg);
    GF2X* p = init_jump_poly(min_poly, jump_size);

    for (size_t i = 0; i < N_BENCH; ++i) {
        bmpis[i] = f2lin_bench_bmpi_init(repetitions);
    }

    for (size_t rep = 0; rep < repetitions; ++rep) {
        double start, end, time, times[N_BENCH + 1];
        times[0] = MPI_Wtime();
        for (size_t i = 0; i < iterations; ++i) {
            GF2X* p = init_jump_poly(min_poly, jump_size);
            GF2X_zero_destroy(p);
        }
        times[1] = MPI_Wtime();

        for (size_t i = 0; i < iterations; ++i) {
            F2LinPolyDecomp* pd = f2lin_poly_decomp_init_from_gf2x(p, 4);
            f2lin_poly_decomp_destroy(pd);
        }
        times[2] = MPI_Wtime();

        f2lin_bench_bmpi_update(&bmpis[0], rep, (times[1] - times[0]) / iterations);
        f2lin_bench_bmpi_update(&bmpis[1], rep, (times[2] - times[1]) / iterations);
    }

    double res_init_jump_poly = f2lin_bench_bmpi_eval(&bmpis[0]);
    double res_init_decomp_poly = f2lin_bench_bmpi_eval(&bmpis[1]);

    if (bmpis[0].rank == bmpis[0].root) {
        printf("jump size: %6zu\tminpoly: %5.2es\tdecomppoly: %5.2es\n", 
               jump_size, res_init_jump_poly, res_init_decomp_poly);
    }
     
    for (size_t i = 0; i < N_BENCH; ++i) {
        f2lin_bench_bmpi_destroy(&bmpis[i]);
    }
    GF2X_zero_destroy(p);
    GF2X_zero_destroy(min_poly);
}

int main(int argc, char* argv[argc + 1]) {
    MPI_Init(&argc, &argv);
    unsigned long long iterations, repetitions;
    const size_t N_JUMPS = argc - 3;
    int rank, root = 0;
    // test for common rng state sizes
    size_t minpoly_sizes[N_DEG] = { 64, 128, 256, 512, 1024, 4096, 19937 };
    unsigned long long jumps[1000];

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc < 3) {
        fprintf(stderr, "Usage: file iterations repetitions\n");
    } else if (argc > 1000) {
        fprintf(stderr, "To many arguments, only suppport a maximum of 1000");
    } else {
        iterations = strtoull(argv[1], 0, 10);
        repetitions = strtoull(argv[2], 0, 10);
    }

    f2lin_bench_parse_argv(argc, &argv[3], jumps);

    for (size_t i = 0; i < N_DEG; ++i) {
        if (rank == root) {
            printf("===========================================\n");
            printf("degree of minimal polynomial: %zu\n", minpoly_sizes[i]);
        }
        for (size_t j = 0; j < N_JUMPS; ++j) {
            compute_jump_poly(minpoly_sizes[i], jumps[j], iterations, repetitions);
        }
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
