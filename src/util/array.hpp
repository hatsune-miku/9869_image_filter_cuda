//
// array.hpp
//
// Part of ENGI-9869 Assignment #5
//
// Author: Zhen Guan
// Email: zguan@mun.ca
// Student Number: 202191382
//

#pragma once

#ifndef ARRAY2D_AT
#define ARRAY2D_AT(array, row_size, row_index, col_index)\
    ((array)[(row_index) * (row_size) + (col_index)])
#endif
