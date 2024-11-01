cmake_minimum_required(VERSION 3.10)
project(bmcc_jobs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(SQLite3 REQUIRED)
find_package(Boost REQUIRED)

# Add executable
add_executable(${PROJECT_NAME} src/main.cpp)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SQLite3_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    /opt/homebrew/include
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${SQLite3_LIBRARIES}  # Changed from SQLite3::SQLite3
    ${Boost_LIBRARIES}
)