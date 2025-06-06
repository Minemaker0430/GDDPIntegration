cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(GDDPIntegration VERSION 1.0.0)

# Set up the mod binary
add_library(${PROJECT_NAME} SHARED
    #src/main.cpp
    # Add your cpp files here
    src/hooks/CreatorLayer.cpp
    src/hooks/LevelInfoLayer.cpp
    src/hooks/LevelCell.cpp
    src/hooks/PlayLayer.cpp
    src/popups/StatsPopup.cpp
    src/Utils.cpp
    src/menus/DPListLayer.cpp
    src/menus/DPLayer.cpp
    src/menus/RecommendedLayer.cpp
    src/popups/SupportPopup.cpp
    src/popups/NewsPopup.cpp
    src/XPUtils.cpp
    src/popups/XPPopup.cpp
    src/RecommendedUtils.cpp
    src/menus/DPSearchLayer.cpp
    src/popups/SearchPopup.cpp
    src/RouletteUtils.cpp
    src/popups/RoulettePopup.cpp
    src/menus/RouletteSafeLayer.cpp
    src/popups/dev/VerificationPopup.cpp
    src/popups/dev/SaveContentsPopup.cpp
    src/base64.cpp
)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# Set up dependencies, resources, link Geode
setup_geode_mod(${PROJECT_NAME})

