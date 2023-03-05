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

find_package(Qt5 CONFIG COMPONENTS Core Gui Widgets REQUIRED)

set(srcs main.cpp)

if (APPLE)
find_library(APPKIT AppKit REQUIRED)
set(srcs ${srcs} macos.mm)
else()
set(srcs ${srcs} default.cpp)
endif()

add_executable(main ${srcs})

target_link_libraries(main PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets)

if (APPLE)
target_link_libraries(main PUBLIC ${APPKIT})
endif()