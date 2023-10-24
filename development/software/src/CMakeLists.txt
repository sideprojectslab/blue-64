# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS "$ENV{BLUEPAD32_PATH}/src/components/bluepad32"
                         "$ENV{BLUEPAD32_PATH}/src/components/bluepad32_arduino"
                         "$ENV{BLUEPAD32_PATH}/src/components/btstack"
                         "$ENV{BLUEPAD32_PATH}/src/components/cmd_nvs"
                         "$ENV{BLUEPAD32_PATH}/src/components/cmd_nvs_4.4"
                         "$ENV{BLUEPAD32_PATH}/src/components/cmd_system"
                         "$ENV{BLUEPAD32_PATH}/src/components/cmd_system_4.4")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(bluepad32-app)
