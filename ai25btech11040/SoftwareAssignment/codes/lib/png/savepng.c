#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

static int write_be32(FILE *f, int v) {
    unsigned char b[4];
    b[0] = (v >> 24) & 0xFF;
    b[1] = (v >> 16) & 0xFF;
    b[2] = (v >> 8) & 0xFF;
    b[3] = v & 0xFF;
    return fwrite(b, 1, 4, f) == 4 ? 0 : -1;
}

static int write_chunk(FILE *f, const char type[4], const unsigned char *data, int len) {
    /* length (4 BE), type (4), data (len), CRC (4 BE) where CRC is over type+data */
    if (write_be32(f, len) != 0) return -1;
    if (fwrite(type, 1, 4, f) != 4) return -1;
    if (len && fwrite(data, 1, len, f) != len) return -1;
    /* compute CRC using zlib's crc32 */
    uLong crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, (const Bytef *)type, 4);
    if (len) crc = crc32(crc, (const Bytef *)data, len);
    if (write_be32(f, (int)crc) != 0) return -1;
    return 0;
}

void savepng(const char *filename, double **image, int ihdr[7]) {
    if (!filename || !image || !ihdr) return;

    int width = ihdr[0];
    int height = ihdr[1];
    int bit_depth = ihdr[2];
    int color_type = ihdr[3];

    /* Only support 8-bit grayscale */
    if (width <= 0 || height <= 0) return;
    if (bit_depth != 8 || color_type != 0) {
        fprintf(stderr, "savepng: only 8-bit grayscale supported (bit_depth=%d color_type=%d)\n",
                bit_depth, color_type);
        return;
    }

    FILE *f = fopen(filename, "wb");
    if (!f) { perror("savepng fopen"); return; }

    /* PNG signature */
    const unsigned char png_sig[8] = {137,80,78,71,13,10,26,10};
    if (fwrite(png_sig, 1, 8, f) != 8) { fclose(f); return; }

    /* IHDR chunk (13 bytes) */
    unsigned char ihdr_buf[13];
    ihdr_buf[0] = (width >> 24) & 0xFF;
    ihdr_buf[1] = (width >> 16) & 0xFF;
    ihdr_buf[2] = (width >> 8) & 0xFF;
    ihdr_buf[3] = width & 0xFF;
    ihdr_buf[4] = (height >> 24) & 0xFF;
    ihdr_buf[5] = (height >> 16) & 0xFF;
    ihdr_buf[6] = (height >> 8) & 0xFF;
    ihdr_buf[7] = height & 0xFF;
    ihdr_buf[8] = (unsigned char)bit_depth;        /* bit depth */
    ihdr_buf[9] = (unsigned char)color_type;       /* color type */
    ihdr_buf[10] = 0; /* compression */
    ihdr_buf[11] = 0; /* filter */
    ihdr_buf[12] = 0; /* interlace */

    if (write_chunk(f, "IHDR", ihdr_buf, sizeof(ihdr_buf)) != 0) {
        fprintf(stderr, "savepng: failed writing IHDR\n");
        fclose(f);
        return;
    }

    /* Build raw image data: each scanline = filter byte (0) + width bytes (grayscale) */
    size_t row_bytes = (size_t)width;
    size_t raw_len = (row_bytes + 1) * (size_t)height;
    unsigned char *raw = (unsigned char *)malloc(raw_len);
    if (!raw) { fclose(f); return; }

    for (int y = 0; y < height; ++y) {
        unsigned char *row = raw + (size_t)y * (row_bytes + 1);
        row[0] = 0; /* no filter (0) */
        for (int x = 0; x < width; ++x) {
            double v = image[y][x];
            if (v < 0.0) v = 0.0;
            if (v > 255.0) v = 255.0;
            row[1 + x] = (unsigned char)(v + 0.5);
        }
    }

    /* Compress raw with zlib (produces zlib stream expected by PNG IDAT) */
    uLongf cmp_bound = compressBound((uLong)raw_len);
    unsigned char *cmp = (unsigned char *)malloc(cmp_bound);
    if (!cmp) { free(raw); fclose(f); return; }
    int zret = compress2(cmp, &cmp_bound, raw, (uLong)raw_len, Z_BEST_COMPRESSION);
    if (zret != Z_OK) {
        fprintf(stderr, "savepng: compress2 failed (%d)\n", zret);
        free(cmp); free(raw); fclose(f);
        return;
    }

    if (write_chunk(f, "IDAT", cmp, (int)cmp_bound) != 0) {
        fprintf(stderr, "savepng: failed writing IDAT\n");
        free(cmp); free(raw); fclose(f);
        return;
    }

    /* IEND */
    if (write_chunk(f, "IEND", NULL, 0) != 0) {
        fprintf(stderr, "savepng: failed writing IEND\n");
    }

    free(cmp);
    free(raw);
    fclose(f);
}