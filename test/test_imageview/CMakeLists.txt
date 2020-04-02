cmake_minimum_required(VERSION 3.5)

project(test_imageview LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB PROJECT_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
    )

add_executable(test_imageview ${PROJECT_SRCS})
target_link_libraries(test_imageview PRIVATE ${PROJECT_DEPS})

install(TARGETS test_imageview
    RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
    )
