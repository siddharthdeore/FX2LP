cmake_minimum_required(VERSION 3.14)

project(FX2LP_SW)


SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})

foreach(subdir ${SUBDIRS})
    add_executable(${subdir} ${subdir}/main.c)
    target_link_libraries(${subdir} usb-1.0 usb)
endforeach()