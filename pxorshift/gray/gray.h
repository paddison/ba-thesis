#ifndef GRAY_H
#define GRAY_H

#include <stdlib.h>

/**
 * Computes the grey code enumeration (https://en.wikipedia.org/wiki/Gray_code)
 * of a @param W bits wide number.
 * The array @param code to store the enumerated values has to be of size 2^W.
 * 
 * The return value is the pointer to the input array @param code.
 */
uint16_t* gray_code(size_t W, uint16_t code[1 << W]);

#endif
