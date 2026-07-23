cmake_minimum_required(VERSION 3.16)
project(secretstool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core DBus)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus)
find_package(OpenSSL REQUIRED)

include(../secrets.cmake)

add_executable(secretstool main.cpp ${SECRETS_FILES})
target_link_libraries(secretstool PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    OpenSSL::Crypto
)
