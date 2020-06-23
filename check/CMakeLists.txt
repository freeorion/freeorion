find_package(CPPCheck)
find_package(ClangFormat)
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

if(TARGET ClangFormat::clang-format)
    add_custom_target(check-cpp-format
        ${CMAKE_CURRENT_SOURCE_DIR}/clang-format-report.py
            --config=${CMAKE_CURRENT_SOURCE_DIR}/clang-format
            --clang-format-executable=$<TARGET_FILE:ClangFormat::clang-format>
            --color
            ${PROJECT_SOURCE_DIR}/client/*.h
            ${PROJECT_SOURCE_DIR}/client/*.cpp
            ${PROJECT_SOURCE_DIR}/client/AI/*.h
            ${PROJECT_SOURCE_DIR}/client/AI/*.cpp
            ${PROJECT_SOURCE_DIR}/client/human/*.h
            ${PROJECT_SOURCE_DIR}/client/human/*.cpp
            ${PROJECT_SOURCE_DIR}/combat/*.h
            ${PROJECT_SOURCE_DIR}/combat/*.cpp
            ${PROJECT_SOURCE_DIR}/Empire/*.h
            ${PROJECT_SOURCE_DIR}/Empire/*.cpp
            ${PROJECT_SOURCE_DIR}/network/*.h
            ${PROJECT_SOURCE_DIR}/network/*.cpp
            ${PROJECT_SOURCE_DIR}/parse/*.h
            ${PROJECT_SOURCE_DIR}/parse/*.cpp
            ${PROJECT_SOURCE_DIR}/python/*.h
            ${PROJECT_SOURCE_DIR}/python/*.cpp
            ${PROJECT_SOURCE_DIR}/server/*.h
            ${PROJECT_SOURCE_DIR}/server/*.cpp
            ${PROJECT_SOURCE_DIR}/test/parse/*.h
            ${PROJECT_SOURCE_DIR}/test/parse/*.cpp
            ${PROJECT_SOURCE_DIR}/UI/*.h
            ${PROJECT_SOURCE_DIR}/UI/*.cpp
            ${PROJECT_SOURCE_DIR}/UI/CombatReport/*.h
            ${PROJECT_SOURCE_DIR}/UI/CombatReport/*.cpp
            ${PROJECT_SOURCE_DIR}/universe/*.h
            ${PROJECT_SOURCE_DIR}/universe/*.cpp
            ${PROJECT_SOURCE_DIR}/util/*.h
            ${PROJECT_SOURCE_DIR}/util/*.cpp
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Check C++ for malformatted code"
        USES_TERMINAL
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

if(NOT TARGET check)
    add_custom_target(check
        COMMENT "Run code quality checks"
    )
endif()

if(TARGET check-cpp-format)
    add_dependencies(check check-cpp-format)
endif()

if(TARGET check-cpp)
    add_dependencies(check check-cpp)
endif()

if(TARGET check-pep8)
    add_dependencies(check check-pep8)
endif()
