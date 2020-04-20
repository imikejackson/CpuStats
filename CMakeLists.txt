
cmake_policy(SET CMP0002 NEW)
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0079 NEW)
cmake_policy(SET CMP0077 NEW)

cmake_minimum_required(VERSION 3.14.0)

# Request C++14 standard, using new CMake variables.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)


project (PowerGadget VERSION 6.6.0.0)


add_executable(pg main.cpp PowerGui.cpp PowerGui.h)
target_link_libraries(pg 
                        /Library/Frameworks/IntelPowerGadget.framework/IntelPowerGadget
                          ncurses)
target_include_directories(pg PUBLIC /Library/Frameworks/IntelPowerGadget.framework)
