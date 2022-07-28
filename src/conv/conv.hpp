//
// conv.hpp
//
// Part of ENGI-9869 Assignment #5
//
// Author: Zhen Guan
// Email: zguan@mun.ca
// Student Number: 202191382
//

namespace conv {
// Kernel size.
constexpr int CONV_KERNEL_SIZE = 9;

// Convolute a square block of the image with the given kernel.
// Points outside the block but still inside the image, will be set correctly;
// Points outside the image will be assumed to 0.
void convolution(
    int num_workers,
    int kernel_2d[CONV_KERNEL_SIZE * CONV_KERNEL_SIZE],
    int kernel_size,
    unsigned char* input_image_2d,
    unsigned char* output_image_2d,
    int image_width,
    int image_height,
    int greyscale_max
);
}
