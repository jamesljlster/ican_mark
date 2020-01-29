cmake_minimum_required(VERSION 3.5)

project(gui_test LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)

if(ANDROID)
  add_library(gui_test SHARED
    main.cpp
    guitest.cpp
    guitest.h
    guitest.ui
  )
else()
  add_executable(gui_test
    main.cpp
    guitest.cpp
    guitest.h
    guitest.ui
  )
endif()

target_link_libraries(gui_test PRIVATE Qt5::Widgets)
install(TARGETS gui_test
    RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
    )
