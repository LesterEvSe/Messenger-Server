cmake_minimum_required(VERSION 3.22)
project(Server)

# Set the minimum required Qt version
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH "/home/zhenia/Qt/5.15.2/gcc_64/lib/cmake")

find_package(Qt5 COMPONENTS Core Gui Widgets Network Sql REQUIRED)

# Add include directories
include_directories(
    /usr/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/res
)

# Add source files
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

# Add UI files
set(FORMS ${CMAKE_CURRENT_SOURCE_DIR}/server.ui)
set(QRC ${CMAKE_CURRENT_SOURCE_DIR}/resource.qrc)

# Generate executable
add_executable(Server ${SOURCES} ${HEADERS} ${FORMS} ${QRC})

# Link libraries
target_link_libraries(Server PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    Qt5::Sql
    -lgmp
)

set_target_properties(Server PROPERTIES AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}
)

