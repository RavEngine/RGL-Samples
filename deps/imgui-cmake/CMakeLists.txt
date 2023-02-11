cmake_minimum_required(VERSION 3.16)

project(imgui)
file(GLOB SOURCES "imgui/*.cpp" "imgui/*.h" "imgui/misc/cpp/*.h" "imgui/misc/cpp/*.cpp" "imgui/misc/fonts/*.h" "imgui/misc/fonts/*.cpp" "imgui/misc/debuggers/*.natvis" "imgui/backends/*.h" "imgui/backends/*.cpp")
add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC 
    "imgui/"
    "imgui/backends"
    PRIVATE
    "imgui/"
    "../SDL2/include"
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
