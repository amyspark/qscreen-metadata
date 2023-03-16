cmake_minimum_required(VERSION 3.20)

project(qscreen)

if (APPLE)
enable_language(OBJCXX)
set(CMAKE_OBJCXX_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
else()
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

set(CMAKE_AUTOUIC ON)

find_package(Qt6 CONFIG COMPONENTS Core Gui Widgets REQUIRED)

set(srcs main.cpp)

if (APPLE)
find_library(APPKIT AppKit REQUIRED)
find_library(COREDISPLAY CoreDisplay)
set(srcs ${srcs} macos.mm)
elseif(WIN32)
set(srcs ${srcs} windows.cpp)
else()
set(srcs ${srcs} default.cpp)
endif()

add_executable(main ${srcs})

target_link_libraries(main PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets)

if (APPLE)
target_link_libraries(main PUBLIC Qt5::EdidSupportPrivate ${APPKIT} ${COREDISPLAY})
elseif (WIN32)
target_link_libraries(main PUBLIC Qt6::GuiPrivate setupapi)
endif()
