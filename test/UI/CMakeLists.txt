macro(ADD_FO_ACCEPTANCE_TEST name)
    add_executable(fo_acceptance_${name}
        ${ARGN}
    )
    target_link_libraries(fo_acceptance_${name}
        fo_acceptance_runner
    )
endmacro()

find_package(GLEW REQUIRED)

include_directories(
    SYSTEM
    ${OPENGL_INCLUDE_DIR}
)

add_library(fo_acceptance_runner STATIC
    runner/Application.cpp
    runner/Dialog.cpp
    runner/ResourceCursor.cpp
    ../../src/client/gigi/ui/SDLGUI.cpp
)

target_link_libraries(fo_acceptance_runner
    PUBLIC
        Boost::system
        GiGi::GiGi
        GLEW::GLEW
        ${SDL_LIBRARIES}
)

target_include_directories(fo_acceptance_runner
    PUBLIC
        ${SDL_INCLUDE_DIR}
)

add_fo_acceptance_test(text_control TestTextControl.cpp)
