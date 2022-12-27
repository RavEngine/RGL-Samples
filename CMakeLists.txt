cmake_minimum_required(VERSION 3.17)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR} CACHE INTERNAL "")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)

if(MSVC)
	add_definitions(/MP)
endif()

project(RGL-Samples)

set(SDL_STATIC ON CACHE INTERNAL "" FORCE)
set(SDL_SHARED OFF CACHE INTERNAL "" FORCE)
set(SDL_LIBC ON CACHE BOOL "" FORCE)
add_subdirectory(deps/SDL2 EXCLUDE_FROM_ALL)

add_subdirectory(deps/RGL EXCLUDE_FROM_ALL)

# App framework using SDL2
add_library(${PROJECT_NAME}-Common "Common/App.cpp" "Common/App.hpp")
target_include_directories(${PROJECT_NAME}-Common PUBLIC "./" PRIVATE "deps/SDL2/include")
target_link_libraries(${PROJECT_NAME}-Common 
	PUBLIC
		SDL2-static SDL2main
		RGL
)

macro(add_sample dir)
	file(GLOB_RECURSE SOURCES "${dir}/*.cpp" "${dir}/*.hpp" "${dir}/*.h")
	add_executable(${dir} ${SOURCES})
	target_compile_features(${dir} PRIVATE cxx_std_20)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${dir}")

	set_target_properties(${dir} PROPERTIES
		MACOSX_BUNDLE TRUE
		XCODE_GENERATE_SCHEME ON
		VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>"
	)
	target_link_libraries(${dir} PRIVATE ${PROJECT_NAME}-Common)
endmacro()

add_sample(HelloTriangle)