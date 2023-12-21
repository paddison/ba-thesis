/*
 * The purpose of this benchmark is to assess the performance of the three different 
 * algorithms which are used to evaluate the jump polynomial: horner, sliding window
 * and sliding window decomp.
 * 
 * Additionally, sliding window and sliding window decomp are also tested for different 
 * "q" values. Q is the degree of the decomposition polynomials used in those two algorithms.
 * Accepted values for q are in the range from 1 to 10.
 * 1 behaves exactly as just running horners algorithm.
 */

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "mpi.h"

#include "tools.h"
#include "jump_ahead.h"
#include "poly_decomp.h"
#include "rng_generic/rng_generic.h"
#include "config.h"
#include "gf2x_wrapper.h"

static
void parse_argv(int argc, char *argv[argc - 3], unsigned long long buf[argc - 3]) {
    // we ignore the first argument, which is the program name
    for (size_t i = 0; i < argc - 3; ++i) {
        unsigned long long n = strtoull(argv[i], 0, 10);
        if (n != ULLONG_MAX) buf[i] = n;
    }
}

static
GF2X* init_n_deg(size_t deg) {
    GF2X* p = GF2X_zero_init();
    F2LinRngGeneric* rng = f2lin_rng_generic_init();
    for (size_t i = 0; i < deg; ++i) {
        GF2X_SetCoeff(p, i, f2lin_rng_generic_gen64(rng) & 1ull);
    }

    return p;
}

static 
void exec(unsigned long long poly_deg, F2LinConfig* cfg, size_t iterations, size_t repetitions) {
    MPI_Comm comm = MPI_COMM_WORLD;
    int root = 0, rank, gsize;
    size_t rsize;
    double compute[repetitions];
    double *rbuf_compute;

    F2LinRngGeneric* rng = f2lin_rng_generic_init();
    F2LinJump* jp = f2lin_jump_ahead_init(poly_deg, cfg);

    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &gsize);

    free(jp->jump_poly);
    free(jp->decomp_poly);

    jp->jump_poly = init_n_deg(poly_deg);
    jp->decomp_poly = f2lin_poly_decomp_init_from_gf2x(jp->jump_poly, jp->q);
    rsize = gsize * repetitions;

    for (size_t rep = 0; rep < repetitions; ++rep) {
        double start, end;
        start = MPI_Wtime();
        for (size_t i = 0; i < iterations; ++i) {
            f2lin_jump_ahead_jump(jp, rng);
        }
        end = MPI_Wtime();
        compute[rep] = end - start;
    }
    if (rank == root) {
        rbuf_compute = malloc(sizeof(double) * rsize);
    }

    MPI_Gather(compute, repetitions, MPI_DOUBLE, rbuf_compute, repetitions, MPI_DOUBLE, root, comm);

    if (rank == root) {
        double avg_loop, avg_compute = 0;
        for (size_t i = 0; i < rsize; ++i) avg_compute += rbuf_compute[i];
        avg_compute = get_result(rsize, rbuf_compute) / (double) iterations;
        printf("degree of polynomial: %llu\ttime: %5.2es\n", 
               poly_deg, avg_compute / (double) rsize);
    }
    
    //f2lin_jump_ahead_destroy(jp);
    //f2lin_rng_generic_destroy(rng);
}

static 
void do_benchmark(size_t n_deg, unsigned long long poly_degs[n_deg], F2LinConfig *cfg, 
                  size_t iterations, size_t repetitions) {
    for (size_t i = 0; i < n_deg; ++i) {
        exec(poly_degs[i], cfg, iterations, repetitions);
    }
}

int main(int argc, char* argv[argc + 1]) {  
    MPI_Init(&argc, &argv);

    if (argc < 3) {
        printf("Usage: mpirun -np x b_jump_ahead_algorithms iterations repetitions [deg1, deg2, ...]\n");
        printf("Iterations: number of function calls between two time measurements\n");
        printf("Repetitions: Number of datapoints collected per process\n");
    }

    unsigned long long buf[100] = { 0 };
    size_t iterations = strtoull(argv[1], 0, 10), repetitions = strtoull(argv[2], 0, 10);;
    if (iterations == ULLONG_MAX || repetitions == ULLONG_MAX) {
        fprintf(stderr, "Got non numberical value for iterations or repetitions"); 
        return EXIT_FAILURE;
    }
    size_t n_deg;
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    F2LinConfig cfg = { .q = 5, .algorithm = HORNER };

    if (argc > 100) {
        fprintf(stderr, "Too many arguments, only support a maximum of 100.\n");
        return EXIT_FAILURE;
    }

    if (argc > 3) { 
        parse_argv(argc, &argv[3], buf);
        n_deg = argc - 3;
    } else {
        n_deg = 10; 
        buf[0] = 1;
        buf[1] = 16;
        buf[2] = 64;
        buf[3] = 256;
        buf[4] = 512;
        buf[5] = 1024;
        buf[6] = 2048;
        buf[7] = 4096;
        buf[8] = 8192;
        buf[9] = 16384;
        buf[9] = 32768;

    }

    if (myrank == 0) {
        printf("horner:\n");
    }
    do_benchmark(n_deg, buf, &cfg, iterations, repetitions);

    for (size_t q = 1; q < 11; ++q) {
        cfg.q = q;
        cfg.algorithm = SLIDING_WINDOW;
        if (myrank == 0) {
            printf("=========================================\nQ: %zu\n", q);
            printf("sliding window\n");
        }
        do_benchmark(n_deg, buf, &cfg, iterations, repetitions);

        cfg.algorithm = SLIDING_WINDOW_DECOMP;
        if (myrank == 0) {
            printf("sliding window decomp\n");
        }

        do_benchmark(n_deg, buf, &cfg, iterations, repetitions);
    }

    MPI_Finalize();

    return EXIT_SUCCESS;
}
