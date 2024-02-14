# CMakeList.txt : CMake project for tdpy, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("tdpy")

# set cpp staandard for all targets unless otherwise specified 
# with target_compile_features() or set_property()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)


set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source)

# Collect all source files and exclude gtest files.
file(GLOB_RECURSE ALL_TDPY_SOURCES 
    "${SOURCE_DIR}/*.cpp" "${SOURCE_DIR}/*.c" "${SOURCE_DIR}/*.cu" "${SOURCE_DIR}/*.h")
list(FILTER ALL_TDPY_SOURCES EXCLUDE REGEX ".*gtest\\.cpp$")

# print all sources if on
set(PRINT_SOURCE_FILES On)
if(PRINT_SOURCE_FILES)
    foreach(source IN LISTS ALL_TDPY_SOURCES)
      message(STATUS "source: ${source}")
    endforeach()
endif()


add_library(tdpy MODULE ${ALL_TDPY_SOURCES})
# add_library(tdpy MODULE source/tdpy.cpp)
set_target_properties(tdpy PROPERTIES PREFIX "" SUFFIX ".pyd")

target_include_directories(tdpy PRIVATE ${SOURCE_DIR})




# Check if VCPKG_ROOT is set as an environment variable
if(DEFINED ENV{VCPKG_ROOT})
  set(VCPKG_ROOT $ENV{VCPKG_ROOT})
endif()

# Allow manual setting of VCPKG_ROOT
if(NOT DEFINED VCPKG_ROOT)
  message(STATUS "VCPKG_ROOT not defined, setting default value.")
  set(VCPKG_ROOT "C:/vcpkg")
endif()
message(STATUS "VCPKG_ROOT=${VCPKG_ROOT}")

# Vulkan
# .\vcpkg install vulkan ### haven't tested this...
#################################################################################################
find_package(Vulkan REQUIRED)
message(STATUS Vulkan_INCLUDE_DIR=${Vulkan_INCLUDE_DIR})
target_link_libraries(tdpy Vulkan::Vulkan)

# VMA
# .\vcpkg install vulkan-memory-allocator
#################################################################################################
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
target_link_libraries(tdpy GPUOpen::VulkanMemoryAllocator)

# OpenImageIO
# .\vcpkg install openimageio
#################################################################################################
find_package(OpenImageIO CONFIG REQUIRED )
target_link_libraries(tdpy OpenImageIO::OpenImageIO)

# CUDA
#################################################################################################
find_package(CUDAToolkit REQUIRED)
message(STATUS CUDAToolkit_INCLUDE_DIRS=${CUDAToolkit_INCLUDE_DIRS})
target_include_directories(tdpy PRIVATE ${CUDAToolkit_INCLUDE_DIRS})

# Python
#################################################################################################
find_package(Python3 COMPONENTS Development Interpreter REQUIRED)
message(STATUS Python3_INCLUDE_DIRS=${Python3_INCLUDE_DIRS})
target_include_directories(tdpy PRIVATE ${Python3_INCLUDE_DIRS})
target_link_libraries(tdpy Python3::Python)


# pybind11
# .\vcpkg install pybind11
#################################################################################################
find_package(pybind11 REQUIRED)
message(STATUS pybind11_INCLUDE_DIRS=${pybind11_INCLUDE_DIRS})
target_include_directories(tdpy PRIVATE ${pybind11_INCLUDE_DIRS})
target_link_libraries(tdpy pybind11::module)

# TouchEngine-Windows
# git submodule add https://github.com/TouchDesigner/TouchEngine-Windows.git
#################################################################################################
target_link_directories(tdpy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/TouchEngine-Windows/lib)
target_link_libraries(tdpy TouchEngine)
target_include_directories(tdpy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/TouchEngine-Windows/include)
add_custom_command(TARGET tdpy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/TouchEngine-Windows/bin $<TARGET_FILE_DIR:tdpy>)


# install
#################################################################################################
#################################################################################################
install(TARGETS tdpy LIBRARY DESTINATION modules)