cmake_minimum_required(VERSION 3.16)
project(kv_store LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/deps/install")

# Dependencies
find_package(ZeroMQ REQUIRED)
find_package(quill REQUIRED)

# EASTL backend
option(EASTL_BACKEND "Enable EASTL Backend" OFF)

if(EASTL_BACKEND STREQUAL "ON")
    message(STATUS "EASTL Backend Selected")
    add_compile_definitions(EASTL_CUSTOM_NEW_ENABLED=0)
    add_subdirectory(${CMAKE_SOURCE_DIR}/deps/EASTL-master)
else()
    message(STATUS "STL Backend Selected")
endif()

# Main
add_executable(kv_store src/server_main.cpp)
target_include_directories(kv_store PRIVATE src
                                    PRIVATE include)
target_link_libraries(kv_store PRIVATE libzmq quill::quill)
if(EASTL_BACKEND)
    target_compile_definitions(kv_store PRIVATE USE_EASTL_BACKEND)
    target_link_libraries(kv_store PRIVATE EASTL)
endif()
if(WIN32)
    add_custom_command(TARGET kv_store POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:libzmq>                      # DLL path
            $<TARGET_FILE_DIR:kv_store>                # Destination directory
        COMMENT "Copying libzmq DLL to output directory"
    )
endif()

# Tests
add_executable(tests tests/unit_tests.cpp)
target_include_directories(tests PRIVATE src
                                 PRIVATE include
				                 PRIVATE deps/doctest-master/doctest)
target_link_libraries(tests PRIVATE quill::quill)
if(EASTL_BACKEND)
    target_compile_definitions(tests PRIVATE USE_EASTL_BACKEND)
    target_link_libraries(tests PRIVATE EASTL)
endif()

enable_testing()
add_test(NAME doctest_all COMMAND tests)