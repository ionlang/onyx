# General project settings.
cmake_minimum_required(VERSION 3.12.4)
project(ilc VERSION 1.0.0 LANGUAGES CXX)

# Setup compiler flag(s) & standard(s).
set(CMAKE_CXX_STANDARD 20)

# General project information.
set(PROJECT_URL "https://github.com/ionlang/ilc")

# Build project statically.
#set(CMAKE_EXE_LINKER_FLAGS "-static")
set(CMAKE_EXE_LINKER_FLAGS "-ftime-report")

# Setup source and build directories.
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Linux installation support.
include(GNUInstallDirs)

# Dependencies: ionshared.
find_package(ionshared QUIET)

# Download & build ionshared if it's not installed on
# the system, via CMake's FetchContent.
if(NOT ionshared_FOUND)
    include(FetchContent)

    FetchContent_Declare(
        ionshared

        # TODO: Using dev. branch temporarily.
        URL https://github.com/ionlang/ionshared/archive/dev.tar.gz
    )

    # Has ionshared been downloaded yet?
    if(NOT ionshared_POPULATED)
        set(FETCHCONTENT_QUIET OFF)
        FetchContent_Populate(ionshared)

        # Do not build tests for it.
        set(IONSHARED_BUILD_TESTS OFF)

        # Build ionshared.
        add_subdirectory(${ionshared_SOURCE_DIR} ${ionshared_BINARY_DIR})
    endif()
endif()

# Dependencies: ionir.
find_package(ionir QUIET)

# Download & build ionir if it's not installed on
# the system, via CMake's FetchContent.
if(NOT ionir_FOUND)
    include(FetchContent)

    FetchContent_Declare(
        ionir

        # TODO: Using dev. branch temporarily.
        URL https://github.com/ionlang/ionir/archive/dev.tar.gz
    )

    # Has ionir been downloaded yet?
    if(NOT ionir_POPULATED)
        set(FETCHCONTENT_QUIET OFF)
        FetchContent_Populate(ionir)

        # Do not build tests for it.
        set(IONSHARED_BUILD_TESTS OFF)

        # Build ionshared.
        add_subdirectory(${ionir_SOURCE_DIR} ${ionir_BINARY_DIR})
    endif()
endif()

# Dependencies: ionlang.
find_package(ionlang QUIET)

# Download & build ionlang if it's not installed on
# the system, via CMake's FetchContent.
if(NOT ionlang_FOUND)
    include(FetchContent)

    FetchContent_Declare(
        ionlang

        # TODO: Using dev. branch temporarily.
        URL https://github.com/ionlang/ionlang/archive/dev.tar.gz
    )

    # Has ionir been downloaded yet?
    if(NOT ionlang_POPULATED)
        set(FETCHCONTENT_QUIET OFF)
        FetchContent_Populate(ionlang)

        # Do not build tests for it.
        set(IONSHARED_BUILD_TESTS OFF)

        # Build ionshared.
        add_subdirectory(${ionlang_SOURCE_DIR} ${ionlang_BINARY_DIR})
    endif()
endif()

find_package(CLI11 CONFIG)

# LLVM dependency.
find_package(LLVM 12.0.1 REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Include LLVM source directories (for lexical support, etc.). The SYSTEM symbol supposedly marks the directory to suppress warning(s).
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Setup versioning.
set(VERSION_MAJOR "1")
set(VERSION_MINOR "0")
set(VERSION_PATCH "0")
set(VERSION "$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)")

# Set source file(s).
file(
    GLOB_RECURSE SOURCES
    "src/*.h"
    "src/*.cpp"
)

# Specify that this project is an executable.
add_executable("${PROJECT_NAME}" ${SOURCES})

target_include_directories(
    ${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>

    # Comment out as it's causing cannot find included file '/project_name'.
    #$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
)

llvm_map_components_to_libnames(llvm_libs all)
#llvm_config("${PROJECT_NAME}" support core irreader)

# Link against libraries.
target_link_libraries("${PROJECT_NAME}" PRIVATE ${llvm_libs} ionshared ionir ionlang)

# Setup unit testing using Google Test (GTest) if applicable. This binds the CMakeLists.txt on the test project.
option(ILC_BUILD_TESTS "Build tests" OFF)

# TODO
if(ILC_BUILD_TESTS)
    add_subdirectory(./libs/googletest)
    add_subdirectory(./test)
endif()

# Add install target.
#install(
#    TARGETS ${LIBRARY_TARGET_NAME}
#    EXPORT ${PROJECT_NAME}
#    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
#)

# Support for CLion install target.
add_custom_target(
    install_${PROJECT_NAME}
    "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target install
    DEPENDS ${PROJECT_NAME}
    COMMENT "Installing ${PROJECT_NAME}"
)
