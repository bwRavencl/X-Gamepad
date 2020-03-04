cmake_minimum_required(VERSION 3.7)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

project(x_gamepad)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(Git)
if(Git_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --long --always
                    OUTPUT_VARIABLE VERSION
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    add_definitions(-DVERSION=\"${VERSION}\")
else()
    message(WARNING "Git executable was not found, the version string will be left undefined.")
endif()

set(LIB_DIR "${PROJECT_SOURCE_DIR}/lib")

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if(CMAKE_HOST_WIN32)
    set(PLATFORM_COMPILE_OPTIONS /wd4100 /wd4267 /wd4996)
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_LINK_FLAGS} /MANIFEST:NO")
else()
    set(PLATFORM_COMPILE_OPTIONS -Wall -Wunreachable-code -Wextra -Wshadow -Wfloat-equal -Wno-unused-parameter -pedantic -Ofast)
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_LINK_FLAGS} -s")
endif()

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.10)
    set(PLATFORM_CORE_DEFINITIONS
        APL=1
        LIN=0
        IBM=0)
    set(PLATFORM_INCLUDE_DIRECTORIES
            ${LIB_DIR}/hidapi)
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
        set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            )
        foreach(CompilerFlag ${CompilerFlags})
            string(REPLACE "/W3" "/W4" ${CompilerFlag} "${${CompilerFlag}}")
            string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
            string(REPLACE "/MDd" "/MTd" ${CompilerFlag} "${${CompilerFlag}}")
        endforeach()
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
    x_gamepad.c)

add_library(x_gamepad MODULE ${SOURCES})

target_compile_options(x_gamepad PRIVATE ${PLATFORM_COMPILE_OPTIONS})
target_compile_options(x_gamepad PUBLIC "$<$<CONFIG:DEBUG>:${PLATFORM_DEBUG_OPTIONS}>")
target_compile_options(x_gamepad PUBLIC "$<$<CONFIG:RELEASE>:${PLATFORM_RELEASE_OPTIONS}>")
target_compile_definitions(x_gamepad PRIVATE ${DEFINITIONS})

if(APPLE)
    target_link_libraries(x_gamepad "-framework ApplicationServices")
    target_link_libraries(x_gamepad "-framework Carbon")
    target_link_libraries(x_gamepad "-framework CoreFoundation")
    target_link_libraries(x_gamepad "-framework OpenGL")
    target_link_libraries(x_gamepad "-framework XPWidgets")
    target_link_libraries(x_gamepad "-framework XPLM")
    set_target_properties(x_gamepad PROPERTIES LINK_FLAGS "\
    -Wl,-F/Library/Frameworks,\
    -Wl,-F${LIB_DIR}/SDK/Libraries/Mac\
    -Wl,-L${LIB_DIR}/hidapi\
    -Wl,-lhidapi\
    -Wl,-exported_symbol -Wl,_XPluginStart\
    -Wl,-exported_symbol -Wl,_XPluginEnable\
    -Wl,-exported_symbol -Wl,_XPluginReceiveMessage\
    -Wl,-exported_symbol -Wl,_XPluginDisable\
    -Wl,-exported_symbol -Wl,_XPluginStop")
    set_target_properties(x_gamepad PROPERTIES OUTPUT_NAME "apl")
else()
    if(LINUX)
        target_compile_options(x_gamepad PRIVATE "-fvisibility=hidden" -static-libgcc -static-libstdc++)
        set_target_properties(x_gamepad PROPERTIES OUTPUT_NAME "lin")
    else()
        if(NOT CMAKE_HOST_WIN32)
            target_link_libraries(x_gamepad -static-libgcc -static-libstdc++)
        endif()
        target_link_libraries(x_gamepad "${LIB_DIR}/SDK/Libraries/Win/XPWidgets_64.lib")
        target_link_libraries(x_gamepad "${LIB_DIR}/SDK/Libraries/Win/XPLM_64.lib")
        target_link_libraries(x_gamepad "${LIB_DIR}/glew-2.1.0/lib/Release/x64/glew32s.lib")
        target_link_libraries(x_gamepad "opengl32")
        target_link_libraries(x_gamepad "xinput")
        target_link_libraries(x_gamepad "${LIB_DIR}/hidapi/hidapi.lib")
        set_target_properties(x_gamepad PROPERTIES OUTPUT_NAME "win")
    endif()
endif()

set_target_properties(x_gamepad PROPERTIES LIBRARY_OUTPUT_DIRECTORY "bin/${CMAKE_BUILD_TYPE}/x_gamepad/64")
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
    set_target_properties(x_gamepad PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} "bin/${OUTPUTCONFIG}/x_gamepad/64")
endforeach()

set_target_properties(x_gamepad PROPERTIES PREFIX "")
set_target_properties(x_gamepad PROPERTIES SUFFIX ".xpl")
