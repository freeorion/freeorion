find_package(Boost ${MINIMUM_BOOST_VERSION}
    COMPONENTS
        unit_test_framework
    REQUIRED)

add_executable(fo_systemtest_game
    main.cpp
    ClientAppFixture.cpp
)

target_compile_definitions(fo_systemtest_game
    PRIVATE
        -DBOOST_TEST_IGNORE_SIGCHLD
)

target_include_directories(fo_systemtest_game
    PRIVATE
        ${PROJECT_SOURCE_DIR}
)

target_link_libraries(fo_systemtest_game
    freeorioncommon
    freeorionparse
    Boost::unit_test_framework
)

target_dependencies_copy_to_build(fo_systemtest_game)

add_coverage(fo_systemtest_game
    unittest
)

add_dependencies(unittest
    freeoriond
    fo_systemtest_game
)

target_sources(fo_systemtest_game
    PUBLIC
        ${PROJECT_SOURCE_DIR}/client/ClientApp.h
        ${PROJECT_SOURCE_DIR}/client/ClientFSMEvents.h
        ${PROJECT_SOURCE_DIR}/client/ClientNetworking.h
    PRIVATE
        ${PROJECT_SOURCE_DIR}/client/ClientApp.cpp
        ${PROJECT_SOURCE_DIR}/client/ClientFSMEvents.cpp
        ${PROJECT_SOURCE_DIR}/client/ClientNetworking.cpp
        ${PROJECT_SOURCE_DIR}/util/DependencyVersions.cpp
)

set(FO_TEST_GAME
        SmokeTestGame
        SmokeTestHostless
        TestChecksum
)

foreach(_TEST ${FO_TEST_GAME})
    target_sources(fo_systemtest_game
        PRIVATE
        ${_TEST}.cpp
    )
    if(NOT APPLE)
        add_test(
            NAME ${_TEST}
            COMMAND fo_systemtest_game --log_level=all "--run_test=${_TEST}" --catch_system_error=yes
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    endif()
endforeach()

