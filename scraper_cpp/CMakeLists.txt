cmake_minimum_required(VERSION 3.14)
project(LyricsScraper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable compile_commands.json for LSP support (e.g., clangd)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Disable curl extras
set(BUILD_CURL_EXE OFF)
set(BUILD_TESTING OFF)

# Add dependencies
add_subdirectory(external/curl)
add_subdirectory(external/json)

# Add executable
add_executable(lyrics_scraper
    main.cpp
    getlyrics.cpp
)

# Include headers
target_include_directories(lyrics_scraper PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/scraper_cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scraper_cpp/external/curl/include
    ${CMAKE_CURRENT_SOURCE_DIR}/scraper_cpp/external/json/include
)

# Link libraries
target_link_libraries(lyrics_scraper PRIVATE libcurl nlohmann_json::nlohmann_json)

