#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "NTL/tools.h"
#include "mpi.h"
#include "NTL/GF2X.h"

#include "rng_generic/rng_generic.h"
#include "bench.h"
#include "tools.h"

using namespace NTL;

static 
void write_results(char* exec_name, size_t N, unsigned long long *jumps, double *results, 
                   double minpoly, double minpoly_seq) {
    char* fname;
    FILE* f;

    asprintf(&fname, "%s.csv", exec_name);
    f = fopen(fname, "a");
    fprintf(f, "jump,jumppoly\n");

    for (size_t i = 0; i < N; ++i) {
        fprintf(f, "%llu,%5.2e\n", jumps[i], results[i]);
    }

    fprintf(f, "state_size,minpoly,minpoly_seq\n");
    fprintf(f, "%zu,%5.2e,%5.2e\n", f2lin_rng_generic_state_size(), minpoly, minpoly_seq);
    fclose(f);
    free(fname);
}

static
GF2X init_p_min() {
    const int state_size = f2lin_rng_generic_state_size();
    const size_t seq_len = 2 * state_size ;

    GF2X p_min;
    vec_GF2 seq(NTL::INIT_SIZE, seq_len);
    F2LinRngGeneric* rng = f2lin_rng_generic_init();

    for (size_t i = 0; i < seq_len; ++i) {
        seq[i] = f2lin_rng_generic_next_state(rng) & 0x01ul;
    }

    NTL::MinPolySeq(p_min, seq, state_size);
    f2lin_rng_generic_destroy(rng);
    return p_min;
}

static
void init_p_jump(const GF2X p_min, size_t jump) {
    GF2X p_jump;
    GF2X x(INIT_MONO, 1);
    GF2XModulus p_min_mod;

    build(p_min_mod, p_min);
    PowerMod(p_jump, x, jump, p_min_mod);
}

static
double benchmark_minimal_polynomial_seq(size_t iterations, size_t repetitions) {
    F2LinBMPI bmpi = f2lin_bench_bmpi_init(repetitions);
    double times[2];
    F2LinRngGeneric* rng = f2lin_rng_generic_init();
    size_t state_size = f2lin_rng_generic_state_size();
    vec_GF2 seq(NTL::INIT_SIZE, state_size * 2);
 
    for (size_t rep = 0; rep < repetitions; ++rep) {
        times[0] = MPI_Wtime();
        // measure how long it takes to collect data for state
        for (size_t i = 0; i < iterations; ++i) {
            for (size_t j = 0; j < state_size * 2; ++j) {
                seq[i] = f2lin_rng_generic_gen64(rng) & 0x01ul;
            }
        }
        times[1] = MPI_Wtime();

        f2lin_bench_bmpi_update(&bmpi, rep, times[1] - times[0]);
    }

    double avg = f2lin_bench_bmpi_eval(&bmpi) / (double) iterations;

    f2lin_rng_generic_destroy(rng);
    f2lin_bench_bmpi_destroy(&bmpi);

    return avg;
}
static
double benchmark_minimal_polynomial(size_t iterations, size_t repetitions) {
    F2LinBMPI bmpi = f2lin_bench_bmpi_init(repetitions);
    double times[2];

    for (size_t rep = 0; rep < repetitions; ++rep) {
        times[0] = MPI_Wtime();
        // measure how long it takes to collect data for state
        for (size_t i = 0; i < iterations; ++i) {
            init_p_min();
        }
        times[1] = MPI_Wtime();

        f2lin_bench_bmpi_update(&bmpi, rep, times[1] - times[0]);
    }

    double avg = f2lin_bench_bmpi_eval(&bmpi) / (double) iterations;

    f2lin_bench_bmpi_destroy(&bmpi);

    return avg;
}

static 
double benchmark_jump_polynomial(size_t iterations, size_t repetitions, size_t jump) {
    F2LinBMPI bmpi = f2lin_bench_bmpi_init(repetitions);
    double times[2];
    const GF2X p_min = init_p_min();

    for (size_t rep = 0; rep < repetitions; ++rep) {
        times[0] = MPI_Wtime();
        for (size_t i = 0; i < iterations; ++i) {
            init_p_jump(p_min, jump);
        }
        times[1] = MPI_Wtime();
        
        f2lin_bench_bmpi_update(&bmpi, rep, times[1] - times[0]);
    }


    double jump_avg = f2lin_bench_bmpi_eval(&bmpi) / (double) iterations;

    f2lin_bench_bmpi_destroy(&bmpi);
    return jump_avg;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    size_t iterations, repetitions, n_jumps = argc - 3;
    int rank;
    unsigned long long jumps[100];

    if (argc < 3) return EXIT_FAILURE;
    if (argc > 103) return EXIT_FAILURE;

    iterations = strtoull(argv[1], 0, 10);
    repetitions = strtoull(argv[2], 0, 10);

    if (iterations == -1 || repetitions == -1) return EXIT_FAILURE;

    f2lin_bench_parse_argv(argc, &argv[3], jumps);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    double results[n_jumps];

    for (size_t i = 0; i < n_jumps; ++i) {
        double avg = benchmark_jump_polynomial(iterations, repetitions, jumps[i]);
        if (rank == 0) printf("jump:%llu\tjumppoly:%5.2e\n", jumps[i], avg); 
    }

    double minpoly = benchmark_minimal_polynomial(iterations, repetitions);
    double minpoly_seq = benchmark_minimal_polynomial_seq(iterations, repetitions);
    if (rank == 0) {
        printf("statesize: %zu\tminpoly:%5.2e\tminpoly_seq:%5.2e\n", 
                f2lin_rng_generic_state_size(), minpoly, minpoly_seq);
        write_results(argv[0], n_jumps, jumps, results, minpoly, minpoly_seq);
    }
    MPI_Finalize();
    return EXIT_SUCCESS;
}
