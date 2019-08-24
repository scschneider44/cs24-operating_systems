#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "ffunc.h"
/**
 * 2 is the smallest set of numbers you can add so it will lead to the fewest
 * amount of rounding errors
 */
#define SMALL_ARRAY 2


/* This function takes an array of single-precision floating point values,
 * and computes a sum in the order of the inputs.  Very simple.
 */
float fsum(FloatArray *floats) {
    float sum = 0;
    int i;

    for (i = 0; i < floats->count; i++)
        sum += floats->values[i];

    return sum;
}

float fsum_helper (FloatArray *floats, unsigned int start, unsigned int end) {
  float s;
  /* length of part of array we want to look at */
  int length = end - start;

  /**
   * If the length is small enough we can sum the floats in the given array
   * without running into the problem of rounding errors
   */
  if (length < SMALL_ARRAY) {
    s = floats->values[start];

    for (unsigned int i = start + 1; i <= end; i++) {
      s += floats->values[i];
    }

    return s;
  }
  /**
   * else we split the array in half and recursively call the function on both
   * havles returning the sum of the recursive calls to get the correct output
   */
  else {
    int m = length / 2;

    return fsum_helper(floats, start, start + m) + \
           fsum_helper(floats, start + m + 1, end);
  }

}


float my_fsum(FloatArray *floats) {
  /* run recursion on the whole FloatArray */
  return fsum_helper(floats, 0, floats->count);
}


int main() {
    FloatArray floats;
    float sum1, sum2, sum3, my_sum;

    load_floats(stdin, &floats);
    printf("Loaded %d floats from stdin.\n", floats.count);

    /* Compute a sum, in the order of input. */
    sum1 = fsum(&floats);

    /* Use my_fsum() to compute a sum of the values.  Ideally, your
     * summation function won't be affected by the order of the input floats.
     */
    my_sum = my_fsum(&floats);

    /* Compute a sum, in order of increasing magnitude. */
    sort_incmag(&floats);
    sum2 = fsum(&floats);

    /* Compute a sum, in order of decreasing magnitude. */
    sort_decmag(&floats);
    sum3 = fsum(&floats);

    /* %e prints the floating-point value in full precision,
     * using scientific notation.
     */
    printf("Sum computed in order of input:  %e\n", sum1);
    printf("Sum computed in order of increasing magnitude:  %e\n", sum2);
    printf("Sum computed in order of decreasing magnitude:  %e\n", sum3);

    printf("My sum:  %e\n", my_sum);

    free(floats.values);
    return 0;
}
