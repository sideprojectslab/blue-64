# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS "components/btstack"
                         "../../../resources/bluepad32/src/components/bluepad32"
                         "../../../resources/bluepad32/src/components/bluepad32_arduino"
                         "../../../resources/bluepad32/src/components/cmd_nvs"
                         "../../../resources/bluepad32/src/components/cmd_nvs_4.4"
                         "../../../resources/bluepad32/src/components/cmd_system"
                         "../../../resources/bluepad32/src/components/cmd_system_4.4")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(bluepad32-app)
