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
set(SDL_RENDER_ENABLED_BY_DEFAULT OFF CACHE INTERNAL "")
set(SDL_AUDIO_ENABLED_BY_DEFAULT OFF CACHE INTERNAL "")
set(SDL_FILE_ENABLED_BY_DEFAULT OFF CACHE INTERNAL "")
set(SDL_CPUINFO_ENABLED_BY_DEFAULT OFF CACHE INTERNAL "")
set(SDL_FILESYSTEM_ENABLED_BY_DEFAULT OFF CACHE INTERNAL "")
set(SDL_SENSOR_ENABLED_BY_DEFAULT OFF CACHE INTERNAL "")
set(SDL_LOCALE_ENABLED_BY_DEFAULT OFF CACHE INTERNAL "")
set(SDL_MISC_ENABLED_BY_DEFAULT OFF CACHE INTERNAL "")
set(SDL_ATOMIC_ENABLED_BY_DEFAULT OFF CACHE INTERNAL "")
set(SDL_TIMERS_ENABLED_BY_DEFAULT OFF CACHE INTERNAL "")
set(SDL_POWER_ENABLED_BY_DEFAULT OFF CACHE INTERNAL "")
set(SDL_HAPTIC_ENABLED_BY_DEFAULT OFF CACHE INTERNAL "")
set(SDL_JOYSTIC_ENABLED_BY_DEFAULT OFF CACHE INTERNAL "")
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

	macro(shader_compile infile stage api extension binary)
		get_filename_component(name_only ${infile} NAME)
		set(outname "${CMAKE_CURRENT_BINARY_DIR}/shaders/${api}/${name_only}.${extension}")
		if (${api} MATCHES "Metal")
			STRING(REPLACE "." "_" name_fixed ${name_only} )
			set(entrypoint "--entrypoint" "${name_fixed}")
		endif()
		list(APPEND all_shaders "${outname}")
		add_custom_command(
			PRE_BUILD
			OUTPUT "${outname}"
			DEPENDS ${infile}
			COMMAND ${rglc_path} -f "${infile}" -o "${outname}" --api ${api} --stage ${stage} --debug ${binary} ${entrypoint}
		)
	endmacro()

	file(GLOB all_shaders "${dir}/*.vert" "${dir}/*.frag")
	file(GLOB all_verts "${dir}/*.vert")
	foreach(FILE ${all_verts})
		if(RGL_VK_AVAILABLE)
			shader_compile("${FILE}" "vertex" "Vulkan" "spv" "")
		endif()
		if(RGL_DX12_AVAILABLE)
			shader_compile("${FILE}" "vertex" "Direct3D12" "cso" "--binary")
		endif()
		if(RGL_MTL_AVAILABLE)
			shader_compile("${FILE}" "vertex" "Metal" "metal" "")
			target_sources(${dir} PUBLIC ${outname})
			set_source_files_properties(${outname} PROPERTIES 
				GENERATED TRUE
				HEADER_FILE_ONLY OFF
				LANGUAGE METAL
				XCODE_EXPLICIT_FILE_TYPE "sourcecode.metal"
			)
		endif()
	endforeach()
	file(GLOB all_frags "${dir}/*.frag")
	foreach(FILE ${all_frags})
		if(RGL_VK_AVAILABLE)
			shader_compile("${FILE}" "fragment" "Vulkan" "spv" "")
		endif()
		if(RGL_DX12_AVAILABLE)
			shader_compile("${FILE}" "fragment" "Direct3D12" "cso" "--binary")
		endif()
		if(RGL_MTL_AVAILABLE)
			shader_compile("${FILE}" "fragment" "Metal" "metal" "")
			target_sources(${dir} PUBLIC ${outname})
			set_source_files_properties(${outname} PROPERTIES 
				GENERATED TRUE
				HEADER_FILE_ONLY OFF
				LANGUAGE METAL
				XCODE_EXPLICIT_FILE_TYPE "sourcecode.metal"
			)
		endif()
	endforeach()

	add_custom_target(${dir}_CompileShaders
		DEPENDS ${all_shaders}
	)
	add_dependencies(${dir} ${dir}_CompileShaders)
	add_dependencies(${dir}_CompileShaders rglc)
endmacro()

add_sample(HelloTriangle)

set_target_properties(
	"SDL2main"
	"SDL2-static"
	"sdl_headers_copy"
	"RGL-Samples-Common"
	PROPERTIES 
	FOLDER "RGL Samples Dependencies"
)
