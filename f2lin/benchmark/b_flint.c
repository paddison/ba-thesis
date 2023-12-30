#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "flint/flint.h"
#include "mpi.h"
#include "flint/nmod_types.h"
#include "flint/fmpz.h"
#include "flint/nmod_poly.h"

#include "rng_generic/rng_generic.h"
#include "bench.h"
#include "tools.h"

typedef struct data data;

struct data {
    double init;
    double jp;
};

static 
void write_results(char exec_name[static 1], size_t N, unsigned long long jumps[N], 
                   data results[N], double minpoly, double seq) {
    char* fname;
    FILE* f;

    asprintf(&fname, "%s.csv", exec_name);
    f = fopen(fname, "a");
    fprintf(f, "jump,alloc,jumppoly\n");

    for (size_t i = 0; i < N; ++i) {
        fprintf(f, "%llu,%5.2e,%5.2e\n", jumps[i], results[i].init, results[i].jp);
    }

    fprintf(f, "state_size,minpoly,minpoly_seq\n");
    fprintf(f, "%zu,%5.2e,%5.2e\n", f2lin_rng_generic_state_size(), minpoly, seq);
    fclose(f);
    free(fname);
}

static
void init_p_min(nmod_berlekamp_massey_t B) {
    F2LinRngGeneric* rng = f2lin_rng_generic_init();
    size_t state_size = f2lin_rng_generic_state_size();
    mp_limb_t a[state_size * 2];
    
    for (size_t i = 0; i < state_size * 2; ++i) {
        a[i] = f2lin_rng_generic_next_state(rng) & 0x01ul;
    }

    nmod_berlekamp_massey_add_points(B, a, state_size);
    nmod_berlekamp_massey_reduce(B);
    f2lin_rng_generic_destroy(rng);
}

static
void init_p_jump(const nmod_poly_t p_min, size_t jump) {
    nmod_poly_t base;
    nmod_poly_t p_jump;
    nmod_poly_init(base, 2);
    nmod_poly_init(p_jump, 2);

    nmod_poly_set_coeff_ui(base, 1, 1);
    nmod_poly_powmod_ui_binexp(p_jump, base, jump, p_min);

    nmod_poly_clear(base);
    nmod_poly_clear(p_jump);
}

static
double benchmark_minimal_polynomial_seq(size_t iterations, size_t repetitions) {
    nmod_berlekamp_massey_t B[iterations];
    F2LinBMPI bmpi = f2lin_bench_bmpi_init(repetitions);

    double times[2];
    F2LinRngGeneric* rng = f2lin_rng_generic_init();
    size_t state_size = f2lin_rng_generic_state_size();
    mp_limb_t seq[state_size * 2];

    for (size_t rep = 0; rep < repetitions; ++rep) {
        // initialize
        for (size_t i = 0; i < iterations; ++i) { 
            nmod_berlekamp_massey_init(B[i], 2);
        }

        times[0] = MPI_Wtime();
        // measure how long it takes to collect data for state
        for (size_t i = 0; i < iterations; ++i) {
            for (size_t j = 0; j < state_size * 2; ++j) {
                seq[i] = f2lin_rng_generic_gen64(rng) & 0x01ul;
            }
        }
        times[1] = MPI_Wtime();

        // free memory
        for (size_t i = 0; i < iterations; ++i) {
            nmod_berlekamp_massey_clear(B[i]);
        }

        f2lin_bench_bmpi_update(&bmpi, rep, times[1] - times[0]);
    }

    double avg = f2lin_bench_bmpi_eval(&bmpi) / (double) iterations;

    f2lin_rng_generic_destroy(rng);
    f2lin_bench_bmpi_destroy(&bmpi);

    return avg;
}

