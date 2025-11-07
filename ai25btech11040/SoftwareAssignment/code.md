# Reading PNGs
Reading PNG files byte by byte requires the understanding of the following sections of the file

### PNG File Signature
The first 8 bytes of a PNG file are always the same and serve as a signature to identify the file as a PNG image. The signature bytes are:
```
137 80 78 71 13 10 26 10
```

### PNG Chunks
After our first 8 bytes, PNG files are organized into chunks, each consisting of four parts:

1. Length (4 bytes): Indicates the length of the chunk's data field.
2. Chunk Type (4 bytes): A 4-character code that identifies the type of chunk.
3. Chunk Data (variable length): The actual data of the chunk.
4. CRC (4 bytes): A cyclic redundancy check value for error-checking the chunk type and data. Due to time constraints I have not implemented CRC checking in my code yet.

### Important Chunk Types

- IHDR: The first chunk in a PNG file, containing image width, height, bit depth, color type, compression method, filter method, and interlace method.
- IDAT: Contains the actual image data, which is compressed.
- IEND: Marks the end of the PNG file.
- pHYS: Contains physical pixel dimensions.
- tEXt: Contains textual information.

### IHDR Chunk Structure
The IHDR chunk is 13 bytes long and contains the following fields:

- Width (4 bytes): The width of the image in pixels.
- Height (4 bytes): The height of the image in pixels.
- Bit Depth (1 byte): The number of bits per sample or per palette index.
- Color Type (1 byte): Indicates the color type of the image (e.g., grayscale, truecolor, indexed-color).
- Compression Method (1 byte): The method used to compress the image data (always 0 for PNG).
- Filter Method (1 byte): The method used to filter the image data (always 0 for PNG).
- Interlace Method (1 byte): Indicates whether the image is interlaced (0 for no interlace, 1 for Adam7 interlace).

Due to time constraints, I have implemented only the reading of PNG files with the following specifications:

- Color Type: 0 (Grayscale with no alpha channel)
- Bit Depth: 8 bits per sample (or 16 bits per sample)
- No interlacing (Interlace Method: 0)

### IDAT Chunk and Image Data
The IDAT chunk contains the compressed image data. The data is compressed using the `DEFLATE` algorithm. We use the `zlib` library to decompress this data due to time-constraints.
The decompressed image data is organized into scanlines, each preceded by a filter type byte, ie we obtain a flattened 2D array of bytes representing the image. Each row contains a filter type byte followed by the pixel data for that row.

#### Filter Types
PNG uses five filter types to improve compression:

- Type 0: None: No filtering is applied, ie each byte represents the actual pixel value.
- Type 1: Sub: Each byte is replaced by the difference between it and the corresponding byte of the prior pixel in the same row. `Raw(x) = (Filtered(x) + Raw(x-1)) % 256`
- Type 2: Up: Each byte is replaced by the sum of it and the corresponding byte of the prior pixel in the same column. `Raw(x) = (Filtered(x) + Raw(x-Width)) % 256`
- Type 3: Average: Each byte is replaced by the average of the corresponding bytes of the prior pixel in the same row and the prior pixel in the same column. `Raw(x) = (Filtered(x) + (Raw(x-1) + Raw(x-Width)) / 2) % 256`
- Type 4: Paeth: Each byte is replaced by the result of the Paeth predictor, which chooses the pixel value that is closest to the sum of the values of the three neighboring pixels. `Raw(x) = (Filtered(x) + PaethPredictor(Raw(x-1), Raw(x-Width), Raw(x-Width-1))) % 256`

The Paeth predictor (coded in python here) is calculated as follows:
```python
def PaethPredictor(a, b, c):
    p = a + b - c
    pa = abs(p - a)
    pb = abs(p - b)
    pc = abs(p - c)
    if pa <= pb and pa <= pc:
        return a
    elif pb <= pc:
        return b
    else:
        return c
```

### IEND Chunk
The IEND chunk marks the end of the PNG file. It has no data and is always the last chunk in the file.

