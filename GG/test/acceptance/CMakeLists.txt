macro(ADD_GIGI_ACCEPTANCE_TEST name)
    add_executable(gg_acceptance_${name}
        ${ARGN}
        $<TARGET_OBJECTS:gigi_acceptance_runner>
    )
    target_link_libraries(gg_acceptance_${name}
        GiGi
        GiGiSDL
        ${OPENGL_gl_LIBRARY}
        ${OPENGL_glu_LIBRARY}
        ${CMAKE_THREAD_LIBS_INIT}
        ${GLEW_LIBRARIES}
        ${SDL_LIBRARIES}
    )
    set_target_properties(gg_acceptance_${name}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endmacro()


message("-- Configuring GIGI acceptance tests")

find_package(GLEW REQUIRED)
include_directories(
    SYSTEM
    ${OPENGL_INCLUDE_DIR}
    ${SDL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
)

add_library(gigi_acceptance_runner OBJECT
    runner/Application.cpp
    runner/Dialog.cpp
    runner/ResourceCursor.cpp
)

add_gigi_acceptance_test(text_control TestTextControl.cpp)
