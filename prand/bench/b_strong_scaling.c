
#include <stdlib.h>
#include <stdio.h>
#include "unistd.h"
#include "mpi.h"

#include "prand48.h"
#include "tools.h"

int rank;
int gsize;
MPI_Comm comm = MPI_COMM_WORLD;

static inline
size_t determine_ppsize(size_t psize) {
    size_t ppsize = psize / gsize; 
    size_t rest = psize % gsize;
    if (rank < rest) ++ppsize;
    return ppsize;
}

int main(int argc, char* argv[argc + 1]) {
    MPI_Init(&argc, &argv);    

    size_t repetitions, iterations, psize, ppsize, jump_size; 
    double times[2], *measurements, *total;
    const int root = 0;

    if (argc < 4) return EXIT_FAILURE;

    repetitions = strtoul(argv[1], 0, 10);
    iterations = strtoul(argv[2], 0, 10);
    psize = strtoul(argv[3], 0, 10);

    if (repetitions == -1 || iterations == -1 || psize == -1) return EXIT_FAILURE;

    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &gsize);
    ppsize = determine_ppsize(psize);
    jump_size = rank * ppsize;
    measurements = calloc(sizeof(double), repetitions);

    for (size_t rep = 0; rep < repetitions; ++rep) {

        times[0] = MPI_Wtime();
        for (size_t i = 0; i < iterations; ++i) {
            prand48_init();
            Prand48* prand = prand48_get();

            prand48_jump_abs(prand, jump_size);

            for (size_t j = 0; j < ppsize; ++j) pdrand48(prand);
            prand48_destroy(prand);
        }
        times[1] = MPI_Wtime();
        measurements[rep] = times[1] - times[0];
    }

    if (rank == root) total = calloc(sizeof(double), repetitions * gsize);

    MPI_Gather(measurements, repetitions, MPI_DOUBLE, 
               total, repetitions, MPI_DOUBLE,
               root, comm);

    if (rank == root) {
        char* fname;
        FILE* f;
        double avg = 
            f2lin_tools_get_result(repetitions * gsize, total, MED) / (double) iterations;

        asprintf(&fname, "%s_%zu.csv", argv[0], psize);

        if (access(fname, F_OK) == -1) {
            f = fopen(fname, "w");
            fprintf(f, "nprocs,time\n");
        } else {
            f = fopen(fname, "a");
        }

        fprintf(f, "%d,%5.2e\n", gsize, avg);
        printf("nprocs: %d\ttime: %5.2es\n", gsize, avg);

        fclose(f);
        free(fname);
    }

    if (rank == root) free(total);
    free(measurements);

    MPI_Finalize();

    return EXIT_SUCCESS;
}
