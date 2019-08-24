#include <stdlib.h>
#include <stdio.h>
#include "gcd.h"
extern int gcd(int a, int b);

int main(int argc, char** argv) {
  if (argc != 3) {
    printf("Usage: There must be exactly two command line arguments\n");
    exit(0);
  }

  // Making string args into ints
  int a = atoi(argv[1]);
  int b = atoi(argv[2]);

  if (a < 0 || b < 0){
    printf("Usage:The integer provided must be greater than or equal to 0\n");
    exit(0);
  }

  // calls gcd function from assembly code
  printf("%d\n", gcd(a, b));

  return 1;
}
