cmake_minimum_required(VERSION 3.10)

# Set variables
set(PROJECT_NAME ical_mark_test)
set(PROJECT_LANGUAGE CXX)

# Compile setting
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Set project
project(${PROJECT_NAME} ${PROJECT_LANGUAGE})

# Set file list
file(GLOB PROJECT_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    )

set(CMAP_IMG ${CMAKE_CURRENT_BINARY_DIR}/color_map.png)
file(DOWNLOAD
    "https://upload.wikimedia.org/wikipedia/commons/7/72/Color_map_fs.png"
    ${CMAP_IMG}
    EXPECTED_MD5 815e5a83f5cbd0a8c288b5c34c656d99
    SHOW_PROGRESS
    )
install(FILES ${CMAP_IMG}
    DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
    )

# Build test
foreach(TEST_FILE_PATH ${PROJECT_SRCS})
    # Get each file name
    get_filename_component(TEST_FILE_NAME ${TEST_FILE_PATH} NAME_WE)

    # Build executable
    add_executable(${TEST_FILE_NAME} ${TEST_FILE_PATH})
    set_target_properties(${TEST_FILE_NAME} PROPERTIES
        CXX_STANDARD 11
        OUTPUT_NAME ${TEST_FILE_NAME}
        )
    target_link_libraries(${TEST_FILE_NAME} ${PROJECT_DEPS})

    # Install
    install(TARGETS ${TEST_FILE_NAME}
        RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
        )
endforeach()

