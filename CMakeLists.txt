cmake_minimum_required(VERSION 3.17)

project(ortseed VERSION 0.0.0.1 LANGUAGES C CXX)

file(GLOB_RECURSE _src include/*.h include/*.cpp src/*.h src/*.cpp)

# CPU
set(_target ${PROJECT_NAME})
add_executable(${_target} ${_src})
target_include_directories(${_target} PUBLIC include)

if(WIN32)
    target_include_directories(${_target} PUBLIC libs/onnxruntime-win-x64-1.14.0/include)
    target_link_directories(${_target} PUBLIC libs/onnxruntime-win-x64-1.14.0/lib)
endif()

if(WIN32)
    # GPU
    set(_target ${PROJECT_NAME}-gpu)
    add_executable(${_target} ${_src})
    target_include_directories(${_target} PUBLIC include)
    target_include_directories(${_target} PUBLIC libs/onnxruntime-win-x64-gpu-1.14.0/include)
    target_link_directories(${_target} PUBLIC libs/onnxruntime-win-x64-gpu-1.14.0/lib)
endif()