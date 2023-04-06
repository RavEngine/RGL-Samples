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
if(CMAKE_SYSTEM_NAME MATCHES "WindowsStore")
	set(SDL_RENDER_ENABLED_BY_DEFAULT ON CACHE INTERNAL "")
endif()
add_subdirectory(deps/SDL2 EXCLUDE_FROM_ALL)

add_subdirectory(deps/glm EXCLUDE_FROM_ALL)
add_subdirectory(deps/fmt EXCLUDE_FROM_ALL)
add_subdirectory(deps/stb_image EXCLUDE_FROM_ALL)
add_subdirectory(deps/imgui-cmake EXCLUDE_FROM_ALL)
add_subdirectory(deps/OpenXR-SDK EXCLUDE_FROM_ALL)
add_subdirectory(deps/tinyobjloader EXCLUDE_FROM_ALL)

if(CMAKE_CROSSCOMPILING)
	set(RGL_ENABLE_RGLC OFF CACHE INTERNAL "")
	set(TOOLS_DIR ${CMAKE_BINARY_DIR}/host-tools CACHE INTERNAL "")
	file(MAKE_DIRECTORY ${TOOLS_DIR})
	execute_process(
		COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" ${LINUX_HOST_CC} ${LINUX_HOST_CXX} -DCMAKE_BUILD_TYPE=Release ${CMAKE_CURRENT_SOURCE_DIR}
		WORKING_DIRECTORY ${TOOLS_DIR}
	)
	if(MSVC)
		set(rglc_ext ".exe")
	endif()
	set(rglc_path "${TOOLS_DIR}/Release/rglc${rglc_ext}")
	add_custom_command(
		PRE_BUILD
		OUTPUT "${rglc_path}" 
		COMMAND ${CMAKE_COMMAND} --build . --config Release --target rglc
		WORKING_DIRECTORY "${TOOLS_DIR}"
		VERBATIM
	)
	add_custom_target("rglc" DEPENDS "${rglc_path}")
endif()
set(RGL_DISABLE_DX OFF CACHE INTERNAL "")
set(RGL_DISABLE_VK OFF CACHE INTERNAL "")
add_subdirectory(deps/RGL EXCLUDE_FROM_ALL)

# App framework using SDL2
add_library(${PROJECT_NAME}-Common "Common/App.cpp" "Common/App.hpp" "Common/ExampleFramework.hpp" "Common/ExampleFramework.cpp" "Common/AppleFns.hpp" "Common/AppleFns.mm")
target_compile_features(${PROJECT_NAME}-Common PRIVATE cxx_std_20)
target_include_directories(${PROJECT_NAME}-Common PUBLIC "./" PRIVATE "deps/SDL2/include")

if (APPLE)
set_source_files_properties("Common/AppleFns.mm" PROPERTIES
    COMPILE_FLAGS "-x objective-c++"
)
endif()


if (CMAKE_SYSTEM_NAME MATCHES WindowsStore)
	add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
	add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
else()
	set(SDLMAIN_LIB "SDL2main")
endif()

target_link_libraries(${PROJECT_NAME}-Common 
	PUBLIC
		SDL2-static ${SDLMAIN_LIB}
		RGL
		glm
        fmt
		stb_image
        imgui
)

