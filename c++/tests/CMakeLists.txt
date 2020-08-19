set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/tests)
set(CTEST_BINARY_DIRECTORY ${PROJECT_BINARY_DIR}/tests)

cmake_minimum_required(VERSION 2.8)

# Find the Qt libraries.
#find_package(Qt5 COMPONENTS Widgets PrintSupport Core REQUIRED)
#find_package(Threads REQUIRED)

# required libraries
#find_library(comedi REQUIRED)
#find_library(iir REQUIRED)
#find_library(qwt-qt5 REQUIRED)

# Qt5 libraries.
#set(QT5_LIBRARIES
#        Qt5::Widgets Qt5::PrintSupport Qt5::Core)
#set(PROJECT_LIBS
#      comedi
#      iir)

add_executable (test_OBPDetection test_OBPDetection.cpp)
#target_link_libraries(test_test ${PROJECT_LIBS} ${QT5_LIBRARIES})
add_test(OBPDetection test_OBPDetection)


