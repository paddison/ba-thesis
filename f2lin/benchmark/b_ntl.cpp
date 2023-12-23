#include <stdio.h>
#include <stdlib.h>

#include "NTL/tools.h"
#include "mpi.h"
#include "NTL/GF2X.h"

#include "rng_generic/rng_generic.h"
#include "bench.h"
#include "tools.h"

using namespace NTL;

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
void benchmark_minimal_polynomial(size_t iterations, size_t repetitions) {
    F2LinBMPI init_p_min_data = f2lin_bench_bmpi_init(repetitions);
    F2LinBMPI seq_data = f2lin_bench_bmpi_init(repetitions);

    double times[3];
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
        for (size_t i = 0; i < iterations; ++i) {
            init_p_min();
        }
        times[2] = MPI_Wtime();

        f2lin_bench_bmpi_update(&seq_data, rep, times[1] - times[0]);
        f2lin_bench_bmpi_update(&init_p_min_data, rep, times[2] - times[1]);
    }

    double seq_avg = f2lin_bench_bmpi_eval(&seq_data) / (double) iterations;
    double init_p_min_avg = f2lin_bench_bmpi_eval(&init_p_min_data) / (double) iterations;

    if (init_p_min_data.rank == init_p_min_data.root) {
        printf("state_size: %zu\t sequence: %5.2e\tminpoly: %5.2e\n",
               state_size, seq_avg, init_p_min_avg);
    }

    f2lin_rng_generic_destroy(rng);
}

static 
void benchmark_jump_polynomial(size_t iterations, size_t repetitions, size_t jump) {
    F2LinBMPI jump_data = f2lin_bench_bmpi_init(repetitions);
    double times[2];
    const GF2X p_min = init_p_min();

    for (size_t rep = 0; rep < repetitions; ++rep) {
        times[0] = MPI_Wtime();
        // measure time it takes to calculate jump polynomial
        for (size_t i = 0; i < iterations; ++i) {
            init_p_jump(p_min, jump);
        }
        times[1] = MPI_Wtime();
        
        f2lin_bench_bmpi_update(&jump_data, rep, times[1] - times[0]);
    }


    double jump_avg = f2lin_bench_bmpi_eval(&jump_data) / (double) iterations;

    if (jump_data.rank == jump_data.root) {
        printf("jump_size: %zu\t jump_poly: %5.2e\n",
               jump, jump_avg);
    }
}

int main(int argc, char* argv[argc + 1]) {
    MPI_Init(&argc, &argv);
    size_t iterations, repetitions, n_jumps = argc - 3;
    unsigned long long jumps[100];

    if (argc < 3) return EXIT_FAILURE;
    if (argc > 103) return EXIT_FAILURE;

    iterations = strtoull(argv[1], 0, 10);
    repetitions = strtoull(argv[2], 0, 10);

    if (iterations == -1 || repetitions == -1) return EXIT_FAILURE;

    f2lin_bench_parse_argv(argc, &argv[3], jumps);

    for (size_t i = 0; i < n_jumps; ++i) {
        benchmark_jump_polynomial(iterations, repetitions, jumps[i]);
    }

    benchmark_minimal_polynomial(iterations, repetitions);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
