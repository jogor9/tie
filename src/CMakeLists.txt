cmake_minimum_required(VERSION 3.27)
project(TIE
        VERSION 0.0.0.0
        LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

option(PNG_SUPPORT "Build editor with PNG export support. Requires libpng")
option(LUA_SUPPORT "Build editor with Lua scripting support. ")
# option(FONTS_SUPPORT "Build library with font rendering support")
# option(USE_LUA_JIT "Use Just-In Time compilation for Lua scripts")
option(BUILD_TESTS "Build tests")
option(BUILD_DOCS "Build documentation")
option(PROFILING "Include profiling information")
option(DEBUG_INFO "Include debug information")
option(ANALYZE "Perform static analysis")
option(GENERATE_ASM "Generate assembly listings")
option(STRIP_EXEC "Strip symbols from executables")

set(SANITIZE "None" CACHE STRING
        "Define sanitization level")
set_property(CACHE SANITIZE PROPERTY STRINGS
        "None" "Fast" "Full")

set(OPTIMIZE "Size" CACHE STRING
        "Define optimization level")
set_property(CACHE OPTIMIZE PROPERTY STRINGS
        "Size" "Speed" "Debug")

if(MSVC)
        message(WARNING "MSVC is not fully supported, expect bugs")
        set(OPTS /Wall /sdl /fp:fast /TC /utf-8 /validate-charset /std:c11 /permissive-)
        if(PROFILING)
                list(APPEND OPTS /fsanitize-coverage)
        endif()
        if(DEBUG_INFO)
                list(APPEND OPTS /Z7)
        endif()
        if(ANALYZE)
                list(APPEND OPTS /fanalyze)
        endif()
        if(GENERATE_ASM)
                list(APPEND OPTS /Fa)
        endif()
        if(${OPTIMIZE} STREQUAL "Size")
                list(APPEND OPTS /O1)
        elseif(${OPTIMIZE} STREQUAL "Speed")
                list(APPEND OPTS /O2)
        else() # Debug
                list(APPEND OPTS /Od)
        endif()
        if(${SANITIZE} STREQUAL "Full")
                list(APPEND OPTS /fsanitize /GS /guard:cf /Gu)
        endif()
else()
        # warn, use C11, and floating-point without NaNs and INFs
        set(OPTS -Wall -Wextra -pedantic -Wno-unused-parameter -ffast-math -std=c11)
        if(PROFILING)
                list(APPEND OPTS -pg --coverage -fprofile-abs-path)
        endif()
        if(DEBUG_INFO)
                list(APPEND OPTS -ggdb3)
        endif()
        if(ANALYZE)
                list(APPEND OPTS -fanalyzer -Wanalyzer-too-complex)
        endif()
        if(GENERATE_ASM)
                list(APPEND OPTS -save-temps=obj -fverbose-asm)
        endif()
        if(${OPTIMIZE} STREQUAL "Size")
                list(APPEND OPTS -Os)
        elseif(${OPTIMIZE} STREQUAL "Speed")
                list(APPEND OPTS -O2)
        else() # Debug
                list(APPEND OPTS -Og)
        endif()
        if(${SANITIZE} STREQUAL "Full")
                list(APPEND OPTS -fsanitize=address,leak,undefined,pointer-compare,pointer-subtract,float-divide-by-zero,float-cast-overflow,bounds-strict -fsanitize-trap=all -D_FORTIFY_SOURCE=3  -D_GLIBCXX_ASSERTIONS -ftrivial-auto-var-init=zero -fstack-protector-strong -fstack-clash-protection -fcf-protection=full -fharden-conditional-branches -fharden-compares -fharden-control-flow-redundancy)
        elseif(${SANITIZE} STREQUAL "Fast")
                list(APPEND OPTS -fsanitize=undefined,float-divide-by-zero,float-cast-overflow,bounds-strict -fsanitize-trap=all)
        endif()
endif()

set(LIBRARY_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/tie/memalloc.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/tie/math.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/tie/algo.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/tie/bit.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/tie/base_array.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/tie/array.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/tie/numeric_array.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/tie/numeric.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/tie/heap.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/tie/functional.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/tie/numeric.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/tie/random.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/tie/attrib.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/tie/pointer.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/tie/geometry.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/tie/geometry.c")
set(EDITOR_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/editor/editor.c")
set(TEST_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/test/test.c")
set(EDITOR_LIBS tie)
set(TEST_LIBS tie)
set(TEST_DIRS "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(SDL2 REQUIRED)
list(APPEND EDITOR_LIBS ${SDL2_LIBRARIES})
list(APPEND EDITOR_DIRS ${SDL2_INCLUDE_DIRS})
list(APPEND LIBRARY_LIBS ${SDL2_LIBRARIES})
list(APPEND LIBRARY_DIRS ${SDL2_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)
list(APPEND EDITOR_LIBS ${OPENGL_LIBRARIES})
list(APPEND EDITOR_DIRS ${OPENGL_INCLUDE_DIR})
list(APPEND LIBRARY_LIBS ${OPENGL_LIBRARIES})
list(APPEND LIBRARY_DIRS ${OPENGL_INCLUDE_DIR})

find_package(Fontconfig REQUIRED)
list(APPEND LIBRARY_DIRS ${Fontconfig_INCLUDE_DIRS})
list(APPEND LIBRARY_LIBS ${Fontconfig_LIBRARIES})
list(APPEND EDITOR_OPTS ${Fontconfig_COMPILE_OPTIONS})
list(APPEND EDITOR_DIRS ${Fontconfig_INCLUDE_DIRS})
list(APPEND EDITOR_LIBS ${Fontconfig_LIBRARIES})

find_package(Freetype REQUIRED)
list(APPEND LIBRARY_LIBS ${FREETYPE_LIBRARIES})
list(APPEND INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS})

find_package(PNG)
if(${PNG_FOUND} AND ${PNG_SUPPORT})
        list(APPEND EDITOR_DEFS HASPNG=1)
        list(APPEND EDITOR_DEFS ${PNG_DEFINITIONS})
        list(APPEND EDITOR_LIBS ${PNG_LIBRARIES})
        list(APPEND EDITOR_DIRS ${PNG_INCLUDE_DIRS})
endif()

find_package(Lua)
if(${LUA_FOUND} AND ${LUA_SUPPORT})
        if(NOT ${LUA_VERSION_MAJOR} EQUAL 5)
                message(WARNING "Unsupported Lua version, expected Lua 5.x")
        else()
                list(APPEND EDITOR_DEFS HASLUA=1)
                list(APPEND EDITOR_LIBS ${LUA_LIBRARIES})
                list(APPEND EDITOR_DIRS ${LUA_INCLUDE_DIR})
        endif()
endif()

add_custom_command(OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/glad/src/gl.c"
        COMMAND glad --api gl:core=3.3 --out-path "${CMAKE_CURRENT_SOURCE_DIR}/glad" --reproducible c
        VERBATIM)
add_library(glad OBJECT "${CMAKE_CURRENT_SOURCE_DIR}/glad/src/gl.c")
target_include_directories(glad
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/glad/include" "${OPENGL_INCLUDE_DIR}")
list(APPEND LIBRARY_SOURCES $<TARGET_OBJECTS:glad>)
list(APPEND EDITOR_SOURCES $<TARGET_OBJECTS:glad>)

add_library(tie ${LIBRARY_SOURCES})
target_compile_definitions(tie PRIVATE ${LIBRARY_DEFS})
target_include_directories(tie PRIVATE ${LIBRARY_DIRS})
target_link_libraries(tie ${LIBRARY_LIBS})
if(MSVC)
        target_compile_options(tie PRIVATE ${OPTS} ${LIBRARY_OPTS})
else()
        target_compile_options(tie PRIVATE ${OPTS} ${LIBRARY_OPTS} -fpic)
endif()
target_link_options(tie PRIVATE ${OPTS} ${LIBRARY_OPTS})

add_executable(tie-edit ${EDITOR_SOURCES})
target_compile_definitions(tie-edit PRIVATE ${EDITOR_DEFS})
target_include_directories(tie-edit PRIVATE ${EDITOR_DIRS})
target_link_libraries(tie-edit ${EDITOR_LIBS})
if(MSVC)
        target_compile_options(tie-edit PRIVATE ${OPTS} ${EDITOR_OPTS})
else()
        target_compile_options(tie-edit PRIVATE ${OPTS} ${EDITOR_OPTS} -fpie)
        if(STRIP_EXEC)
                target_link_options(tie-edit PRIVATE ${OPTS} ${EDITOR_OPTS} -s -pie)
        else()
                target_link_options(tie-edit PRIVATE ${OPTS} ${EDITOR_OPTS} -pie)
        endif()
endif()

if(BUILD_TESTS)
        add_executable(tie-test ${TEST_SOURCES})
        target_compile_definitions(tie-test PRIVATE ${TEST_DEFS})
        target_include_directories(tie-test PRIVATE ${TEST_DIRS})
        target_link_libraries(tie-test ${TEST_LIBS})
        if(MSVC)
                target_compile_options(tie-test PRIVATE ${OPTS} ${TEST_OPTS})
        else()
                target_compile_options(tie-test PRIVATE ${OPTS} ${TEST_OPTS} -fpie)
                if(STRIP_EXEC)
                        target_link_options(tie-test PRIVATE ${OPTS} ${TEST_OPTS} -s -pie)
                else()
                        target_link_options(tie-test PRIVATE ${OPTS} ${TEST_OPTS} -pie)
                endif()
        endif()
endif()

# set_target_properties(tie PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")
# include(GNUInstallDirs)
# install(TARGETS tie
#         PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/tie")
# install(TARGETS tie-edit)
