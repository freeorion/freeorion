find_package(CPPCheck)
find_package(Flake8)
find_package(PythonInterp ${MINIMUM_PYTHON_VERSION})

if(TARGET Cppcheck::cppcheck)
    add_custom_target(check-cpp
        COMMAND
            "$<TARGET_FILE:Cppcheck::cppcheck>"
            --quiet
            --template=gcc
            --std=c++14
            ${PROJECT_SOURCE_DIR}
        COMMENT "Check C++ for problematic code"
    )
endif()

if(TARGET Flake8::flake8)
    add_custom_target(check-pep8
        COMMAND "$<TARGET_FILE:Flake8::flake8>"
        COMMENT "Check python code for PEP-8 style conformance"
        VERBATIM
        SOURCES "${PROJECT_SOURCE_DIR}/default/python/tox.ini"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/default/python"
    )
endif()

if(PYTHON_EXECUTABLE)
    add_custom_target(check-python
        COMMAND
        "${PYTHON_EXECUTABLE}" -m compileall -qf .
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/default/python"
    )
endif()

if(NOT TARGET check)
    add_custom_target(check
        COMMENT "Run code quality checks"
    )
endif()

if(TARGET check-cpp)
    add_dependencies(check check-cpp)
endif()

if(TARGET check-pep8)
    add_dependencies(check check-pep8)
endif()

if(TARGET check-python)
    add_dependencies(check check-python)
endif()
