cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

IF (WIN32)
	set(CMAKE_TOOLCHAIN_FILE "C:/Users/svyut/vcpkg/scripts/buildsystems/vcpkg.cmake")
	project(blockfs)
	find_package(libssh CONFIG REQUIRED)
ELSE()
	include_directories(/usr/include/)
	link_directories(/usr/lib/x86_64-linux-gnu/)
	project(blockfs)
ENDIF()

add_library(sftp STATIC Sftp.cpp)
target_link_libraries(sftp PRIVATE ssh)

add_library(block STATIC Block.cpp)

add_library(entry STATIC Entry.cpp)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} sftp block entry)