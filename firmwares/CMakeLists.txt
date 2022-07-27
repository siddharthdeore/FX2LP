cmake_minimum_required(VERSION 3.14)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# the name of the target operating system
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR 8051)

set(CMAKE_C_FLAGS " -mmcs51 -model-large --std-c99")

#set(CMAKE_C_FLAGS " -mmcs51 --code-size 0x1c00 --xram-size 0x0200 --xram-loc 0x1c00 --std-c99")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
# which compilers to use for C and C++
set(CMAKE_C_COMPILER sdcc)

# here is the target environment is located
SET(CMAKE_FIND_ROOT_PATH  /usr/share/sdcc)

project(FX2LP LANGUAGES C)
include_directories("libraries/fx2lib/include")
link_directories("libraries/fx2lib/lib")
# List subdirectories macro
macro(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
        SET(dirlist ${dirlist} ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
endmacro()

add_subdirectory(firmwares)
