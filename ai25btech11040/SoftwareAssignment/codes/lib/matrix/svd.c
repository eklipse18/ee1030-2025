// Implementing svd

#include <stdio.h>
#include <stdlib.h>
#include "helper.h"
#include <math.h>

double *** svd(int m, int n, double **A) {
    // Placeholder for SVD implementation
    // This function should compute the SVD of matrix A (m x n)
    // and return matrices U, S, and V as a 3D array.
    // For simplicity, we will return NULL here.
    double *** ret = malloc(3 * sizeof(double**));
    ret[0] = NULL; // U: mxm
    ret[1] = NULL; // S: mxn
    ret[2] = NULL; // V: nxn

    // We shall follow the eigenvaluedecomposition method for SVD
    // Find A^T * A
    double ** at = transpose(m, n, A);
    double ** at_a = multiply(n, m, at, m, n, A);

    // Compute eigenvalues and eigenvectors of A^T * A
    // We will get n eigenvalues and n eigenvectors
    double *ev = (double *)malloc(n * sizeof(double));
    double **evec = (double **)malloc(n * sizeof(double *));
    for (int i = 0; i < n; i++) {
        evec[i] = (double *)malloc(n * sizeof(double));
    }
    eigen_decomposition(n, at_a, ev, evec);

    //Sort eigenvalues and eigenvectors according to eigenvalues
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (ev[j] < ev[j + 1]) {
                // Swap eigenvalues
                double temp = ev[j];
                ev[j] = ev[j + 1];
                ev[j + 1] = temp;
                // Swap eigenvectors
                double *temp_vec = malloc(n * sizeof(double));
                for (int k = 0; k < n; k++) {
                    temp_vec[k] = evec[k][j];
                }
                for (int k = 0; k < n; k++) {
                    evec[k][j] = evec[k][j + 1];
                }
                for (int k = 0; k < n; k++) {
                    evec[k][j + 1] = temp_vec[k];
                }
            }
        }
    }
    // Build matrix V, equal to the eigenvectors of A^T * A
    ret[2] = (double **)malloc(n * sizeof(double *));
    for (int i = 0; i < n; i++) {
        ret[2][i] = (double *)malloc(n * sizeof(double));
        for (int j = 0; j < n; j++) {
            ret[2][i][j] = evec[i][j];
        }
    }

    // Build matrix S
    ret[1] = (double **)malloc(m * sizeof(double *));
    for (int i = 0; i < m; i++) {
        ret[1][i] = (double *)malloc(n * sizeof(double));
        for (int j = 0; j < n; j++) {
            if (i == j) {
                ret[1][i][j] = (ev[i] > 0) ?
                                sqrt(ev[i]) : 0.0;
            } else {
                ret[1][i][j] = 0.0;
            }
        }
    }

    // Build matrix U
    int r = (m < n) ? m : n;
    ret[0] = (double **)malloc(m * sizeof(double *));
    for (int i = 0; i < m; i++) {
        ret[0][i] = (double *)malloc(m * sizeof(double));
    }
    // Compute first r left singular vectors: u_i = (1/sigma_i) * A * v_i
    double eps = 1e-12;
    for (int i = 0; i < r; i++) {
        double sigma = ret[1][i][i];
        if (sigma < eps) {
            for (int row = 0; row < m; row++) ret[0][row][i] = 0.0;
            continue;
        }
        for (int row = 0; row < m; row++) {
            double s = 0.0;
            for (int k = 0; k < n; k++) {
                s += A[row][k] * ret[2][k][i]; // ret[2] stores V (n x n), column i is v_i
            }
            ret[0][row][i] = s / sigma;
        }
    }

    // Orthonormalize the first r columns (modified Gram-Schmidt)
    for (int i = 0; i < r; i++) {
        // subtract projections onto previous columns
        for (int j = 0; j < i; j++) {
            double dot = 0.0;
            for (int row = 0; row < m; row++) dot += ret[0][row][j] * ret[0][row][i];
            for (int row = 0; row < m; row++) ret[0][row][i] -= dot * ret[0][row][j];
        }
        // normalize
        double norm = 0.0;
        for (int row = 0; row < m; row++) norm += ret[0][row][i] * ret[0][row][i];
        norm = sqrt(norm);
        if (norm < eps) {
            // fallback: make a canonical unit vector at position i (if possible)
            for (int row = 0; row < m; row++) ret[0][row][i] = 0.0;
            if (i < m) ret[0][i][i] = 1.0;
        } else {
            for (int row = 0; row < m; row++) ret[0][row][i] /= norm;
        }
    }

    // Complete U to an orthonormal m x m matrix using Gram-Schmidt on standard basis seeds
    for (int col = r; col < m; col++) {
        // initialize with standard basis vector e_col
        double *vec = (double *)malloc(m * sizeof(double));
        for (int row = 0; row < m; row++) vec[row] = 0.0;
        vec[col] = 1.0;

        // orthogonalize against all previously computed columns
        for (int j = 0; j < col; j++) {
            double dot = 0.0;
            for (int row = 0; row < m; row++) dot += ret[0][row][j] * vec[row];
            for (int row = 0; row < m; row++) vec[row] -= dot * ret[0][row][j];
        }

        // if vec is (nearly) zero, try other seeds
        double norm = 0.0;
        for (int row = 0; row < m; row++) norm += vec[row] * vec[row];
        norm = sqrt(norm);
        if (norm < eps) {
            for (int seed = 0; seed < m && norm < eps; seed++) {
                for (int row = 0; row < m; row++) vec[row] = 0.0;
                vec[seed] = 1.0;
                for (int j = 0; j < col; j++) {
                    double dot = 0.0;
                    for (int row = 0; row < m; row++) dot += ret[0][row][j] * vec[row];
                    for (int row = 0; row < m; row++) vec[row] -= dot * ret[0][row][j];
                }
                norm = 0.0;
                for (int row = 0; row < m; row++) norm += vec[row] * vec[row];
                norm = sqrt(norm);
            }
        }

        // final fallback if still degenerate
        if (norm < eps) {
            for (int row = 0; row < m; row++) vec[row] = 0.0;
            vec[0] = 1.0;
            // orthogonalize against previous columns once more
            for (int j = 0; j < col; j++) {
                double dot = 0.0;
                for (int row = 0; row < m; row++) dot += ret[0][row][j] * vec[row];
                for (int row = 0; row < m; row++) vec[row] -= dot * ret[0][row][j];
            }
            norm = 0.0;
            for (int row = 0; row < m; row++) norm += vec[row] * vec[row];
            norm = sqrt(norm);
        }

        // normalize and store
        if (norm < eps) {
            // as an absolute last resort, set to unit vector at index col (if valid)
            for (int row = 0; row < m; row++) ret[0][row][col] = 0.0;
            if (col < m) ret[0][col][col] = 1.0;
        } else {
            for (int row = 0; row < m; row++) ret[0][row][col] = vec[row] / norm;
        }
        free(vec);
    }

    // DEBUG
    // printf("Eigenvalues computed\n");
    // for (int i = 0; i < n; i++) {
    //     printf("%lf ", ev[i]);
    // }
    // printf("Eigenvectors computed\n");
    // for (int i = 0; i < n; i++) {
    //     for (int j = 0; j < n; j++) {
    //         printf("%lf ", evec[i][j]);
    //     }
    //     printf("\n");
    // }

    return ret;
}

