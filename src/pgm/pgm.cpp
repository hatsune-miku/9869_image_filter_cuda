//
// pgm.cpp
//
// Part of ENGI-9869 Assignment #5
//
// Author: Zhen Guan
// Email: zguan@mun.ca
// Student Number: 202191382
//

#include <stdexcept>

#include "pgm/pgm.hpp"
#include "util/array.hpp"

pgm::pgm_header_t pgm::pgm_header_read(FILE *f) {
    char line[256];
    pgm::pgm_header_t ret;

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
    sscanf_s(line, "%d %d", &ret.width, &ret.height);
    fgets(line, sizeof(line), f);
    sscanf_s(line, "%d", &ret.greyscale_max);
    return ret;
}

void pgm::pgm_header_write(FILE *f, pgm::pgm_header_t header) {
    fprintf(f, "P2\n");
    fprintf(f, "%d %d\n", header.width, header.height);
    fprintf(f, "%d\n", header.greyscale_max);
}

void pgm::pgm_image_read(FILE *f, pgm::pgm_header_t header, unsigned char *out_image_2d) {
    int requiredSize = header.height * header.width;
    int tmp;
    while (requiredSize--) {
        fscanf_s(f, "%d", &tmp);
        *(out_image_2d++) = (unsigned char) tmp;
    }
}

void pgm::pgm_image_write(FILE *f, pgm::pgm_header_t header, const unsigned char *image_2d) {
    for (int i = 0; i < header.height; ++i) {
        for (int j = 0; j < header.width; ++j) {
            fprintf(f, "%d ", (int) ARRAY2D_AT(image_2d, header.width, i, j));
        }
        fprintf(f, "\n");
    }
}

// Check if two PGM files are the same.
bool pgm::is_pgm_file_equal(const char *filename1, const char *filename2) {
    pgm::pgm_header_t header1;
    pgm::pgm_header_t header2;

    FILE* f1 = nullptr;
    FILE* f2 = nullptr;

    fopen_s(&f1, filename1, "r");
    fopen_s(&f2, filename2, "r");

    if (f1 == nullptr || f2 == nullptr) {
        throw std::runtime_error("Error: Cannot open file.");
    }

    header1 = pgm::pgm_header_read(f1);
    header2 = pgm::pgm_header_read(f2);

    if (header1.greyscale_max != header2.greyscale_max
        || header1.width != header2.width
        || header1.height != header2.height) {
        return false;
    }

    unsigned char *data1 = new unsigned char[header1.width * header1.height];
    unsigned char *data2 = new unsigned char[header2.width * header2.height];
    bool ret = true;

    pgm::pgm_image_read(f1, header1, data1);
    pgm::pgm_image_read(f2, header2, data2);

    for (int i = 0; i < header1.height; ++i) {
        for (int j = 0; j < header1.width; ++j) {
            if (ARRAY2D_AT(data1, header1.width, i, j) != ARRAY2D_AT(data2, header2.width, i, j)) {
                ret = false;
                goto end;
            }
        }
    }

end:
    delete[] data1;
    delete[] data2;
    fclose(f1);
    return ret;
}
