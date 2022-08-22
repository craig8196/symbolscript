
cmake_minimum_required(VERSION 3.16)
enable_testing()
project(symbolscript VERSION 0.0.1 DESCRIPTION "Heirarchical time management tool.")

include(GNUInstallDirs)

if(CODE_COVERAGE)
    set(CMAKE_BUILD_TYPE DEBUG)
    include(cmake-scripts/code-coverage.cmake)
endif()

add_subdirectory(test)

set(SOURCES src/tokenizer.c)

add_library(symbolscript STATIC ${SOURCES})
set_target_properties(symbolscript PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(symbolscript PROPERTIES PUBLIC_HEADER include/symbolscript.h)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_C_FLAGS_DEBUG "-O0")

configure_file(symbolscript.pc.in symbolscript.pc @ONLY)

target_include_directories(symbolscript PRIVATE include)
target_include_directories(symbolscript PRIVATE src)

if(CODE_COVERAGE)
    target_code_coverage(symbolscript)
endif()

if(BUILD_DOCUMENTATION)
    set(MYPROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
    set(MYPROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
    set(PROJECT_README_FILE ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    configure_file(${doxyfile_in} ${doxyfile} @ONLY)
    include(cmake-scripts/doxygen.cmake)
    build_docs(ADD_TO_DOC symbolscript)
endif()

install(TARGETS symbolscript
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_BINARY_DIR}/symbolscript.pc
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)


