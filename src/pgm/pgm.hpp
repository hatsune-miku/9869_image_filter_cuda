//
// pgm.hpp
//
// Part of ENGI-9869 Assignment #5
//
// Author: Zhen Guan
// Email: zguan@mun.ca
// Student Number: 202191382
//

#pragma once

#include <cstdio>
#include <cstdlib>

namespace pgm {
// PGM file header and body.
typedef struct {
    int width;
    int height;
    int greyscale_max;
} pgm_header_t;


// Read pgm header and skip the comment.
// The file pointer must be at the beginning of the file (ftell=0).
pgm_header_t pgm_header_read(FILE *f);

// Write the pgm (version: P2) header at the position that f is pointing to.
void pgm_header_write(FILE *f, pgm_header_t header);

// Read pgm image data.
// The file pointer must be at the end of the header, aka
// the beginning of the image data.
void pgm_image_read(FILE *f, pgm_header_t header, unsigned char *out_image_2d);

// Write the pgm image data at the position that f is pointing to.
void pgm_image_write(FILE *f, pgm_header_t header, const unsigned char *image_2d);

bool is_pgm_file_equal(const char *filename1, const char *filename2);
}
