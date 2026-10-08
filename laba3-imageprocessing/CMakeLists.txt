cmake_minimum_required(VERSION 3.15)
project(ImageProcessing CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(FLTK_BUILD_TEST OFF CACHE BOOL "" FORCE)
set(FLTK_BUILD_FLUID OFF CACHE BOOL "" FORCE)
set(FLTK_BUILD_FLTK_OPTIONS OFF CACHE BOOL "" FORCE)
set(FLTK_BUILD_HTML_DOCS OFF CACHE BOOL "" FORCE)
set(FLTK_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

add_subdirectory(third_party/fltk-1.4.5)

find_package(Threads REQUIRED)

set(SOURCES
    src/main.cpp
    src/core/image_buffer.cpp
    src/core/parallel.cpp
    src/core/color.cpp
    src/core/processing.cpp
    src/core/clahe.cpp
    src/core/image_io.cpp
    src/core/cli.cpp
    src/ui/main_window.cpp
    src/ui/image_view.cpp
    src/ui/histogram_view.cpp
    src/ui/worker.cpp
)

add_executable(ImageProcessing ${SOURCES})

target_include_directories(ImageProcessing PRIVATE src)

target_link_libraries(ImageProcessing PRIVATE fltk fltk_images Threads::Threads)

if(WIN32)
    target_compile_definitions(ImageProcessing PRIVATE _WIN32_WINNT=0x0501 WINVER=0x0501)
    target_link_options(ImageProcessing PRIVATE -mwindows -static -static-libgcc -static-libstdc++)
endif()
