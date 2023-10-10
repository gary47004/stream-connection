project(server)
set(CMAKE_CXX_STANDARD 17)
cmake_minimum_required(VERSION 3.13)

include(/app/stream-connection/cmake/grpc.cmake)

file(GLOB _grpc_cc $(/app/stream-connection/proto/*.cc))
message(STATUS "grpc cc: ${_grpc_cc}")

include_directories("/app/stream-connection/")
link_directories("/app/stream-connection/")

add_executable(server "stream_server.cpp" ${_grpc_cc})
target_link_libraries(server
${_REFLECTION}
${_GRPC_GRPCPP}
${_PROTOBUF_LIBPROTOBUF})
