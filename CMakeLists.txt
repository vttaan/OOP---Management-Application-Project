cmake_minimum_required(VERSION 3.16)
project(OOP_Management_Application LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON) 
set(CMAKE_AUTORCC ON) 
set(CMAKE_AUTOUIC ON) 

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

# Danh sách các file code
set(PROJECT_SOURCES
    src/main.cpp
    resources/res.qrc
)

qt_add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Sql)
target_include_directories(${PROJECT_NAME} PRIVATE src)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/database 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
