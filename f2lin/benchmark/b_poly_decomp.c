/*
 * The goal of this benchmark is to measure the initialization routines of the jump 
 * algorithm. 
 * Important here are the initialization of the decomposition polynomial, and the 
 * calculation of the jump polynomial.
 *
 * We don't benchmark the initialization of the sliding window algorithm, since this would
 * only be useful to compare it to horners rule, which never calls these anywayk when 
 * jumping ahead.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "gf2x_wrapper.h"
#include "mpi.h"
#include "tools.h"
#include "poly_decomp.h"
#include "poly_rand.h"
#include "bench.h"

void compute_poly_decomp(size_t deg, int q, size_t iterations, size_t repetitions) {
    GF2X* poly = f2lin_poly_rand_init(deg);
    F2LinBMPI bmpi = f2lin_bench_bmpi_init(repetitions);

    for (size_t rep = 0; rep < repetitions; ++rep) {
        double start, end, time;
        F2LinPolyDecomp* pds[iterations];
        start = MPI_Wtime();
        for (size_t i = 0; i < iterations; ++i) {
            pds[i] = f2lin_poly_decomp_init_from_gf2x(poly, q);
        }
        end = MPI_Wtime();

        for (size_t i = 0; i < iterations; ++i) {
            f2lin_poly_decomp_destroy(pds[i]);
        }
    
        time = (end - start) / (double) iterations;
         
        f2lin_bench_bmpi_update(&bmpi, rep, time);
    }

    double res = f2lin_bench_bmpi_eval(&bmpi);
    
    if (bmpi.rank == bmpi.root) {
        printf("q: %2d\ttime: %5.2es\n", 
               q, res);
    }
}

int main(int argc, char* argv[argc + 1]) {
    MPI_Init(&argc, &argv);

    size_t iterations, repetitions, N_DEGS = argc - 3;
    unsigned long long degs[100];
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc < 3) {
        fprintf(stderr, "Usage: file iterations repetitions\n");
    } else if (argc > 100) {
        fprintf(stderr, "To many arguments, only suppport a maximum of 1000");
    } else {
        iterations = strtoull(argv[1], 0, 10);
        repetitions = strtoull(argv[2], 0, 10);
    }

    f2lin_bench_parse_argv(argc, &argv[3], degs);

    for (size_t i = 0; i < N_DEGS; ++i) {
        if (rank == 0) {
            printf("===================================");
            printf("Degree of polynomial: %7llu\n", degs[i]);
        }
        for (size_t q = 2; q <= 10; ++q) {
            compute_poly_decomp(degs[i], q, 100, 100);
        }
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
