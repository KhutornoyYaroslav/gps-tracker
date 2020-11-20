cmake_minimum_required(VERSION 3.12)

project(gpstracker)
set(APPLICATION_NAME GpsTracker)

set (QT_PATH "" CACHE PATH "Path to QT")
if(WIN32 AND NOT QT_PATH)
	message(FATAL_ERROR "Please set path to qt msvc library. Example: \"C:/Qt/5.12.5/msvc2017_64\"")
endif()

LIST(APPEND CMAKE_PREFIX_PATH ${QT_PATH})
LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

if(WIN32 AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "" CACHE STRING "Build type")
endif()

if(NOT CMAKE_BUILD_TYPE)
	message(FATAL_ERROR "CMAKE_BUILD_TYPE was not set, please set it manually")
endif()
message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Third party section
if(NOT DEPENDENCIES_BUILD_TYPE)
	set(DEPENDENCIES_BUILD_TYPE ${CMAKE_BUILD_TYPE})
endif()

set(CMAKE_INSTALL_PREFIX
		"${CMAKE_CURRENT_SOURCE_DIR}/install/${CMAKE_SYSTEM_PROCESSOR}-${APPLICATION_NAME_UPPERCASE}-${DEPENDENCIES_BUILD_TYPE}")

if(NOT DEPENDENCIES_DIR)
	set(DEPENDENCIES_DIR ${PROJECT_SOURCE_DIR}/thirdParty)
	message(STATUS "Dependencies directory is " ${DEPENDENCIES_DIR})
endif()

set(DEPENDENCIES_DOWNLOAD_DIR ${DEPENDENCIES_DIR}/download)
set(DEPENDENCIES_SOURCE_DIR   ${DEPENDENCIES_DIR}/src)
set(DEPENDENCIES_INSTALL_DIR  ${DEPENDENCIES_DIR}/install)

# Main section
option(BUILD_DEPENDENCIES "Build only third party libraries" ON)

if (BUILD_DEPENDENCIES)
	message(STATUS "Building dependencies...")
    add_subdirectory(thirdParty)
else()
	message(STATUS "Building sources...")
    add_subdirectory(src)
endif()