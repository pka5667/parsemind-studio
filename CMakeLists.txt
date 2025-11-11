cmake_minimum_required(VERSION 3.16)
project(ParseMindStudio LANGUAGES CXX)

# Qt settings for all submodules
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdirectories
add_subdirectory(ui_shell)
