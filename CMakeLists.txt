# CMakeList.txt : CMake project for touchpy
#
cmake_minimum_required (VERSION 3.21)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_CUDA_ARCHITECTURES 75;80;86;89)

# set cpp standard for all targets unless otherwise specified 
# with target_compile_features() or set_property()
set(CMAKE_CXX_STANDARD 17) # need to upgrade CUDA to 12.1 to use C++20
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


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
      message(STATUS "touchpy source: ${source}")
    endforeach()
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/nanobind)
nanobind_add_module(touchpy NB_DOMAIN NOMINSIZE ${ALL_TPY_SOURCES})
target_include_directories(touchpy PRIVATE ${SOURCE_DIR})


# Google Test
#################################################################################################
#################################################################################################
set(GTEST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest)
file(GLOB_RECURSE TEST_SOURCES "${GTEST_SOURCE_DIR}/*gtest*.cpp")

set(TOUCHPY_TEST_SOURCES ${ALL_TPY_SOURCES})
list(FILTER TOUCHPY_TEST_SOURCES EXCLUDE REGEX ".*py\\.cpp$")

add_executable (touchpygtest ${TEST_SOURCES} ${TOUCHPY_TEST_SOURCES})
target_include_directories(touchpygtest PRIVATE ${SOURCE_DIR})


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

target_compile_definitions(touchpy PRIVATE ${COMPILE_DEFINITIONS})
target_compile_definitions(touchpygtest PRIVATE ${COMPILE_DEFINITIONS})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(touchpy PRIVATE DISABLE_DEBUG_BUILD)
endif()

add_custom_command(TARGET touchpy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:touchpy> "${CMAKE_CURRENT_SOURCE_DIR}/install/modules") 



# Google Test
# .\vcpkg install gtest
#################################################################################################
find_package(GTest CONFIG REQUIRED)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

target_link_libraries(touchpygtest PRIVATE GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(touchpygtest)


# Vulkan
# .\vcpkg install vulkan ### haven't tested this, probably best to just install the SDK
#################################################################################################
find_package(Vulkan REQUIRED)
message(STATUS Vulkan_INCLUDE_DIR=${Vulkan_INCLUDE_DIR})
target_link_libraries(touchpy PRIVATE Vulkan::Vulkan)
target_link_libraries(touchpygtest PRIVATE Vulkan::Vulkan)


### not used atm but vri includes it... 
# VMA
# .\vcpkg install vulkan-memory-allocator 
#################################################################################################
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
target_link_libraries(touchpy PRIVATE GPUOpen::VulkanMemoryAllocator)
target_link_libraries(touchpygtest PRIVATE GPUOpen::VulkanMemoryAllocator)

# causing CUDA to not compile, commented calls to it for now
# # OpenImageIO
# # .\vcpkg install openimageio ### only needed for VRI to compile, keeping it for now
# #################################################################################################
# find_package(OpenImageIO CONFIG REQUIRED )
# target_link_libraries(touchpy OpenImageIO::OpenImageIO)

# CUDA
#################################################################################################
find_package(CUDAToolkit REQUIRED)
message(STATUS CUDAToolkit_INCLUDE_DIRS=${CUDAToolkit_INCLUDE_DIRS})
target_include_directories(touchpy PRIVATE ${CUDAToolkit_INCLUDE_DIRS})
target_link_libraries(touchpy PRIVATE CUDA::cudart)
target_include_directories(touchpygtest PRIVATE ${CUDAToolkit_INCLUDE_DIRS})
target_link_libraries(touchpygtest PRIVATE CUDA::cudart)

# spdlog
# .\vcpkg install spdlog
#################################################################################################
find_package(spdlog REQUIRED)
target_link_libraries(touchpy PRIVATE spdlog::spdlog)
target_link_libraries(touchpygtest PRIVATE spdlog::spdlog)
add_custom_command(TARGET touchpy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:spdlog::spdlog> "${CMAKE_CURRENT_SOURCE_DIR}/install/modules")
add_custom_command(TARGET touchpy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:fmt::fmt> "${CMAKE_CURRENT_SOURCE_DIR}/install/modules")


# Post-build command to copy the CUDA runtime DLL to the output directory
set(cuda_runtime_dll "${CUDAToolkit_BIN_DIR}/cudart64_110.dll")
add_custom_command(TARGET touchpy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${cuda_runtime_dll} $<TARGET_FILE_DIR:touchpy>)

add_custom_command(TARGET touchpy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${cuda_runtime_dll} "${CMAKE_CURRENT_SOURCE_DIR}/install/modules")

# Python
# make sure path to python is in the system PATH
#################################################################################################
find_package(Python COMPONENTS Interpreter Development.Module Development.SABIModule REQUIRED)
# target_link_libraries(touchpy Python::Module Python::SABIModule)

# find_package(Python COMPONENTS Interpreter Development REQUIRED)
# target_link_libraries(touchpy Python::Python)
# target_include_directories(touchpy PRIVATE ${Python_INCLUDE_DIRS})

# TouchEngine-Windows
# should come with clone of the repo (use git clone --recursive touchpy_url) 
#################################################################################################
target_link_directories(touchpy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/TouchEngine-Windows/lib)
target_link_libraries(touchpy PRIVATE TouchEngine)
target_include_directories(touchpy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/TouchEngine-Windows/include)

add_custom_command(TARGET touchpy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/external/TouchEngine-Windows/bin $<TARGET_FILE_DIR:touchpy>)

add_custom_command(TARGET touchpy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/external/TouchEngine-Windows/bin "${CMAKE_CURRENT_SOURCE_DIR}/install/modules")    

target_link_directories(touchpygtest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/TouchEngine-Windows/lib)
target_link_libraries(touchpygtest PRIVATE TouchEngine)
target_include_directories(touchpygtest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/TouchEngine-Windows/include)

# dependencies so touchpy depends on touchloader being built first
#################################################################################################

add_definitions(-DCMAKE_PROJECT_DIR="${CMAKE_SOURCE_DIR}")

# install
#################################################################################################
#################################################################################################

install(TARGETS touchpy LIBRARY DESTINATION modules CONFIGURATIONS Release)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/external/TouchEngine-Windows/bin/TouchEngine.dll DESTINATION modules CONFIGURATIONS Release)
install(FILES ${cuda_runtime_dll} DESTINATION modules CONFIGURATIONS Release)
