# Assignment \#4 Image Filtering

Student Number: 202191382 (zguan # mun.ca)

## Features

- The same result as `Expected_output_peppers.ascii`
- Written in C and based on MPI
- PGM parser with comment support
- PGM Image Comparison

## Idea

### Shared Memory

The first worker (rank = 0) is responsible for reading the input image and writing the output image. It first parse the PGM file and allocate shared memory. The shared memory contains the entire image and is easily accessable by all the co-workers.

### Barriers

`MPI_Barrier`s are commonly used to synchronize the workers.

## Usage

### Compile & Run with default config

```bash
./build_run.sh
```

### Custom config

```bash
mpiexec -n <workers> ./main /path/to/input.pgm /path/to/output.pgm
```

### Run without MPI

```bash
./main /path/to/input.pgm /path/to/output.pgm
```

### Comparison Mode

```bash
./main compare /path/to/a.pgm /path/to/b.pgm
# => Yay, The two images are the same! :)
# => The two images are NOT the same :(
```

## Configurable Items

- Number of Workers (Must be a square number)
- Convolution Kernel (Size must be an odd number)

## Drawbacks

- Only support square images (width == height)
- Maximum greyscale value is 255

## Build Script

I have prepared a build script to compile my code easily. The following items should be confirmed inside the build script `build_run.sh`:

- Output executable file name
- MPI include path
- MPI library path
- MPI link flags
- Number of MPI workers

This script runs the program after build it. To run it manually:

```bash
mpiexec -n 16 ./main /path/to/input.pgm /path/to/output.pgm
```
