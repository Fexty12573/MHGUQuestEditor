cmake_minimum_required(VERSION 3.16)
project(MHGUQuestEditor LANGUAGES CXX)

include(qt.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Widgets
)
qt_standard_project_setup()


set(PROJECT_SOURCES
    main.cpp
    MHGUQuestEditor.ui
    MHGUQuestEditor.h
    MHGUQuestEditor.cpp
    Common.h
    Resources/QuestData.h
    Resources/QuestData.cpp
    Resources/Arc.h
    Resources/Arc.cpp
)

qt_add_executable(MHGUQuestEditor ${PROJECT_SOURCES})

target_include_directories(MHGUQuestEditor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(MHGUQuestEditor
    PROPERTIES
        WIN32_EXECUTABLE TRUE
)

target_link_libraries(MHGUQuestEditor
    PUBLIC
        Qt::Core
        Qt6::Core
        Qt::Gui
        Qt::Widgets
        zlibstatic
)
