cmake_minimum_required(VERSION 3.16.1)
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

include_directories("./include")

add_library(sftp STATIC ./src/Sftp.cpp)
target_link_libraries(sftp PRIVATE ssh)

add_library(cache STATIC ./src/Cache.cpp)
target_link_libraries(cache mariadbcpp)

add_library(forecache STATIC ./src/Forecache.cpp)
target_link_libraries(forecache pthread)

add_library(block STATIC ./src/Block.cpp)

add_library(entry STATIC ./src/Entry.cpp)
target_link_libraries(entry PRIVATE block)

add_library(manager STATIC ./src/Manager.cpp)
target_link_libraries(manager PRIVATE forecache)

add_executable(test ./src/Test.cpp)
target_link_libraries(test sftp entry manager cache)

add_executable(reset ./src/Reset.cpp)
target_link_libraries(reset mariadbcpp)

add_executable(${PROJECT_NAME} ./src/Blockfs.cpp)
target_link_libraries(${PROJECT_NAME} fuse3 sftp entry manager cache)
