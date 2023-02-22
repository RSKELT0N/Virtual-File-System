cmake_minimum_required(VERSION 3.21...3.41)

# Setting project definitions.
set(SOURCE_SRC src)
set(SOURCE_INCLUDE include)
set(EXECUTABLE_FLAGS "-Wall" "-Wpedantic" "-Wextra" "-g")

set(DISK_NAME disks)
set(DISK_PATH \"${CMAKE_CURRENT_BINARY_DIR}/${DISK_NAME}/\")

# Retrieve source files, under SOURCE_FOLDER
file(GLOB SOURCES
    ${SOURCE_INCLUDE}/*.hpp
    ${SOURCE_SRC}/*.cpp
)

# Set source files to compile for generating executable
add_executable(${EXECUTABLE_NAME})

# Set target options
target_sources(${EXECUTABLE_NAME} PRIVATE ${SOURCES})
target_include_directories(${EXECUTABLE_NAME} PRIVATE ${SOURCE_INCLUDE})
target_compile_options(${EXECUTABLE_NAME} PRIVATE ${EXECUTABLE_FLAGS})

# Passing defined macros into executable
target_compile_definitions(${EXECUTABLE_NAME} PRIVATE -DDISK_PATH=${DISK_PATH})

add_custom_target(build-time-make-directory ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DISK_NAME})

# linking library folders to target executable
find_package(Threads REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE Threads::Threads)