if (NOT TARGET check)
    add_custom_target(check
        COMMENT "Run code quality checks"
    )
endif ()
