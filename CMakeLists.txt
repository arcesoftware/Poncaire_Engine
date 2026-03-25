cmake_minimum_required(VERSION 3.20)

# 1. Update project name to reflect the current simulation
project(PetrieHypercube)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. SDL3 paths (Keep these as they are specific to your machine)
set(SDL3_INCLUDE_DIR "C:/SDL3/include")
set(SDL3_LIB_DIR "C:/SDL3/lib/x64")

include_directories(${SDL3_INCLUDE_DIR})
link_directories(${SDL3_LIB_DIR})

# 3. Rename the executable to something descriptive
# This will result in "PetrieHypercube.exe"
add_executable(PetrieHypercube main.cpp)

# 4. Link against SDL3
target_link_libraries(PetrieHypercube SDL3)

# 5. Automatically copy the DLL to the build folder
# This ensures PetrieHypercube.exe can find SDL3.dll at runtime
add_custom_command(TARGET PetrieHypercube POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/SDL3/lib/x64/SDL3.dll"
        "$<TARGET_FILE_DIR:PetrieHypercube>"
)