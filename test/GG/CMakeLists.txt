message("-- Configuring GG Tests")

find_package(GLEW REQUIRED)
include_directories(
    ${OPENGL_INCLUDE_DIR}
)

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
    ${Boost_LIBRARIES}
    ${ZLIB_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${GLEW_LIBRARIES}
)


macro (make_gigi_test name)
    add_executable(${name} ${ARGN} ${freeorion_UTIL_SOURCES})
    target_link_libraries(${name} ${gg_test_LINK_LIBS})
    set_target_properties(${name}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ..
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
    )
endmacro ()

make_gigi_test(text_control TestTextControl.cpp)
