# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS "components/btstack"
                         "../../../ext/bluepad32/src/components/bluepad32"
                         "../../../ext/bluepad32/src/components/cmd_nvs"
                         "../../../ext/bluepad32/src/components/cmd_nvs_4.4"
                         "../../../ext/bluepad32/src/components/cmd_system"
                         "../../../ext/bluepad32/src/components/cmd_system_4.4")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(application)
