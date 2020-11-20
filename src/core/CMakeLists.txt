cmake_minimum_required(VERSION 3.12)

project(${APPLICATION_NAME} CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED)

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

### coreLib DEPENDENCY ###
LIST(APPEND ${PROJECT_NAME}_LIBRARIES coreLib)
### coreLib DEPENDENCY END ###

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

### Executable target
FILE(GLOB_RECURSE ${PROJECT_NAME}_SOURCES
        src/*.cpp
        src/*.h
        include/*.h)
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})

target_include_directories(${PROJECT_NAME}
        PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS} include)

target_compile_definitions(${PROJECT_NAME}
        PRIVATE ${${PROJECT_NAME}_DEFINITIONS} PROJECT_NAME="${PROJECT_NAME}")
if(MSVC)
	target_compile_definitions(${PROJECT_NAME}
			PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_LIBRARIES})

### POSTBUILD ###
if(WIN32)
	LIST(APPEND DEPENDENCIES_TO_BE_COPIED
			$<TARGET_FILE:opencv_world>
			$<TARGET_FILE:Qt5::Core>
			$<TARGET_FILE:Qt5::Xml>
			# $<TARGET_FILE:SomeOtherComponent>
			)

	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${DEPENDENCIES_TO_BE_COPIED}
			$<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()

### Installing target
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
### Installing dependencies
install(FILES ${DEPENDENCIES_TO_BE_COPIED} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)