find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS unit_test_framework REQUIRED)

add_executable(fo_unittest_save
    main.cpp
)

target_include_directories(fo_unittest_save
    PRIVATE
        ${PROJECT_SOURCE_DIR}
)

target_link_libraries(fo_unittest_save
    freeorioncommon
    freeorionparse
    Threads::Threads
    Boost::boost
    Boost::disable_autolinking
    Boost::dynamic_linking
    Boost::unit_test_framework
)

target_dependencies_copy_to_build(fo_unittest_save)

add_dependencies(unittest
    fo_unittest_save
)

target_sources(fo_unittest_save
    PUBLIC
        ${PROJECT_SOURCE_DIR}/server/SaveLoad.h
    PRIVATE
        ${PROJECT_SOURCE_DIR}/server/SaveLoad.cpp
)

set(FO_TEST_SAVE
    EmptySave
)

foreach(_TEST ${FO_TEST_SAVE})
    target_sources(fo_unittest_save
        PRIVATE
        ${_TEST}.cpp
    )
    add_test(
        NAME ${_TEST}
        COMMAND fo_unittest_save --log_level=all "--run_test=${_TEST}" --catch_system_error=yes
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endforeach()
