cmake_minimum_required(VERSION 3.17)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR} CACHE INTERNAL "")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)

if(MSVC)
	add_definitions(/MP)
endif()

project(RGL-Samples)

if (CMAKE_SYSTEM_NAME MATCHES "Emscripten")
	set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE INTERNAL "")
	# required for higher memory, atomics, and threads
	add_definitions(-fexceptions)
	add_definitions(-pthread)
	add_definitions(-sUSE_SDL=2)
        set(EM_LINK
                        "-s USE_WEBGPU"
						"--preload-file \"${CMAKE_BINARY_DIR}/shaders/\"@shaders"
                        "-fexceptions"
						"-s USE_PTHREADS"
						"-s PROXY_TO_PTHREAD"
                        "-s LLD_REPORT_UNDEFINED"
                        "-s NO_DISABLE_EXCEPTION_CATCHING"
                        "-s NO_DISABLE_EXCEPTION_THROWING"
                        "-s PTHREAD_POOL_SIZE=4"
                        "-s ASSERTIONS=1"
                        "-s USE_SDL=2"
                        "-s WASM_BIGINT"
						"-s ASYNCIFY"			# required for WebGPU!!
                        "-s TOTAL_MEMORY=150MB"
	)
else()
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
endif()

add_subdirectory(deps/glm EXCLUDE_FROM_ALL)
add_subdirectory(deps/fmt EXCLUDE_FROM_ALL)
add_subdirectory(deps/stb_image EXCLUDE_FROM_ALL)
add_subdirectory(deps/imgui-cmake EXCLUDE_FROM_ALL)
add_subdirectory(deps/tinyobjloader EXCLUDE_FROM_ALL)

if (NOT CMAKE_SYSTEM_NAME MATCHES "Emscripten")
	add_subdirectory(deps/OpenXR-SDK EXCLUDE_FROM_ALL)
endif()

if(NOT (CMAKE_VS_PLATFORM_NAME STREQUAL ""))
	if(CMAKE_VS_PLATFORM_NAME_DEFAULT STREQUAL CMAKE_VS_PLATFORM_NAME)
		set(VS_CROSSCOMP OFF CACHE INTERNAL "")
	else()
		set(VS_CROSSCOMP ON CACHE INTERNAL "")
	endif()
else()
	set(VS_CROSSCOMP OFF CACHE INTERNAL "")
endif()

if(CMAKE_CROSSCOMPILING OR (VS_CROSSCOMP AND WIN32))
	set(RGL_ENABLE_RGLC OFF CACHE INTERNAL "")
	set(TOOLS_DIR "${CMAKE_BINARY_DIR}/host-tools" CACHE INTERNAL "")
	file(MAKE_DIRECTORY ${TOOLS_DIR})

	set(CC_GENERATOR "-G \"${CMAKE_GENERATOR}\"")
	if (CMAKE_SYSTEM_NAME MATCHES "Emscripten")
		if (CMAKE_HOST_WIN32)
			set(CC_GENERATOR "")		# otherwise it'll try to use ninja/make which may not exist in that context
		endif()
		set(HT_ENABLE_WGSL "-DRGL_ENABLE_WEBGPU=ON")	# otherwise the separate compiler exe will not know that WGSL output should be enabled
	endif()

	if (APPLE)	# no idea why this is required, but without it, the host tools get generated for iOS, even though nowhere in the command is that specified
		set(RGL_APPLE_HOST "-DCMAKE_SYSTEM_NAME=\"Darwin\"")
	endif()

	add_custom_command(
		PRE_BUILD
		WORKING_DIRECTORY ${TOOLS_DIR}
		COMMAND ${CMAKE_COMMAND} ${CC_GENERATOR} ${RGL_APPLE_HOST} -DCMAKE_BUILD_TYPE=Release ${HT_ENABLE_WGSL} "${CMAKE_CURRENT_SOURCE_DIR}/deps/RGL"
		OUTPUT "${TOOLS_DIR}/CMakeCache.txt"
	)
	if(MSVC)
		set(rglc_ext ".exe")
	endif()
	set(rglc_path "${TOOLS_DIR}/Release/rglc${rglc_ext}")
	add_custom_command(
		PRE_BUILD
		DEPENDS "${TOOLS_DIR}/CMakeCache.txt"
		OUTPUT "${rglc_path}" 
		COMMAND ${CMAKE_COMMAND} --build . --config Release --target rglc
		WORKING_DIRECTORY "${TOOLS_DIR}"
		VERBATIM
	)
	add_custom_target("rglc" DEPENDS "${rglc_path}")
