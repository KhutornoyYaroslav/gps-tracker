cmake_minimum_required(VERSION 3.12)

project(coreLib CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED)

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

### SPDLOG DEPENDENCY ###
if(WIN32)
	find_package(spdlog REQUIRED PATHS ${DEPENDENCIES_INSTALL_DIR})
else()
	find_package(spdlog REQUIRED)
endif()
LIST(APPEND ${PROJECT_NAME}_LIBRARIES spdlog::spdlog)
### SPDLOG DEPENDENCY END ###

### OPENCV DEPENDENCY ###
if(WIN32)
	find_package(OpenCV 4.3.0 COMPONENTS core highgui imgproc imgcodecs
			PATHS ${DEPENDENCIES_INSTALL_DIR} )
else()
	find_package(OpenCV 3.4 COMPONENTS core highgui imgproc imgcodecs)
endif()
LIST(APPEND ${PROJECT_NAME}_LIBRARIES ${OpenCV_LIBS})
### OPENCV DEPENDENCY END ###

### QT DEPENDENCY ###
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt5 COMPONENTS Core Xml REQUIRED)
LIST(APPEND ${PROJECT_NAME}_LIBRARIES Qt5::Core Qt5::Xml)
### QT DEPENDENCY END ###

### Library target
FILE(GLOB_RECURSE ${PROJECT_NAME}_SOURCES
        src/*.cpp
        src/*.h
        include/*.h)

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC include)

#set_target_properties(${PROJECT_NAME} PROPERTIES
#        PUBLIC_HEADER "${${PROJECT_NAME}_PUBLIC_HEADERS}")
target_compile_definitions(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_DEFINITIONS})
target_link_libraries(${PROJECT_NAME} ${${PROJECT_NAME}_LIBRARIES})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} Pdh.lib)
endif()

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()