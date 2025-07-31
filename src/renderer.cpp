#include "renderer.hpp"
#include <opencv2/opencv.hpp>
#include <algorithm> 

void Renderer::showMaterialGrid(const Grid& grid, const std::vector<Material>& materials, int delay_ms) {
    int width = grid.width();
    int height = grid.height();
    const int* material_ids = grid.material_ids();

    cv::Mat image(height, width, CV_8UC3);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int id = material_ids[y * width + x];
            cv::Vec3b color;
            if (id >= 0 && id < (int)materials.size()) {
                const auto& col = materials[id].color;
                color = cv::Vec3b(
                    col[0],  // Blue channel
                    col[1],  // Green channel
                    col[2]   // Red channel
                );
            }
            else {
                color = cv::Vec3b(0, 0, 0);
            }
            image.at<cv::Vec3b>(y, x) = color;
        }
    }

    int scale = std::max(1, 600 / std::max(width, height));
    cv::Mat resized;
    cv::resize(image, resized, cv::Size(), scale, scale, cv::INTER_NEAREST);

    cv::imshow("Material Grid", resized);
    cv::waitKey(delay_ms);
}

void Renderer::showHeatmapOverlay(const Grid& grid,
    const std::vector<Material>& materials,
    float blend_factor,
    int delay_ms) {
    int width = grid.width();
    int height = grid.height();

    const int* material_ids = grid.material_ids();
    const float* temperature = grid.temperature();

    cv::Mat material_image(height, width, CV_8UC3);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int id = material_ids[y * width + x];
            cv::Vec3b color;
            if (id >= 0 && id < (int)materials.size()) {
                const auto& col = materials[id].color;
                color = cv::Vec3b(
                    col[0],  // Blue channel
                    col[1],  // Green channel
                    col[2]   // Red channel
                );
            }
            else {
                color = cv::Vec3b(0, 0, 0);
            }
            material_image.at<cv::Vec3b>(y, x) = color;
        }
    }

    float min_temp = *std::min_element(temperature, temperature + width * height);
    float max_temp = *std::max_element(temperature, temperature + width * height);
    if (max_temp - min_temp < 1e-5f) max_temp = min_temp + 1e-5f;

    cv::Mat heatmap_gray(height, width, CV_8UC1);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float temp = temperature[y * width + x];
            int gray = static_cast<int>(255.0f * (temp - min_temp) / (max_temp - min_temp));
            gray = std::clamp(gray, 0, 255);
            heatmap_gray.at<uchar>(y, x) = static_cast<uchar>(gray);
        }
    }

    cv::Mat heatmap_color;
    cv::applyColorMap(heatmap_gray, heatmap_color, cv::COLORMAP_JET);

    cv::Mat blended(height, width, CV_8UC3);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            cv::Vec3b mat_col = material_image.at<cv::Vec3b>(y, x);
            cv::Vec3b heat_col = heatmap_color.at<cv::Vec3b>(y, x);

            cv::Vec3b blended_col;
            for (int c = 0; c < 3; ++c) {
                blended_col[c] = static_cast<uchar>(
                    blend_factor * mat_col[c] + (1.0f - blend_factor) * heat_col[c]
                    );
            }
            blended.at<cv::Vec3b>(y, x) = blended_col;
        }
    }

    int scale = std::max(1, 600 / std::max(width, height));
    cv::Mat resized;
    cv::resize(blended, resized, cv::Size(), scale, scale, cv::INTER_NEAREST);

    cv::imshow("Heatmap Overlay", resized);
    cv::waitKey(delay_ms);
}