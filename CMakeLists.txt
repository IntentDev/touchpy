# CMakeList.txt : CMake project for tdpy, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.21)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# if(MSVC)
#   set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
# endif()

# # Set runtime library for different build types
# foreach(flag_var
#         CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
#         CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
#    if(${flag_var} MATCHES "/MD")
#       string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
#    endif()
# endforeach()

# # Set debug information flags for Debug build
# if(MSVC)
#     set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /MDd")
#     set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od")
# endif()


# set(CMAKE_CUDA_ARCHITECTURES 89)
set(CMAKE_CUDA_ARCHITECTURES 75;80;86;89)

# set cpp staandard for all targets unless otherwise specified 
# with target_compile_features() or set_property()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

project ("TouchPy" LANGUAGES CXX CUDA)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source)

# Collect all source files and exclude gtest files.
file(GLOB_RECURSE ALL_TPY_SOURCES 
    "${SOURCE_DIR}/*.cpp" "${SOURCE_DIR}/*.c" "${SOURCE_DIR}/*.cu" "${SOURCE_DIR}/*.h")
list(FILTER ALL_TPY_SOURCES EXCLUDE REGEX ".*gtest\\.cpp$")
list(FILTER ALL_TPY_SOURCES EXCLUDE REGEX ".*main\\.cpp$")

# print all sources if on
set(PRINT_SOURCE_FILES Off)
if(PRINT_SOURCE_FILES)
    foreach(source IN LISTS ALL_TPY_SOURCES)
      message(STATUS "tdpy source: ${source}")
    endforeach()
endif()

add_library(tdpy MODULE ${ALL_TPY_SOURCES})
set_target_properties(tdpy PROPERTIES PREFIX "" SUFFIX ".pyd")
target_include_directories(tdpy PRIVATE ${SOURCE_DIR})

file(GLOB_RECURSE ALL_TL_SOURCES 
    "${SOURCE_DIR}/*.cpp" "${SOURCE_DIR}/*.c" "${SOURCE_DIR}/*.cu" "${SOURCE_DIR}/*.h")
list(FILTER ALL_TL_SOURCES EXCLUDE REGEX ".*gtest\\.cpp$")
list(FILTER ALL_TL_SOURCES EXCLUDE REGEX ".*py\\.cpp$")

# print all sources if on
set(PRINT_SOURCE_FILES On)
if(PRINT_SOURCE_FILES)
    foreach(source IN LISTS ALL_TL_SOURCES)
      message(STATUS "tl source: ${source}")
    endforeach()
endif()

add_executable(touchloader ${ALL_TL_SOURCES})
target_include_directories(touchloader PRIVATE ${SOURCE_DIR})


# Compile definitions
if (WIN32)
    list(APPEND COMPILE_DEFINITIONS 
        WIN32_LEAN_AND_MEAN # Exclude rarely-used stuff from Windows headers
        NOMINMAX # Exclude min and max macros from Windows headers
        # _CRT_SECURE_NO_WARNINGS # Disable warnings for unsafe functions in MSVC
        _UNICODE # Use Unicode character set
        UNICODE # Use Unicode character set
        VK_USE_PLATFORM_WIN32_KHR # use Vulkan on Windows
    )
endif ()

target_compile_definitions(tdpy PRIVATE ${COMPILE_DEFINITIONS})
target_compile_definitions(touchloader PRIVATE ${COMPILE_DEFINITIONS})

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
# .\vcpkg install vulkan ### haven't tested this, probably best to just install the SDK
#################################################################################################
find_package(Vulkan REQUIRED)
message(STATUS Vulkan_INCLUDE_DIR=${Vulkan_INCLUDE_DIR})
target_link_libraries(tdpy Vulkan::Vulkan)
target_link_libraries(touchloader Vulkan::Vulkan)


### not used atm but vri includes it... 
# VMA
# .\vcpkg install vulkan-memory-allocator 
#################################################################################################
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
target_link_libraries(tdpy GPUOpen::VulkanMemoryAllocator)
target_link_libraries(touchloader GPUOpen::VulkanMemoryAllocator)

# causing CUDA to not compile, commented calls to it for now
# # OpenImageIO
# # .\vcpkg install openimageio ### only needed for VRI to compile, keeping it for now
# #################################################################################################
# find_package(OpenImageIO CONFIG REQUIRED )
# target_link_libraries(tdpy OpenImageIO::OpenImageIO)

# CUDA
#################################################################################################
find_package(CUDAToolkit REQUIRED)
message(STATUS CUDAToolkit_INCLUDE_DIRS=${CUDAToolkit_INCLUDE_DIRS})
target_include_directories(tdpy PRIVATE ${CUDAToolkit_INCLUDE_DIRS})
target_link_libraries(tdpy CUDA::cudart)
target_include_directories(touchloader PRIVATE ${CUDAToolkit_INCLUDE_DIRS})
target_link_libraries(touchloader CUDA::cudart)


# Python
# make sure path to python is in the system PATH
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
# should come with clone of the repo (use git clone --recursive touchpy_url) 
#################################################################################################
target_link_directories(tdpy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/TouchEngine-Windows/lib)
target_link_libraries(tdpy TouchEngine)
target_include_directories(tdpy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/TouchEngine-Windows/include)
add_custom_command(TARGET tdpy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/TouchEngine-Windows/bin $<TARGET_FILE_DIR:tdpy>)

target_link_directories(touchloader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/TouchEngine-Windows/lib)
target_link_libraries(touchloader TouchEngine)
target_include_directories(touchloader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/TouchEngine-Windows/include)
add_custom_command(TARGET touchloader POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_CURRENT_SOURCE_DIR}/TouchEngine-Windows/bin $<TARGET_FILE_DIR:touchloader>)


# dependencies so tdpy depends on tocuhloader being built first
add_dependencies(tdpy touchloader)

add_definitions(-DCMAKE_PROJECT_DIR="${CMAKE_SOURCE_DIR}")

# install
#################################################################################################
#################################################################################################
install(TARGETS tdpy LIBRARY DESTINATION modules)
install(TARGETS touchloader RUNTIME DESTINATION bin)