macro(add_sample dir)
	set(dir "${CMAKE_CURRENT_LIST_DIR}/${dir}")
	file(GLOB_RECURSE SOURCES "${dir}/*.cpp" "${dir}/*.hpp" "${dir}/*.h")
    file(GLOB_RECURSE RESOURCES "${dir}/*.png" "${dir}/*.obj")
	if (APPLE)
		set(ADDITIONALS ${RESOURCES})
	else()
		foreach(FILE ${RESOURCES})
			get_filename_component(output_name "${FILE}" NAME)
			set(outname "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/${output_name}")
			add_custom_command(PRE_BUILD 
				OUTPUT "${outname}" 
				COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FILE} "${outname}"
				DEPENDS ${FILE}
			)
		endforeach()
	endif()
	add_executable(${dir} ${SOURCES} ${ADDITIONALS} "iosLaunchScreen.storyboard")
	target_compile_features(${dir} PRIVATE cxx_std_20)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${dir}")

	set_target_properties(${dir} PROPERTIES
		MACOSX_BUNDLE TRUE
		XCODE_GENERATE_SCHEME ON
        RESOURCE "${RESOURCES}"
		VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>"
		XCODE_ATTRIBUTE_BUNDLE_IDENTIFIER "com.ravbug.${APPNAME}"		# with templated plist we can set this
		XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.ravbug.${dir}"
		XCODE_ATTRIBUTE_CURRENTYEAR "${CURRENTYEAR}"
		XCODE_ATTRIBUTE_MTL_ENABLE_DEBUG_INFO "INCLUDE_SOURCE"
    	XCODE_ATTRIBUTE_MTL_LANGUAGE_REVISION "Metal24"
		MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
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

	target_sources(${dir} PUBLIC ${all_shaders})
	set_source_files_properties(${all_shaders} PROPERTIES
		HEADER_FILE_ONLY ON
		XCODE_EXPLICIT_FILE_TYPE "sourcecode.glsl"
	)
	source_group(Shaders FILES ${all_shaders})

	file(GLOB all_verts "${dir}/*.vert")
	foreach(FILE ${all_verts})
		if(RGL_VK_AVAILABLE)
			shader_compile("${FILE}" "vertex" "Vulkan" "spv" "")
		endif()
		if(RGL_DX12_AVAILABLE)
			shader_compile("${FILE}" "vertex" "Direct3D12" "hlsl" "")
			target_sources(${dir} PUBLIC ${outname})
			set_source_files_properties(${outname} PROPERTIES 
				GENERATED TRUE
				VS_SHADER_MODEL "6.4"
				VS_SHADER_TYPE "Vertex"
				VS_SHADER_ENABLE_DEBUG ON
				VS_SHADER_DISABLE_OPTIMIZATIONS ON
			)
		endif()
		if(RGL_MTL_AVAILABLE)
			shader_compile("${FILE}" "vertex" "Metal" "metal" "")
			target_sources(${dir} PUBLIC ${outname})
			set_source_files_properties(${outname} PROPERTIES 
				GENERATED TRUE
				HEADER_FILE_ONLY OFF
				LANGUAGE METAL
				XCODE_EXPLICIT_FILE_TYPE "sourcecode.metal"
                COMPILE_FLAGS "-gline-tables-only -frecord-sources" # enable shader source debugging
			)
		endif()
	endforeach()
	file(GLOB all_frags "${dir}/*.frag")
	foreach(FILE ${all_frags})
		if(RGL_VK_AVAILABLE)
			shader_compile("${FILE}" "fragment" "Vulkan" "spv" "")
		endif()
		if(RGL_DX12_AVAILABLE)
			shader_compile("${FILE}" "fragment" "Direct3D12" "hlsl" "")
			target_sources(${dir} PUBLIC ${outname})
			set_source_files_properties(${outname} PROPERTIES 
				GENERATED TRUE
				VS_SHADER_MODEL "6.4"
				VS_SHADER_TYPE "Pixel"
				VS_SHADER_ENABLE_DEBUG ON
				VS_SHADER_DISABLE_OPTIMIZATIONS ON
			)
		endif()
		if(RGL_MTL_AVAILABLE)
			shader_compile("${FILE}" "fragment" "Metal" "metal" "")
			target_sources(${dir} PUBLIC ${outname})
			set_source_files_properties(${outname} PROPERTIES 
				GENERATED TRUE
				HEADER_FILE_ONLY OFF
				LANGUAGE METAL
				XCODE_EXPLICIT_FILE_TYPE "sourcecode.metal"
                COMPILE_FLAGS "-gline-tables-only -frecord-sources" # enable shader source debugging
			)
		endif()
	endforeach()

	add_custom_target(${dir}_CompileShaders
		DEPENDS ${all_shaders}
	)
	add_dependencies(${dir} ${dir}_CompileShaders)
	add_dependencies(${dir}_CompileShaders rglc)
endmacro()

add_sample(01-HelloTriangle)
add_sample(02-Cubes)
add_sample(03-Imgui)
add_sample(04-Deferred)
add_sample(05-XR)
add_sample(06-Asteroids)

target_link_libraries(05-XR
	PUBLIC
		openxr_loader
)

target_link_libraries(06-Asteroids
	PUBLIC
		tinyobjloader
)

set_target_properties(
	${SDLMAIN_LIB}
	"SDL2-static"
	"sdl_headers_copy"
	"RGL-Samples-Common"
    "fmt"
	"stb_image"
	"imgui"
	"xr_global_generated_files"
	"generate_openxr_header"
	"openxr_loader"
	PROPERTIES 
	FOLDER "RGL Samples Dependencies"
)
