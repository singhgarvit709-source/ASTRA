cmake_minimum_required(VERSION 3.21)
project(ASTRA_GUI VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(ASTRA_GUI
    main.cpp
)

target_link_libraries(ASTRA_GUI PRIVATE Qt6::Widgets)
