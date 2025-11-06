#ifndef READPNG_H
#define READPNG_H

int paeth(int a, int b, int c);

int **readpng(const char *filename, int ihdr_[7]);

#endif