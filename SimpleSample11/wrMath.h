#pragma once
#include <random>

static inline float randf()
{
    return (float)rand() / RAND_MAX;
}