endif()
set(RGL_DISABLE_DX OFF CACHE INTERNAL "")
set(RGL_DISABLE_VK OFF CACHE INTERNAL "")
set(RGL_RUNTIME_COMPILATION ON CACHE INTERNAL "")
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
endif()

if(NOT CMAKE_SYSTEM_NAME MATCHES "Emscripten")
	set(SDLLIB "SDL2-static")
	set(SDL_HEADERS "sdl_headers_copy")
endif()
if(NOT CMAKE_SYSTEM_NAME MATCHES "Emscripten" AND NOT CMAKE_SYSTEM_NAME MATCHES "WindowsStore")
	set(SDLMAIN_LIB "SDL2main")
endif()

target_link_libraries(${PROJECT_NAME}-Common 
	PUBLIC
		${SDLLIB} ${SDLMAIN_LIB}
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
	endif()

	add_executable(${dir} ${SOURCES} ${ADDITIONALS} "iosLaunchScreen.storyboard")

	if (NOT APPLE)
		foreach(FILE ${RESOURCES})
			get_filename_component(output_name "${FILE}" NAME)
			set(outname "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/${output_name}")
			add_custom_command(PRE_BUILD 
				OUTPUT "${outname}" 
				COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FILE} "${outname}"
				DEPENDS ${FILE}
			)
			list(APPEND target_assets "${outname}")
		endforeach()
	endif()

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
	target_link_libraries(${dir} PRIVATE ${PROJECT_NAME}-Common ${EM_LINK})

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

	file(GLOB all_shaders "${dir}/*.vert" "${dir}/*.frag" "${dir}/*.comp")

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
		if(RGL_WEBGPU_AVAILABLE)
			shader_compile("${FILE}" "vertex" "WebGPU" "wgsl" "")
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
		if(RGL_WEBGPU_AVAILABLE)
			shader_compile("${FILE}" "fragment" "WebGPU" "wgsl" "")
		endif()
	endforeach()
	file(GLOB all_computes "${dir}/*.comp")
	foreach(FILE ${all_computes})
		if(RGL_VK_AVAILABLE)
			shader_compile("${FILE}" "compute" "Vulkan" "spv" "")
		endif()
		if(RGL_DX12_AVAILABLE)
			shader_compile("${FILE}" "compute" "Direct3D12" "hlsl" "")
			target_sources(${dir} PUBLIC ${outname})
			set_source_files_properties(${outname} PROPERTIES 
				GENERATED TRUE
				VS_SHADER_MODEL "6.4"
				VS_SHADER_TYPE "Compute"
				VS_SHADER_ENABLE_DEBUG ON
				VS_SHADER_DISABLE_OPTIMIZATIONS ON
			)
		endif()
		if(RGL_MTL_AVAILABLE)
			shader_compile("${FILE}" "compute" "Metal" "metal" "")
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
		DEPENDS ${all_shaders} ${target_assets}
	)
	add_dependencies(${dir} ${dir}_CompileShaders)
	add_dependencies(${dir}_CompileShaders rglc)
endmacro()

add_sample(01-HelloTriangle)
add_sample(02-Cubes)
add_sample(03-Imgui)
add_sample(04-Deferred)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")				# OpenXR only on windows fornow
	add_sample(05-XR)
	target_link_libraries(05-XR
	PUBLIC
		openxr_loader
	)
endif()
add_sample(06-Asteroids)


target_link_libraries(06-Asteroids
	PUBLIC
		tinyobjloader
)

if (NOT CMAKE_SYSTEM_NAME MATCHES "Emscripten")
	set(EM_PROJECTS "xr_global_generated_files" "generate_openxr_header" "openxr_loader")
endif()

set_target_properties(
	${SDLMAIN_LIB}
	${SDLLIB}
	${SDL_HEADERS}
	"RGL-Samples-Common"
    "fmt"
	"stb_image"
	"imgui"
	"tinyobjloader"
	${EM_PROJECTS}
	PROPERTIES 
	FOLDER "RGL Samples Dependencies"
)
