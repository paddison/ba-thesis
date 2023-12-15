#include <cstdlib>
#include <ctime>
#include "../src/polynomials.cpp"
#include "tools.h"
/* Compare how long it takes to calculate a minimal polynomial
 * and how long it takes to calculate the jump polynomial for different sizes
 */
#define N_REP 100

double benchmark_minimal_polynomial() {
    struct timespec start, end; 
    uint64_t times[N_REP];

    NTL::GF2X p_min;
    for (size_t i = 0; i < N_REP; ++i) {
        timespec_get(&start, TIME_UTC);
        init_min_poly(p_min);
        timespec_get(&end, TIME_UTC);
        times[i] = (end.tv_nsec - start.tv_nsec);
        printf("%llu\n", times[i]);
    }

    return get_result(N_REP, times) / 1000.;
}

double benchmark_jump_polynomial(NTL::GF2X& p_min, size_t jump) {
    struct timespec start, end; 
    uint64_t times[N_REP];
    
    for (size_t i = 0; i < N_REP; ++i) {
        NTL::GF2X p_jump;
        timespec_get(&start, TIME_UTC);
        init_jump_poly(p_min, p_jump, jump);
        timespec_get(&end, TIME_UTC);
        times[i] = (end.tv_nsec - start.tv_nsec);
    }
    
    return get_result(N_REP, times) / 1000.;
}

int main(void) {
    NTL::GF2X p_min;
    init_min_poly(p_min);

    printf("Minpoly: %.2fus\n", benchmark_minimal_polynomial());

    for (size_t jump = 1; jump <= 100000000000ull; jump *= 10) {
        printf("Jumppoly: jump: %zu: %.2f us\n", 
                jump, benchmark_jump_polynomial(p_min, jump));
    }

    return EXIT_SUCCESS;
}
