#pragma once

#include "grid.hpp"
#include "config.hpp"
#include <vector>

class Renderer {
public:
    static void showMaterialGrid(const Grid& grid, const std::vector<Material>& materials, int delay_ms);

    static void showHeatmapOverlay(const Grid& grid,
        const std::vector<Material>& materials,
        float blend_factor,
        int delay_ms);
};
