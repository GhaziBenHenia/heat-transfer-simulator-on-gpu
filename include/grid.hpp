#pragma once
#include "config.hpp"
#include <vector>

class Grid {
public:
    Grid(int width, int height, float initial_temp);

    void applyMaterialRegions(const std::vector<Region>& regions,
        const std::vector<Material>& materials);
    void applyHeatSources(const std::vector<HeatSource>& sources, float dt);
    void applyBoundaryConditions(const std::map<std::string, BoundaryCondition>& boundaries);

    const float* temperature() const { return temperature_.data(); }
    float* temperature() { return temperature_.data(); }

    const int* material_ids() const { return material_ids_.data(); }
    int* material_ids() { return material_ids_.data(); }

    int getMaterialAt(int x, int y) const;

    int width() const { return width_; }
    int height() const { return height_; }

private:
    int width_;
    int height_;
    std::vector<float> temperature_;
    std::vector<int> material_ids_;
};