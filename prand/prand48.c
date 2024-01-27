#include "prand48.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

struct Prand48 {
    uint16_t buf[3];
};

/**
 * @brief the global state of prand. Numbers are generated using the
 * formula r(n + 1) = a * r(n) + c mod m. 
 * m always equals 2^48, while default values the fields are:
 * a: 0x5deece66d (25214903917) 
 * c: 0xb (11)
 *
 * It can be initialized using ::prand48_init, ::prand48_init_48,
 * ::prand48_init_32 or ::prand_init_man
 */
typedef struct PrandState PrandState;

struct PrandState {
    uint16_t seed[3];
    uint16_t c;
    uint64_t a;
    bool init;
};
/**************************
 * Internal Functionality *
 **************************/

/* Read only state of the rng */
static PrandState state = { 0 };

static inline
uint64_t __iterate(uint64_t r, uint64_t a, uint16_t c) {
    return (a * r + c) % M;
}

static inline
void __prand_next(uint16_t buf[3]) {
    uint64_t r = SPLIT_BUF(buf);

    r = __iterate(r, state.a, state.c);
    assert(r < ((uint64_t ) 1 << 48));

    MERGE_BUF(buf, r);
}

static uint64_t __powmod(uint64_t base, uint64_t exp, uint64_t mod) {
    // currently, we always calculate mod 2^64, since this 
    // is the only way which calulates a number correctly
    if (mod == 1) return 0;

    uint64_t result = 1;

    base = base % mod;

    while (exp > 0) {
        if (exp & 1) {
            result = (result * base);// % mod;
        }
        exp >>= 1;
        base = (base * base); // % mod;
    }

    return result;
}

/*
 * Calculates c * sum_(i=0)to(k-1):(a^i)
 */
static uint64_t __algorithm_c(uint64_t c, uint64_t g, uint64_t n) {
    uint64_t C = 0, f = c, h = g, i = (n + M) % M;
    while (i > 0) {
        if (i & 1) C = (C * h + f) % M;
        f = (f * (h + 1)) % M;
        h = (h * h) % M;
        i = i / 2;
    }

    return C;
}

static uint64_t __jump_intern(uint64_t a, uint64_t c, uint64_t n, uint64_t r) {
    // g is a
    // evaluate the first term
    uint64_t first_term = (r * __powmod(a, n, M)) % M;
    uint64_t second_term;
    if (c) second_term = __algorithm_c(c, a, n);
    else second_term = 0;
    return (first_term + second_term) % M;
}


/*******************************
 * Header file implementations *
 *******************************/

void IEEE754Double_new(union IEEE754Double* n, 
                       uint8_t sign, uint16_t exp, uint16_t mantissa[3]) {

    /* ieee754 double precision representation:
     *
     * seeeeeee eeeemmmm mmmmmmmm mmmmmmmm mmmmmmmm mmmmmmmm mmmmmmmm mmmmmmmm
     * 00111111_1111nnnn_nnnnnnnn_nnnnnnnn_nnnnnnnn_nnnnnnnn_nnnnnnnn_nnnn0000
     * b[0]     b[1]     b[2]     b[3]     b[4]     b[5]     b[6]     b[7]
     *              r[0]              r[1]              r[2]
     *              fedc_ba987654_3210fedc_ba987654_3210fedc_ba987654_3210
     */
    n->b[7] = ((sign & 1) << 7) | ((exp >> 4) & 0x7f);
    n->b[6] = ((exp << 4) & 0xf0) | (mantissa[2] >> 12);
    n->b[5] = (mantissa[2] >> 4) & 0xff;
    n->b[4] = ((mantissa[2] & 0xf) << 4) | (mantissa[1] >> 12);
    n->b[3] = (mantissa[1] >> 4) & 0xff;
    n->b[2] = ((mantissa[1] & 0xf) << 4) | (mantissa[0] >> 12);
    n->b[1] = (mantissa[0] >> 4) & 0xff;
    n->b[0] = (mantissa[0] & 0xf) << 4;
}

void prand48_init() {
    state.seed[0] = 0x1234; 
    state.seed[1] = 0xabcd; 
    state.seed[2] = 0x330e; 
    
    state.c = C_DEFAULT;
    state.a = A_DEFAULT;

    state.init = true;
}

void prand48_init32(uint32_t seed) {
    state.seed[0] = seed >> 16; 
    state.seed[1] = seed & 0xffff; 
    state.seed[2] = 0x330e; 
    
    state.c = C_DEFAULT;
    state.a = A_DEFAULT;

    state.init = true;
}

void prand48_init48(uint16_t seed[3]) {
    state.seed[0] = seed[0]; 
    state.seed[1] = seed[1]; 
    state.seed[2] = seed[2]; 
    
    state.c = C_DEFAULT;
    state.a = A_DEFAULT;

    state.init = true;
}

void prand48_init_man(uint16_t seed[3], uint64_t a, uint16_t c) {
    state.seed[0] = seed[0]; 
    state.seed[1] = seed[1]; 
    state.seed[2] = seed[2]; 
    
    state.c = c;
    state.a = a;

    state.init = true;
}

Prand48* prand48_get() {
    Prand48* prand = 0;
    if (!state.init) {
        fprintf(stderr, "Warning: State not initialized yet, returning 0!\n");
        return prand;
    }
    prand = calloc(1, sizeof(Prand48));
    prand->buf[0] = state.seed[0];
    prand->buf[1] = state.seed[1];
    prand->buf[2] = state.seed[2];
    
    return prand;
}

void prand48_destroy(Prand48* prand) {
    free(prand); 
}

void prand48_jump_abs(Prand48* prand, uint64_t n) {
    prand->buf[0] = state.seed[0];
    prand->buf[1] = state.seed[1];
    prand->buf[2] = state.seed[2];

    uint64_t r = SPLIT_BUF(prand->buf);
    r = __jump_intern(state.a, state.c, n, r);

    assert(r < ((uint64_t ) 1 << 48));

    MERGE_BUF(prand->buf, r);
} 

void prand48_jump_rel(Prand48* prand, uint64_t n) {
    uint64_t r = SPLIT_BUF(prand->buf);
    r = __jump_intern(state.a, state.c, n, r);

    assert(r < ((uint64_t ) 1 << 48));

    MERGE_BUF(prand->buf, r);
}

double pdrand48(Prand48* prand) {
    if (!prand || !state.init) return -1.; 

    union IEEE754Double ret;

    __prand_next(prand->buf);
    IEEE754Double_new(&ret, 0, 0x3ff, prand->buf);

    return ret.d - 1.0;
}

uint32_t plrand48(Prand48* prand) {
    if (!prand || !state.init) return -1;

    __prand_next(prand->buf);
    uint32_t ret = ((uint32_t) prand->buf[2] << 15) | (prand->buf[1] >> 1);
    return ret;
}

int32_t pmrand48(Prand48* prand) {
    if (!prand || !state.init) return -1;

    __prand_next(prand->buf);
    int32_t ret = ((int32_t) prand->buf[2] << 16) | prand->buf[1];
    return ret;
}

void prand48_next(Prand48* prand) {
   __prand_next(prand->buf); 
}
