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
add_subdirectory(deps/glm EXCLUDE_FROM_ALL)

# App framework using SDL2
add_library(${PROJECT_NAME}-Common "Common/App.cpp" "Common/App.hpp")
target_include_directories(${PROJECT_NAME}-Common PUBLIC "./" PRIVATE "deps/SDL2/include")
target_link_libraries(${PROJECT_NAME}-Common 
	PUBLIC
		SDL2-static SDL2main
		RGL
		glm
)

macro(add_sample dir)
	set(dir "${CMAKE_CURRENT_LIST_DIR}/${dir}")
	file(GLOB_RECURSE SOURCES "${dir}/*.cpp" "${dir}/*.hpp" "${dir}/*.h")
	add_executable(${dir} ${SOURCES})
	target_compile_features(${dir} PRIVATE cxx_std_20)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${dir}")

	set_target_properties(${dir} PROPERTIES
		MACOSX_BUNDLE TRUE
		XCODE_GENERATE_SCHEME ON
		VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/"
	)
	target_link_libraries(${dir} PRIVATE ${PROJECT_NAME}-Common)

	# TODO: replace glslc with custom compiler
	macro(shader_compile infile stage)
		get_filename_component(name_only ${infile} NAME)
		set(outname "${CMAKE_CURRENT_BINARY_DIR}/${name_only}.spv")
		list(APPEND all_shaders "${outname}")
		add_custom_command(
			PRE_BUILD
			OUTPUT "${outname}"
			DEPENDS ${infile}
			COMMAND ${rglc_path} -f "${infile}" -o "${outname}" --api Vulkan --stage ${stage} --debug
		)
	endmacro()

	file(GLOB all_shaders "${dir}/*.vert" "${dir}/*.frag")
	file(GLOB all_verts "${dir}/*.vert")
	foreach(FILE ${all_verts})
		shader_compile("${FILE}" "vertex")
	endforeach()
	file(GLOB all_frags "${dir}/*.frag")
	foreach(FILE ${all_frags})
		shader_compile("${FILE}" "fragment")
	endforeach()

	add_custom_target(${dir}_CompileShaders
		DEPENDS ${all_shaders}
	)
	add_dependencies(${dir} ${dir}_CompileShaders)
	add_dependencies(${dir}_CompileShaders rglc)
endmacro()

add_sample(HelloTriangle)