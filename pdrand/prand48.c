#include "prand48.h"
#include <assert.h>
#include <stdio.h>

/**************************
 * Internal Functionality *
 **************************/

/* Read only state of the rng */
static PrandState state = { 0 };

static uint64_t __iterate(uint64_t r, uint64_t a, uint16_t c) {
    return (a * r + c) % M;
}

static void __prand_next(uint16_t buf[3]) {
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

static uint64_t __ext_euclid(uint64_t a, uint64_t mod) {
    uint64_t q, t, r, new_t, new_r, tmp;

    t = 0;
    r = mod;
    new_t = 1;
    new_r = a;

    while (new_r != 0) {
        q = r / new_r;

        tmp = new_t;
        new_t = t - q * new_t;
        t = tmp;
        
        tmp = new_r;
        new_r = r - q * new_r;
        r = tmp;
    }

    if (r > 1) return -1;
    if (t < 0) return t + mod;

    return t;
}

static uint64_t __seek_intern(uint64_t a, uint64_t c, uint64_t n, uint64_t r) {
    uint64_t a_1, a_pow_n, a_pow_n_1, inv;

    a_pow_n = __powmod(a, n, M);
    a_pow_n_1 = (a_pow_n - 1) / 4;

    a_1 = (a - 1) / 4;
    
    assert(a_1 % 2 == 1);

    inv = __ext_euclid(a_1, M);

    assert((a_1 * inv) % M == 1);

    if (inv == -1) return -1;

    return (a_pow_n_1 * inv * c + a_pow_n * r) % M;
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

static uint64_t __seek_intern2(uint64_t a, uint64_t c, uint64_t n, uint64_t r) {
    // g is a
    // evaluate the first term
    uint64_t first_term = (r * __powmod(a, n, M)) % M;
    uint64_t second_term = __algorithm_c(c, a, n);
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

void prand48_seek(uint16_t buf[3], uint64_t n) {
    if (!state.init) {
        fprintf(stderr, "Need to initialize before calling seek()\n");
        return;
    }

    buf[0] = state.seed[0];
    buf[1] = state.seed[1];
    buf[2] = state.seed[2];

    uint64_t r = SPLIT_BUF(buf);
    r = __seek_intern2(state.a, state.c, n, r);

    assert(r < ((uint64_t ) 1 << 48));

    MERGE_BUF(buf, r);
} 

double pdrand48(uint16_t buf[3]) {
    if (!buf || !state.init) return -1.; 

    union IEEE754Double ret;

    __prand_next(buf);
    IEEE754Double_new(&ret, 0, 0x3ff, buf);

    return ret.d - 1.0;
}

uint32_t plrand48(uint16_t buf[3]) {
    if (!buf || !state.init) return -1;

    __prand_next(buf);
    uint32_t ret = ((uint32_t) buf[2] << 15) | (buf[1] >> 1);
    return ret;
}

int32_t pmrand48(uint16_t buf[3]) {
    if (!buf || !state.init) return -1;

    __prand_next(buf);
    int32_t ret = ((int32_t) buf[2] << 16) | buf[1];
    return ret;
}
