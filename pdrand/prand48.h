#ifndef _PRAND48_H
#define _PRAND48_H 1

#include <stdint.h>
#include <stdbool.h>

/* Modulo is set to 2^48 */
#define M 281474976710656

/* Default values for generator function */
#define A_DEFAULT 0x5deece66d
#define C_DEFAULT 0xb

/**
 * @brief returns a new 48 bit number which is stored right to left in @a B.
 * @a B must be of type uint16_t[3]
 */
#define SPLIT_BUF(B) ((uint64_t) B[2] << 32 | (uint32_t) B[1] << 16 | B[0])

/** 
 * @brief writes the 48 lower bits of @a r into @B.
 * @a r must be of type uint64_t
 * @a B must be of type uint16_t[3]
 */
#define MERGE_BUF(B, r)        \
    B[2] = (r >> 32) & 0xffff; \
    B[1] = (r >> 16) & 0xffff; \
    B[0] = r & 0xffff;

/**
 * @brief Represents the bytes of a ieee754 double precision 
 * floating point number.
 */
union IEEE754Double {
    double d;
    char b[sizeof(double)];
};

/*
 * @brief Write the lowest bit of @a sign, the 11 lower bits of @a exp
 * and all 48 bits of @a mantissa into @a n. The 4 lower bits of the actual
 * mantissa in n will be set to 0.
 *
 * It is required that all bits of n are set to 0 before calling this
 * function.
 */
void IEEE754Double_new(union IEEE754Double * n,
                       uint8_t sign, uint16_t exp, uint16_t mantissa[3]);

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
    bool init;
    uint16_t c;
    uint64_t a;
};

/**
 * @brief Initialize the global state of the RNG with default values.
 * The default seed is 0x1234abcd330e = 20017429951246.
 */
void prand48_init(void);

/**
 * @brief Initialize the global state of the RNG with @a seed
 */
void prand48_init_48(uint16_t seed[3]);

/**
 * @brief Initialize the global state. @a seed specifies the upper 
 * 32 bit of the seed. The lower 16 bit of the seed will always be 0x330e.
 */
void prand48_init_32(uint32_t seed);

/**
 * @brief Initialize the global state manually, setting the @a seed
 * mulitplier @a a, and addend @a c.
 */
void prand48_init_man(uint16_t seed[3], uint64_t a, uint16_t c);

/**
 * @brief Fast forward to the @a n th random number. The number itself
 * will be stored in @a buf, which can be seen as the thread local state
 * of the rng.
 */
void prand48_seek(uint16_t buf[3], uint64_t n);

/**
 * @brief Calculate the next random number. This will update @a buf.
 * @return A random double between 0.0 and 1.0
 */
double pdrand48(uint16_t buf[3]);

/**
 * @brief Calculate the next random number. This will update @a buf.
 *
 * It will load the 31 upper bits of the generated number into the
 * return value.
 *
 * @return A random 32 bit unsigned integer between 0 and 2^31 - 1 
 */
uint32_t plrand48(uint16_t buf[3]);

/**
 * @brief Calculate the next random number. This will update @a buf.
 *
 * It will load the 32 upper bits of the generated number into the 
 * return value.
 *
 * @return A random 32 bit signed integer between -2^31 and 2^31 - 1 
 */
int32_t pmrand48(uint16_t buf[3]);

#endif
