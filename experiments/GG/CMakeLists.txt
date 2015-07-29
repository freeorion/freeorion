cmake_minimum_required(VERSION 2.6)

message("-- Configuring GG Tests")

find_package(OpenAL REQUIRED)
find_package(Ogg REQUIRED)
find_package(Vorbis REQUIRED)
find_package(Bullet REQUIRED)
find_package(GLEW REQUIRED)
include_directories(
    ${OPENGL_INCLUDE_DIR}
    ${OPENAL_INCLUDE_DIR}
    ${OGG_INCLUDE_DIR}
    ${VORBIS_INCLUDE_DIR}
    ${BULLET_INCLUDE_DIRS}
)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

# Include some useful utilities from the main code
set(freeorion_UTIL_SOURCES
    TestApp.cpp
    TestDialog.cpp
)

set(gg_test_LINK_LIBS
    GiGi
    GiGiSDL
    ${OPENGL_gl_LIBRARY}
    ${OPENGL_glu_LIBRARY}
    ${OPENAL_LIBRARY}
    ${Boost_LIBRARIES}
    ${OGG_LIBRARY}
    ${VORBIS_LIBRARIES}
    ${BULLET_LIBRARIES}
    ${ZLIB_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${GLEW_LIBRARIES}
)

if (WIN32)
    link_directories(${BOOST_LIBRARYDIR})
endif ()


macro (make_gigi_test name)
    add_executable(${name} ${ARGN} ${freeorion_UTIL_SOURCES})
    target_link_libraries(${name} ${gg_test_LINK_LIBS})
    set_target_properties(${name}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ..
    )
endmacro ()

make_gigi_test(text_control TestTextControl.cpp)