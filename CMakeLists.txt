cmake_minimum_required(VERSION 3.15)

project(SAMPPOpenMP LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(SAMPP_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/Build/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${SAMPP_OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${SAMPP_OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${SAMPP_OUTPUT_DIR}")

add_library(sampp_build_flags INTERFACE)
target_include_directories(sampp_build_flags INTERFACE
	"${PROJECT_SOURCE_DIR}/Include"
	"${PROJECT_SOURCE_DIR}/Include/amx"
	"${PROJECT_SOURCE_DIR}/Include/RakNet"
)
target_compile_definitions(sampp_build_flags INTERFACE
	SAMPGDK_AMALGAMATION
	HAVE_STDINT_H
)

if(WIN32)
	target_compile_definitions(sampp_build_flags INTERFACE
		WIN32
		_WINDOWS
		_USRDLL
		_CRT_SECURE_NO_WARNINGS
		_CRT_SECURE_NO_DEPRECATE
	)
else()
	target_compile_definitions(sampp_build_flags INTERFACE
		LINUX
		_LINUX
		_LINUX_
		_GNU_SOURCE
	)
	target_compile_options(sampp_build_flags INTERFACE
		-m32
		-fPIC
		-w
		$<$<COMPILE_LANGUAGE:CXX>:-fpermissive>
		$<$<COMPILE_LANGUAGE:C>:-Wno-pointer-to-int-cast>
		$<$<COMPILE_LANGUAGE:C>:-Wno-attributes>
	)
	target_link_options(sampp_build_flags INTERFACE -m32 -fshort-wchar)
endif()

if(MSVC)
	target_compile_options(sampp_build_flags INTERFACE /MP /W0)
endif()

add_subdirectory(Common)
add_subdirectory(Server)

option(OMPPLUS_BUILD_COMPONENT "Build the native open.mp component transport" ON)

if(OMPPLUS_BUILD_COMPONENT)
	set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Allow older third-party CMake policy ranges" FORCE)
	add_subdirectory(ThirdParty/openmp-sdk)
	add_subdirectory(ThirdParty/openmp-network)

	target_compile_features(OMP-SDK INTERFACE cxx_std_17)
	target_compile_features(OMP-Network PUBLIC cxx_std_17)

	add_subdirectory(components/omp-plus)
endif()
