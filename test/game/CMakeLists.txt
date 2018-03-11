find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS unit_test_framework REQUIRED)

add_executable(fo_unittest_game
    main.cpp
)

target_compile_definitions(fo_unittest_game
    PRIVATE
        -DFREEORION_BUILD_SERVER
        -DBOOST_TEST_DYN_LINK
)

target_include_directories(fo_unittest_game
    PRIVATE
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/GG/
)

target_link_libraries(fo_unittest_game
    freeorioncommon
    freeorionparse
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

add_coverage(fo_unittest_game
    unittest
)

add_dependencies(unittest
    fo_unittest_game
)

set(FO_TEST_GAME
    TestGame
)

foreach(_TEST ${FO_TEST_GAME})
    target_sources(fo_unittest_game
        PRIVATE
        ${_TEST}.cpp
    )
    add_test(
        NAME ${_TEST}
        COMMAND fo_unittest_game --log_level=message "--run_test=${_TEST}" --catch_system_error=yes
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endforeach()

