#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "bench.h"
#include "prand48.h"

static 
void write_results(char exec_name[static 1], size_t N, unsigned long long jumps[N], 
                   double results[N]) {
    char* fname;
    FILE* f;

    asprintf(&fname, "%s.csv", exec_name);
    f = fopen(fname, "a");
    fprintf(f, "jump,time\n");

    for (size_t i = 0; i < N; ++i) {
        fprintf(f, "%llu,%5.2e\n", jumps[i], results[i]);
    }

    fclose(f);
    free(fname);
}


static
double bench_jump(size_t iterations, size_t repetitions, unsigned long long jump) {
    F2LinBMPI bmpi = f2lin_bench_bmpi_init(repetitions);
    double times[2];
    prand48_init();
    uint16_t state[3];


    for (size_t rep = 0; rep < repetitions; ++rep) {
        times[0] = MPI_Wtime();
        for (size_t i = 0; i < iterations; ++i) {
            prand48_jump(state, jump);
        }
        times[1] = MPI_Wtime();

        f2lin_bench_bmpi_update(&bmpi, rep, times[1] - times[0]);
    }

    double res = f2lin_bench_bmpi_eval(&bmpi) / (double) iterations;

    f2lin_bench_bmpi_destroy(&bmpi);
    return res;
}

int main(int argc, char* argv[argc + 1]) {
    MPI_Init(&argc, &argv);
    
    size_t iterations, repetitions, n_args = argc - 3;
    unsigned long long buf[BUF_MAX];
    double *results;
    int rank;

    if (argc < 3) return EXIT_FAILURE;
    if (argc > BUF_MAX + 3) return EXIT_FAILURE;

    iterations = strtoul(argv[1], 0, 10); 
    repetitions = strtoul(argv[2], 0, 10); 

    if (iterations == -1 || repetitions == -1) return EXIT_FAILURE;

    f2lin_bench_parse_argv(argc, &argv[3], buf);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == ROOT) results = calloc(sizeof(double), n_args);

    for (size_t i = 0; i < n_args; ++i) {
        /* run benchmark */
        double avg = bench_jump(iterations, repetitions, buf[i]);
        if (rank == ROOT) {
            results[i] = avg;
            printf("jump: %llu\ttime: %5.2es\n", buf[i], results[i]);
        }
    }

    if (rank == ROOT) write_results(argv[0], n_args, buf, results);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
