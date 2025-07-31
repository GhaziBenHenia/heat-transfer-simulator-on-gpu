#include "config.hpp"
#include "grid.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    try {
        Config config("config/default_config.json");

        Grid grid(config.width(), config.height(), config.initialTemperature());
        grid.applyMaterialRegions(config.regions(), config.materials());

        int width = grid.width();
        int height = grid.height();
        const int* material_ids = grid.material_ids();
        const auto& materials = config.materials();
        bool use_colors = config.useMaterialColors();

        float timestep = config.timestep();     
        float total_time = config.totalTime();  
        int num_steps = static_cast<int>(total_time / timestep);

        // Dynamically choose scale based on grid size
        int scale = 1;
        if (width <= 50 && height <= 50)
            scale = 10;
        else if (width <= 100 && height <= 100)
            scale = 5;
        else if (width <= 200 && height <= 200)
            scale = 3;
        else
            scale = 1;

        cv::namedWindow("Material Grid Visualization", cv::WINDOW_AUTOSIZE);

        for (int step = 0; step < num_steps; ++step) {
            cv::Mat image(height, width, CV_8UC3);

            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    int idx = y * width + x;
                    int mat_id = material_ids[idx];
                    cv::Vec3b color = { 50, 50, 50 };

                    if (use_colors && mat_id >= 0 && mat_id < static_cast<int>(materials.size())) {
                        const auto& col = materials[mat_id].color;
                        if (col.size() == 3) {
                            color = {
                                static_cast<uchar>(col[2]),
                                static_cast<uchar>(col[1]),
                                static_cast<uchar>(col[0])
                            };
                        }
                    }

                    image.at<cv::Vec3b>(y, x) = color;
                }
            }

            cv::Mat display;
            cv::resize(image, display, cv::Size(width * scale, height * scale), 0, 0, cv::INTER_NEAREST);
            cv::imshow("Material Grid Visualization", display);

            int wait_ms = static_cast<int>(timestep * 1000);
            int key = cv::waitKey(wait_ms);
            if (key == 27) break;  // ESC to exit
        }

        cv::destroyAllWindows();
    }
    catch (const std::exception& ex) {
        std::cerr << "Error in test_grid: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
