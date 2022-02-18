cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-Wall -ggdb")

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

add_library(cache STATIC Cache.cpp)
target_link_libraries(cache mariadbcpp)

add_library(forecache STATIC forecache.cpp)
target_link_libraries(forecache pthread)

add_library(block STATIC Block.cpp)

add_library(entry STATIC Entry.cpp)
target_link_libraries(entry PRIVATE block)

add_library(manager STATIC Manager.cpp)
target_link_libraries(manager PRIVATE forecache)

add_executable(test test.cpp)
target_link_libraries(test sftp entry manager cache)

add_executable(reset reset.cpp)
target_link_libraries(reset mariadbcpp)

add_executable(${PROJECT_NAME} blockfs.cpp)
target_link_libraries(${PROJECT_NAME} fuse3 sftp entry manager cache)
