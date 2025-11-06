#include "helper.h"
#include <stdlib.h>
#include <stdio.h>

double **low_rank_approx(int m, int n, double ***svd, int k) {
    if (!svd || !svd[0] || !svd[1] || !svd[2]) return NULL;
    if (m <= 0 || n <= 0) return NULL;

    int r = (m < n) ? m : n;
    if (k <= 0) return NULL;
    if (k > r) k = r; // cap k to rank

    double **U = svd[0];
    double **S = svd[1];
    double **V = svd[2];

    double **Ak = (double **)malloc(m * sizeof(double *));
    for (int i = 0; i < m; ++i) {
        Ak[i] = (double *)calloc(n, sizeof(double));
    }

    /* Compute A_k = sum_{t=0..k-1} sigma_t * U[:,t] * V[:,t]^T */
    for (int t = 0; t < k; ++t) {
        double sigma = S[t][t];
        if (sigma == 0.0) continue;
        for (int i = 0; i < m; ++i) {
            double u = U[i][t];
            if (u == 0.0) continue;
            double coeff = sigma * u;
            double *Ai = Ak[i];
            for (int j = 0; j < n; ++j) {
                /* V[j][t] is component j of t-th right singular vector
                   so contribution to (i,j) is coeff * V[j][t] */
                Ai[j] += coeff * V[j][t];
            }
        }
    }

    return Ak;
}