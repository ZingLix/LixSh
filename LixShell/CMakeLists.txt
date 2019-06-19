# CMakeList.txt : CMake project for LixShell, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
file(GLOB src "*.cpp")
add_executable (LixShell ${src})
target_link_libraries(LixShell readline stdc++fs)
# TODO: Add tests and install targets if needed.
