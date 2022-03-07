# Copyright 2018 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# cmake build file for C++ helloworld example.
# Assumes protobuf and gRPC have been installed using cmake.
# See cmake_externalproject/CMakeLists.txt for all-in-one cmake build
# that automatically builds all the dependencies before building helloworld.

cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-Wall -ggdb")

project(UserTaste C CXX)

include(./common.cmake)

# Proto file
get_filename_component(ut_proto "./protos/usertaste.proto" ABSOLUTE)
get_filename_component(ut_proto_path "${ut_proto}" PATH)

# Generated sources
set(ut_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/usertaste.pb.cc")
set(ut_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/usertaste.pb.h")
set(ut_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/usertaste.grpc.pb.cc")
set(ut_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/usertaste.grpc.pb.h")
add_custom_command(
      OUTPUT "${ut_proto_srcs}" "${ut_proto_hdrs}" "${ut_grpc_srcs}" "${ut_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${ut_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${ut_proto}"
      DEPENDS "${ut_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# ut_grpc_proto
add_library(ut_grpc_proto
  ${ut_grpc_srcs}
  ${ut_grpc_hdrs}
  ${ut_proto_srcs}
  ${ut_proto_hdrs})
target_link_libraries(ut_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

include_directories("./include")

add_library(sftp STATIC ./src/Sftp.cpp)
target_link_libraries(sftp PRIVATE ssh)

add_library(cache STATIC ./src/Cache.cpp)
target_link_libraries(cache mariadbcpp)

add_executable(usertaste_client "usertaste_client.cpp")
target_link_libraries(usertaste_client ut_grpc_proto ${_REFLECTION}
    ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF} sftp cache)

# Targets greeter_[async_](client|server)
#foreach(_target
#  greeter_client greeter_server 
#  greeter_callback_client greeter_callback_server 
#  greeter_async_client greeter_async_client2 greeter_async_server)
#  add_executable(${_target} "${_target}.cc")
#  target_link_libraries(${_target}
#    hw_grpc_proto
#    ${_REFLECTION}
#    ${_GRPC_GRPCPP}
#    ${_PROTOBUF_LIBPROTOBUF})
#endforeach()
