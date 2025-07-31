#include "config.hpp"
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

Config::Config(const std::string& json_path) {
    std::ifstream file(json_path);
    if (!file) {
        throw std::runtime_error("Could not open config file: " + json_path);
    }

    json j;
    file >> j;
    parse(j);
}

void Config::parse(const json& j) {
    const auto& sim = j.at("simulation");

    width_ = sim.at("width").get<int>();
    height_ = sim.at("height").get<int>();
    timestep_ = sim.at("time_step").get<float>();
    total_time_ = sim.at("total_time").get<float>();
    initial_temperature_ = sim.at("initial_temperature").get<float>();

    boundaries_.clear();
    if (sim.contains("boundary_conditions")) {
        for (const auto& [side, b] : sim.at("boundary_conditions").items()) {
            BoundaryCondition bc;
            bc.type = b.at("type").get<std::string>();
            if (b.contains("value")) {
                bc.value = b.at("value").get<float>();
            }
            boundaries_[side] = bc;
        }
    }

    materials_.clear();
    if (j.contains("materials")) {
        for (const auto& m : j.at("materials")) {
            Material mat;
            mat.name = m.at("name").get<std::string>();
            mat.thermal_conductivity = m.at("thermal_conductivity").get<float>();
            mat.density = m.at("density").get<float>();
            mat.specific_heat = m.at("specific_heat").get<float>();
            if (m.contains("color")) {
                mat.color = m.at("color").get<std::vector<int>>();
            }
            materials_.push_back(mat);
        }
    }

    regions_.clear();
    if (j.contains("regions")) {
        for (const auto& r : j.at("regions")) {
            Region reg;
            reg.material_id = r.at("material").get<int>(); 
            reg.x = r.at("x").get<int>();
            reg.y = r.at("y").get<int>();
            reg.width = r.at("width").get<int>();
            reg.height = r.at("height").get<int>();
            regions_.push_back(reg);
        }
    }

    heat_sources_.clear();
    if (j.contains("heat_sources")) {
        for (const auto& hs : j.at("heat_sources")) {
            HeatSource source;
            source.power = hs.at("value").get<float>();
            source.x = hs.at("x").get<int>();
            source.y = hs.at("y").get<int>();
            source.radius = hs.at("radius").get<int>();
            heat_sources_.push_back(source);
        }
    }

    if (j.contains("output")) {
        const auto& o = j.at("output");
        output_.write_interval = o.value("write_interval", 1.0f);
        output_.csv = o.value("csv", false);
        output_.image = o.value("image", false);
        output_.video = o.value("video", false);
        output_.gif = o.value("gif", false);
        output_.use_material_colors = o.value("use_material_colors", true);
    }
}

int Config::width() const { return width_; }
int Config::height() const { return height_; }
float Config::timestep() const { return timestep_; }
float Config::totalTime() const { return total_time_; }
float Config::initialTemperature() const { return initial_temperature_; }

BoundaryCondition Config::getBoundary(const std::string& side) const {
    return boundaries_.at(side);
}

const std::map<std::string, BoundaryCondition>& Config::boundaries() const {
    return boundaries_;
}

const std::vector<Material>& Config::materials() const {
    return materials_;
}

const std::vector<Region>& Config::regions() const {
    return regions_;
}

const std::vector<HeatSource>& Config::heatSources() const {
    return heat_sources_;
}

const OutputSettings& Config::output() const {
    return output_;
}

bool Config::useMaterialColors() const {
    return output_.use_material_colors;
}
