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
#include "poly_rand.h"

#define BUF_MAX 100
int grank;

/* thank you Kingshuk :) */
#define DEBUG_WAIT(a)                                                   \
  if(a) {                                                               \
    printf("Rank-%d: pid= %d\n", grank, getpid()); fflush(stdout);      \
    int temp_inside= -1;                                                \
    if(0== grank) {                                                     \
      if(1!= scanf("%d", &temp_inside)) { temp_inside= 0; }             \
    }                                                                   \
    MPI_Bcast(&temp_inside, 1, MPI_INT, 0, MPI_COMM_WORLD);                      \
  }

typedef struct data data;

struct data {
    size_t deg;
    double h;
    double sw[10];
    double swd[10];
};

static
void write_results(size_t N, unsigned long long buf[N], data results[N]) {
        char* fname;
        FILE* f;

        asprintf(&fname, "jump_ahead_algorithms.csv");

        f = fopen(fname, "a");
        fprintf(f, "deg,horner,\
sw1,sw2,sw3,sw4,sw5,sw6,sw7,sw8,sw9,sw10,\
swd1,swd2,swd3,swd4,swd5,swd6,swd7,swd8,swd9,swd10\n");

        for (size_t i = 0; i < N; ++i) {
            fprintf(f, "%llu,%5.2e,\
%5.2e,%5.2e,%5.2e,%5.2e,%5.2e,%5.2e,%5.2e,%5.2e,%5.2e,%5.2e,\
%5.2e,%5.2e,%5.2e,%5.2e,%5.2e,%5.2e,%5.2e,%5.2e,%5.2e,%5.2e\n",
                    buf[i], results[i].h,
                    results[i].sw[0],results[i].sw[1],results[i].sw[2],results[i].sw[3],results[i].sw[4],
                    results[i].sw[5],results[i].sw[6],results[i].sw[7],results[i].sw[8],results[i].sw[9],
                    results[i].swd[0],results[i].swd[1],results[i].swd[2],results[i].swd[3],results[i].swd[4],
                    results[i].swd[5],results[i].swd[6],results[i].swd[7],results[i].swd[8],results[i].swd[9]);
        }
        fclose(f);
        free(fname);

}

static 
double exec(unsigned long long poly_deg, F2LinConfig* cfg, size_t iterations, size_t repetitions) {
    double avg;
    F2LinBMPI bmpi = f2lin_bench_bmpi_init(repetitions);

    F2LinRngGeneric* rng = f2lin_rng_generic_init();
    F2LinJump* jp = f2lin_jump_ahead_init(poly_deg, cfg);

    GF2X_zero_destroy(jp->jump_poly);
    
    jp->jump_poly = f2lin_poly_rand_init(poly_deg);

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

    avg = f2lin_bench_bmpi_eval(&bmpi) / (double) iterations;

    f2lin_jump_ahead_destroy(jp);
    f2lin_rng_generic_destroy(rng);
    f2lin_bench_bmpi_destroy(&bmpi);

    return avg;
}

static 
void do_benchmark(size_t n_deg, unsigned long long poly_degs[n_deg], F2LinConfig *cfg, 
                  size_t iterations, size_t repetitions) {
}

int main(int argc, char* argv[argc + 1]) {  
    MPI_Init(&argc, &argv);

    if (argc < 3) {
        printf("Usage: mpirun -np x b_jump_ahead_algorithms iterations repetitions [deg1, deg2, ...]\n");
        printf("Iterations: number of function calls between two time measurements\n");
        printf("Repetitions: Number of datapoints collected per process\n");
    }

    unsigned long long buf[BUF_MAX] = { 0 };
    size_t iterations = strtoull(argv[1], 0, 10), repetitions = strtoull(argv[2], 0, 10);;
    size_t n_deg = argc - 3;

    if (iterations == ULLONG_MAX || repetitions == ULLONG_MAX) {
        fprintf(stderr, "Got non numberical value for iterations or repetitions"); 
        return EXIT_FAILURE;
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &grank);


    if (argc > BUF_MAX + 3) {
        fprintf(stderr, "Too many arguments, only support a maximum of %d.\n", BUF_MAX);
        return EXIT_FAILURE;
    }

    f2lin_bench_parse_argv(argc, &argv[3], buf);

    F2LinConfig cfg = { .q = 5, .algorithm = HORNER };
    data results[n_deg];

    for (size_t i = 0; i < n_deg; ++i) {
        double avg = exec(buf[i], &cfg, iterations, repetitions);
        results[i].h = avg;
        if (grank == 0) printf("polydeg: %llu\nhorner:%5.2e\n", buf[i], avg);
    }

    for (size_t q = 1; q <= 10; ++q) {
        cfg.q = q;
        cfg.algorithm = SLIDING_WINDOW;

        for (size_t i = 0; i < n_deg; ++i) {
            double avg = exec(buf[i], &cfg, iterations, repetitions);
            results[i].sw[q - 1] = avg;
            if (grank == 0) printf("sw%zu: %5.2e\n", q, avg);
        }

        cfg.algorithm = SLIDING_WINDOW_DECOMP;

        for (size_t i = 0; i < n_deg; ++i) {
            double avg = exec(buf[i], &cfg, iterations, repetitions);
            results[i].swd[q - 1] = avg;
            if (grank == 0) printf("swd%zu: %5.2e\n", q, avg);
        }
    }

    /* write the results */
    if (grank == 0) write_results(n_deg, buf, results);

    MPI_Finalize();

    return EXIT_SUCCESS;
}
