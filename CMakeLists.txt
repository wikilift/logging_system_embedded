cmake_minimum_required(VERSION 3.13)
project(wl_log LANGUAGES C)


add_library(wl_log STATIC src/wl_log.c)

target_include_directories(wl_log PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
