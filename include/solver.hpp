#pragma once

#include "grid.hpp"
#include "config.hpp"

class GPUSolver {
public:
    GPUSolver(const Config& config, const Grid& grid);
    ~GPUSolver();

    void step(float* temp, float time_step);
    void sync();

private:
    int width_;
    int height_;
    float* d_temp_in_;
    float* d_temp_out_;
    int* d_material_ids_;
};