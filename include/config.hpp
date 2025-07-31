#pragma once
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <map>

constexpr int MAX_MATERIALS = 10;

struct BoundaryCondition {
    std::string type;
    float value = 0.0f;
};

struct Material {
    std::string name;
    float thermal_conductivity;
    float density;
    float specific_heat;
    std::vector<int> color;
};

struct Region {
    int material_id;
    int x, y;
    int width, height;
};

struct HeatSource {
    float power;
    int x, y;
    int radius;
};

struct OutputSettings {
    float write_interval;
    bool csv;
    bool image;
    bool video;
    bool gif;
    bool use_material_colors;
};

class Config {
public:
    explicit Config(const std::string& json_path);

    int width() const;
    int height() const;
    float timestep() const;
    float totalTime() const;
    float initialTemperature() const;

    BoundaryCondition getBoundary(const std::string& side) const;
    const std::map<std::string, BoundaryCondition>& boundaries() const;

    const std::vector<Material>& materials() const;
    const std::vector<Region>& regions() const;
    const std::vector<HeatSource>& heatSources() const;
    const OutputSettings& output() const;

    bool useMaterialColors() const;

private:
    void parse(const nlohmann::json& j);

    int width_;
    int height_;
    float timestep_;
    float total_time_;
    float initial_temperature_;

    std::map<std::string, BoundaryCondition> boundaries_;
    std::vector<Material> materials_;
    std::vector<Region> regions_;
    std::vector<HeatSource> heat_sources_;
    OutputSettings output_;
};