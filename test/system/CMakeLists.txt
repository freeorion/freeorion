find_package(Boost ${MINIMUM_BOOST_VERSION}
    COMPONENTS
        unit_test_framework
        log
        log_setup
    REQUIRED)

add_executable(fo_systemtest_game
    main.cpp
    ClientAppFixture.cpp
)

target_compile_definitions(fo_systemtest_game
    PRIVATE
        -DFREEORION_BUILD_HUMAN
        -DBOOST_TEST_DYN_LINK
        -DBOOST_TEST_NO_LIB
        -DBOOST_LOG_DYN_LINK
        -DBOOST_LOG_NO_LIB
        -DBOOST_TEST_IGNORE_SIGCHLD
)

target_include_directories(fo_systemtest_game
    PRIVATE
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/GG/
        ${Boost_INCLUDE_DIRS}
)

target_link_libraries(fo_systemtest_game
    freeorioncommon
    freeorionparse
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES}
    ${Boost_LOG_LIBRARY}
    ${Boost_LOG_SETUP_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
)

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
        ${PROJECT_SOURCE_DIR}/GG/GG/ClrConstants.h
        ${PROJECT_SOURCE_DIR}/network/ClientNetworking.h
    PRIVATE
        ${PROJECT_SOURCE_DIR}/client/ClientApp.cpp
        ${PROJECT_SOURCE_DIR}/client/ClientFSMEvents.cpp
        ${PROJECT_SOURCE_DIR}/network/ClientNetworking.cpp
)

set(FO_TEST_GAME
        SmokeTestGame
        SmokeTestHostless
)

foreach(_TEST ${FO_TEST_GAME})
    target_sources(fo_systemtest_game
        PRIVATE
        ${_TEST}.cpp
    )
    # ToDo: Change `--catch_system_errors=no` to `yes` after moving to boost >= 1.59
    add_test(
        NAME ${_TEST}
        COMMAND fo_systemtest_game --log_level=message "--run_test=${_TEST}" --catch_system_error=no
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endforeach()

