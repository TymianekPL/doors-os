cmake_minimum_required(VERSION 3.28)

if (NOT DEFINED PROJECT_NAME)
  set(PROJECT_NAME "Doors")
endif()     

project(${PROJECT_NAME}
  VERSION 0.1
  LANGUAGES CXX
)                  
                   
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)          
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

function(enablePCH target)
  target_precompile_headers(${target} PRIVATE
    <memory>
    <algorithm>
    <functional>
    <thread>
    <utility>
    <limits>
    <type_traits>
    <bit>
    <concepts>
    <compare>
    <array>
    <span>
    <initializer_list>
    <cstddef>
    <cstdint>
    <atomic>
    <ranges>

    <intrin.h>
    <xmmintrin.h>
    <emmintrin.h>
    <immintrin.h>
  )
endfunction()

add_subdirectory(Shared)

add_subdirectory(Boot)
add_subdirectory(Kernel)
add_subdirectory(Drivers)
add_subdirectory(UM)
