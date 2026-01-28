cmake_minimum_required(VERSION 3.12)
project(HFTools VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform-specific settings
if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find nlohmann/json (header-only library)
find_package(nlohmann_json 3.2.0 QUIET)
if(NOT nlohmann_json_FOUND)
    # If not found via package manager, use FetchContent
    include(FetchContent)
    FetchContent_Declare(json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    )
    FetchContent_MakeAvailable(json)
endif()

# Library source files
set(LIB_SOURCES
    src/database/IDatabase.cpp
    src/database/Connection.cpp
    src/database/ResultSet.cpp
    src/database/PostgreSQLDatabase.cpp
    src/database/SybaseDatabase.cpp
    src/model/User.cpp
    src/model/FXInstrument.cpp
    src/model/Trade.cpp
)

# Create library
add_library(hftools STATIC ${LIB_SOURCES})

# Link nlohmann/json
if(nlohmann_json_FOUND)
    target_link_libraries(hftools PUBLIC nlohmann_json::nlohmann_json)
else()
    target_link_libraries(hftools PUBLIC nlohmann_json)
endif()

# Console application
add_executable(hftools_app src/main.cpp)
target_link_libraries(hftools_app PRIVATE hftools)

# Add platform-specific libraries
if(UNIX AND NOT APPLE)
    # Linux-specific libraries (e.g., for PostgreSQL/Sybase client libraries)
    # target_link_libraries(hftools PUBLIC pq sybdb)
endif()

if(WIN32)
    # Windows-specific libraries
    # target_link_libraries(hftools PUBLIC libpq sybdb)
endif()

# Installation
install(TARGETS hftools hftools_app
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)
