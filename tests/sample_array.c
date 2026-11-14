
#include <math.h>
#include <stdio.h>

#define SAMPLES_PER_SEC 48000.0f
#include "../audio/util.h"

static int failures;

static void check_near(const char *name, float actual,
                       float expected)
{
    if (!isfinite(actual) || fabsf(actual - expected) > 0.0001f) {
        printf("FAIL: %s: got %.6f, expected %.6f\n",
               name, actual, expected);
        failures++;
    }
}

int main(void)
{
    float samples[8] = { 0 };
    unsigned idx = 7;

    /* Write enough samples to wrap around the array. */
    for (unsigned i = 1; i <= 10; i++)
        sample_array_write((float)i, &idx, samples);

    check_near("latest sample",
               sample_array_read(0.0f, &idx, samples), 10.0f);

    check_near("previous sample",
               sample_array_read(1.0f, &idx, samples), 9.0f);

    check_near("two samples ago",
               sample_array_read(2.0f, &idx, samples), 8.0f);

    /* Fractional delay should interpolate between samples. */
    check_near("fractional delay",
               sample_array_read(1.5f, &idx, samples), 8.5f);

    if (failures) {
        printf("%d sample array test(s) failed\n", failures);
        return 1;
    }

    printf("All sample array tests passed\n");
    return 0;
}
