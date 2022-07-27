cmake_minimum_required(VERSION 3.14)

project(FX2LP_FW LANGUAGES C)

SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})

foreach(subdir ${SUBDIRS})
    add_executable(${subdir}_fw ${subdir}/main.c)
    target_link_libraries(${subdir}_fw fx2.lib)
endforeach()