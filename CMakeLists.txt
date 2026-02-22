cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 99)
project(DRStool C)

include_directories(include)
add_executable(DRStool src/main.c src/tinyfiledialogs.c)

# If compiling on Windows, required native UI libraries
if(WIN32)
    target_link_libraries(DRStool comdlg32 ole32 shell32)
endif()