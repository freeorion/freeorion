find_package(CPPCheck)
find_package(PyCodestyle)

if(CPPCHECK_FOUND)
    add_custom_target(check-cpp
        ${CPPCHECK_EXECUTABLE}
            --quiet
            --template=gcc
            --std=c++11
            ${PROJECT_SOURCE_DIR}
        COMMENT "Check C++ for problematic code"
    )
endif()

if(PYCODESTYLE_FOUND)
    configure_file(pep8.ini.in pep8.ini)
    # The script wrapper discards the exit code of `pep8` to allow
    # continuing the build even when there are errors found by `pep8`.
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/pep8.cmake"
        "execute_process(
            COMMAND \"${PYCODESTYLE_EXECUTABLE}\" \"--config=${CMAKE_CURRENT_BINARY_DIR}/pep8.ini\"
                \"${PROJECT_SOURCE_DIR}\"
            WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\")
    ")
    add_custom_target(check-pep8
        "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/pep8.cmake"
        DEPENDS pep8.ini.in
        COMMENT "Check python code for PEP-8 style conformance"
        VERBATIM
        SOURCES pep8.ini.in
    )
endif()

if(NOT TARGET check)
    add_custom_target(check
        COMMENT "Run code quality checks"
    )
endif()

if(CPPCHECK_FOUND)
    add_dependencies(check check-cpp)
endif()

if(PYCODESTYLE_FOUND)
    add_dependencies(check check-pep8)
endif()
