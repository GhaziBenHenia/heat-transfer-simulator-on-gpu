#include "grid.hpp"
#include <cmath>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Grid::Grid(int width, int height, float initial_temp)
    : width_(width), height_(height),
    temperature_(width* height, initial_temp),
    material_ids_(width* height, -1) {
}

void Grid::applyMaterialRegions(const std::vector<Region>& regions,
    const std::vector<Material>& materials) {
    for (const auto& region : regions) {
        int mat_id = region.material_id;
        for (int y = region.y; y < region.y + region.height; y++) {
            for (int x = region.x; x < region.x + region.width; x++) {
                if (x >= 0 && x < width_ && y >= 0 && y < height_) {
                    material_ids_[y * width_ + x] = mat_id;
                }
            }
        }
    }
}

void Grid::applyHeatSources(const std::vector<HeatSource>& sources, float dt) {
    for (const auto& source : sources) {
        float total_area = 0.0f;
        for (int dy = -source.radius; dy <= source.radius; dy++) {
            for (int dx = -source.radius; dx <= source.radius; dx++) {
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist <= source.radius) {
                    total_area += 1.0f - (dist / source.radius);
                }
            }
        }

        if (total_area < 1e-5f) continue;

        for (int dy = -source.radius; dy <= source.radius; dy++) {
            for (int dx = -source.radius; dx <= source.radius; dx++) {
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist <= source.radius) {
                    int x = source.x + dx;
                    int y = source.y + dy;
                    if (x >= 0 && x < width_ && y >= 0 && y < height_) {
                        int idx = y * width_ + x;
                        float intensity = (1.0f - (dist / source.radius)) / total_area;
                        temperature_[idx] += source.power * intensity * dt;
                    }
                }
            }
        }
    }
}

void Grid::applyBoundaryConditions(const std::map<std::string, BoundaryCondition>& boundaries) {
    for (const auto& [side, bc] : boundaries) {
        if (bc.type == "fixed") {
            if (side == "top") {
                for (int x = 0; x < width_; x++) {
                    temperature_[0 * width_ + x] = bc.value;
                }
            }
            else if (side == "bottom") {
                for (int x = 0; x < width_; x++) {
                    temperature_[(height_ - 1) * width_ + x] = bc.value;
                }
            }
            else if (side == "left") {
                for (int y = 0; y < height_; y++) {
                    temperature_[y * width_ + 0] = bc.value;
                }
            }
            else if (side == "right") {
                for (int y = 0; y < height_; y++) {
                    temperature_[y * width_ + (width_ - 1)] = bc.value;
                }
            }
        }
        else if (bc.type == "insulated") {
            // Insulated boundaries maintain the current temperature
            // No special handling needed in this implementation
        }
    }
}

int Grid::getMaterialAt(int x, int y) const {
    if (x >= 0 && x < width_ && y >= 0 && y < height_) {
        return material_ids_[y * width_ + x];
    }
    return -1;
}