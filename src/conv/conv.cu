//
// conv.cu
//
// Part of ENGI-9869 Assignment #5
//
// Author: Zhen Guan
// Email: zguan@mun.ca
// Student Number: 202191382
//

#include <stdexcept>
#include <chrono>

#include "conv/conv.hpp"
#include "util/array.hpp"

// Convolution on a single pixel.
// For a NxN picture, there bill be NxN cells running simultaneously.
__global__ void device_convolution_cell(
    const int *kernel_2d,
    const int kernel_size,
    const unsigned char *input_image_2d,
    unsigned char *output_image_2d,
    const int image_width,
    const int image_height,
    const int num_workers_sqrt,
    const int greyscale_max
) {
    const int my_rank = (int) threadIdx.x;
    const int block_size = image_width / num_workers_sqrt;
    const int from_row_index = (my_rank / num_workers_sqrt) * block_size;
    const int to_row_index = from_row_index + block_size;
    const int from_col_index = (my_rank % num_workers_sqrt) * block_size;
    const int to_col_index = from_col_index + block_size;

    for (int i = from_row_index; i < to_row_index; ++i) {
        for (int j = from_col_index; j < to_col_index; ++j) {
            // Convolution at input_image_2d[i][j];
            int sum = 0;
            for (int x = 0; x < kernel_size; ++x) {
                for (int y = 0; y < kernel_size; ++y) {
                    int row_index = i + x - kernel_size / 2;
                    int col_index = j + y - kernel_size / 2;
                    if (row_index < 0 || row_index >= image_height
                        || col_index < 0 || col_index >= image_width) {
                        // greyscale at outside the image (e.g. minus index) is zero.
                        sum += 0;
                    }
                    else {
                        sum += ARRAY2D_AT(kernel_2d, kernel_size, x, y)
                            * (int)ARRAY2D_AT(input_image_2d, image_width, row_index, col_index);
                    }
                }
            }
            if (sum < 0) {
                sum = 0;
            }
            else if (sum > greyscale_max) {
                sum = greyscale_max;
            }
            ARRAY2D_AT(output_image_2d, image_width, i, j) = (unsigned char)sum;
        }
    }

}

static void assertCudaError(cudaError_t error) {
    if (error != cudaSuccess) {
        throw std::runtime_error("CUDA error: " + std::string(cudaGetErrorString(error)));
    }
}

// Check if an integer has an integer square root or not.
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

// Convolute a square block of the image with the given kernel.
// Points outside the block but still inside the image, will be set correctly;
// Points outside the image will be assumed to 0.
void conv::convolution(
    int num_workers,
    int kernel_2d[conv::CONV_KERNEL_SIZE * conv::CONV_KERNEL_SIZE],
    int kernel_size,
    unsigned char *input_image_2d,
    unsigned char *output_image_2d,
    int image_width,
    int image_height,
    int greyscale_max
) {
    // Check if the image is square.
    if (image_width != image_height) {
        throw std::runtime_error("Image is not square.");
    }

    // Check if the num_workers is a square number.
    const int num_workers_sqrt = sqrt_integer_only(num_workers);
    if (num_workers_sqrt == 0) {
        throw std::runtime_error("num_workers is not a square number.");
    }

    // const int num_block_cells = image_width * image_height / num_workers;

    // Prepare device memory.
    int *kernel_2d_device;
    unsigned char *input_image_2d_device;
    unsigned char *output_image_2d_device;

    assertCudaError(cudaMalloc(&kernel_2d_device, sizeof(int) * kernel_size * kernel_size));
    assertCudaError(cudaMalloc(&input_image_2d_device, sizeof(unsigned char) * image_width * image_height));
    assertCudaError(cudaMalloc(&output_image_2d_device, sizeof(unsigned char) * image_width * image_height));

    // Prepare inputs.
    assertCudaError(cudaMemcpy(kernel_2d_device, kernel_2d, sizeof(int) * kernel_size * kernel_size, cudaMemcpyHostToDevice));
    assertCudaError(cudaMemcpy(input_image_2d_device, input_image_2d, sizeof(unsigned char) * image_width * image_height, cudaMemcpyHostToDevice));

    // Launch the kernel.

    float time;
    cudaEvent_t start;
    cudaEvent_t stop;

    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);

    device_convolution_cell<<<1, num_workers>>>(
        kernel_2d_device,
        kernel_size,
        input_image_2d_device,
        output_image_2d_device,
        image_width,
        image_height,
        num_workers_sqrt,
        greyscale_max
    );

    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&time, start, stop);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    printf("Time elapsed: %.0fms\n", time);

    // Copy output.
    assertCudaError(cudaMemcpy(output_image_2d, output_image_2d_device, sizeof(unsigned char) * image_width * image_height, cudaMemcpyDeviceToHost));
    assertCudaError(cudaFree(kernel_2d_device));
    assertCudaError(cudaFree(input_image_2d_device));
    assertCudaError(cudaFree(output_image_2d_device));
}
