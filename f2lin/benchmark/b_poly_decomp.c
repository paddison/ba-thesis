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

typedef struct data data;
struct data {
    double p[9];
};

static
void write_results(char exec_name[static 1], size_t N, unsigned long long degs[N], data results[N]) {
        char* fname;
        FILE* f;

        asprintf(&fname, "%s.csv", exec_name);
        f = fopen(fname, "a");
        fprintf(f, "deg,2,3,4,5,6,7,8,9,10\n");

        for (size_t i = 0; i < N; ++i) {
            double *d = results[i].p;
            fprintf(f, "%llu,", degs[i]);

            for (size_t j = 0; j < 9; ++j) {
                fprintf(f, "%5.2e,", d[j]);
            }
            fprintf(f, "\n");

        }
        fclose(f);
        free(fname);
}

double compute_poly_decomp(size_t deg, int q, size_t iterations, size_t repetitions) {
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

    f2lin_bench_bmpi_destroy(&bmpi);
    GF2X_zero_destroy(poly);
    
    return res;
}

int main(int argc, char* argv[argc + 1]) {
    MPI_Init(&argc, &argv);

    size_t iterations, repetitions, N_DEGS = argc - 3;
    unsigned long long degs[BUF_MAX];
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc < 3) {
        fprintf(stderr, "Usage: file iterations repetitions\n");
        return EXIT_FAILURE;
    } else if (argc > BUF_MAX + 3) {
        fprintf(stderr, "To many arguments, only suppport a maximum of 1000");
        return EXIT_FAILURE;
    } else {
        iterations = strtoull(argv[1], 0, 10);
        repetitions = strtoull(argv[2], 0, 10);
    }

    f2lin_bench_parse_argv(argc, &argv[3], degs);
    data results[N_DEGS];

    for (size_t i = 0; i < N_DEGS; ++i) {
        if (rank == 0) printf("deg poly: %7llu\t", degs[i]);
        for (size_t q = 2; q <= 10; ++q) {
            double avg = compute_poly_decomp(degs[i], q, 100, 100);
            results[i].p[q - 2] = avg;
            if (rank == 0) printf("pd%2zu: %5.2e\t", q, avg); 
        }
        if (rank == 0) printf("\n");
    }

    /* write the result to file */
    if (rank == 0) write_results(argv[0], N_DEGS, degs, results);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
