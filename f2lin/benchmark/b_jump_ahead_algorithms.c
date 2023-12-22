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
#include <unistd.h>

#include "mpi.h"

#include "tools.h"
#include "bench.h"
#include "jump_ahead.h"
#include "poly_decomp.h"
#include "rng_generic/rng_generic.h"
#include "config.h"
#include "gf2x_wrapper.h"

int grank;

#define DEBUG_WAIT(a)                                                   \
  if(a) {                                                               \
    printf("Rank-%d: pid= %d\n", grank, getpid()); fflush(stdout);      \
    int temp_inside= -1;                                                \
    if(0== grank) {                                                     \
      if(1!= scanf("%d", &temp_inside)) { temp_inside= 0; }             \
    }                                                                   \
    MPI_Bcast(&temp_inside, 1, MPI_INT, 0, MPI_COMM_WORLD);                      \
  }

static
void parse_argv(int argc, char *argv[argc - 3], unsigned long long buf[argc - 3]) {
    // we ignore the first argument, which is the program name
    for (size_t i = 0; i < argc - 3; ++i) {
        unsigned long long n = strtoull(argv[i], 0, 10);
        if (n != ULLONG_MAX) buf[i] = n;
    }
}

static 
void exec(unsigned long long poly_deg, F2LinConfig* cfg, size_t iterations, size_t repetitions) {
    double avg;
    F2LinBMPI bmpi = f2lin_bench_bmpi_init(repetitions);

    F2LinRngGeneric* rng = f2lin_rng_generic_init();
    F2LinJump* jp = f2lin_jump_ahead_init(poly_deg, cfg);

    GF2X_zero_destroy(jp->jump_poly);
    
    jp->jump_poly = f2lin_tools_n_deg_poly_random(poly_deg);

    if (cfg->algorithm != HORNER) {
        f2lin_poly_decomp_destroy(jp->decomp_poly);
        jp->decomp_poly = f2lin_poly_decomp_init_from_gf2x(jp->jump_poly, jp->q);
    }

    for (size_t rep = 0; rep < repetitions; ++rep) {
        double start, end;
        start = MPI_Wtime();
        for (size_t i = 0; i < iterations; ++i) {
            f2lin_jump_ahead_jump(jp, rng);
        }
        end = MPI_Wtime();
        f2lin_bench_bmpi_update(&bmpi, rep, end - start);
    }

    avg = f2lin_bench_bmpi_eval(&bmpi);

    if (bmpi.rank == bmpi.root) {
        printf("degree of polynomial: %llu\ttime: %5.2es\n", 
               poly_deg, avg / (double) iterations);
    }
    
    f2lin_jump_ahead_destroy(jp);
    f2lin_rng_generic_destroy(rng);
    f2lin_bench_bmpi_destroy(&bmpi);
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
    MPI_Comm_rank(MPI_COMM_WORLD, &grank);

    DEBUG_WAIT(0);
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
        buf[9] = 19937;
    }

    if (grank == 0) {
        printf("horner:\n");
    }
    do_benchmark(n_deg, buf, &cfg, iterations, repetitions);

    for (size_t q = 1; q <= 10; ++q) {
        cfg.q = q;
        cfg.algorithm = SLIDING_WINDOW;
        if (grank == 0) {
            printf("=========================================\nQ: %zu\n", q);
            printf("sliding window\n");
        }
        do_benchmark(n_deg, buf, &cfg, iterations, repetitions);

        cfg.algorithm = SLIDING_WINDOW_DECOMP;
        if (grank == 0) {
            printf("sliding window decomp\n");
        }

        do_benchmark(n_deg, buf, &cfg, iterations, repetitions);
    }

    MPI_Finalize();

    return EXIT_SUCCESS;
}
