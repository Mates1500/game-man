# CMakeList.txt : CMake project for game-man, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (game-man "game-man.cpp" "game-man.h" "cpu.h" "memory.h" "memory.cpp" "file_handle.h" "file_handle.cpp")

# TODO: Add tests and install targets if needed.
