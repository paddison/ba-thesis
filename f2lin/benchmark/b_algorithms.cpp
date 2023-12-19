// compare horner to poly to poly decomp
#include "../src/polynomials.hpp"
#include "../src/polynomials.cpp"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cstdio>

#define N_REPS 100

// also check different Q values

int sort_uint64(const void* a, const void* b) {
    uint64_t aa = *(uint64_t*) a;
    uint64_t bb = *(uint64_t*) b;
    if (a > b) return 1;
    if (a < b) return -1;
    else return 0;
}

uint64_t get_average(uint64_t times[N_REPS]) {
    uint64_t sum = 0;
    for (size_t i = 0; i < N_REPS; ++i) {
        sum += times[i];
    }

    return sum / N_REPS;
}

uint64_t get_median(uint64_t times[N_REPS]) {
    qsort(times, N_REPS, sizeof(uint64_t), sort_uint64);
    return times[N_REPS / 2];
}

uint64_t bench() {
    uint64_t sum = 0;
    uint64_t times[N_REPS];
    timespec start, end;
    StateRng64 rng;

    state_rng_init(&rng);

    for (size_t i = 0; i < N_REPS; ++i) {
        timespec_get(&start, TIME_UTC);
        polynomials_jump(&rng);  
        timespec_get(&end, TIME_UTC);
        times[i] = (end.tv_nsec - start.tv_nsec);
    }
    
    return get_average(times);
}

// test for different jump sizes
// give the option to specify jump values via commandline
int main(int argc, char** argv) {
    size_t jumps[argc - 1];
    size_t q;
    Config c;

    if (argc == 2) {
        printf("Usage: ./file q_val jump1 jump2 ...\n");
        return EXIT_SUCCESS;
    }

    if (!(q = strtoull(argv[1], 0, 10))) {
        printf("Q value must be number\n");
        return EXIT_SUCCESS;
    }

    for (size_t i = 2; i < argc; ++i) {
        size_t jump;
        if ((jump = strtoull(argv[i], 0, 10))) {
            jumps[i - 1] = jump;
        }
    }

    // write results to file?
    c = { .q = q, .algorithm = HORNER };
    init_polynomials(&c);

    for (size_t i = 0; i < argc - 2; ++i) {
        init_jump(jumps[i]);
        printf("jump: %zu\n", jumps[i]);
        set_jump_algorithm(HORNER);
        printf("horner:\t%llu ns\n", bench());
        set_jump_algorithm(SLIDING_WINDOW);
        printf("sw:\t%llu ns\n", bench());
        set_jump_algorithm(SLIDING_WINDOW_DECOMP);
        printf("swd:\t%llu ns\n", bench());
    }

    return EXIT_SUCCESS;
}
