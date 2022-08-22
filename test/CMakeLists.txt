
cmake_minimum_required(VERSION 3.16)
enable_testing()

project(test_tokener)

add_subdirectory(ss)

add_executable(test_tokener test_tokener.c)
target_include_directories(test_tokener PRIVATE ../include)
target_include_directories(test_tokener PRIVATE ss/include)
target_link_libraries(test_tokener PRIVATE ss)
target_link_libraries(test_tokener PRIVATE symbolscript)
if (CODE_COVERAGE)
    target_code_coverage(test_tokener)
endif()
add_test(NAME test_tokener COMMAND test_tokener)

