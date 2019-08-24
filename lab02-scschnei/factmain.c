#include <stdlib.h>
#include <stdio.h>
#include "fact.h"

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("Usage: There must be exactly one command line argument\n");
    exit(0);
  }

  // Cast string to int
  int num = atoi(argv[1]);

  if (num < 0){
    printf("Usage:The integer provided must be greater than or equal to 0\n");
    exit(0);
  }

  // Call factorial function from assembly file
  printf("%d\n", fact(num));

  return 1;
}
