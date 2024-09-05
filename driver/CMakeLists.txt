cmake_minimum_required(VERSION 3.20)
project(DriverApp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories for Protobuf and header files
include_directories("${CMAKE_SOURCE_DIR}/include")

# Find the protobuf package
find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})

# Define the proto file and the output paths
set(PROTO_FILE "${CMAKE_SOURCE_DIR}/proto/message.proto")
set(GENERATED_PROTO_SRCS "${CMAKE_SOURCE_DIR}/src/message.pb.cc")
set(GENERATED_PROTO_HDRS "${CMAKE_SOURCE_DIR}/include/message.pb.h")

# Custom command to generate C++ source and header from .proto
add_custom_command(
    OUTPUT ${GENERATED_PROTO_SRCS} ${GENERATED_PROTO_HDRS}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --cpp_out=${CMAKE_SOURCE_DIR}/src --proto_path=${CMAKE_SOURCE_DIR}/proto ${PROTO_FILE}
    COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_SOURCE_DIR}/src/message.pb.h ${GENERATED_PROTO_HDRS}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating C++ source in src/ and header in include/ from ${PROTO_FILE}"
    VERBATIM
)

# Add the generated files to the project
add_library(proto_files STATIC ${GENERATED_PROTO_SRCS})

# Add the source files for the driver
set(SOURCES
    src/main.cpp
    src/driver.cpp
    src/file_processor.cpp
    src/save_task.cpp
    ${GENERATED_PROTO_SRCS}
)

set(CMAKE_BUILD_TYPE Debug)

# Create the executable target for the driver
add_executable(driver ${SOURCES})

# Ensure that the proto files are generated before building the driver
add_dependencies(driver proto_files)

# Link against protobuf
target_link_libraries(driver ${Protobuf_LIBRARIES} proto_files)
