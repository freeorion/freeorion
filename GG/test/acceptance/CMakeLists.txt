macro(ADD_GIGI_ACCEPTANCE_TEST name)
    add_executable(gg_acceptance_${name}
        ${ARGN}
    )
    target_link_libraries(gg_acceptance_${name}
        gigi_acceptance_runner
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
)

add_library(gigi_acceptance_runner STATIC
    runner/Application.cpp
    runner/Dialog.cpp
    runner/ResourceCursor.cpp
)

target_link_libraries(gigi_acceptance_runner
    PUBLIC
        GiGi
        GiGiSDL
        GLEW::GLEW
        ${SDL_LIBRARIES}
)

target_include_directories(gigi_acceptance_runner
    PUBLIC
        ${SDL_INCLUDE_DIR}
)

add_gigi_acceptance_test(text_control TestTextControl.cpp)
