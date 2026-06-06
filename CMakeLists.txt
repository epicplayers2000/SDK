cmake_minimum_required(VERSION 3.10)
project(UE4Dumper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIE -fPIC -O2 -Wall -Wextra -DANDROID")

# Main executable
add_executable(ue4dumper
    src/main.cpp
    src/memory/process_memory.cpp
    src/memory/pattern_scanner.cpp
    src/memory/memory_utils.cpp
    src/engine/ue_types.cpp
    src/engine/gworld_scanner.cpp
    src/engine/gobject_scanner.cpp
    src/engine/gnames_scanner.cpp
    src/dumper/class_dumper.cpp
    src/dumper/offset_dumper.cpp
    src/dumper/json_dumper.cpp
    src/utils/logger.cpp
    src/utils/file_utils.cpp
)

# Include directories
target_include_directories(ue4dumper PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(ue4dumper PRIVATE log)

# Set output directory
set_target_properties(ue4dumper PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
