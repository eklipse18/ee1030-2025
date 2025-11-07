# Basic Concepts and Terminologies

Let 
$$A_{m \times n} = \begin{bmatrix} a_{11} & a_{12} & \cdots & a_{1n} \\ a_{21} & a_{22} & \cdots & a_{2n} \\ \vdots & \vdots & \ddots & \vdots \\ a_{m1} & a_{m2} & \cdots & a_{mn} \end{bmatrix} = \begin{bmatrix} a_{ij} \end{bmatrix}
$$ Represent a matrix of size m x n.

The transpose of matrix A is represented as
$$A^T_{n \times m} = \begin{bmatrix} a_{ji} \end{bmatrix}$$

# Singular Value Decomposition (SVD)
Any real matrix A of size m x n can be decomposed into three matrices U, S, and V such that
$$A = U S V^T$$
Where,

- U is an m x m orthogonal matrix whose columns are the left singular vectors of A
- S is an m x n diagonal matrix with non-negative real numbers on the diagonal, known as singular values of A
- V is an n x n orthogonal matrix whose columns are the right singular vectors of A

# K Low Rank Approximation
To approximate a matrix A using its top k singular values, we can truncate the matrices U, S, and V to retain only the first k columns of U, the first k rows and columns of S, and the first k columns of V. The k-rank approximation of A is given by
$$A_{k} = U_{m \times k} S_{k \times k} V^T_{k \times n}$$

# Image Representation
A grayscale image can be represented as a 2D matrix where each element corresponds to the intensity of a pixel, 0 representing black and 255 representing white. For a color image, it can be represented as three 2D matrices (one for each color channel: Red, Green, and Blue).

# Image Compression using SVD
To compress an image using SVD, we can follow these steps:

1. Represent the image as a matrix A.
2. Perform SVD on matrix A to obtain U, S, and V.
3. Retain only the top k singular values in matrix S and set the rest to zero. This reduces the rank of the matrix and hence compresses the image.
4. Reconstruct the compressed image using the modified matrices U, S, and V: $$A_{compressed} = U S_{k} V^T$$
Where $S_{k}$ is the diagonal matrix with only the top k singular values retained.
5. The resulting matrix $A_{compressed}$ represents the compressed image.

# Mathematical Justification
The effectiveness of SVD in image compression lies in the fact that many images have a lot of redundancy, and the singular values often decay rapidly. By retaining only the top k singular values, we can capture most of the important features of the image while significantly reducing the amount of data needed to represent it. This leads to a compressed image that is visually similar (albeit lesser in quality and detail) to the original, but requires less storage space.