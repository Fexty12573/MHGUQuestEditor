cmake_minimum_required(VERSION 3.16)
project(MHGUQuestEditor LANGUAGES CXX)

include(qt.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Widgets
)
qt_standard_project_setup()

if (WIN32)
    set(RESOURCE_FILES 
        ${CMAKE_SOURCE_DIR}/Resource.qrc
        ${CMAKE_SOURCE_DIR}/MHGUQuestEditor.rc
    )
else()
    set(RESOURCE_FILES 
        ${CMAKE_SOURCE_DIR}/Resource.qrc
    )
endif()


set(PROJECT_SOURCES
    main.cpp
    MHGUQuestEditor.ui
    MHGUQuestEditor.h
    MHGUQuestEditor.cpp
    SettingsDialog.ui
    SettingsDialog.h
    SettingsDialog.cpp
    Widgets/EmSetListEditor/EmSetListEditor.ui
    Widgets/EmSetListEditor/EmSetListEditor.h
    Widgets/EmSetListEditor/EmSetListEditor.cpp
    Widgets/BossSetEditor/BossSetEditor.ui
    Widgets/BossSetEditor/BossSetEditor.h
    Widgets/BossSetEditor/BossSetEditor.cpp
    Common.h
    Monster/Id.h
    Util/Crc32.h
    Resources/QuestData.h
    Resources/QuestData.cpp
    Resources/Gmd.h
    Resources/Gmd.cpp
    Resources/Rem.h
    Resources/Rem.cpp
    Resources/QuestLink.h
    Resources/QuestLink.cpp
    Resources/EmSetList.h
    Resources/EmSetList.cpp
    Resources/BossSet.h
    Resources/BossSet.cpp
    Resources/Arc.h
    Resources/Arc.cpp
    Resources/QuestArc.h
    Resources/QuestArc.cpp
    Resources/ExtensionResolver.h
    Resources/ExtensionResolver.cpp
    Resources/StatTable.h
    Resources/StatTable.cpp
    ${RESOURCE_FILES}
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
