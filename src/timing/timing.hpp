//
// timing.hpp
//
// Part of ENGI-9869 Assignment #5
//
// Author: Zhen Guan
// Email: zguan@mun.ca
// Student Number: 202191382
//

#pragma once

#include <chrono>

using std::chrono::steady_clock;
using std::chrono::high_resolution_clock;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

template<typename FunctionType>
long long time_milliseconds(FunctionType proc) {

    time_point<steady_clock> time_start = high_resolution_clock::now();
    proc();
    time_point<steady_clock> time_end = high_resolution_clock::now();
    return duration_cast<milliseconds>(time_end - time_start).count();
}
