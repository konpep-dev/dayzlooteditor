cmake_minimum_required(VERSION 3.16)

project(DayZTypesEditor VERSION 1.0.0 LANGUAGES CXX)

# ── C++17 ──
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Qt6 auto-tools ──
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ── Qt6 packages ──
find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Widgets Xml)

# ── Source files ──
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/TypesModel.cpp
    src/TypeItem.cpp
    src/XmlParser.cpp
    src/BackupManager.cpp
    src/BulkEditDialog.cpp
    src/SearchFilterWidget.cpp
)

set(HEADERS
    src/MainWindow.h
    src/TypesModel.h
    src/TypeItem.h
    src/XmlParser.h
    src/BackupManager.h
    src/BulkEditDialog.h
    src/SearchFilterWidget.h
    src/DarkTheme.h
)

# ── Executable ──
# WIN32 flag: no console window on Windows
if(WIN32)
    add_executable(DayZTypesEditor WIN32 ${SOURCES} ${HEADERS})
else()
    add_executable(DayZTypesEditor ${SOURCES} ${HEADERS})
endif()

# ── Link Qt6 ──
target_link_libraries(DayZTypesEditor
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Xml
)

# ── Include path ──
target_include_directories(DayZTypesEditor PRIVATE src)

# ── Windows: deploy Qt DLLs after build (optional, requires windeployqt) ──
if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt
        HINTS "${Qt6_DIR}/../../../bin"
    )
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET DayZTypesEditor POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                    --no-translations
                    --no-opengl-sw
                    "$<TARGET_FILE:DayZTypesEditor>"
            COMMENT "Running windeployqt..."
        )
    endif()
endif()

# ── Compiler warnings ──
if(MSVC)
    target_compile_options(DayZTypesEditor PRIVATE /W3 /utf-8)
else()
    target_compile_options(DayZTypesEditor PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ── Installation (optional) ──
install(TARGETS DayZTypesEditor
    RUNTIME DESTINATION bin
)
