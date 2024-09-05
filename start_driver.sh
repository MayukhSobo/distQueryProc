#!/bin/bash

# Hardcoded base path for file processing
base_path="$PWD/data/student_scores"

# Expand the base path to an absolute path
base_path=$(realpath "$base_path")

# Check if at least one engine port is provided
if [ -z "$1" ]; then
  echo "Usage: ./start_driver.sh <engine_ports...>"
  exit 1
fi

# Get the engine ports passed as arguments
engine_ports="$@"

# Set the driver binary path in the ./driver/build directory
driver_binary="./driver/build/driver"

# Check if the driver binary exists
if [ ! -f "$driver_binary" ]; then
  echo "Driver binary not found. Attempting to build the driver..."

  # Navigate to the driver directory
  if [ ! -d "./driver" ]; then
    echo "Error: ./driver directory not found."
    exit 1
  fi

  # Run cmake and make to build the binary, CMake will handle the creation of the build directory
  echo "Running cmake and make..."
  cmake -S ./driver -B ./driver/build || { echo "Error: cmake failed."; exit 1; }
  cmake --build ./driver/build --clean-first || { echo "Error: make failed."; exit 1; }

  # Check if the driver binary was successfully created
  if [ ! -f "$driver_binary" ]; then
    echo "Error: Driver binary was not created successfully."
    exit 1
  fi
fi

# Set the FILE_PATHS environment variable and include the ./driver/build directory in the PATH
export FILE_PATHS="$base_path"
export PATH="$PATH:$(pwd)/driver/build"

# Start the driver with the specified base path and engine ports
echo "Starting driver with FILE_PATHS='$FILE_PATHS' and engine ports '$engine_ports'..."
$driver_binary $engine_ports

# Exit the script
exit 0
