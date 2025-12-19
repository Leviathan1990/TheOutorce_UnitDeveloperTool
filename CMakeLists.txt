cmake_minimum_required(VERSION 3.16)
project(UnitDeveloperTool VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 or Qt5
find_package(Qt6 COMPONENTS Widgets QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Widgets)
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

# Source files
set(SOURCES
    main.cpp
    OpfParser.cpp
    OpfExporter.cpp
    SettingsManager.cpp

    ui/MainWindow.cpp
    ui/AssetTreeWidget.cpp
    ui/AssetPreviewWidget.cpp
    ui/CustomSettingsWidget.cpp
    ui/SettingsDialog.cpp

    ai/AIParser.cpp
    ai/AIWriter.cpp
    ai/BuildFitnessWidget.cpp
    ai/AIEditorWindow.cpp

)

# Header files
set(HEADERS
    OpfStructs.h
    OpfParser.h
    OpfExporter.h
    SettingsManager.h

    ui/MainWindow.h
    ui/AssetTreeWidget.h
    ui/AssetPreviewWidget.h
    ui/CustomSettingsWidget.h
    ui/SettingsDialog.h

    ai/AIStructs.h
    ai/AIParser.h
    ai/AIWriter.h
    ai/BuildFitnessWidget.h
    ai/AIEditorWindow.h
)

# UI files
set(UI_FILES
    ui/MainWindow.ui
)

# Create executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    OpfWriter.h
    OpfWriter.cpp
    resources.qrc
    EffectsStructs.h
    EffectsParser.h
    EffectsWriter.h
    EffectsParser.cpp
    EffectsWriter.cpp
    ui/ColorsEditorWidget.h
    ui/EffectsEditorWindow.h
    ui/EmitterEditorWidget.h
    ui/ExplosionEditorWidget.h
    ui/ColorsEditorWidget.cpp
    ui/EffectsEditorWindow.cpp
    ui/EmitterEditorWidget.cpp
    ui/ExplosionEditorWidget.cpp
    ai/InitParamsWidget.cpp
    ai/InitParamsWidget.h
    ai/MaxUnitsWidget.h
    ai/MaxUnitsWidget.cpp
)

# Link Qt libraries
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/ai
)

# Platform-specific settings
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Install
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
