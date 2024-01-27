#ifndef TOOLS_H
#define TOOLS_H

#ifdef __cplusplus 
extern "C" {
#endif

#include <stdlib.h>

enum ResType {
    MAD, MED
};

double f2lin_tools_get_result(const size_t len, double *data, enum ResType rtype);

#ifdef __cplusplus 
}
#endif

#endif
