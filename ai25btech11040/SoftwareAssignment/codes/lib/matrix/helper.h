#ifndef HELPER_H
#define HELPER_H

#include <stdlib.h>

double **multiply(int m1, int n1, double **A, int m2, int n2, double **B);

double **transpose(int m, int n, double **A);

void eigen_decomposition(int n, double **A, double *ev, double **evec);

void normalize(double *v, int n);

void jacobi(double **A, double *eigvals, double **eigvecs, int n);

double frobenius_norm(int m, int n, double **A);

#endif