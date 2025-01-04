cmake_minimum_required(VERSION 3.15)

# Project Metadata
project(LibException VERSION 1.0 LANGUAGES C)

# Settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_TESTING "Enable building of tests" ON)

# Add the library
add_library(exception src/exception.c)
target_include_directories(exception PUBLIC include)
target_compile_features(exception PRIVATE c_std_11)

# Enable testing
if(BUILD_TESTING)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install targets (optional)
install(
    TARGETS exception
    EXPORT ExceptionLibraryConfig
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(
    DIRECTORY include/
    DESTINATION include
)
