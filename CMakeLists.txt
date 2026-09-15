cmake_minimum_required(VERSION 3.16)
project(Flip VERSION 1.2.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(Qt6 6.2 QUIET COMPONENTS Widgets Gui Network)
if(Qt6_FOUND)
    set(FLIP_QT Qt6)
else()
    find_package(Qt5 5.15 REQUIRED COMPONENTS Widgets Gui Network)
    set(FLIP_QT Qt5)
    message(STATUS "Qt 6 not found; falling back to Qt ${Qt5_VERSION}")
endif()

message(STATUS "Building Flip ${PROJECT_VERSION} with ${FLIP_QT}")

set(FLIP_SOURCES
    src/main.cpp
    src/I18n.cpp
    src/FolderPager.cpp
    src/ImageView.cpp
    src/MainWindow.cpp
    src/UpdateFeed.cpp
    src/DownloadRace.cpp
    src/InPlaceUpdater.cpp
    src/LaunchPrompts.cpp
    src/SelfTest.cpp
    resources/flip.qrc
)

if(FLIP_QT STREQUAL "Qt6")
    qt_add_executable(Flip ${FLIP_SOURCES})
else()
    add_executable(Flip ${FLIP_SOURCES})
endif()

target_include_directories(Flip PRIVATE src)
target_link_libraries(Flip PRIVATE ${FLIP_QT}::Widgets ${FLIP_QT}::Gui ${FLIP_QT}::Network)
target_compile_definitions(Flip PRIVATE
    FLIP_VERSION="${PROJECT_VERSION}"
    FLIP_QT_MAJOR=$<IF:$<BOOL:${Qt6_FOUND}>,6,5>
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(Flip PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS Flip RUNTIME DESTINATION bin)
install(FILES packaging/flip.desktop DESTINATION share/applications)
install(FILES resources/icons/flip.svg DESTINATION share/icons/hicolor/scalable/apps)
