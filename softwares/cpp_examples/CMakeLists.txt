cmake_minimum_required(VERSION 3.14)

project(FX2LP_SW_CPP)


SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})
foreach(subdir ${SUBDIRS})
    add_executable(${subdir}_cpp ${subdir}/main.cc)
    target_link_libraries(${subdir}_cpp usb-1.0)
endforeach()