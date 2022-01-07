cmake_minimum_required(VERSION 3.22.1)

IF (WIN32)
	set(CMAKE_TOOLCHAIN_FILE "C:/Users/svyut/vcpkg/scripts/buildsystems/vcpkg.cmake")
	project(blockfs)
	add_executable(${PROJECT_NAME} main.cpp)
	find_package(libssh CONFIG REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE ssh)
ELSE()
  # set stuff for other systems
ENDIF()
