message("-- Configuring GIGI acceptance tests")

find_package(GLEW REQUIRED)
include_directories(
    SYSTEM
    ${OPENGL_INCLUDE_DIR}
)

# Include some useful utilities from the main code
set(GIGI_TEST_ACCEPTANCE_SOURCES
    runner/Application.cpp
    runner/Dialog.cpp
    runner/ResourceCursor.cpp
)

set(GIGI_TEST_ACCEPTANCE_LINK_LIBS
    GiGi
    GiGiSDL
    ${OPENGL_gl_LIBRARY}
    ${OPENGL_glu_LIBRARY}
    ${Boost_LIBRARIES}
    ${ZLIB_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${GLEW_LIBRARIES}
)


macro (make_gigi_acceptance_test name)
    add_executable(gg_acceptance_${name} ${ARGN} ${GIGI_TEST_ACCEPTANCE_SOURCES})
    target_link_libraries(gg_acceptance_${name} ${GIGI_TEST_ACCEPTANCE_LINK_LIBS})
    set_target_properties(gg_acceptance_${name}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endmacro ()

make_gigi_acceptance_test(text_control TestTextControl.cpp)
