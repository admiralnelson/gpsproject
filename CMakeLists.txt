# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
set( CMAKE_SYSTEM_NAME  "Generic")
set(CMAKE_C_COMPILER "D:/dev/Espressif/tools/xtensa-esp32-elf/esp-2022r1-11.2.0/xtensa-esp32-elf/bin/xtensa-esp32-elf-gcc.exe")
cmake_minimum_required(VERSION 3.5)
set(EXTRA_COMPONENT_DIRS main components)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(solar)
