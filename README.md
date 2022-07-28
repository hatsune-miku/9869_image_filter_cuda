# Assignment #5 Image Filtering Cuda Version

Student Number: 202191382 (zguan # mun.ca)

## Introduction

The CUDA implementation of Assignment #4. The ideas are the same.

See `report.pdf` for more details.

## Build

### Prerequisites

CUDA Toolkit

### Method 1: Use CMake

```bash
cd <project-dir>
mkdir build
cd build
cmake ..
make
mv conv_gpu ../
cd ..
```

### Method 2: Manually

```bash
cd <project-dir>
cd src
nvcc -I./ conv/conv.cu pgm/pgm.cpp main.cpp -o ../conv_gpu
cd ..
```

## Run

```bash
cd <project-dir>

# Run with threads = 16
./conv_gpu data/pepper.ascii.pgm out.pgm -t 16

# Run with threads = 64
./conv_gpu data/pepper.ascii.pgm out.pgm -t 64

# Check if two images are the same
./conv_gpu compare data/out.pgm data/Expected_output_peppers.ascii.pgm
```

### Please note that the number of threads must:

- be a square number,
- divisible by the image side length (e.g. 256).
