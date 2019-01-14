cmake_minimum_required(VERSION 3.8)

project(FileManager)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")
set(CMAKE_PREFIX_PATH "/Users/elena/Qt/5.11.2/clang_64/lib/cmake")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Widgets REQUIRED)

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})

add_executable(FileManager
        main.cpp
        mainwindow.h
        mainwindow.cpp
        mainwindow.ui
        finderOfCopies.cpp
        finderOfCopies.h
        mainwindow2.cpp
        mainwindow2.h
        mainwindow1.ui
        fileTrigram.cpp fileTrigram.h finderOfStrings.cpp finderOfStrings.h trigram_process.cpp trigram_process.h)

target_link_libraries(FileManager Qt5::Core)
target_link_libraries(FileManager Qt5::Widgets)
target_link_libraries(FileManager stdc++fs)

qt5_use_modules(FileManager Widgets)