cmake_minimum_required(VERSION 3.20)
project(AccessibilityColorDetector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(AccessibilityColorDetector WIN32 src/main.cpp)

target_link_libraries(AccessibilityColorDetector PRIVATE dwmapi)
