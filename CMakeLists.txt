cmake_minimum_required(VERSION 3.16)

project(Formica VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt6_standard_project_setup()

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/filetree.cpp
    src/editor.cpp
    src/search.cpp
    src/linkparser.cpp
    src/settings.cpp
    src/preferencesdialog.cpp
    src/vaultmanager.cpp
    src/vaultdialog.cpp
)

set(HEADERS
    src/mainwindow.h
    src/filetree.h
    src/editor.h
    src/search.h
    src/linkparser.h
    src/settings.h
    src/preferencesdialog.h
    src/vaultmanager.h
    src/vaultdialog.h
)

qt6_add_executable(formica ${SOURCES} ${HEADERS})

target_link_libraries(formica PRIVATE Qt6::Core Qt6::Widgets)

# Install
install(TARGETS formica
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)