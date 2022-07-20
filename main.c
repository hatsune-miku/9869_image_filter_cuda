// 
// main.c
// 
// Part of ENGI-9869 Assignment #4
//
// Author: Zhen Guan
// Email: zguan@mun.ca
// Student Number: 202191382
// 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mpi.h>

// Kernel size.
#define KERNEL_SIZE (9)

// Some annotations to improve readability of the code.
#define IN
#define OUT

// Access a 2D array that represented by a 1D array.
#define ARRAY2D_AT(array, row_size, row_index, col_index)\
    ((array)[(row_index) * (row_size) + (col_index)])

// PGM file header and body.
typedef struct {
    int width;
    int height;
    int greyscale_max;
} pgm_header_t;

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

// Read pgm header and skip the comment.
// The file pointer must be at the beginning of the file (ftell=0).
static pgm_header_t read_pgm_header(FILE *f) {
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
    sscanf(line, "%d %d", &ret.width, &ret.height);
    fgets(line, sizeof(line), f);
    sscanf(line, "%d", &ret.greyscale_max);

    return ret;
}

// Write the pgm (version: P5) header at the position that f is pointing to.
static void write_pgm_header(FILE* f, pgm_header_t header) {
    fprintf(f, "P5\n");
    fprintf(f, "# Created by Zhen Guan\n");
    fprintf(f, "%d %d\n", header.width, header.height);
    fprintf(f, "%d\n", header.greyscale_max);
}

// Read pgm image data.
// The file pointer must be at the end of the header, aka
// the beginning of the image data.
static void read_pgm_image(FILE *f, pgm_header_t header, unsigned char *out_image_2d) {
    int requiredSize = header.height * header.width;
    int tmp;
    while (requiredSize--) {
        fscanf(f, "%d", &tmp);
        *(out_image_2d++) = (char)tmp;
    }
}

