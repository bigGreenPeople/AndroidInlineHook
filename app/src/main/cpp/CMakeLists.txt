cmake_minimum_required(VERSION 3.4.1)

add_library(inlineHook SHARED
        hooktest.cpp
        inlineHook.c
        relocate.c)

target_link_libraries(
        inlineHook
        android
        log)
find_library(
        log-lib
        log )

