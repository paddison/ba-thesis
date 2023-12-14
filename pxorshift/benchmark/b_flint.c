#include "flint/fmpz.h"
#include "flint/nmod_poly.h"
#include "flint/fmpz_mod.h"
#include "flint/fmpz_mod_poly.h"
#include "../src/xorshift64.h"
#include <time.h>

void init_p_min(nmod_berlekamp_massey_t B) {
    nmod_poly_struct V;
    StateRng rng;
    size_t state_size = sizeof(StateRng) * 8 * 2;
    mp_limb_t a[state_size];
    state_rng_init(&rng);
    
    for (size_t i = 0; i < state_size; ++i) {
        step(&rng);
        a[i] = rng.s & 0x01ul;
    }

    nmod_berlekamp_massey_add_points(B, a, state_size);
    nmod_berlekamp_massey_reduce(B);
}

void init_p_jump(const nmod_poly_t p_min, nmod_poly_t p_jump, size_t jump) {
    nmod_poly_t base;
    nmod_poly_init(base, 2);

    nmod_poly_set_coeff_ui(base, 1, 1);
    nmod_poly_powmod_ui_binexp(p_jump, base, jump, p_min);
}

void init_p_min_fmpz(fmpz_mod_poly_t poly) {
    StateRng rng;
    size_t state_size = sizeof(StateRng) * 8 * 2;
    fmpz_mod_ctx_t ctx;
    fmpz mod = 2;
    fmpz seq[state_size];

    fmpz_mod_ctx_init(ctx, &mod);
    state_rng_init(&rng);

    for (size_t i = 0; i < state_size; ++i) {
        step(&rng);
        seq[i] = rng.s & 0x01ul;
    }
    fmpz_mod_poly_minpoly(poly, seq, state_size, ctx);
}


#define N_REP 10

uint64_t benchmark_minimal_polynomial() {
    StateRng rng;
    nmod_berlekamp_massey_t B;
    clock_t start, end; 
    unsigned long times[N_REP];
    uint64_t sum = 0;

    state_rng_init(&rng);

    printf("cps: %lu\n", CLOCKS_PER_SEC);
    
    for (size_t i = 0; i < N_REP; ++i) {
        nmod_berlekamp_massey_init(B, 2);
        
        start = clock();
        init_p_min(B);
        times[i] = (clock() - start);
        nmod_berlekamp_massey_clear(B);
        printf("%lu\n", times[i]);
    }

    for (size_t i = 0; i < N_REP; ++i) {
        sum += times[i];
    }
    return sum / N_REP;
}

uint64_t benchmark_jump_polynomial(const nmod_poly_t p_min, size_t jump) {
    nmod_poly_t p_jump;
    struct timespec start, end; 
    uint64_t times[N_REP];
    uint64_t sum = 0;


    for (size_t i = 0; i < N_REP; ++i) {
        nmod_poly_init(p_jump, 2);
        nmod_poly_set_coeff_ui(p_jump, 1, 1);

        timespec_get(&start, TIME_UTC);
        init_p_jump(p_min, p_jump, jump);
        timespec_get(&end, TIME_UTC);

        times[i] = (end.tv_nsec - start.tv_nsec);
    
        //printf("%lu-%lu\n", start.tv_nsec, end.tv_nsec);
        nmod_poly_clear(p_jump);
    }

    for (size_t i = 0; i < N_REP; ++i) {
        sum += times[i];
    }
    return sum / N_REP;
}

int main(void) {
    nmod_berlekamp_massey_t B;
    nmod_poly_t p_jump;

    nmod_berlekamp_massey_init(B, 2);
    init_p_min(B);
    nmod_poly_init(p_jump, 2);

    init_p_jump(nmod_berlekamp_massey_V_poly(B), p_jump, 10000);

    nmod_poly_print(p_jump); flint_printf("\n");

    printf("min_poly: %llu\n", benchmark_minimal_polynomial());

    for (size_t i = 1; i <= 10000000; i *= 10) {
        printf("jump poly: jump: %zu: %llu\n", 
                i, benchmark_jump_polynomial(nmod_berlekamp_massey_V_poly(B), i));
    }



    nmod_berlekamp_massey_clear(B);
    return EXIT_SUCCESS;
}
