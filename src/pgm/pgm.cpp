#include "pgm/pgm.hpp"

pgm_header_t pgm_header_read(FILE *f) {
    char line[256];
    pgm_header_t ret;

    fgets(line, sizeof(line), f);
    if (line[0] != 'P') {
        printf("Error: Not a PGM file.\n");
        exit(1);
    }

    // Comments are consequent lines starting with '#', I suppose.
    // Also assumed that comments won't appear in anywhere else.
    fgets(line, sizeof(line), f);
    while (line[0] == '#') {
        fgets(line, sizeof(line), f);
    }
    sscanf_s(line, "%d %d %d", &ret.width, &ret.height, &ret.greyscale_max);
    return ret;
}

void pgm_header_write(FILE *f, pgm_header_t header) {
    fprintf(f, "P2\n");
    fprintf(f, "%d %d\n", header.width, header.height);
    fprintf(f, "%d\n", header.greyscale_max);
}

void pgm_image_read(FILE *f, pgm_header_t header, const unsigned char *out_image_2d) {
    int requiredSize = header.height * header.width;
    int tmp;
    while (requiredSize--) {
        fscanf_s(f, "%d", &tmp);
        *(out_image_2d++) = (unsigned char) tmp;
    }
}

void pgm_image_write(FILE *f, pgm_header_t header, const unsigned char *image_2d) {
    for (int i = 0; i < header.height; ++i) {
        for (int j = 0; j < header.width; ++j) {
            fprintf(f, "%d ", (int) ARRAY2D_AT(image_2d, header.width, i, j));
        }
        fprintf(f, "\n");
    }
}