// // DEBUG
// int main(void) {
//     double **A = (double **)malloc(3 * sizeof(double *));
//     A[0] = (double []){1, 2, 3};
//     A[1] = (double []){4, 5, 6};
//     A[2] = (double []){7, 8, 9};
//     double *** svd_result = svd(3, 3, A);
//     // Print svd_result here
//     if (svd_result[0] && svd_result[1] && svd_result[2]) {
//         // DEBUG
//         // printf("SVD computed (placeholders)\n");
//         // printf("U:\n");
//         // for (int i = 0; i < 3; i++) {
//         //     for (int j = 0; j < 3; j++) {
//         //         printf("%f ", svd_result[0][i][j]);
//         //     }
//         //     printf("\n");
//         // }
//         // printf("S:\n");
//         // for (int i = 0; i < 3; i++) {
//         //     for (int j = 0; j < 3; j++) {
//         //         printf("%f ", svd_result[1][i][j]);
//         //     }
//         //     printf("\n");
//         // }
//         // printf("V:\n");
//         // for (int i = 0; i < 3; i++) {
//         //     for (int j = 0; j < 3; j++) {
//         //         printf("%f ", svd_result[2][i][j]);
//         //     }
//         //     printf("\n");
//         // }

//         // DEBUG
//         // test reconstruction A = U * S * V^T
//         // double **Vt = transpose(3, 3, svd_result[2]);
//         // double **US = multiply(3, 3, svd_result[0], 3, 3, svd_result[1]);
//         // double **A_reconstructed = multiply(3, 3, US, 3, 3, Vt);
//         // printf("Reconstructed A:\n");
//         // for (int i = 0; i < 3; i++) {
//         //     for (int j = 0; j < 3; j++) {
//         //         printf("%f ", A_reconstructed[i][j]);
//         //     }
//         //     printf("\n");
//         // }
//     } else {
//         printf("SVD computation failed\n");
//     }
//     return 0;
// }