#include "bench.h"
#include "mpi.h"
#include "tools.h"

F2LinBMPI f2lin_bench_bmpi_init(size_t repetitions) {
    int rank, gsize;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &gsize);
    
    return (F2LinBMPI) { 
        .rank = rank,
        .gsize = gsize,
        .root = 0,
        .repetitions = repetitions,
        .times = calloc(repetitions, sizeof(double)),
    };
}

double f2lin_bench_bmpi_eval(F2LinBMPI* bmpi) {
    double *result;
    size_t ndata = bmpi->gsize * bmpi->repetitions;
    if (bmpi->rank == bmpi->root) {
        result = calloc(sizeof(double), ndata);
    }

    MPI_Gather(bmpi->times, bmpi->repetitions, MPI_DOUBLE, 
               result, bmpi->repetitions, MPI_DOUBLE, bmpi->root, MPI_COMM_WORLD);

    if (bmpi->rank == bmpi->root) {
        double bavg = f2lin_tools_get_result(ndata, result);
        free(result);
        return bavg;
    } else {
        return 0.0;
    }
}
