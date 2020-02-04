set(CMAKE_LEGACY_CYGWIN_WIN32 0)
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
project(hfst-ospell CXX)

# Release or Debug
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MACOSX_RPATH ON)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/configure.ac" _VERSION_FILE)
string(REGEX REPLACE ".*AC_INIT\\(\\[hfstospell\\], \\[([0-9]+)\\.([0-9]+)\\.([0-9]+)\\].*" "\\1.\\2.\\3" VERSION ${_VERSION_FILE})
string(REGEX REPLACE ".*AC_INIT\\(\\[hfstospell\\], \\[([0-9]+).*" "\\1" SOVERSION ${_VERSION_FILE})

option(BUILD_SHARED_LIBS "Set to OFF to use static library" ON)

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /std:c++latest /permissive- /W4 /MP")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
else()
	set(_FLAGS_COMMON "-Wall -Wextra -Wno-missing-field-initializers -Wno-deprecated -Wno-unused-parameter -fPIC")

	include(CheckCXXCompilerFlag)

	foreach(flag "-Wno-unused-result" "-flto")
		string(REGEX REPLACE "[^A-Za-z0-9]" "-" _flag ${flag})
		CHECK_CXX_COMPILER_FLAG(${flag} COMPILER_SUPPORTS_${_flag})
		if(COMPILER_SUPPORTS_${_flag})
			set(_FLAGS_COMMON "${_FLAGS_COMMON} ${flag}")
		endif()
	endforeach()
	if(COMPILER_SUPPORTS_flto)
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")
	endif()

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_FLAGS_COMMON} -fvisibility-inlines-hidden")

	# Require latest possible C++ standard
	foreach(flag "-std=c++20" "-std=c++2a" "-std=c++17" "-std=c++1z")
		string(REGEX REPLACE "[^a-z0-9]" "-" _flag ${flag})
		CHECK_CXX_COMPILER_FLAG(${flag} COMPILER_SUPPORTS_${_flag})
		if(COMPILER_SUPPORTS_${_flag})
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
			set(_ENABLED_CXX ${flag})
			break()
		endif()
	endforeach()
	if(NOT _ENABLED_CXX)
		message(FATAL_ERROR "Could not enable at least C++17 (C++1z) - upgrade your compiler")
	endif()
endif()

include(GNUInstallDirs)

include(TestBigEndian)
TEST_BIG_ENDIAN(BIG_ENDIAN)
if(BIG_ENDIAN)
	add_definitions(-DWORDS_BIGENDIAN)
endif()

if(WIN32)
	add_definitions(-D_SECURE_SCL=0 -D_ITERATOR_DEBUG_LEVEL=0 -D_CRT_SECURE_NO_DEPRECATE -DWIN32_LEAN_AND_MEAN -DVC_EXTRALEAN -DNOMINMAX)
endif()

# Boost
find_package(Boost 1.53.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# getopt
find_path(GETOPT_INCLUDE getopt.h)
find_library(GETOPT_LIB NAMES getopt)
include_directories(${GETOPT_INCLUDE})

# libarchive
find_package(LibArchive REQUIRED)
include_directories(${LibArchive_INCLUDE_DIRS})

# ICU
find_package(ICU REQUIRED)
if(ICU_FOUND)
	if(ICU_VERSION VERSION_LESS "50.0")
		message(FATAL_ERROR "ICU is too old - ${ICU_VERSION} was found, but we require 50.0 or newer")
	endif()
	include_directories(${ICU_INCLUDE_DIRS})
	message(STATUS "Found ICU version ${ICU_VERSION}")
endif()

# Shared defines
add_definitions(
	-DHAVE_GETOPT_H
	-DHAVE_LIBARCHIVE
	-DUSE_LIBARCHIVE_2
	-DZHFST_EXTRACT_TO_MEM=1
	"-DPACKAGE_NAME=\"hfstospell\""
	"-DPACKAGE_BUGREPORT=\"hfst-bugs@helsinki.fi\""
	"-DPACKAGE_STRING=\"hfstospell ${VERSION}\""
	)

# Library libhfstospell
add_library(hfstospell
	hfst-ol.cc
	ospell.cc
	ZHfstOspeller.cc
	ZHfstOspellerXmlMetadata.cc
	)
target_link_libraries(hfstospell PRIVATE ${LibArchive_LIBRARIES})
set_target_properties(hfstospell PROPERTIES COMPILE_DEFINITIONS LIBHFSTOSPELL_EXPORTS=1)

# Binary hfst-ospell
add_executable(hfst-ospell main.cc)
target_link_libraries(hfst-ospell PRIVATE hfstospell)
if(GETOPT_LIB)
	target_link_libraries(hfst-ospell PRIVATE ${GETOPT_LIB})
endif()

# Library libhfstospelloffice
add_library(hfstospelloffice liboffice.cc liboffice.h)
target_link_libraries(hfstospelloffice PRIVATE hfstospell ${ICU_LIBRARIES} ${ICU_IO_LIBRARIES} ${ICU_I18N_LIBRARIES})
set_target_properties(hfstospelloffice PROPERTIES COMPILE_DEFINITIONS LIBHFSTOSPELLOFFICE_EXPORTS=1)

# Binary hfst-ospell-office
add_executable(hfst-ospell-office office.cc liboffice.h)
target_link_libraries(hfst-ospell-office PRIVATE hfstospelloffice)

set_target_properties(hfstospell hfstospelloffice
	PROPERTIES
	VERSION ${VERSION}
	SOVERSION ${SOVERSION}
	)

install(TARGETS hfstospell hfstospelloffice hfst-ospell hfst-ospell-office
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
