cmake_minimum_required(VERSION 3.22)
project(Server)

# Set the minimum required Qt version
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find in Linux file system
if(UNIX AND NOT APPLE)
    # Use an external command to find the path to the directory (d), starting from /home.
    # Write the result in QT_DIR.
    # Last argument - strip the string of extra characters and spaces
    execute_process(COMMAND find /home -type d -path "*Qt/5*/*/lib/cmake"
        OUTPUT_VARIABLE QT_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE)
        
    if (QT_DIR)
        list(APPEND CMAKE_PREFIX_PATH "${QT_DIR}")
        message(STATUS "Set CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
    else()
        message(FATAL_ERROR "Could not find Qt directory")
    endif()
    
endif()

find_package(Qt5 COMPONENTS Core Gui Widgets Network Sql REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/res
)

if (WIN32)
    include_directories("C:/gmp-6.2.1")
endif()

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/serverback.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/encryption.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/database.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# moc files will not work unless the headers are assembled
set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/server.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/serverback.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/encryption.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/database.hpp
)

set(FORMS ${CMAKE_CURRENT_SOURCE_DIR}/server.ui)
set(QRC   ${CMAKE_CURRENT_SOURCE_DIR}/resource.qrc)

# Generate executable
add_executable(Server ${SOURCES} ${HEADERS} ${FORMS} ${QRC})

# Set path to GMP library in different OS
if (UNIX AND NOT APPLE)
    set(GMP_LIB -lgmp)
elseif (WIN32)
    set(GMP_LIB "C:/gmp-6.2.1/.libs/libgmp.a")
endif()

# Link libraries
target_link_libraries(Client PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    ${GMP_LIB}
)

# To specify where the .ui files are located
set_target_properties(Server PROPERTIES AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}
)

