cmake_minimum_required(VERSION 3.15)
project(heat_simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_language(CUDA)
find_package(CUDAToolkit REQUIRED)

add_executable(heat_simulator
    src/main.cpp
    src/config.cpp
    src/grid.cpp
    src/renderer.cpp
    src/solver.cu 
)


target_include_directories(heat_simulator PUBLIC
    "${CMAKE_SOURCE_DIR}/include"
)

find_package(OpenCV REQUIRED)
find_package(nlohmann_json REQUIRED)

target_link_libraries(heat_simulator PRIVATE
    ${OpenCV_LIBS}
    nlohmann_json::nlohmann_json
    CUDA::cudart  # Add CUDA runtime
)

set_target_properties(heat_simulator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

# Telling the linker to ignore default zlib because of conflicts
target_link_options(heat_simulator PRIVATE "/FORCE:MULTIPLE")

add_executable(test_grid
    tests/test_grid.cpp
    src/grid.cpp
    src/config.cpp
)

target_include_directories(test_grid PUBLIC
    "${CMAKE_SOURCE_DIR}/include"
)

target_link_libraries(test_grid PRIVATE
    ${OpenCV_LIBS}
    nlohmann_json::nlohmann_json
)

set_target_properties(test_grid PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

target_link_options(test_grid PRIVATE "/FORCE:MULTIPLE")





add_executable(test_heat_prop
    tests/test_heat_prop.cpp
    src/grid.cpp
    src/config.cpp
    src/renderer.cpp
    src/solver.cu
)

target_include_directories(test_heat_prop PUBLIC
    "${CMAKE_SOURCE_DIR}/include"
)

target_link_libraries(test_heat_prop PRIVATE
    ${OpenCV_LIBS}
    nlohmann_json::nlohmann_json
    CUDA::cudart  
)

set_target_properties(test_heat_prop PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

target_link_options(test_heat_prop PRIVATE "/FORCE:MULTIPLE")