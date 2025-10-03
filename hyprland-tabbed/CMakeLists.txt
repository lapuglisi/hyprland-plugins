cmake_minimum_required(VERSION 3.27)

project(hyprland-tabbed
    DESCRIPTION "hyprland-tabbed plugin for Hyprland"
    VERSION 0.1
)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE SRC "*.cpp")

add_library(hyprland-tabbed SHARED ${SRC})
#target_compile_definitions(hyprland-tabbed PUBLIC _DEBUG)

find_package(PkgConfig REQUIRED)
pkg_check_modules(deps REQUIRED IMPORTED_TARGET
    hyprland
    libdrm
    libinput
    libudev
    pangocairo
    pixman-1
    wayland-server
    xkbcommon
)
target_link_libraries(hyprland-tabbed PRIVATE rt PkgConfig::deps)

install(TARGETS hyprland-tabbed)
