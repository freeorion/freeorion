cmake_minimum_required(VERSION 2.6)

message("-- Configuring Tutorials")

macro (make_gigi_tutorial name)
    if (TARGET GiGiSDL)
        add_executable(${name} ${ARGN})
        target_link_libraries(${name} GiGiSDL ${Boost_LIBRARIES})
    endif ()
endmacro ()

make_gigi_tutorial(minimal minimal.cpp)
make_gigi_tutorial(controls controls.cpp)
make_gigi_tutorial(adam adam.cpp)
if (BUILD_EXPERIMENTAL_EVE_SUPPORT)
    make_gigi_tutorial(eve eve.cpp)
endif ()
