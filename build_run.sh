#!/bin/bash

INPUT_FILE="data/pepper.ascii.pgm"
OUTPUT_FILE="data/output.pgm"

SOURCES=main.c
MPICC=mpicc
MPIEXEC=mpiexec
MPI_INCLUDE_PATH=/opt/homebrew/include
MPI_LIBRARY_PATH=/opt/homebrew/lib
MPI_LIB_NAME=mpi
ADDITIONAL_FLAGS="-Wall -Werror -O3"

MPI_WORKERS=16

COLOR_BOLD_CYAN="\033[1;36m"
COLOR_RESTORE='\033[0m'

###############################################################################

SCRIPT="$MPICC $SOURCES\
    -I$MPI_INCLUDE_PATH\
    -L$MPI_LIBRARY_PATH\
    -l$MPI_LIB_NAME\
    $ADDITIONAL_FLAGS\
    -o main"
printf ">> compiling: $COLOR_BOLD_CYAN$SCRIPT$COLOR_RESTORE\n"
$SCRIPT

SCRIPT="$MPIEXEC -n $MPI_WORKERS ./main $INPUT_FILE $OUTPUT_FILE"
printf ">> executing: $COLOR_BOLD_CYAN$SCRIPT$COLOR_RESTORE\n"
$SCRIPT

printf ">> cleaning up dSYM files...\n"
rm -rf ./*.dSYM
