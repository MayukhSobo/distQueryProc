cmake_minimum_required(VERSION 3.20)
project(ComputeEngine)

set(CMAKE_CXX_STANDARD 20)

# Include directories for xsimd, protobuf, and your header files
include_directories("${CMAKE_SOURCE_DIR}/xsimd/include")
include_directories("${CMAKE_SOURCE_DIR}/include")  # Include your headers from the include directory

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
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating C++ source and header from ${PROTO_FILE}"
    VERBATIM
)

# Add the generated files to the project
add_library(proto_files STATIC ${GENERATED_PROTO_SRCS})

# Add the source files for the engine
set(SOURCES
    src/main.cpp
    src/engine.cpp
    src/file_processor.cpp
    ${GENERATED_PROTO_SRCS}  # Include the generated .pb.cc file here
)

# Create the executable target for the engine
add_executable(engine ${SOURCES})

# Ensure that the proto files are generated before building the engine
add_dependencies(engine proto_files)

# Link against protobuf
target_link_libraries(engine ${Protobuf_LIBRARIES} proto_files)
