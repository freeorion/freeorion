find_package(Boost ${MINIMUM_BOOST_VERSION}
    COMPONENTS
        unit_test_framework
        log
        log_setup
    REQUIRED)

add_executable(fo_unittest_game
    main.cpp
)

target_compile_definitions(fo_unittest_game
    PRIVATE
        -DFREEORION_BUILD_HUMAN
        -DBOOST_TEST_DYN_LINK
        -DBOOST_LOG_DYN_LINK
        -DBOOST_TEST_IGNORE_SIGCHLD
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
    ${Boost_LOG_LIBRARY}
    ${Boost_LOG_SETUP_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
)

add_coverage(fo_unittest_game
    unittest
)

add_dependencies(unittest
    fo_unittest_game
)

add_dependencies(fo_unittest_game
    freeoriond
)

target_sources(fo_unittest_game
    PUBLIC
        ${PROJECT_SOURCE_DIR}/client/ClientApp.h
        ${PROJECT_SOURCE_DIR}/client/ClientFSMEvents.h
        ${PROJECT_SOURCE_DIR}/network/ClientNetworking.h
        ${PROJECT_SOURCE_DIR}/util/Directories.h
        ${PROJECT_SOURCE_DIR}/util/binreloc.h
        ${PROJECT_SOURCE_DIR}/util/Process.h
    PRIVATE
        ${PROJECT_SOURCE_DIR}/client/ClientApp.cpp
        ${PROJECT_SOURCE_DIR}/client/ClientFSMEvents.cpp
        ${PROJECT_SOURCE_DIR}/network/ClientNetworking.cpp
        ${PROJECT_SOURCE_DIR}/util/Directories.cpp
        ${PROJECT_SOURCE_DIR}/util/binreloc.c
        ${PROJECT_SOURCE_DIR}/util/Process.cpp
)

set(FO_TEST_GAME
    TestGame
)

foreach(_TEST ${FO_TEST_GAME})
    target_sources(fo_unittest_game
        PRIVATE
        ${_TEST}.cpp
    )
    # ToDo: Change `--catch_system_errors=no` to `yes` after moving to boost >= 1.59
    add_test(
        NAME ${_TEST}
        COMMAND fo_unittest_game --log_level=message "--run_test=${_TEST}" --catch_system_error=no
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endforeach()

