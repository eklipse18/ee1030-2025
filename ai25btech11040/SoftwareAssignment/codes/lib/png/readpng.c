#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

/* Table of CRCs of all 8-bit messages. */
unsigned long crc_table[256];

/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;

/* Make the table for a fast CRC. */
void make_crc_table(void) {
  unsigned long c;
  int n, k;

  for (n = 0; n < 256; n++) {
    c = (unsigned long)n;
    for (k = 0; k < 8; k++) {
      if (c & 1)
        c = 0xedb88320L ^ (c >> 1);
      else
        c = c >> 1;
    }
    crc_table[n] = c;
  }
  crc_table_computed = 1;
}

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
   should be initialized to all 1's, and the transmitted value
   is the 1's complement of the final running CRC (see the
   crc() routine below). */

unsigned long update_crc(unsigned long crc, unsigned char *buf, int len) {
  unsigned long c = crc;
  int n;

  if (!crc_table_computed)
    make_crc_table();
  for (n = 0; n < len; n++) {
    c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
  }
  return c;
}

/* Return the CRC of the bytes buf[0..len-1]. */
unsigned long crc(unsigned char *buf, int len) {
  return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
}

int **readpng(const char *filename, int ihdr_[7]) {
  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    printf("Error opening file");
    return NULL;
  }
  long filelen;
  fseek(file, 0, SEEK_END);
  filelen = ftell(file);
  rewind(file);
  // printf("File length: %ld bytes\n", filelen); // DEBUG

  // Every PNG file starts with the following 8-byte signature
  unsigned char signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
  int ihdr[7] = {}; // Represents width, height, bit depth, color type,
                    // compression method, filter method, interlace method
  int x = 0, y = 0;

  unsigned long long i = 0;
  int **array = NULL;
  Bytef *decompressed = NULL;
  int **unfiltered = NULL;

  while (i < filelen && !feof(file)) {
    // Make sure its a PNG image
    if (i < 8) {
      unsigned char k;
      fread(&k, 1, 1, file);
      if (k != signature[i])
        goto malformed;
      i++;
      continue;
    }

    // start processing chunks
    // The first four bytes represent the length of chunk data in bytes
    // The bytes are stored in big-endian, and we shall read it as such
    // We can't directly read it as an unsigned int because modern systems store
    // integers in little-endian
    unsigned int chunkLength = 0;
    for (int j = 0; j < 4; j++) {
      unsigned char byte;
      fread(&byte, 1, 1, file);
      chunkLength = (chunkLength << 8) | byte;
    }
    // The next four bytes represent the chunk type, restricted to an ASCII
    // string
    unsigned char chunkType[5] = {0};
    fread(chunkType, 1, 4, file);
    unsigned char *chunkData = malloc(chunkLength);
    fread(chunkData, 1, chunkLength, file);
    unsigned char crc[4];
    fread(crc, 1, 4, file);
    printf("%s %u\n", chunkType, chunkLength); // DEBUG
    // TODO: Implement CRC check

    /* PROCESS CHUNK DATA */

    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    int ret = inflateInit(&strm);
    if (ret != Z_OK) goto malformed;

    if (strcmp((const char *)chunkType, "IHDR") == 0) {
      if (chunkLength != 13)
        goto malformed; // IHDR chunk must be 13 bytes long

      // First 4 bytes: width
      ihdr[0] = 0;
      for (int j = 0; j < 4; j++) {
        ihdr[0] = (ihdr[0] << 8) | chunkData[j];
      }
      // Next 4 bytes: height
      ihdr[1] = 0;
      for (int j = 4; j < 8; j++) {
        ihdr[1] = (ihdr[1] << 8) | chunkData[j];
      }

      ihdr[2] = chunkData[8];  // bit depth
      ihdr[3] = chunkData[9];  // color type
      ihdr[4] = chunkData[10]; // compression method
      ihdr[5] = chunkData[11]; // filter method
      ihdr[6] = chunkData[12]; // interlace method
      ihdr_ = ihdr;

      printf(
          "Width: %d, Height: %d, Bit depth: %d, Color type: %d, Compression "
          "method: %d, Filter method: %d, Interlace method: %d\n",
          ihdr[0], ihdr[1], ihdr[2], ihdr[3], ihdr[4], ihdr[5],
          ihdr[6]); // DEBUG
      if (ihdr[3] != 0 || ihdr[6] != 0)
        goto notimplemented; // TODO:
      // Implement for all types of PNGs

      if (ihdr[4] != 0 || ihdr[5] != 0)
        goto malformed;
    } else if (strcmp((const char *)chunkType, "IEND") == 0) {
      // Image end chunk
      // TODO
      break;
    } else if (strcmp((const char *)chunkType, "IDAT") == 0) {
      // Image data chunk
      if (chunkLength == 0)
        goto malformed;
      if (ihdr[3] == 0) {
        // Grayscale image, no alpha
        if (decompressed == NULL) {
          decompressed =
              malloc(ihdr[1] * (ihdr[0] + 1) * ihdr[2]/8 * sizeof(Bytef));
        } // TODO: Implement ZLIB decompression
        strm.next_out = decompressed;
        strm.avail_out = ihdr[1] * (ihdr[0] + 1) * sizeof(Bytef);
        strm.next_in = chunkData;
        strm.avail_in = chunkLength;
        ret = inflate(&strm, Z_NO_FLUSH);
        switch (ret) {
        case Z_NEED_DICT:
        case Z_DATA_ERROR:
        case Z_MEM_ERROR: {
          inflateEnd(&strm);
          goto malformed; // Error during decompression
          break;
        }
        case Z_STREAM_END: inflateEnd(&strm);
        }
      }

      // TODO Read IDAT for all other types of PNGs
    } else if (strcmp((const char *)chunkType, "pHYs") == 0) {
      // Physical pixel dimensions chunk
      if (chunkLength != 9)
        goto malformed;

      unsigned int x_pixels_per_unit = 0;
      unsigned int y_pixels_per_unit = 0;
      unsigned char unitSpecifier = 0;

      for (int j = 0; j < 4; j++) {
        x_pixels_per_unit = (x_pixels_per_unit << 8) | chunkData[j];
      }

      for (int j = 4; j < 8; j++) {
        y_pixels_per_unit = (y_pixels_per_unit << 8) | chunkData[j];
      }

      unitSpecifier = chunkData[8];

      printf("Pixels per unit: %u x %u, Unit: %s\n", x_pixels_per_unit,
             y_pixels_per_unit,
             unitSpecifier == 1 ? "meter" : "unknown"); // DEBUG
    } else if (strcmp((const char *)chunkType, "tEXt") == 0) {
      // Textual data chunk
      int i = 0;
      while (chunkData[i] != 0 && i < chunkLength) {
        printf("%c", chunkData[i]);
        i++;
      }
      i++; // Skip null separator
      printf(": ");
      while (i < chunkLength) {
        printf("%c", chunkData[i]);
        i++;
      }
      printf("\n");
    } else if (islower(chunkType[0])) {
      // Ancillary chunk we don't care about, or want to implement later, eg
      // zTXt,
      // TODO
    }

    i += 4 + 4 + chunkLength + 4;
    free(chunkData);
  }
  // DEBUG: Printing decompressed
  int bd = ihdr[2]; // bit depth
  for (int i = 0; i < ihdr[1]; i++) {
    int ftype = decompressed[i * (ihdr[0] + 1)];
    printf("Scanline %d filter type: %d\n", i, ftype);
    for (int j = 1; j <= ihdr[0]; j++) {
      int k = 0;
      for (int z = 0; z < bd; z++) {
        k = (k << 8) | decompressed[i * (ihdr[0] + 1) + bd*j + z];
      }
      switch (ftype) {
      case 0:
        unfiltered[i][j - 1] = k;
        break;
      case 1:
        // Sub filter
        if (j != 1)
          unfiltered[i][j] = (k + decompressed[i * (ihdr[0] + 1) + j - 1]) % 256;
        break;
      case 2:
        // Up filter
        if (i == 0)
          k = k;
        else
          k = (k + decompressed[(i - 1) * (ihdr[0] + 1) + j]) % 256;
        break;
      case 3:
        // Average filter
        {
          int left = 0, up = 0;
          if (j != 1)
            left = decompressed[i * (ihdr[0] + 1) + j - 1];
          if (i != 0)
            up = decompressed[(i - 1) * (ihdr[0] + 1) + j];
          k = (k + ((left + up) / 2)) % 256;
        }
      }
    }
  }

  array = malloc(ihdr[1] * sizeof(int *));
  for (int j = 0; j < ihdr[1]; j++) {
    array[j] = calloc(ihdr[0], sizeof(int));
  }

  /* Section for various error labels */
  if (0) {
    free(array);
    fclose(file);
    if (0) {
    malformed:
      printf("Malformed input");
    }
    if (0) {
    notimplemented:
      printf("Feature not implemented");
    }
    return NULL;
  }

  fclose(file);
  return array;
}

/* DEBUG */
int main(int argc, const char *argv[]) {
  int ihdr[7];
  int **data = readpng(argv[1], ihdr);
  if (data == NULL) {
    return 1;
  }
  free(data);
  return 0;
}