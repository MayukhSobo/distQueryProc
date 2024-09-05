#!/bin/bash

# Check if a port number is provided
if [ -z "$1" ]; then
  echo "Usage: ./start_engine.sh <port_number>"
  exit 1
fi

# Assign the first argument to port_number
port_number=$1

# Set the engine binary path in the ./engine/build directory
engine_binary="./engine/build/engine"

# Check if the engine binary exists
if [ ! -f "$engine_binary" ]; then
  echo "Engine binary not found. Attempting to build the engine..."

  # Navigate to the engine directory
  if [ ! -d "./engine" ]; then
    echo "Error: ./engine directory not found."
    exit 1
  fi

  # Run cmake and make to build the binary, CMake will handle the creation of the build directory
  echo "Running cmake and make..."
  cmake -S ./engine -B ./engine/build || { echo "Error: cmake failed."; exit 1; }
  cmake --build ./engine/build --clean-first || { echo "Error: make failed."; exit 1; }

  # Check if the engine binary was successfully created
  if [ ! -f "$engine_binary" ]; then
    echo "Error: Engine binary was not created successfully."
    exit 1
  fi
fi

# Start the engine with the specified port number
echo "Starting engine on port $port_number..."
$engine_binary $port_number

# Exit the script
exit 0