static
double benchmark_minimal_polynomial(size_t iterations, size_t repetitions) {
    nmod_berlekamp_massey_t B[iterations];
    F2LinBMPI bmpi = f2lin_bench_bmpi_init(repetitions);

    double times[2];
    F2LinRngGeneric* rng = f2lin_rng_generic_init();

    for (size_t rep = 0; rep < repetitions; ++rep) {
        // initialize
        for (size_t i = 0; i < iterations; ++i) { 
            nmod_berlekamp_massey_init(B[i], 2);
        }

        times[0] = MPI_Wtime();
        for (size_t i = 0; i < iterations; ++i) {
            init_p_min(B[i]);
        }
        times[1] = MPI_Wtime();

        // free memory
        for (size_t i = 0; i < iterations; ++i) {
            nmod_berlekamp_massey_clear(B[i]);
        }

        f2lin_bench_bmpi_update(&bmpi, rep, times[1] - times[0]);
    }

    double avg = f2lin_bench_bmpi_eval(&bmpi) / (double) iterations;

    f2lin_rng_generic_destroy(rng);
    f2lin_bench_bmpi_destroy(&bmpi);

    return avg;
}

static
double benchmark_jump_polynomial_init(size_t iterations, size_t repetitions, size_t jump) {
    nmod_berlekamp_massey_t B;
    F2LinBMPI bmpi = f2lin_bench_bmpi_init(repetitions);
    double times[2];
    nmod_berlekamp_massey_init(B, 2);
    init_p_min(B);

    for (size_t rep = 0; rep < repetitions; ++rep) {
        nmod_poly_t dummy;
        times[0] = MPI_Wtime();

        // measure how long initialization takes
        for (size_t i = 0; i < iterations; ++i) {
            nmod_poly_init(dummy, 2);
            nmod_poly_clear(dummy);
        }
        times[1] = MPI_Wtime();
        
        f2lin_bench_bmpi_update(&bmpi, rep, times[1] - times[0]);
    }

    double avg = f2lin_bench_bmpi_eval(&bmpi) / (double) iterations;

    nmod_berlekamp_massey_clear(B);
    f2lin_bench_bmpi_destroy(&bmpi);

    return avg;
}

static 
double benchmark_jump_polynomial(size_t iterations, size_t repetitions, size_t jump) {
    nmod_berlekamp_massey_t B;
    F2LinBMPI bmpi = f2lin_bench_bmpi_init(repetitions);
    double times[2];
    nmod_berlekamp_massey_init(B, 2);
    init_p_min(B);

    for (size_t rep = 0; rep < repetitions; ++rep) {
        nmod_poly_t dummy;
        times[0] = MPI_Wtime();
        // measure how long initialization takes
        for (size_t i = 0; i < iterations; ++i) {
            init_p_jump(nmod_berlekamp_massey_V_poly(B), jump);
        }
        times[1] = MPI_Wtime();
        
        f2lin_bench_bmpi_update(&bmpi, rep, times[1] - times[0]);
    }


    double avg = f2lin_bench_bmpi_eval(&bmpi) / (double) iterations;

    nmod_berlekamp_massey_clear(B);
    f2lin_bench_bmpi_destroy(&bmpi);

    return avg;
}

int main(int argc, char* argv[argc + 1]) {
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
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    data results[n_jumps];

    for (size_t i = 0; i < n_jumps; ++i) {
        results[i].jp = benchmark_jump_polynomial(iterations, repetitions, jumps[i]);
        results[i].init = benchmark_jump_polynomial_init(iterations, repetitions, jumps[i]);
        if (rank == 0) printf("jump: %llu\tinit: %5.2e\tjp: %5.2e\n", 
                              jumps[i], results[i].init, results[i].jp);
    }

    double minpoly = benchmark_minimal_polynomial(iterations, repetitions);
    double seq = benchmark_minimal_polynomial_seq(iterations, repetitions);

    if (rank == 0) { 
        printf("state size: %zu\tminpoly: %5.2e\tminpoly_seq: %5.2e\n",
                f2lin_rng_generic_state_size(), minpoly, seq);
        write_results(argv[0], n_jumps, jumps, results, minpoly, seq);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
