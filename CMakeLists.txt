# CMake version specified
cmake_minimum_required(VERSION 3.21...3.41)

# Project declaration of base project.
project(
        VFS
        VERSION 0.1
        DESCRIPTION "Virtual file system that is able to mount different file systems, locally or remotely."
        LANGUAGES CXX)

# Only perform if this is the root project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(EXECUTABLE_NAME filesystem)   # Define executable name for project
  set(CMAKE_CXX_STANDARD 17)                # Define the C++ standard
  message("-- C++ standard is set to -std=c++${CMAKE_CXX_STANDARD}")
  set(CMAKE_CXX_EXTENSIONS off)             # Ensures -std=c++xx is provided, rather than -std=g++xx

  set(SOURCE_FOLDER vfs)                    # Defining source folder for project to generate executable
  add_subdirectory(${SOURCE_FOLDER})        # Adding and declaring specification for executable code

# Location for building library folders

  message("-- ${PROJECT_NAME} [${PROJECT_VERSION}] build files has been defined")
else()
  message("-- ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt must be the root config and not added as a subdirectory")
endif()