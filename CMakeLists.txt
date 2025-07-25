cmake_minimum_required(VERSION 3.11)
project(raisim VERSION 1.1.8 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include CMake helper modules
include(GNUInstallDirs)

# Add custom cmake modules (optional)

# Determine target platform
set(RAISIM_OS linux)

# Add Raisim library path
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/raisim/${RAISIM_OS})
message("-- Detected OS/Architecture is ${RAISIM_OS}")

# Install library and header files
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/raisim/${RAISIM_OS}/lib
        DESTINATION ${CMAKE_INSTALL_PREFIX})

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/raisim/${RAISIM_OS}/include
        DESTINATION ${CMAKE_INSTALL_PREFIX})

add_subdirectory(src)