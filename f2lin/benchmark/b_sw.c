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
#define Q_START 2
#define Q_END 10

static int grank;

typedef struct data data;

struct data {
    size_t deg;
    double sw[10];
};

static
void write_results(char exec_name[static 1], size_t N, unsigned long long buf[N], data results[N]) {
        char* fname;
        FILE* f;


        asprintf(&fname, "%s.csv", exec_name);

        printf("writing to: %s\n", fname);

        f = fopen(fname, "w");
        fprintf(f, "deg,");
        for (size_t i = Q_START; i <= Q_END; ++i) fprintf(f, "%zu,", i);
        fprintf(f, "\n");


        for (size_t i = 0; i < N; ++i) {
            fprintf(f, "%llu,", buf[i]);
            for (size_t j = Q_START; j <= Q_END; ++j) fprintf(f, "%5.2e,", results[i].sw[j - 1]);
            fprintf(f, "\n");
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
    GF2X* rand = f2lin_poly_rand_init(poly_deg);

    GF2X_zero_destroy(jp->jp.sw.jp);
    jp->jp.sw.jp = rand;
    
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

    F2LinConfig cfg;
    data results[n_deg];

    for (size_t i = 0; i < n_deg; ++i) {

        if (grank == 0 )printf("deg: %llu\t", buf[i]);
        for (size_t q = Q_START; q <= Q_END; ++q) {
            cfg.q = q;
            cfg.algorithm = SLIDING_WINDOW;
            results[i].sw[q - 1] = exec(buf[i], &cfg, iterations, repetitions);

            if (grank == 0) printf("%zu: %5.2e\t", q, results[i].sw[q - 1]);
        }
        if (grank == 0) printf("\n");
    }


    /* write the results */
    if (grank == 0) write_results(argv[0], n_deg, buf, results);

    MPI_Finalize();

    return EXIT_SUCCESS;
}