### Other Chunks
Other chunks like pHYS and tEXt can be present in PNG files, but they are not essential for reading the image data and can be ignored for basic PNG reading functionality.

# Performing SVD for the obtained matrix
To decompose the image matrix using Singular Value Decomposition (SVD), I have used the following algorithm:

1. Compute the covariance matrix $C = A^TA$, where $A$ is the image matrix.
2. Compute the eigenvalues and eigenvectors of the covariance matrix $C$ using the Jacobi method.
3. Sort the eigenvalues in descending order and arrange the corresponding eigenvectors accordingly.
4. The right singular vectors ($V$) are the eigenvectors of $C$.
5. The singular values ($\Sigma$) are the square roots of the eigenvalues of $C$.
6. Compute the left singular vectors ($U$) using the Gram-Schmidt process on the set of vectors $\{A v_i / \sigma_i\}$, where $v_i$ are the right singular vectors and $\sigma_i$ are the singular values.

## Jacobi Method for Eigenvalue Decomposition
The Jacobi method is an iterative algorithm used to compute the eigenvalues and eigenvectors of a symmetric matrix (this works because $A^\mathrm{T}A$ is symmetric). The algorithm works by performing a series of rotations to zero out the off-diagonal elements of the matrix. The steps are as follows:

1. Initialize the eigenvector matrix as the identity matrix.
2. Repeat until convergence:
   1. Find the largest off-diagonal element in the matrix.
   2. Compute the rotation angle to zero out this element.
   3. Apply the rotation to the matrix and update the eigenvector matrix.
3. The diagonal elements of the resulting matrix are the eigenvalues, and the columns of the eigenvector matrix are the corresponding eigenvectors.

Here we define the algorithm to converge when the maximum off-diagonal element is less than a small threshold value (e.g., $1 \times 10^{-12}$).

## Gram-Schmidt Process
The Gram-Schmidt process is used to orthogonalize a set of vectors. To compute the left singular vectors ($U$), we apply the Gram-Schmidt process to the set of vectors $\{A v_i / \sigma_i\}$:

1. Start with the set of vectors $\{A v_1 / \sigma_1, A v_2 / \sigma_2, \ldots, A v_n / \sigma_n\}$.
2. For each vector in the set:
   1. Subtract the projections of the vector onto all previously computed orthogonal vectors.
   2. Normalize the resulting vector to obtain an orthonormal vector.
3. The resulting set of orthonormal vectors forms the columns of the matrix $U$.

## K Low-Rank Approximation
To obtain a rank-$k$ approximation of the original image matrix $A$, we retain only the top $k$ singular values and their corresponding singular vectors:

1. Construct the diagonal matrix $\Sigma_k$ by keeping only the top $k$ singular values and setting the rest to zero.
2. Construct the matrices $U_k$ and $V_k$ by retaining only the first $k$ columns of $U$ and $V$, respectively.
3. The rank-$k$ approximation of the original matrix $A$ is given by:
$$A_k = U_k \Sigma_k V_k^T$$

# Saving the compressed image as PNG
To save the compressed image as a PNG file, we need to reverse the steps taken during the reading process:

1. **Reconstruct the Image Data**: Using the rank-$k$ approximation, reconstruct the image matrix.
2. **Apply PNG Filters**: Due to time constraints, I have implemented only the "None" filter (Type 0) for simplicity. We can obtain higher compression ratios by implementing other filter types in the future.
3. **Compress the Image Data**: Use the `zlib` library to compress the filtered image data using the DEFLATE algorithm.
4. **Create PNG Chunks**: Construct the necessary PNG chunks (IHDR, IDAT, IEND) with the appropriate data.
5. **Write to File**: Write the PNG signature followed by the constructed chunks to a new PNG file.

# References
- [PNG Specification (Second Edition)](https://www.w3.org/TR/PNG/)
- [Jacobi Method](https://en.wikipedia.org/wiki/Jacobi_method)
- [Gram-Schmidt Process](https://en.wikipedia.org/wiki/Gram%E2%80%93Schmidt_process)