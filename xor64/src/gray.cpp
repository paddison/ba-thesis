#include "gray.hpp"

uint16_t* gray_code(size_t W, uint16_t code[1 << W]) {
    // the implementation is taken from
    // https://www.eetimes.com/gray-code-fundamentals-part-2/
    // it implements the prefix and mirror method
    size_t len;

    code[0] = 0; code[1] = 1;

    for (size_t i = 1; i < W; ++i) {
        len = (2 << i) - 1;
        for (size_t j = 0; j < 1 << i; ++j) {
            code[len - j] = code[j] | (1 << i);
        }
    }

    return code;
}
