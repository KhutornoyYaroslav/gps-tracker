cmake_minimum_required(VERSION 3.12)

project(gps_thirdparty)

cmake_policy(VERSION 3.12)

set (EXTERNAL_DEPENDENCIES)

include(ExternalProject)

set(DEPENDENCIES_CXX_DEBUG_FLAGS "/MDd /O2 /Ob2")
set(DEPENDENCIES_CXX_RELEASE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")

option(EXTRACT_DEPENDENCIES "If enabled - precompiled dependencies will be unpacked, if possible" OFF)

add_custom_target(target_create_install_directory
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DEPENDENCIES_INSTALL_DIR})

if(NOT WIN32)
	if(EXTRACT_DEPENDENCIES)
		message(FATAL_ERROR "Extracting unsupported on not Windows systems")
	endif()

else()
	## spdlog ##
	find_package(spdlog PATHS ${DEPENDENCIES_INSTALL_DIR})
	option(FORCE_REBUILD_SPDLOG "" OFF)

	if(NOT spdlog_FOUND AND NOT EXTRACT_DEPENDENCIES OR FORCE_REBUILD_SPDLOG)
	     externalproject_add(spdlog_dep
	             DOWNLOAD_DIR ${DEPENDENCIES_DOWNLOAD_DIR}
	             SOURCE_DIR ${DEPENDENCIES_SOURCE_DIR}/spdlog
	#             GIT_REPOSITORY https://github.com/gabime/spdlog.git
	#             GIT_TAG v1.5.0
	#             GIT_SHALLOW true
	             URL https://github.com/gabime/spdlog/archive/v1.5.0.zip
	             URL_MD5 6d2169d76d699cabb7bb3696cb3e5a6e
	             EXCLUDE_FROM_ALL FALSE
	             CMAKE_ARGS
	             -DCMAKE_BUILD_TYPE=${DEPENDENCIES_BUILD_TYPE}
	             -DCMAKE_CXX_FLAGS_DEBUG=${DEPENDENCIES_CXX_DEBUG_FLAGS}
	             -DCMAKE_CXX_FLAGS_RELEASE=${DEPENDENCIES_CXX_RELEASE_FLAGS}
	             -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
	             -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
	             -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
	             -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
	             -DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_INSTALL_DIR}
	             #-DBUILD_SHARED_LIBS=ON
	             #------------------
	             #-DSPDLOG_BUILD_SHARED=ON not supported under windows
	             -DSPDLOG_BUILD_TESTS=OFF
	             -DSPDLOG_BUILD_EXAMPLE=OFF
	             )
	     LIST(APPEND EXTERNAL_DEPENDENCIES spdlog_dep)
	elseif(NOT spdlog_FOUND AND EXTRACT_DEPENDENCIES)
		add_custom_target(spdlog_dep
	        DEPENDS target_create_install_directory
	        COMMAND ${CMAKE_COMMAND} -E tar xzf ${DEPENDENCIES_DIR}/precompiled/spdlog.zip
	        WORKING_DIRECTORY ${DEPENDENCIES_INSTALL_DIR})
	    LIST(APPEND EXTERNAL_DEPENDENCIES spdlog_dep)
	endif()
	## spdlog end ##

	## opencv ##
	find_package(OpenCV 4.3.0 COMPONENTS core highgui imgproc imgcodecs
			PATHS ${DEPENDENCIES_INSTALL_DIR} )
	option(FORCE_REBUILD_OPENCV "" OFF)

	set(_depends)
	if(TARGET protobuf_dep)
		message(STATUS "Adding protobuf_dep to opencv DEPENDS cause it is going to be built")
		set(_depends  protobuf_dep)
	endif()
	if(TARGET freetype_dep)
		message(STATUS "Adding freetype_dep to opencv DEPENDS cause it is going to be built")
		set(_depends freetype_dep)
	endif()

	if(NOT OpenCV_FOUND AND NOT EXTRACT_DEPENDENCIES OR FORCE_REBUILD_OPENCV)
		externalproject_add(opencv_contrib_dep
				DOWNLOAD_DIR ${DEPENDENCIES_DOWNLOAD_DIR}
				SOURCE_DIR ${DEPENDENCIES_SOURCE_DIR}/opencv_contrib
				URL https://github.com/opencv/opencv_contrib/archive/4.3.0.tar.gz
				URL_MD5 924ed270a66b5234442cf84da8c7e4b1
				DOWNLOAD_NAME opencv_contrib_4.3.0.tar.gz
				EXCLUDE_FROM_ALL FALSE
				CONFIGURE_COMMAND ""
				BUILD_COMMAND ""
				INSTALL_COMMAND ""
				)

		externalproject_add(opencv_dep
				DOWNLOAD_DIR ${DEPENDENCIES_DOWNLOAD_DIR}
				SOURCE_DIR ${DEPENDENCIES_SOURCE_DIR}/opencv
				URL https://github.com/opencv/opencv/archive/4.3.0.tar.gz
				URL_MD5 559953ef6ffaad03421cbba7f31d891c
				DOWNLOAD_NAME opencv_4.3.0.tar.gz
				EXCLUDE_FROM_ALL FALSE
				STEP_TARGETS configure
				DEPENDS opencv_contrib_dep ${_depends}
	#            PATCH_COMMAND ${CMAKE_COMMAND} -E copy_if_different
	#                ${CMAKE_CURRENT_SOURCE_DIR}/patch/opencv/OpenCVInstallLayout.cmake
	#                ${DEPENDENCIES_SOURCE_DIR}/opencv/cmake/
				CMAKE_ARGS
				-DCMAKE_BUILD_TYPE=${DEPENDENCIES_BUILD_TYPE}
				-DCMAKE_CXX_FLAGS_DEBUG=${DEPENDENCIES_CXX_DEBUG_FLAGS}
				-DCMAKE_CXX_FLAGS_RELEASE=${DEPENDENCIES_CXX_RELEASE_FLAGS}
				-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
				-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
				-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
				-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
				-DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_INSTALL_DIR}
				#-DBUILD_SHARED_LIBS=ON
				#------------------
				-DBUILD_EXAMPLES=OFF
				-DBUILD_opencv_apps=OFF
				-DBUILD_DOCS=OFF
				-DBUILD_PERF_TESTS=OFF
				-DBUILD_TESTS=OFF
				-DBUILD_JAVA=OFF
				-DBUILD_PROTOBUF=OFF
				-DBUILD_TIFF=OFF
				-DBUILD_WEBP=OFF
				-DBUILD_LIST=core,highgui,imgcodecs,imgproc
	#            -DBUILD_opencv_calib3d=OFF
	#            -DBUILD_opencv_java_bindings_generator=OFF
	#            -DBUILD_opencv_python_bindings_generator=OFF
	#            -DBUILD_opencv_python_tests=OFF
				-DBUILD_opencv_world=ON
				-DINSTALL_CREATE_DISTRIB=ON
				-DOPENCV_EXTRA_MODULES_PATH=${DEPENDENCIES_SOURCE_DIR}/opencv_contrib/modules
				-DWITH_FREETYPE=ON
				-DCV_TRACE=OFF
				)

		LIST(APPEND EXTERNAL_DEPENDENCIES opencv_dep)

	elseif(NOT OpenCV_FOUND AND EXTRACT_DEPENDENCIES)
	#    message(FATAL_ERROR "Not implemented yet")
		add_custom_target(opencv_dep
				DEPENDS target_create_install_directory
				COMMAND ${CMAKE_COMMAND} -E tar xzf ${DEPENDENCIES_DIR}/precompiled/opencv_4_3_0.zip
				WORKING_DIRECTORY ${DEPENDENCIES_INSTALL_DIR})
		LIST(APPEND EXTERNAL_DEPENDENCIES opencv_dep)
	endif()
	unset(_depends)
	## opencv end ##
endif()
add_custom_target(dependencies_build ALL)
if(EXTERNAL_DEPENDENCIES)
    add_dependencies(dependencies_build ${EXTERNAL_DEPENDENCIES})
endif()