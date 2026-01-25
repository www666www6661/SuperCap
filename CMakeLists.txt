cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME ENTERPRIZE_SUPERCAP)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Local Dirs
set(BSP_Dirs ${CMAKE_CURRENT_SOURCE_DIR}/bsp)
set(DEV_Dirs ${CMAKE_CURRENT_SOURCE_DIR}/Src/dev)
set(APP_Dirs ${CMAKE_CURRENT_SOURCE_DIR}/Src/app)
set(COMP_Dirs ${CMAKE_CURRENT_SOURCE_DIR}/Src/comp)
set(MOD_Dirs ${CMAKE_CURRENT_SOURCE_DIR}/Src/module)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${BSP_Dirs}/bsp_pwm.c
    ${DEV_Dirs}/dev_buzzer.c
    ${APP_Dirs}/SuperCap.c
    ${BSP_Dirs}/bsp_adc.c
    ${BSP_Dirs}/bsp_can.c
    ${BSP_Dirs}/bsp_gpio.c
    ${COMP_Dirs}/comp_pid.c
    ${COMP_Dirs}/comp_utils.c
    ${MOD_Dirs}/mod_errchecker.c
    ${MOD_Dirs}/mod_powerctrl.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${BSP_Dirs}
    ${DEV_Dirs}
    ${APP_Dirs}
    ${COMP_Dirs}
    ${MOD_Dirs}
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wmissing-field-initializers
    -Werror 
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
