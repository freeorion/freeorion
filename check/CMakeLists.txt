if(ENABLE_CPPCHECK)
    find_package(CPPCheck REQUIRED)

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
endif()

if(NOT TARGET check)
    add_custom_target(check
        COMMENT "Run code quality checks"
    )
endif()

if(ENABLE_CPPCHECK AND TARGET check-cpp)
    add_dependencies(check check-cpp)
endif()
