// 
// main.cpp
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
#include <stdexcept>

#include "pgm/pgm.hpp"
#include "conv/conv.hpp"

// The convolution kernel.
static int g_convolution_kernel_size = conv::CONV_KERNEL_SIZE;
static int g_convolution_kernel_2d[conv::CONV_KERNEL_SIZE * conv::CONV_KERNEL_SIZE] = {
    0, 0, 3,   2,   2,   2,  3, 0, 0,
    0, 2, 3,   5,   5,   5,  3, 2, 0,
    3, 3, 5,   3,   0,   3,  5, 3, 3,
    2, 5, 3, -12, -23, -12,  3, 5, 2,
    2, 5, 0, -23, -40, -23,  0, 5, 2,
    2, 5, 3, -12, -23, -12,  3, 5, 2,
    3, 3, 5,   3,   0,   3,  5, 3, 3,
    0, 2, 3,   5,   5,   5,  3, 2, 0,
    0, 0, 3,   2,   2,   2,  3, 0, 0
};

static void handle_compare(const char **argv) {
    if (strcmp(argv[1], "compare") != 0) {
        printf("Error: Invalid mode.\n");
        exit(1);
    }
    // Not generating output pgm, just compare the two inputs.
    const char* input_file1 = argv[2];
    const char* input_file2 = argv[3];

    if (pgm::is_pgm_file_equal(input_file1, input_file2)) {
        printf("Yay, The two images are the same! :)\n");
    }
    else {
        printf("The two images are NOT the same :(\n");
    }
}

static void handle_convolution(const char **argv) {
    const char *input_file = argv[1];
    const char *output_file = argv[2];
    const int num_workers = static_cast<int>(strtol(argv[4], nullptr, 10));

    // Check if num_workers is valid.
    if (num_workers > 1024) {
        throw std::runtime_error("CUDA does not allow threads > 1024.");
    }

    FILE *input_file_fp = nullptr;
    FILE *output_file_fp = nullptr;

    fopen_s(&input_file_fp, input_file, "r");
    fopen_s(&output_file_fp, output_file, "w");

    if (input_file_fp == nullptr || output_file_fp == nullptr) {
        throw std::runtime_error("Error: Cannot open file.");
    }

    pgm::pgm_header_t header = pgm::pgm_header_read(input_file_fp);

    if (header.greyscale_max > 255 || header.width != header.height) {
        throw std::runtime_error("Only support square images with greyscale_max <= 255.");
    }

    unsigned char* input_image_2d = new unsigned char[header.width * header.height];
    unsigned char* output_image_2d = new unsigned char[header.width * header.height];
    pgm::pgm_image_read(input_file_fp, header, /* out */ input_image_2d);

    printf("Starting convolution...\n");
    conv::convolution(
        num_workers,
        g_convolution_kernel_2d,
        g_convolution_kernel_size,
        input_image_2d,
        /* out */ output_image_2d,
        header.width,
        header.height,
        header.greyscale_max
    );
    printf("Convolution finished.\n");

    // Write the output image to the output file.
    pgm::pgm_header_write(output_file_fp, header);
    pgm::pgm_image_write(output_file_fp, header, output_image_2d);
    printf("Output image written to file.\n");

    delete[] input_image_2d;
    delete[] output_image_2d;

    fclose(input_file_fp);
    fclose(output_file_fp);
}

static void usage() {
    printf(
        "Usage:\n"
        "   PGM Image Processing:\n"
        "         ./conv_gpu <input_pgm_file> <output_pgm_file> -t <num_workers>\n"
        "\n"
        "   PGM Image Comparison:\n"
        "         ./conv_gpu compare <input1> <input2>\n"
        "\n"
        "\n"
        "Example:\n"
        "   ./conv_gpu input.pgm output.pgm -t 64\n"
        "   ./conv_gpu compare expected_output.pgm output.pgm\n"
    );
}

// The main function.
int main(int argc, const char **argv) {
    // Print the usage if the number of arguments is not correct.
    if (argc != 5 && argc != 4) {
        usage();
        return 1;
    }

    try {
        if (argc == 4) {
            handle_compare(argv);
        }
        else {
            handle_convolution(argv);
        }
    }
    catch (const std::runtime_error& e) {
        fprintf(stderr, "Error: %s\n", e.what());
        return 1;
    }

    return 0;
}
