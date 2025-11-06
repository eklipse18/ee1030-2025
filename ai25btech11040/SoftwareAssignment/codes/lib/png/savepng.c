#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

void savepng(const char *filename, double **image, int ihdr[7]) {
  FILE *file = fopen(filename, "wb");
  if (!file) {
    fprintf(stderr, "Error: Could not open file %s for writing\n", filename);
    return;
  }
  int width = ihdr[0];
  int height = ihdr[1];
  int bit_depth = ihdr[2];
  int color_type = ihdr[3];
  int compression_method = ihdr[4];
  int filter_method = ihdr[5];
  int interlace_method = ihdr[6];
  // Prepare PNG signature
  unsigned char png_signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
  fwrite(png_signature, 1, 8, file);

  // Prepare IHDR chunk
  unsigned char ihdr_data[13];
  ihdr_data[0] = (width >> 24) & 0xFF;
  ihdr_data[1] = (width >> 16) & 0xFF;
  ihdr_data[2] = (width >> 8) & 0xFF;
  ihdr_data[3] = width & 0xFF;
  ihdr_data[4] = (height >> 24) & 0xFF;
  ihdr_data[5] = (height >> 16) & 0xFF;
  ihdr_data[6] = (height >> 8) & 0xFF;
  ihdr_data[7] = height & 0xFF;
  ihdr_data[8] = bit_depth;
  ihdr_data[9] = color_type;
  ihdr_data[10] = compression_method;
  ihdr_data[11] = filter_method;
  ihdr_data[12] = interlace_method;

  // Write IHDR chunk
  fwrite(ihdr_data, 1, 13, file);

  // Prepare IDAT chunk
  unsigned char *idat_data = (unsigned char *)malloc((width * height) + height);
  for (int y = 0; y < height; y++) {
    idat_data[y * (width + 1)] = 0; // No filter
    for (int x = 0; x < width; x++) {
      idat_data[y * (width + 1) + x + 1] = (unsigned char)(image[y][x]);
    }
  }
  // Compress IDAT data
  uLongf compressed_size = compressBound((width * height) + height);
  unsigned char *compressed_data = (unsigned char *)malloc(compressed_size);
  compress(compressed_data, &compressed_size, idat_data,
           (width * height) + height);
  // Write IDAT chunk
  fwrite(compressed_data, 1, compressed_size, file);
  free(compressed_data);
  free(idat_data);
}