// 
// main.c
// 
// Part of ENGI-9869 Assignment #5
//
// Author: Zhen Guan
// Email: zguan@mun.ca
// Student Number: 202191382
//

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "pgm/pgm.hpp"

// Kernel size.
#define KERNEL_SIZE (9)

// Some annotations to improve readability of the code.
#define IN
#define OUT

// Access a 2D array that represented by a 1D array.
#define ARRAY2D_AT(array, row_size, row_index, col_index)\
    ((array)[(row_index) * (row_size) + (col_index)])


// The convolution kernel.
int g_convolution_kernel_size = KERNEL_SIZE;
int g_convolution_kernel[][KERNEL_SIZE] = {
    {0, 0, 3,   2,   2,   2,  3, 0, 0},
    {0, 2, 3,   5,   5,   5,  3, 2, 0},
    {3, 3, 5,   3,   0,   3,  5, 3, 3},
    {2, 5, 3, -12, -23, -12,  3, 5, 2},
    {2, 5, 0, -23, -40, -23,  0, 5, 2},
    {2, 5, 3, -12, -23, -12,  3, 5, 2},
    {3, 3, 5,   3,   0,   3,  5, 3, 3},
    {0, 2, 3,   5,   5,   5,  3, 2, 0},
    {0, 0, 3,   2,   2,   2,  3, 0, 0},    
};

// Shared memory.
static pgm_header_t *g_shared_header;
static unsigned char *g_shared_image_data_2d;
static unsigned char *g_shared_output_image_data_2d;

// Util functions.

// Convolute a square block of the image with the given kernel.
// Points outside the block but still inside the image, will be set correctly;
// Points outside the image will be assumed to 0.
static void convolution(
    int kernel[][KERNEL_SIZE],
    int kernel_size,
    unsigned char* input_image_2d,
    unsigned char* output_image_2d,
    int image_width,
    int image_height,
    int row_index_start,
    int row_index_end,
    int col_index_start,
    int col_index_end
) {
    for (int i = row_index_start; i < row_index_end; ++i) {
        for (int j = col_index_start; j < col_index_end; ++j) {
            // Convolution at input_image_2d[i][j];
            int sum = 0;
            for (int x = 0; x < kernel_size; ++x) {
                for (int y = 0; y < kernel_size; ++y) {
                    int row_index = i + x - kernel_size / 2;
                    int col_index = j + y - kernel_size / 2;
                    if (row_index < 0 || row_index >= image_height 
                    || col_index < 0 || col_index >= image_width) {
                        // greyscale at outside of the image (e.g. minus index) is zero.
                        sum += 0;
                    }
                    else {
                        sum += kernel[x][y] * (int)ARRAY2D_AT(input_image_2d, image_width, row_index, col_index);
                    }
                }
            }
            if (sum < 0) {
                sum = 0;
            }
            else if (sum > g_shared_header->greyscale_max) {
                sum = g_shared_header->greyscale_max;
            }
            ARRAY2D_AT(output_image_2d, image_width, i, j) = (unsigned char)sum;
        }
    }

}

// Check if an integer has a integer square root or not.
// If it has, return the integer square root; 
// Otherwise, return 0.
static int sqrt_integer_only(int n) {
    if (n == 1) {
        return 1;
    }

    int n_half = n / 2;
    for (int i = 1; i <= n_half; ++i) {
        if (i * i == n) {
            return i;
        }
    }
    return 0;
}

// Check if two PGM files are the same.
static int is_pgm_file_equal(
    pgm_header_t header1, const unsigned char* data1,
    pgm_header_t header2, const unsigned char* data2
) {
    if (header1.greyscale_max != header2.greyscale_max
    || header1.width != header2.width
    || header1.height != header2.height) {
        return 0;
    }

    for (int i = 0; i < header1.height; ++i) {
        for (int j = 0; j < header1.width; ++j) {
            if (ARRAY2D_AT(data1, header1.width, i, j) != ARRAY2D_AT(data2, header2.width, i, j)) {
                return 0;
            }
        }
    }
    return 1;
}

// The main function.
int main(int argc, char **argv) {
    // Print the usage if the number of arguments is not correct.
    if (argc != 3 && argc != 4) {
        printf(
            "Usage:\n"
            "   PGM Image Processing:\n"
            "         %s <input_file> <output_file>\n"
            "\n"
            "   PGM Image Comparison:\n"
            "         %s compare <input1> <input2>\n",
            argv[0], argv[0]
        );
        return 1;
    }

    // Compare mode?
    if (argc == 4) {
        if (strcmp(argv[1], "compare") != 0) {
            printf("Error: Invalid mode.\n");
            exit(1);
        }
        // Not generating output pgm, just compare the two inputs.
        char* input_file1 = argv[2];
        char* input_file2 = argv[3];

        FILE* f1 = nullptr;
        FILE* f2 = nullptr;

        fopen_s(&f1, input_file1, "r");
        fopen_s(&f2, input_file2, "r");

        if (f1 == nullptr || f2 == nullptr) {
            printf("Error: Cannot open file.\n");
            exit(1);
        }

        pgm_header_t header1 = pgm_header_read(f1);
        pgm_header_t header2 = pgm_header_read(f2);

        const unsigned char* data1 = new unsigned char[header1.width * header1.height];
        const unsigned char* data2 = new unsigned char[header2.width * header2.height];

        if (!data1 || !data2) {
            printf("Error: Cannot allocate memory.\n");
            exit(1);
        }

        pgm_image_read(f1, header1, data1);
        pgm_image_read(f2, header2, data2);

        if (is_pgm_file_equal(header1, data1, header2, data2)) {
            printf("Yay, The two images are the same! :)\n");
        }
        else {
            printf("The two images are NOT the same :(\n");
        }

        fclose(f1);
        fclose(f2);

        delete[] data1;
        delete[] data2;

        exit(0);
    }
    return 0;
}
