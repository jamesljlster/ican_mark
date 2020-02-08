# Project Dependences Configuration

# Find Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
include_directories(${Qt5Widgets_INCLUDE_DIRS})

# Find yaml
find_package(yaml-cpp REQUIRED)
include_directories(${YAML_CPP_INCLUDE_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Include subdirectories
include_directories(${DEPS_PATHS})

# Backup and set build type to release
if(NOT MSVC)
    set(CMAKE_BUILD_TYPE_BAK ${CMAKE_BUILD_TYPE})
    set(CMAKE_BUILD_TYPE Release)
endif()

# Add subdirectory
foreach(DEPS_PATH ${DEPS_PATHS})
    add_subdirectory(${DEPS_PATH})
endforeach()

# Restore origin build type
if(NOT MSVC)
    set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE_BAK})
endif()

# Set project global dependencies
set(PROJECT_DEPS
    mark_widget
    mark_action
    mark_instance
    ${YAML_CPP_LIBRARIES}
    Qt5::Widgets
    )
