#include <stdio.h>
#include <stdlib.h>
#include "cmp_swap.h"
 /* This file includes 4 tests of compare and swap. 2 cases should result in a
  * swap and 2 should not. Error messages will be printed out in case of
  * faulire
  */
void test1() {
  printf("Test1: Running test that should swap\n");
  // Initialize variables
  int pass = 1;
  uint32_t *target = malloc(sizeof(uint32_t));
  *target = 5;
  uint32_t old = 5;
  uint32_t new = 10;
  // call compare and swap
  uint32_t ret_val = compare_and_swap(target, old, new);
  // test return value
  if (ret_val == 0) {
    pass = 0;
    printf("cmp_swap returned incorrect value\n");
  }
  // check value of target
  if (*target != new) {
    pass = 0;
    printf("swap did not occur when it was supposed to\n");
  }
  // if no errors, then print pass
  if (pass) {
    printf("PASSED!\n");
  }
  // free target pointer
  free(target);
}

void test2() {
  printf("Test2: Running test that should not swap.\n");
  // Initialize variables
  int pass = 1;
  uint32_t *target = malloc(sizeof(uint32_t));
  *target = 5;
  uint32_t old = 10;
  uint32_t new = 4000;
  // call compare and swap
  uint32_t ret_val = compare_and_swap(target, old, new);
  // test return value
  if (ret_val != 0) {
    pass = 0;
    printf("cmp_swap returned incorrect value\n");
  }
  // check value of target
  if (*target == new) {
    pass = 0;
    printf("swap occured when it was not supposed to\n");
  }
  // if no errors, then print pass
  if (pass) {
    printf("PASSED!\n");
  }
  // free target pointer
  free(target);
}

void test3() {
  printf("Test3: Running test that should not swap.\n");
  // Initialize variables
  int pass = 1;
  uint32_t *target = malloc(sizeof(uint32_t));
  *target = 0;
  uint32_t old = 4000;
  uint32_t new = 10;
  // call compare and swap
  uint32_t ret_val = compare_and_swap(target, old, new);
  // test return value
  if (ret_val != 0) {
    pass = 0;
    printf("cmp_swap returned incorrect value\n");
  }
  // check value of target
  if (*target == new) {
    pass = 0;
    printf("swap occured when it was not supposed to\n");
  }
  // if no errors, then print pass
  if (pass) {
    printf("PASSED!\n");
  }
  // free target pointer
  free(target);
}

void test4() {
  printf("Test4: Running test that should swap.\n");
  // Initialize variables
  int pass = 1;
  uint32_t *target = malloc(sizeof(uint32_t));
  *target = 500;
  uint32_t old = 500;
  uint32_t new = 12;
  // call compare and swap
  uint32_t ret_val = compare_and_swap(target, old, new);
  // test return value
  if (ret_val == 0) {
    pass = 0;
    printf("cmp_swap returned incorrect value\n");
  }
  // check value of target
  if (*target != new) {
    pass = 0;
    printf("swap did not occur when it was supposed to\n");
  }
  // if no errors, then print pass
  if (pass) {
    printf("PASSED!\n");
  }
  // free target pointer
  free(target);
}


int main() {
  // run tests
  test1();
  test2();
  test3();
  test4();
  return 0;
}
