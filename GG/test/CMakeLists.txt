cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

message("-- Configuring Tests")

find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS unit_test_framework REQUIRED)

macro (make_test_exec name)
    add_executable(${name}-test Test${name}.cpp)
    target_link_libraries(${name}-test GiGi ${Boost_LIBRARIES})
    if (TARGET GiGiSDL)
        target_link_libraries(${name}-test GiGiSDL ${Boost_LIBRARIES})
    endif ()
endmacro ()

macro (add_test_and_data_files name)
    set(suffix "")
    set(args "")
    foreach (arg ${ARGN})
        if (EXISTS ${CMAKE_SOURCE_DIR}/test/${arg})
            get_filename_component(filename_without_path ${arg} NAME)
            configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${arg}
                           ${CMAKE_CURRENT_BINARY_DIR}/${filename_without_path}
                           COPYONLY)
            set(suffix ${suffix}-${filename_without_path})
            set(args "${args};${filename_without_path}")
        else ()
            set(suffix ${suffix}-${arg})
            set(args "${args};${arg}")
        endif ()
    endforeach ()
    add_test (${name}${suffix} ${CMAKE_BINARY_DIR}/${name}-test ${args})
endmacro ()

make_test_exec(StrongIntegralTypedef)
make_test_exec(StrongSizeTypedef)
make_test_exec(Lexer)
make_test_exec(ExpressionParser)
make_test_exec(ExpressionWriter)
make_test_exec(AdamParser)
make_test_exec(AdamWriter)
make_test_exec(EveParser)
make_test_exec(EveWriter)
make_test_exec(EveLayout)

add_test_and_data_files(StrongIntegralTypedef)
add_test_and_data_files(StrongSizeTypedef)
add_test_and_data_files(Lexer test_expressions adam_test_expressions_tokens)
add_test_and_data_files(Lexer test_expressions eve_test_expressions_tokens)
add_test_and_data_files(ExpressionParser test_expressions adam)
add_test_and_data_files(ExpressionParser test_expressions eve)
add_test_and_data_files(ExpressionWriter test_expressions adam)
add_test_and_data_files(ExpressionWriter test_expressions eve)

file(GLOB adam_test_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} asl_1.0.43_adam_files/*.adm)

foreach (test_file ${adam_test_files})
    add_test_and_data_files(AdamParser ${test_file})
endforeach ()

foreach (test_file ${adam_test_files})
    add_test_and_data_files(AdamWriter ${test_file})
endforeach ()

file(GLOB eve_test_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} asl_1.0.43_eve_files/*.eve)

foreach (test_file ${eve_test_files})
    add_test_and_data_files(EveParser ${test_file})
endforeach ()

foreach (test_file ${eve_test_files})
    add_test_and_data_files(EveWriter ${test_file})
endforeach ()

if (TARGET GiGiSDL)
    file(GLOB pngs *.png)
    file(COPY EveLayout_vs_Adobe_Begin.html begin_images ${pngs} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    foreach (test_file ${eve_test_files})
        add_test_and_data_files(EveLayout ${test_file} ${CMAKE_CURRENT_BINARY_DIR})
    endforeach ()
endif ()
