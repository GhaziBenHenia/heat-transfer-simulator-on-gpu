#include "grid.hpp"
#include "renderer.hpp"
#include "config.hpp"
#include "solver.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>

int main() {
    try {
        Config config("config/default_config.json");
        Grid grid(config.width(), config.height(), config.initialTemperature());
        grid.applyMaterialRegions(config.regions(), config.materials());

        GPUSolver solver(config, grid);

        const float total_time = config.totalTime();
        const float time_step = config.timestep();
        const int num_frames = static_cast<int>(total_time / time_step);

        const int visualization_interval = std::max(1, static_cast<int>(0.5f / time_step));  // Update every 500ms

        bool visualize = true;

        if (visualize) {
            Renderer::showHeatmapOverlay(grid, config.materials(), 0.5f, 1);
        }

        auto start = std::chrono::high_resolution_clock::now();
        int frame_count = 0;

        for (int frame = 0; frame < num_frames; ++frame) {
            grid.applyBoundaryConditions(config.boundaries());

            grid.applyHeatSources(config.heatSources(), time_step);

            solver.step(grid.temperature(), time_step);

            if (visualize && (frame % visualization_interval == 0)) {
                Renderer::showHeatmapOverlay(grid, config.materials(), 0.5f, 1);
                frame_count++;
            }

            int key = cv::waitKey(visualize ? 1 : 0);
            if (key == 27) break;  // ESC to exit        
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << "Simulation speed: "
            << (1000.0 * num_frames / duration) << " steps/second\n";
        std::cout << "Frames rendered: " << frame_count << "\n";

        if (visualize) {
            Renderer::showHeatmapOverlay(grid, config.materials(), 0.5f, 0);
            cv::waitKey(0);
        }

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}