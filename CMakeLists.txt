cmake_minimum_required(VERSION 3.16)

project(wavy VERSION 1.0.0 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstandalone-debug")

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(wavy_gui  
  gui/main.cpp
  gui/mainwindow.cpp
)

target_link_libraries(wavy_gui PRIVATE Qt6::Widgets)

set_target_properties(wavy_gui PROPERTIES
WIN32_EXECUTABLE ON
MACOSX_BUNDLE ON
)

qt_add_executable(wavy_vcd-parser
  cli/vcd-parser/top-vcd-parser.cpp
)

qt_add_library(vcd-parser SHARED 
  cli/vcd-parser/vcd-parser.cpp
  cli/vcd-parser/vcd-char-stream.cpp
  cli/vcd-parser/vcd-token-stream.cpp
)

target_link_libraries(wavy_vcd-parser PRIVATE vcd-parser)
target_link_libraries(vcd-parser PUBLIC Qt6::Widgets Qt6::Core)
set_target_properties(wavy_vcd-parser PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
