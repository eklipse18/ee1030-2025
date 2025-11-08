#include "helper.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double **multiply(int m1, int n1, double **A, int m2, int n2, double **B) {
  if (n1 != m2) {
    return NULL; // Incompatible dimensions
  }
  double **C = (double **)malloc(m1 * sizeof(double *));
  for (int i = 0; i < m1; i++) {
    C[i] = (double *)malloc(n2 * sizeof(double));
    for (int j = 0; j < n2; j++) {
      C[i][j] = 0;
      for (int k = 0; k < n1; k++) {
        C[i][j] += A[i][k] * B[k][j];
      }
    }
  }
  return C;
}

double **transpose(int m, int n, double **A) {
  double **T = (double **)malloc(n * sizeof(double *));
  for (int i = 0; i < n; i++) {
    T[i] = (double *)malloc(m * sizeof(double));
    for (int j = 0; j < m; j++) {
      T[i][j] = A[j][i];
    }
  }
  return T;
}

void normalize(double *v, int n) {
  double norm = 0;
  for (int i = 0; i < n; i++)
    norm += v[i] * v[i];
  norm = sqrt(norm);
  for (int i = 0; i < n; i++)
    v[i] /= norm;
}

// Algorithm to find eigenvalues and eigenvectors using Jacobi method (only for
// symmetric matrices)
void jacobi(double **A, double *eigvals, double **eigvecs, int n) {
    // initialize eigenvectors as identity
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            eigvecs[i][j] = (i == j) ? 1.0 : 0.0;

    const double eps = 1e-12;
    while (1) {
        // find largest off-diagonal element
        int p = 0, q = 1;
        double max_off = 0.0;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                double aij = fabs(A[i][j]);
                if (aij > max_off) { max_off = aij; p = i; q = j; }
            }
        }
        if (max_off < eps) break;

        double app = A[p][p], aqq = A[q][q], apq = A[p][q];
        double theta = 0.5 * atan2(2.0 * apq, (aqq - app));
        double c = cos(theta), s = sin(theta);

        // update A in-place: set A[p][q]=0 and update affected rows/cols
        double app_new = c*c*app - 2.0*c*s*apq + s*s*aqq;
        double aqq_new = s*s*app + 2.0*c*s*apq + c*c*aqq;
        A[p][p] = app_new;
        A[q][q] = aqq_new;
        A[p][q] = A[q][p] = 0.0;

        for (int k = 0; k < n; ++k) {
            if (k == p || k == q) continue;
            double akp = A[k][p], akq = A[k][q];
            A[k][p] = A[p][k] = c * akp - s * akq;
            A[k][q] = A[q][k] = s * akp + c * akq;
        }

        // update eigenvector matrix: rotate columns p and q
        for (int k = 0; k < n; ++k) {
            double vip = eigvecs[k][p], viq = eigvecs[k][q];
            eigvecs[k][p] = c * vip - s * viq;
            eigvecs[k][q] = s * vip + c * viq;
        }
    }

    // diagonal of A contains eigenvalues
    for (int i = 0; i < n; ++i)
        eigvals[i] = A[i][i];

    // normalize eigenvector columns
    for (int j = 0; j < n; ++j) {
        double norm = 0.0;
        for (int i = 0; i < n; ++i) norm += eigvecs[i][j] * eigvecs[i][j];
        norm = sqrt(norm);
        if (norm > 0.0) {
            for (int i = 0; i < n; ++i) eigvecs[i][j] /= norm;
        }
    }
}

void eigen_decomposition(int n, double **A, double *ev, double **evec) {
  // This function should compute the eigenvalues and eigenvectors of matrix A
  // (n x n) and store them in ev and evec respectively.
  jacobi(A, ev, evec, n);
}

double frobenius_norm(int m, int n, double **A) {
  double norm = 0.0;
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      norm += A[i][j] * A[i][j];
    }
  }
  return sqrt(norm);
}