// Write the pgm image data at the position that f is pointing to.
static void write_pgm_image(FILE *f, pgm_header_t header, unsigned char *image_2d) {
    for (int i = 0; i < header.height; ++i) {
        for (int j = 0; j < header.width; ++j) {
            fprintf(f, "%d ", (int)ARRAY2D_AT(image_2d, header.width, i, j));
        }
        fprintf(f, "\n");
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
    pgm_header_t header1, unsigned char* data1,
    pgm_header_t header2, unsigned char* data2    
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
        if (strcmp(argv[1], "compare")) {
            printf("Error: Invalid mode.\n");
            exit(1);
        }
        // Not generating output pgm, just compare the two inputs.
        char* input_file1 = argv[2];
        char* input_file2 = argv[3];

        FILE* f1 = fopen(input_file1, "r");
        FILE* f2 = fopen(input_file2, "r");

        if (f1 == NULL || f2 == NULL) {
            printf("Error: Cannot open file.\n");
            exit(1);
        }

        pgm_header_t header1 = read_pgm_header(f1);
        pgm_header_t header2 = read_pgm_header(f2);

        unsigned char* data1 = (unsigned char*)malloc(header1.height * header1.width);
        unsigned char* data2 = (unsigned char*)malloc(header2.height * header2.width);

        if (!data1 || !data2) {
            printf("Error: Cannot allocate memory.\n");
            exit(1);
        }

        read_pgm_image(f1, header1, data1);
        read_pgm_image(f2, header2, data2);

        if (is_pgm_file_equal(header1, data1, header2, data2)) {
            printf("Yay, The two images are the same! :)\n");
        }
        else {
            printf("The two images are NOT the same :(\n");
        }

        fclose(f1);
        fclose(f2);

        free(data1);
        free(data2);

        exit(0);
    }

    // Input parameters.
    char* input_file_name = argv[1];
    char* output_file_name = argv[2];

    // MPI basic info.
    int num_workers;
    int my_rank;
    int num_workers_square_root;

    // unsigned char should be enough.
    // TODO: consider if greyscale_max > 255.
    int memory_disp_unit = sizeof(char);

    // Shared memory related.
    // memory_size == memory_units * memory_disp_unit
    MPI_Win memory_window_header;
    MPI_Win memory_window_image;
    MPI_Aint memory_units;
    MPI_Aint memory_size;

    // Place for unwanted outputs.
    MPI_Aint dummy;


    // MPI initialization.
    if (MPI_Init(&argc, &argv) != 0) {
        printf("Error initializing MPI\n");
        exit(1);
    }

    // Number of processors and my rank.
    if (MPI_Comm_size(MPI_COMM_WORLD, &num_workers) != 0
        || MPI_Comm_rank(MPI_COMM_WORLD, &my_rank) != 0) {
        printf("Error getting number of processors or rank\n");
        exit(1);
    }

    num_workers_square_root = sqrt_integer_only(num_workers);
    if (num_workers_square_root == 0) {
        printf("Error: Number of processors must be a square number.\n");
        exit(1);
    }

    // One worker allocate the shared memory.
    // The other workers wait for the allocation.
    if (my_rank == 0) {
        printf("%d: Target file: %s\n", my_rank, input_file_name);
        FILE* f = fopen(input_file_name, "r");
        if (!f) {
            printf("Error opening file %s\n", input_file_name);
            exit(1);
        }

        printf("%d: Reading PGM header...\n", my_rank);
        pgm_header_t header = read_pgm_header(f);
        memory_units = header.height * header.width;
        memory_size = memory_units * memory_disp_unit;

        printf("%d: PGM header: width=%d, height=%d, greyscale_max=%d\n", 
         my_rank, header.width, header.height, header.greyscale_max);

         if (header.width != header.height) {
            printf("Error: PGM image must be square.\n");
            exit(1);
         }

         if (header.greyscale_max > 255) {
            printf("Error: PGM image greyscale_max must be <= 255.\n");
            exit(1);
         }

        // Allocate shared header data.
        printf("%d: Allocating %ld bytes of shared memory for header\n", my_rank, sizeof(pgm_header_t));
        MPI_Win_allocate_shared(
            1, sizeof(pgm_header_t),
            MPI_INFO_NULL, MPI_COMM_WORLD,
            &g_shared_header, &memory_window_header
        );

        if (!memory_window_header) {
            printf("Error allocating shared memory for header\n");
            exit(1);
        }
        *g_shared_header = header;

        // Allocate shared image data.
        printf("%d: Allocating %ld bytes of shared memory for input image\n", my_rank, memory_size);
        printf("%d: Allocating %ld bytes of shared memory for output image\n", my_rank, memory_size);

        // Why multiply by 2? Because we need to allocate 2 times the size of the image,
        // One for the input image, one for the output image.
        // Also, Writing the convolution result directly on the original image will
        // affect the next convolutions.
        MPI_Win_allocate_shared(
            2 * memory_units, memory_disp_unit,
            MPI_INFO_NULL, MPI_COMM_WORLD,
            &g_shared_image_data_2d, &memory_window_image
        );
        if (!memory_window_image) {
            printf("Error allocating shared memory for image\n");
            exit(1);
        }

        // Output image is the second half of the shared memory.
        // Input image is the first half.
        memset(g_shared_image_data_2d, 0, 2 * header.height * header.width);
        g_shared_output_image_data_2d = g_shared_image_data_2d + memory_size;

        // Read the image data.
        printf("%d: Reading PGM image...\n", my_rank);
        read_pgm_image(f, header, g_shared_image_data_2d);

        // Oh I nearly forgot to close the file.
        fclose(f);
    }
    else {
        // For other workers,
        // retrieve the header data.
        MPI_Win_allocate_shared(
            0, memory_disp_unit,
            MPI_INFO_NULL, MPI_COMM_WORLD,
            &g_shared_header, &memory_window_header
        );
        MPI_Win_shared_query(
            memory_window_header, 0,
            OUT &dummy, OUT &memory_disp_unit, OUT &g_shared_header
        );

        // Retrieve the image data.
        MPI_Win_allocate_shared(
            0, memory_disp_unit,
            MPI_INFO_NULL, MPI_COMM_WORLD,
            &g_shared_image_data_2d, &memory_window_image
        );
        MPI_Win_shared_query(
            memory_window_image, 0,
            OUT &memory_size, OUT &memory_disp_unit, OUT &g_shared_image_data_2d
        );
        memory_size /= 2;

        if (memory_size != g_shared_header->width * g_shared_header->height) {
            printf("Error: shared memory size mismatch\n");
            exit(1);
        }

        g_shared_output_image_data_2d = g_shared_image_data_2d + memory_size;
    }
    MPI_Win_fence(0, memory_window_header);
    MPI_Win_fence(0, memory_window_image);

    if (my_rank == 0) {
        printf("%d: All workers are able to access the shared image data now.\n", my_rank);
    }

    // Final prepare for covolution.
    MPI_Barrier(MPI_COMM_WORLD);

    // Dispatch the work.
    int block_size = g_shared_header->width / num_workers_square_root;
    int from_row_index = (my_rank / num_workers_square_root) * block_size;
    int to_row_index = (my_rank / num_workers_square_root) * block_size + block_size;
    int from_col_index = (my_rank % num_workers_square_root) * block_size;
    int to_col_index = (my_rank % num_workers_square_root) * block_size + block_size;

    printf("%d: Starting convolution...\n", my_rank);
    convolution(
        g_convolution_kernel, g_convolution_kernel_size,
        g_shared_image_data_2d, OUT g_shared_output_image_data_2d,
        g_shared_header->width, g_shared_header->height,
        from_row_index, to_row_index,
        from_col_index, to_col_index
    );

    MPI_Barrier(MPI_COMM_WORLD);
    if (my_rank == 0) {
        printf("%d: All workers are done.\n", my_rank);
    }

    if (my_rank == 0) {
        FILE* f;
        char s[1024];

        printf("%d: Writing output image...\n", my_rank);

        // Quiet redundant logic here.
        // Just to ensure the calculated data won't be lost.
        while (1) {
            f = fopen(output_file_name, "w");
            if (!f) {
                printf("Error opening file %s for writing.\n", output_file_name);
                while (1) {
                    printf("Do you want to try again? (Y/N): ");
                    fgets("%s%*c", sizeof(s) / sizeof(char), stdin);

                    if ((s[0] == 'y' || s[0] == 'Y') && s[1] == 0) {
                        break;
                    }
                    else if ((s[0] == 'n' || s[0] == 'N') && s[1] == 0) {
                        // In this case, goto is of higher readability 
                        // than a flag variable to exit the nested loop.
                        goto end;
                    }
                    // else, illegal input, re-prompt.
                }
            }
            else {
                write_pgm_header(f, *g_shared_header);
                write_pgm_image(f, *g_shared_header, g_shared_output_image_data_2d);

                printf("%d: Successfully wrote output file %s\n", my_rank, output_file_name);
                break;
            }
            // try again.
        }
        // Don't forget to close the file :)
        fclose(f);
    }

end:
    MPI_Win_free(&memory_window_header);
    MPI_Win_free(&memory_window_image);
    MPI_Finalize();

    return 0;
}
