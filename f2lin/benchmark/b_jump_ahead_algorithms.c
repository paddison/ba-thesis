#include "mpi.h"
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "tools.h"
#include "jump_ahead.h"
#include "rng_generic/rng_generic.h"
#include "config.h"
#include "gf2x_wrapper.h"

static
void parse_argv(int argc, char *argv[argc], unsigned long long buf[argc - 1]) {
    // we ignore the first argument, which is the program name
    for (size_t i = 1; i < argc; ++i) {
        unsigned long long n = strtoull(argv[i], 0, 10);
        if (n != ULLONG_MAX) buf[i - 1] = n;
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
void bla(unsigned long long jump_size, F2LinConfig* cfg, size_t iterations, size_t repetitions) {
    MPI_Comm comm = MPI_COMM_WORLD;
    int root = 0, rank, gsize;
    size_t rsize;
    double compute[repetitions];
    double *rbuf_compute;
    F2LinRngGeneric* rng = f2lin_rng_generic_init();
    F2LinJump* jp = f2lin_jump_ahead_init(jump_size, cfg);

    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &gsize);
    printf("rank: %d\t, gsize: %d\n", rank, gsize);

    jp->jump_poly = init_n_deg(jump_size);
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
    }
    //MPI_Barrier(comm);
    rbuf_compute = malloc(sizeof(double) * rsize);

    MPI_Gather(compute, repetitions, MPI_DOUBLE, rbuf_compute, rsize, MPI_DOUBLE, root, comm);

    if (rank == root) {
        double avg_loop, avg_compute;
        avg_compute = get_result(rsize, rbuf_compute);
        printf("Jump: %llu\ttime: %5.2es\n", 
               jump_size, avg_compute);
    }

    //MPI_Barrier(comm);
}

static 
void benchmark_jump(size_t jump_size, F2LinConfig *cfg, 
                    size_t iterations, size_t repetitions) {
    // iterations: number of times a loop is run when collecting a sample.
    // repetitions: the number of samples that are collected
    volatile size_t dummy = 0;
    int root = 0, myrank, g_size;
    double loop[repetitions], compute[repetitions];
    size_t rsize;
    double *rbuf_loop, *rbuf_compute;
    F2LinRngGeneric* rng = f2lin_rng_generic_init();
    F2LinJump* jp = f2lin_jump_ahead_init(jump_size, cfg);
    MPI_Comm comm = MPI_COMM_WORLD;
    jp->jump_poly = init_n_deg(jump_size);
    myrank = MPI_Comm_rank(comm, &myrank); 
    g_size = MPI_Comm_size(comm, &g_size);
    printf("rank: %d, NP: %d\n", myrank, g_size);
    rsize = g_size * repetitions;

    // measure the loop
    for (size_t rep = 0; rep < repetitions; ++rep) {
        // start measurement
        double start, middle, end;
        start = MPI_Wtime();
        for (size_t i = 0; i < iterations; ++i) {
            dummy = i;
        }

        middle = MPI_Wtime();

        for (size_t i = 0; i < iterations; ++i) {
            // do some heavy computation
            f2lin_jump_ahead_jump(jp, rng);
        }
        end = MPI_Wtime();
        // print something to avoid cpu optimization 
        //printf("did repetition, %zu iterations\n", dummy);
        loop[rep] = middle - start;
        compute[rep] = end - middle;
    }
    
    if (myrank == root) {
        rbuf_loop = malloc(sizeof(double) * rsize);
        rbuf_compute = malloc(sizeof(double) * rsize);

    }
    MPI_Gather(loop, repetitions, MPI_DOUBLE, 
               rbuf_loop, rsize, MPI_DOUBLE, root, 
               comm);

    
    MPI_Gather(compute, repetitions, MPI_DOUBLE, 
               rbuf_compute, rsize, MPI_DOUBLE, root,
               comm);

    if (myrank == root) {
        double avg_loop, avg_compute;
        avg_loop = get_result(rsize, rbuf_loop);
        avg_compute = get_result(rsize, rbuf_compute);
        printf("Jump: %zu\ttime: %5.2es\tloop_time: %5.2ens\n", 
               jump_size, avg_compute, avg_loop);
    }
}

static 
void do_benchmark(size_t n_jumps, unsigned long long jumps[n_jumps], F2LinConfig *cfg, 
                  size_t iterations, size_t repetitions) {

    /*
    MPI_Comm comm = MPI_COMM_WORLD;
    int rank, gsize;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &gsize);
    */

    for (size_t i = 0; i < n_jumps; ++i) {
        //benchmark_jump(jumps[i], cfg, iterations, repetitions);
        bla(jumps[i], cfg, iterations, repetitions);
    }
}

int main(int argc, char* argv[argc + 1]) {  
    MPI_Init(&argc, &argv);

    unsigned long long buf[100];
    size_t iterations = 100, repetitions = 100;
    size_t n_jumps;
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    F2LinConfig cfg = { .q = 5, .algorithm = HORNER };

    if (argc > 100) {
        fprintf(stderr, "Too many arguments, only support a maximum of 100.\n");
        return EXIT_FAILURE;
    }

    if (argc > 1) { 
        parse_argv(argc, argv, buf);
        n_jumps = argc - 1;
    } else {
        n_jumps = 10; 
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
    do_benchmark(n_jumps, buf, &cfg, iterations, repetitions);

    cfg.algorithm = SLIDING_WINDOW;
    if (myrank == 0) {
        printf("sliding window:\n");
    }
    do_benchmark(n_jumps, buf, &cfg, iterations, repetitions);

    cfg.algorithm = SLIDING_WINDOW_DECOMP;
    if (myrank == 0) {
        printf("sliding window decomp:\n");
    }
    do_benchmark(n_jumps, buf, &cfg, iterations, repetitions);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
