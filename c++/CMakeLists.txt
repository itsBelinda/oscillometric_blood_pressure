cmake_minimum_required(VERSION 3.1.0)

project(obp)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

# required packages
find_package(Qt5 COMPONENTS Widgets PrintSupport REQUIRED)

# required libraries
find_library(comedi REQUIRED)
find_library(iir REQUIRED)
find_library(qwt-qt5 REQUIRED)

add_executable(obp
        main.cpp
        obp.cpp
        dataplot.cpp
        datarecord.cpp datarecord.h Processing.cpp Processing.h)

target_link_libraries(obp Qt5::Widgets Qt5::PrintSupport comedi iir qwt-qt5)


