#include "solver.hpp"
#include <cuda_runtime.h>
#include <stdexcept>
#include <vector>
#include <device_launch_parameters.h>
#include <iostream>

__constant__ float c_conductivities[MAX_MATERIALS];
__constant__ float c_densities[MAX_MATERIALS];
__constant__ float c_specific_heats[MAX_MATERIALS];

__global__ void heatStepKernel(
    float* out,
    const float* in,
    const int* material_ids,
    int width,
    int height,
    float dt
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x < 1 || x >= width - 1 || y < 1 || y >= height - 1) {
        if (x < width && y < height) {
            int idx = y * width + x;
            out[idx] = in[idx];  
        }
        return;
    }

    int idx = y * width + x;
    int mat_id = material_ids[idx];

    if (mat_id < 0) {
        out[idx] = in[idx];
        return;
    }

    float k_center = c_conductivities[mat_id];
    float rho = c_densities[mat_id];
    float c = c_specific_heats[mat_id];

    int id_left = material_ids[y * width + (x - 1)];
    int id_right = material_ids[y * width + (x + 1)];
    int id_up = material_ids[(y - 1) * width + x];
    int id_down = material_ids[(y + 1) * width + x];

    float k_left = (id_left >= 0) ?
        2.0f * k_center * c_conductivities[id_left] / (k_center + c_conductivities[id_left]) : k_center;
    float k_right = (id_right >= 0) ?
        2.0f * k_center * c_conductivities[id_right] / (k_center + c_conductivities[id_right]) : k_center;
    float k_up = (id_up >= 0) ?
        2.0f * k_center * c_conductivities[id_up] / (k_center + c_conductivities[id_up]) : k_center;
    float k_down = (id_down >= 0) ?
        2.0f * k_center * c_conductivities[id_down] / (k_center + c_conductivities[id_down]) : k_center;

    float temp_center = in[idx];
    float temp_left = in[y * width + (x - 1)];
    float temp_right = in[y * width + (x + 1)];
    float temp_up = in[(y - 1) * width + x];
    float temp_down = in[(y + 1) * width + x];

    float flux = k_left * (temp_left - temp_center) +
        k_right * (temp_right - temp_center) +
        k_up * (temp_up - temp_center) +
        k_down * (temp_down - temp_center);

    out[idx] = temp_center + (dt * flux) / (rho * c);
}

GPUSolver::GPUSolver(const Config& config, const Grid& grid)
    : width_(grid.width()), height_(grid.height()),
    d_temp_in_(nullptr), d_temp_out_(nullptr),
    d_material_ids_(nullptr)
{
    size_t grid_size = width_ * height_;
    size_t temp_bytes = grid_size * sizeof(float);
    size_t id_bytes = grid_size * sizeof(int);

    cudaMalloc(&d_temp_in_, temp_bytes);
    cudaMalloc(&d_temp_out_, temp_bytes);
    cudaMalloc(&d_material_ids_, id_bytes);

    const auto& materials = config.materials();
    size_t mat_count = materials.size();

    if (mat_count > MAX_MATERIALS) {
        throw std::runtime_error("Too many materials! Increase MAX_MATERIALS");
    }

    std::vector<float> conductivities(MAX_MATERIALS, 0.0f);
    std::vector<float> densities(MAX_MATERIALS, 0.0f);
    std::vector<float> specific_heats(MAX_MATERIALS, 0.0f);

    for (size_t i = 0; i < mat_count; ++i) {
        conductivities[i] = materials[i].thermal_conductivity;
        densities[i] = materials[i].density;
        specific_heats[i] = materials[i].specific_heat;
    }

    cudaMemcpyToSymbol(c_conductivities, conductivities.data(), MAX_MATERIALS * sizeof(float));
    cudaMemcpyToSymbol(c_densities, densities.data(), MAX_MATERIALS * sizeof(float));
    cudaMemcpyToSymbol(c_specific_heats, specific_heats.data(), MAX_MATERIALS * sizeof(float));

    cudaMemcpy(d_material_ids_, grid.material_ids(), id_bytes, cudaMemcpyHostToDevice);
}

GPUSolver::~GPUSolver() {
    cudaFree(d_temp_in_);
    cudaFree(d_temp_out_);
    cudaFree(d_material_ids_);
}

void GPUSolver::step(float* temp, float dt) {
    size_t grid_size = width_ * height_;
    size_t temp_bytes = grid_size * sizeof(float);

    cudaMemcpy(d_temp_in_, temp, temp_bytes, cudaMemcpyHostToDevice);

    dim3 blockDim(16, 16);
    dim3 gridDim((width_ + 15) / 16, (height_ + 15) / 16);

    heatStepKernel << <gridDim, blockDim >> > (
        d_temp_out_,
        d_temp_in_,
        d_material_ids_,
        width_,
        height_,
        dt
        );

    cudaMemcpy(temp, d_temp_out_, temp_bytes, cudaMemcpyDeviceToHost);
}

void GPUSolver::sync() {
    cudaDeviceSynchronize();
}