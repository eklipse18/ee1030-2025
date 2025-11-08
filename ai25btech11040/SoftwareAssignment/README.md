# Requirements
Installing zLib headers and libraries, and so is libpng is required to compile this project.
On Ubuntu, you can install it using the following command:
```bash
sudo apt-get install zlib1g-dev libpng-dev
```

# Compilation
To compile the code, use the following command:
```bash
clang main.c lib/*/*.c -lm -lz -lpng -O3
```
If you prefer using the clang compiler. This will link all the necessary libraries, and produce an executable named `a.out`.

# Usage
Use the following command to run the program:
```bash
./a.out <input_image.png> <k>
```
Where `<input_image.png>` is the path to the input PNG image file, and `<k>` is the number of singular values to retain during compression.

# Output
The program will generate a compressed image file named `out.png` in the current directory.

### Mathematical workings and explanations can be found in the [`math.md`](./math.md) file included in this repository.
### To understand the code structure and implementation details, refer to [`code.md`](./code.md)


# References
- [Low-rank Approximation](https://en.wikipedia.org/wiki/Low-rank_approximation)
- [Singular Value Decomposition](https://en.wikipedia.org/wiki/Singular_value_decomposition)