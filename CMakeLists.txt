# CMakeList.txt : CMake project for touchpy
#
cmake_minimum_required (VERSION 3.21)

function(add_copy_file_cmds target_name file_list dest_dir)
    foreach(file IN LISTS file_list)
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${file} ${dest_dir}
        )
    endforeach()
endfunction()

function(add_copy_dir_cmds target_name dir_list dest_dir)
    foreach(dir IN LISTS dir_list)
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${dir} ${dest_dir}
        )
    endforeach()
endfunction()


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

# .\vcpkg install vulkan ### haven't tested this, probably best to just install the SDK
find_package(Vulkan REQUIRED)
message(STATUS Vulkan_INCLUDE_DIR=${Vulkan_INCLUDE_DIR})

### not used atm but vri includes it... 
# .\vcpkg install vulkan-memory-allocator 
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

find_package(CUDAToolkit REQUIRED)
message(STATUS CUDAToolkit_INCLUDE_DIRS=${CUDAToolkit_INCLUDE_DIRS})

# .\vcpkg install spdlog
find_package(spdlog REQUIRED)

# .\vcpkg install gtest
find_package(GTest CONFIG REQUIRED)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# touchpy
#################################################################################################
find_package(Python ${python_version} EXACT 
    REQUIRED COMPONENTS Interpreter Development.Module
    OPTIONAL_COMPONENTS Development.SABIModule
)

message(STATUS "Python executable: ${Python_EXECUTABLE}")
if(DEFINED Python_SOABI)
    message(STATUS "Python SOABI: ${Python_SOABI}")
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/nanobind)
nanobind_add_module(touchpy NB_DOMAIN NOMINSIZE ${ALL_TPY_SOURCES})

target_compile_definitions(touchpy PRIVATE ${COMPILE_DEFINITIONS})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(touchpy PRIVATE DISABLE_DEBUG_BUILD)
endif()

target_include_directories(touchpy PRIVATE 
    ${SOURCE_DIR} 
    ${CUDAToolkit_INCLUDE_DIRS} 
    "${CMAKE_CURRENT_SOURCE_DIR}/external/TouchEngine-Windows/include"
)

target_link_directories(touchpy PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/external/TouchEngine-Windows/lib"
)

target_link_libraries(touchpy PRIVATE 
    TouchEngine 
    Vulkan::Vulkan 
    GPUOpen::VulkanMemoryAllocator
    CUDA::cudart 
    spdlog::spdlog
)

add_custom_command(TARGET touchpy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:touchpy> "${CMAKE_CURRENT_SOURCE_DIR}/install/modules"
)

# copy dlls to the build directory
set(target_dependencies
    "${CMAKE_CURRENT_SOURCE_DIR}/external/TouchEngine-Windows/bin/TouchEngine.dll"
    "${CUDAToolkit_BIN_DIR}/cudart64_110.dll"
    $<TARGET_FILE:spdlog::spdlog>
    $<TARGET_FILE:fmt::fmt>
)

add_copy_file_cmds(touchpy "${target_dependencies}" "${CMAKE_CURRENT_SOURCE_DIR}/install/modules")

# generate stub file to enable Sphinx to generate docs
#################################################################################################
if (Python_VERSION_MAJOR EQUAL 3 AND Python_VERSION_MINOR EQUAL 12)

    message(STATUS "Python version 3.12 detected. Generating stubs.")

    # copy dlls to the build directory for the module to work
    add_copy_file_cmds(touchpy "${target_dependencies}" $<TARGET_FILE_DIR:touchpy>)

    nanobind_add_stub(
        touchpy_stub
        MODULE touchpy
        OUTPUT touchpy.pyi
        PYTHON_PATH $<TARGET_FILE_DIR:touchpy>
        DEPENDS touchpy
    )

endif()

# touchpygtest
#################################################################################################
set(GTEST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest)
file(GLOB_RECURSE TEST_SOURCES "${GTEST_SOURCE_DIR}/*gtest*.cpp")

set(TOUCHPY_TEST_SOURCES ${ALL_TPY_SOURCES})
list(FILTER TOUCHPY_TEST_SOURCES EXCLUDE REGEX ".*py\\.cpp$")
add_executable (touchpygtest ${TEST_SOURCES} ${TOUCHPY_TEST_SOURCES})

target_compile_definitions(touchpygtest PRIVATE ${COMPILE_DEFINITIONS})

target_include_directories(touchpygtest PRIVATE 
    ${SOURCE_DIR}
    ${CUDAToolkit_INCLUDE_DIRS}
    "${CMAKE_CURRENT_SOURCE_DIR}/external/TouchEngine-Windows/include"
)

target_link_directories(touchpygtest PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/external/TouchEngine-Windows/lib
)

target_link_libraries(touchpygtest PRIVATE 
    GTest::gtest_main
    TouchEngine
    Vulkan::Vulkan
    GPUOpen::VulkanMemoryAllocator
    CUDA::cudart
    spdlog::spdlog
)

include(GoogleTest)
gtest_discover_tests(touchpygtest)
add_definitions(-DCMAKE_PROJECT_DIR="${CMAKE_SOURCE_DIR}")


