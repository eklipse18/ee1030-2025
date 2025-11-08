#include "lib/matrix/lra.h"
#include "lib/matrix/svd.h"
#include "lib/png/readpng.h"
#include "lib/png/savepng.h"
#include "lib/matrix/helper.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char *argv[]) {
  int ihdr[7];
  int k;
  sscanf(argv[2], "%d", &k);
  int **array = readpng(argv[1], ihdr);
  if (!array) {
    fprintf(stderr, "Failed to read PNG file %s\n", argv[1]);
    return -1;
  }
  // convert arr to doubles
  double **double_array = (double **)malloc(ihdr[1] * sizeof(double *));
  for (int i = 0; i < ihdr[1]; i++) {
    double_array[i] = (double *)malloc(ihdr[0] * sizeof(double));
    for (int j = 0; j < ihdr[0]; j++) {
      double_array[i][j] = (double)array[i][j];
    }
  }
  double ***svd_result = svd(ihdr[1], ihdr[0], double_array);
  double **A_k = low_rank_approx(ihdr[1], ihdr[0], svd_result, k);
  int **A_k_int = (int **)malloc(ihdr[1] * sizeof(int *));
  for (int i = 0; i < ihdr[1]; i++) {
    A_k_int[i] = (int *)malloc(ihdr[0] * sizeof(int));
    for (int j = 0; j < ihdr[0]; j++) {
      A_k_int[i][j] = (int)A_k[i][j];
    }
  }

  // Frobenius norm calculation
  double **diff_arr = (double **)malloc(ihdr[1] * sizeof(double *));
  for (int i = 0; i < ihdr[1]; i++) {
    diff_arr[i] = (double *)malloc(ihdr[0] * sizeof(double));
    for (int j = 0; j < ihdr[0]; j++) {
      diff_arr[i][j] = array[i][j] - A_k_int[i][j];
    }
  }
  double frob_norm = frobenius_norm(ihdr[1], ihdr[0], diff_arr);
  printf("Frobenius norm of the difference between original and A_k: %.5lf\n",
         frob_norm);
  printf("Frobenius norm error per pixel: %.5lf\n", frob_norm / (ihdr[1] * ihdr[0]));

  // free memory
  for (int i = 0; i < ihdr[1]; i++) {
    free(array[i]);
    free(double_array[i]);
  }
  free(array);
  free(double_array);
  for (int i = 0; i < ihdr[1]; i++) {
    free(svd_result[0][i]);
    free(svd_result[1][i]);
  }
  free(svd_result[0]);
  free(svd_result[1]);
  free(svd_result[2]);
  free(svd_result);

  // DEBUG: print A_k in the block format in lib/png/readpng.c
  // for (int i = 0; i < ihdr[1]; i++) {
  //   for (int j = 0; j < ihdr[0]; j++) {
  //     printf("\x1B[48;5;%dm  \x1B[0m",
  //            232 + (A_k_int[i][j]) * 23 / ((1 << 8) - 1));
  //   }
  //   printf("\n");
  // }
  savepng("out.png", A_k, ihdr);
  return 0;
}