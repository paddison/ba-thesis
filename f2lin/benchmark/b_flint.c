#include <stdio.h>
#include <stdlib.h>

#include "flint/flint.h"
#include "mpi.h"
#include "flint/nmod_types.h"
#include "flint/fmpz.h"
#include "flint/nmod_poly.h"
#include "flint/fmpz_mod.h"
#include "flint/fmpz_mod_poly.h"

#include "rng_generic/rng_generic.h"
#include "bench.h"
#include "tools.h"

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
void init_p_min_fmpz(fmpz_mod_poly_t poly) {
    F2LinRngGeneric* rng = f2lin_rng_generic_init();
    size_t state_size = f2lin_rng_generic_state_size();
    fmpz_mod_ctx_t ctx;
    fmpz mod = 2;
    fmpz seq[state_size];

    fmpz_mod_ctx_init(ctx, &mod);

    for (size_t i = 0; i < state_size; ++i) {
        f2lin_rng_generic_next_state(rng);
        seq[i] = f2lin_rng_generic_gen64(rng) & 0x01ul;
    }
    fmpz_mod_poly_minpoly(poly, seq, state_size, ctx);
}

static
void benchmark_minimal_polynomial(size_t iterations, size_t repetitions) {
    nmod_berlekamp_massey_t B[iterations];
    F2LinBMPI init_p_min_data = f2lin_bench_bmpi_init(repetitions);
    F2LinBMPI seq_data = f2lin_bench_bmpi_init(repetitions);

    double times[3];
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
        for (size_t i = 0; i < iterations; ++i) {
            init_p_min(B[i]);
        }
        times[2] = MPI_Wtime();

        // free memory
        for (size_t i = 0; i < iterations; ++i) {
            nmod_berlekamp_massey_clear(B[i]);
        }

        f2lin_bench_bmpi_update(&seq_data, rep, times[1] - times[0]);
        f2lin_bench_bmpi_update(&init_p_min_data, rep, times[2] - times[1]);
    }

    double seq_avg = f2lin_bench_bmpi_eval(&seq_data) / (double) iterations;
    double init_p_min_avg = f2lin_bench_bmpi_eval(&init_p_min_data) / (double) iterations;

    if (init_p_min_data.rank == init_p_min_data.root) {
        printf("state_size: %zu\t sequence: %5.2e\tminpoly: %5.2e\n",
               state_size, seq_avg, init_p_min_avg);
    }

}

static 
void benchmark_jump_polynomial(size_t iterations, size_t repetitions, size_t jump) {
    nmod_berlekamp_massey_t B;
    F2LinBMPI init_data = f2lin_bench_bmpi_init(repetitions);
    F2LinBMPI jump_data = f2lin_bench_bmpi_init(repetitions);
    double times[3];
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
        // measure time it takes to calculate jump polynomial
        for (size_t i = 0; i < iterations; ++i) {
            init_p_jump(nmod_berlekamp_massey_V_poly(B), jump);
        }
        times[2] = MPI_Wtime();
        
        f2lin_bench_bmpi_update(&init_data, rep, times[1] - times[0]);
        f2lin_bench_bmpi_update(&jump_data, rep, times[2] - times[1]);
    }


    double init_avg = f2lin_bench_bmpi_eval(&init_data) / (double) iterations;
    double jump_avg = f2lin_bench_bmpi_eval(&jump_data) / (double) iterations;

    if (init_data.rank == init_data.root) {
        printf("jump_size: %zu\t allocations: %5.2e\tjump_poly: %5.2e\n",
               jump, init_avg, jump_avg);
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
