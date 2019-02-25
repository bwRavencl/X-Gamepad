cmake_minimum_required(VERSION 3.0)
project(x_gamepad)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(Git)
if(Git_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --long --always
                    OUTPUT_VARIABLE VERSION
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    add_compile_definitions(VERSION=\"${VERSION}\")
else()
    message(WARNING "Git executable was not found, the version string will be left undefined.")
endif()


set(LIB_DIR "${PROJECT_SOURCE_DIR}/lib")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if(UNIX)
    set(PLATFORM_COMPILE_OPTIONS -Wall -Wunreachable-code -Wextra -Wshadow -Wfloat-equal -pedantic -O3 -s)
else()
    set(PLATFORM_COMPILE_OPTIONS /W3 /wd4996)
endif()

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.12)
    set(PLATFORM_CORE_DEFINITIONS
        APL=1
        LIN=0
        IBM=0)
else()
    if(LINUX)
        set(PLATFORM_CORE_DEFINITIONS
            APL=0
            LIN=1
            IBM=0
            GL_GLEXT_PROTOTYPES=1)
    else()
        set(PLATFORM_CORE_DEFINITIONS
            APL=0
            LIN=0
            IBM=1
            WIN32_LEAN_AND_MEAN=1
            GLEW_STATIC=1)
        set(PLATFORM_INCLUDE_DIRECTORIES
            ${LIB_DIR}/glew-2.1.0/include/GL
            ${LIB_DIR}/hidapi)
        set(PLATFORM_DEBUG_OPTIONS "-MTd")
        set(PLATFORM_RELEASE_OPTIONS "-MT")
    endif()
endif()

set(DEFINITIONS
    XPLM200=1
    XPLM210=1
    XPLM300=1
    XPLM301=1
    ${PLATFORM_CORE_DEFINITIONS})

include_directories(${PROJECT_BINARY_DIR}
                    ${LIB_DIR}/SDK/CHeaders/XPLM
                    ${LIB_DIR}/SDK/CHeaders/Widgets
                    ${PLATFORM_INCLUDE_DIRECTORIES})

set(SOURCES
    x_gamepad.cpp)

add_library(x_gamepad MODULE ${SOURCES})

target_compile_options(x_gamepad PRIVATE ${PLATFORM_COMPILE_OPTIONS})
target_compile_options(x_gamepad PUBLIC "$<$<CONFIG:DEBUG>:${PLATFORM_DEBUG_OPTIONS}>")
target_compile_options(x_gamepad PUBLIC "$<$<CONFIG:RELEASE>:${PLATFORM_RELEASE_OPTIONS}>")
target_compile_definitions(x_gamepad PRIVATE ${DEFINITIONS})

if(APPLE)
    target_link_libraries(x_gamepad "-framework OpenGL")
    target_link_libraries(x_gamepad "-framework CoreFoundation")
    target_link_libraries(x_gamepad "-framework XPWidgets")
    target_link_libraries(x_gamepad "-framework XPLM")
    set_target_properties(x_gamepad PROPERTIES LINK_FLAGS "-Wl,-F/Library/Frameworks, -Wl,-FSDK/Libraries/Mac\
    -Wl,-exported_symbol -Wl,_XPluginStart\
    -Wl,-exported_symbol -Wl,_XPluginEnable\
    -Wl,-exported_symbol -Wl,_XPluginReceiveMessage\
    -Wl,-exported_symbol -Wl,_XPluginDisable\
    -Wl,-exported_symbol -Wl,_XPluginStop")
    set_target_properties(x_gamepad PROPERTIES OUTPUT_NAME "apl")
else()
    if(LINUX)
        target_compile_options(x_gamepad PRIVATE "-fvisibility=hidden")
        set_target_properties(x_gamepad PROPERTIES OUTPUT_NAME "lin")
    else()
        target_link_libraries(x_gamepad "${LIB_DIR}/SDK/Libraries/Win/XPWidgets_64.lib")
        target_link_libraries(x_gamepad "${LIB_DIR}/SDK/Libraries/Win/XPLM_64.lib")
        target_link_libraries(x_gamepad "${LIB_DIR}/glew-2.1.0/lib/Release/x64/glew32s.lib")
        target_link_libraries(x_gamepad "Opengl32.lib")
        target_link_libraries(x_gamepad "${LIB_DIR}/hidapi/hidapi.lib")
        set_target_properties(x_gamepad PROPERTIES OUTPUT_NAME "win")
    endif()
endif()

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
    set_target_properties(x_gamepad PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} "bin/${OUTPUTCONFIG}/x_gamepad/64")
endforeach(OUTPUTCONFIG)

set_target_properties(x_gamepad PROPERTIES PREFIX "")
set_target_properties(x_gamepad PROPERTIES SUFFIX ".xpl")
