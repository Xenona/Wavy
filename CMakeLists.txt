cmake_minimum_required(VERSION 3.16)

project(PiVOX VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(PiVOX_gui  
  main.cpp mainwindow.cpp mainwindow.h mainwindow.ui
)

target_link_libraries(PiVOX_gui PRIVATE Qt6::Widgets)

set_target_properties(PiVOX_gui PROPERTIES
WIN32_EXECUTABLE ON
MACOSX_BUNDLE ON
)

qt_add_executable(PiVOX_vcd-parser
  cli/vcd-parser/top-vcd-parser.cpp
)

qt_add_library(vcd-parser SHARED 
  cli/vcd-parser/vcd-parser.cpp
  cli/vcd-parser/vcd-char-stream.cpp
  cli/vcd-parser/vcd-token-stream.cpp
)

target_link_libraries(PiVOX_vcd-parser PRIVATE vcd-parser)
target_link_libraries(vcd-parser PUBLIC Qt6::Widgets Qt6::Core)
set_target_properties(PiVOX_vcd-parser PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)