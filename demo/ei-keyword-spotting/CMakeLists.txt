# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)


set(PRJ_BUILD_TARGET "$ENV{PRJ_BUILD_TARGET}")
set(PRJ_TARGET_SDKCONFIG "${CMAKE_SOURCE_DIR}/sdkconfig.${PRJ_BUILD_TARGET}")

set(SDKCONFIG_DEFAULTS "sdkconfig.defaults" "${PRJ_TARGET_SDKCONFIG}")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ei-keyword-spotting)
