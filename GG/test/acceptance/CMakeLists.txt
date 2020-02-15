macro(ADD_GIGI_ACCEPTANCE_TEST name)
    add_executable(gg_acceptance_${name}
        ${ARGN}
        $<TARGET_OBJECTS:gigi_acceptance_runner>
    )
    target_link_libraries(gg_acceptance_${name}
        GiGi
        GiGiSDL
        Threads::Threads
        GLEW::GLEW
        ${OPENGL_gl_LIBRARY}
        ${OPENGL_glu_LIBRARY}
        ${SDL_LIBRARIES}
    )
    set_target_properties(gg_acceptance_${name}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endmacro()


message("-- Configuring GIGI acceptance tests")

include_directories(
    SYSTEM
    ${OPENGL_INCLUDE_DIR}
    ${SDL_INCLUDE_DIR}
)

add_library(gigi_acceptance_runner OBJECT
    runner/Application.cpp
    runner/Dialog.cpp
    runner/ResourceCursor.cpp
)

# Object libraries only understand the modern CMake approach of defining
# dependencies via imported targets and target_link_libraries starting 3.12.
# For now manually unpack the compiler definitions and include directories.
target_compile_definitions(gigi_acceptance_runner
    INTERFACE
        $<TARGET_PROPERTY:GLEW::GLEW,COMPILE_DEFINITIONS>
)

target_include_directories(gigi_acceptance_runner
    INTERFACE
        $<TARGET_PROPERTY:GLEW::GLEW,INCLUDE_DIRECTORIES>
)

add_gigi_acceptance_test(text_control TestTextControl.cpp)
