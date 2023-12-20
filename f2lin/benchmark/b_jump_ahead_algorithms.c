#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "statistic.h"
#include "jump_ahead.h"
#include "rng_generic/rng_generic.h"
#include "config.h"

// copied from modern-c book, i did not write this function myself
static
double timespec_diff(struct timespec const* later,
                     struct timespec const* sooner){
  /* Be careful: tv_sec could be an unsigned type */
  if (later->tv_sec < sooner->tv_sec)
    return -timespec_diff(sooner, later);
  else
    return (later->tv_sec - sooner->tv_sec) +
        (later->tv_nsec - sooner->tv_nsec) * 10E-9;
}

static
void parse_argv(int argc, char *argv[argc], unsigned long long buf[argc - 1]) {
    // we ignore the first argument, which is the program name
    for (size_t i = 1; i < argc; ++i) {
        unsigned long long n = strtoull(argv[i], 0, 10);
        if (n != ULLONG_MAX) buf[i - 1] = n;
    }
}

static 
void benchmark_jump(size_t jump_size, F2LinConfig *cfg, 
                  size_t iterations, size_t repetitions) {
    // iterations: number of times a loop is run when collecting a sample.
    // repetitions: the number of samples that are collected
    volatile size_t dummy = 0;
    Statistic loop = { 0 };
    Statistic comp = { 0 };
    struct timespec times[3];

    F2LinRngGeneric* rng = f2lin_rng_generic_init();
    F2LinJump* jp = f2lin_jump_ahead_init(jump_size, cfg);

    // measure the loop
    for (size_t rep = 0; rep < repetitions; ++rep) {
        // start measurement
        timespec_get(&times[0], TIME_UTC);
        for (size_t i = 0; i < iterations; ++i) {
            dummy = i;
        }

        timespec_get(&times[1], TIME_UTC);

        for (size_t i = 0; i < iterations; ++i) {
            // do some heavy computation
            f2lin_jump_ahead_jump(jp, rng);
        }

        timespec_get(&times[2], TIME_UTC);
        // print something to avoid cpu optimization 
        //printf("did repetition, %zu iterations\n", dummy);

        statistic_update(&loop, timespec_diff(&times[1], &times[0]) / (double) iterations);
        statistic_update(&comp, timespec_diff(&times[2], &times[1]) / (double) iterations);
    }
    
    printf("Jump: %zu\ttime: %5.2es\tloop_time: %5.2ens\n", 
           jump_size, statistic_mean(&comp), statistic_mean(&loop));
}

static inline
void do_benchmark(size_t n_jumps, unsigned long long jumps[n_jumps], F2LinConfig *cfg, 
                  size_t iterations, size_t repetitions) {

    for (size_t i = 0; i < n_jumps; ++i) {
        benchmark_jump(jumps[i], cfg, iterations, repetitions);
    }
}

int main(int argc, char* argv[argc + 1]) {  
    printf("argc: %d\n", argc);
    unsigned long long buf[100];
    size_t iterations = 100, repetitions = 100;
    size_t n_jumps;
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
        buf[1] = 10;
        buf[2] = 100;
        buf[3] = 1000;
        buf[4] = 10000;
        buf[5] = 100000;
        buf[6] = 1000000;
        buf[7] = 10000000;
        buf[8] = 100000000;
        buf[9] = 1000000000;
    }

    printf("horner:\n");
    do_benchmark(n_jumps, buf, &cfg, iterations, repetitions);

    cfg.algorithm = SLIDING_WINDOW;
    printf("sliding window:\n");
    do_benchmark(n_jumps, buf, &cfg, iterations, repetitions);

    cfg.algorithm = SLIDING_WINDOW_DECOMP;
    printf("sliding window decomp:\n");
    do_benchmark(n_jumps, buf, &cfg, iterations, repetitions);

    return EXIT_SUCCESS;
}
