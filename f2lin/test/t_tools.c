#include "../src/minunit.h"
#include "../benchmark/tools.c"
#include <string.h>

#define LEN 7

int tests_run = 0;
static const uint64_t data_c[LEN] = { 4, 6 ,2, 1, 2, 1, 9 };
static const uint64_t data_s[LEN] = { 1, 1, 2, 2, 4, 6, 9 };
static const uint64_t sanitized_c[LEN] = { 1, 1, 2, 2, 4 };

static void copy_data(uint64_t target[LEN], const uint64_t source[LEN]) {
    memcpy(target, source, sizeof(uint64_t) * LEN);
}

static char* test_median() {
    mu_assert("median gives wrong result, expected 2", median(LEN, data_s) == 2);
    
    return 0;
}

static char* test_cmp_uint64_t() {
    uint64_t data[LEN];
    copy_data(data, data_c);

    qsort(data, LEN, sizeof(uint64_t), cmp_uint64_t);

    for (size_t i = 0; i < LEN; ++i) 
        mu_assert("Compare function for sort gives wrong result", data[i] == data_s[i]);

    return 0;
}


static char* test_median_absolute_deviation() {
    mu_assert("median_absolute_deviation gives wrong result, expected 1",
               median_absolute_deviation(LEN, data_s, 2));

    return 0;
}

static char* test_remove_outliers() {
    uint64_t sanitized[LEN];
    size_t len = remove_outliers(LEN, data_s, sanitized, 1, 2);

    mu_assert("removed the wrong amount of outliers, expected 1", len == LEN - 2);

    for (size_t i = 0; i < len; ++i) 
        mu_assert("got wrong value in sanitized data", sanitized[i] == sanitized_c[i]);

    return 0;
}

static char* test_average() {
    mu_assert("Got wrong result for average, expected 2", 
               2 == average(LEN - 2, sanitized_c));
    return 0;
}

static char* all_tests() {
    mu_run_test(test_median);
    mu_run_test(test_cmp_uint64_t);
    mu_run_test(test_median_absolute_deviation);
    mu_run_test(test_remove_outliers);
    mu_run_test(test_average);

    return 0;
}

int main(void) {
    char* result = all_tests(); 

    if (result != 0) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }

    printf("Tests run: %d\n", tests_run);

    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
