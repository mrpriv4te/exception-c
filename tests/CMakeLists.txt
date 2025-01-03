# Define the test project
project(LibExceptionTests LANGUAGES C)

# Add the test executable
add_executable(exception_tests exception_tests.c)
target_include_directories(exception_tests PRIVATE ../include)
target_link_libraries(exception_tests PRIVATE exception cmocka)

# Enable standard C
target_compile_features(exception_tests PRIVATE c_std_11)

# Add test cases
add_test(NAME ExceptionTests COMMAND exception_tests)
