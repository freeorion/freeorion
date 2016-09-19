cmake_minimum_required(VERSION 2.8.12)

find_package(Doxygen)
# git allows shallow clones pushing only with version 1.9 or later
find_package(Git 1.9)

if(DOXYGEN_FOUND)
    set(FreeOrion_CPP_APIDOC_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/cpp-apidoc)

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        @ONLY
    )

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/apidoc-commit-message.in
        ${CMAKE_CURRENT_BINARY_DIR}/apidoc-commit-message
        @ONLY
    )

    if(NOT TARGET doc)
        add_custom_target(doc)
    endif()

    add_custom_target(cpp-apidoc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${FreeOrion_SOURCE_DIR}
        COMMENT "Generating FreeOrion API documentation with Doxygen" VERBATIM
    )
    add_dependencies(doc cpp-apidoc)

    if(GIT_FOUND AND (DEFINED ENV{TRAVIS}))
        set(FreeOrion_APIDOC_GIT_REMOTE https://github.com/freeorion/freeorion.github.io.git)
        set(FreeOrion_APIDOC_GIT_LOCAL  ${CMAKE_CURRENT_BINARY_DIR}/apidoc-deploy.git)
        set(FreeOrion_CPP_APIDOC_PATH   cppapi/master)

        add_custom_target(apidoc-deploy
            COMMAND
                ${CMAKE_COMMAND} -E remove_directory ${FreeOrion_APIDOC_GIT_LOCAL}
            COMMAND
                ${CMAKE_COMMAND} -E make_directory ${FreeOrion_APIDOC_GIT_LOCAL}
            COMMAND
                ${CMAKE_COMMAND} -E chdir ${FreeOrion_APIDOC_GIT_LOCAL} ${GIT_EXECUTABLE} init
            COMMAND
                ${CMAKE_COMMAND} -E chdir ${FreeOrion_APIDOC_GIT_LOCAL} ${GIT_EXECUTABLE} pull --depth 1 ${FreeOrion_APIDOC_GIT_REMOTE} master:master
            COMMAND
                ${CMAKE_COMMAND} -E remove_directory ${FreeOrion_APIDOC_GIT_LOCAL}/${FreeOrion_CPP_APIDOC_PATH}
            COMMAND
                ${CMAKE_COMMAND} -E copy_directory ${FreeOrion_CPP_APIDOC_OUTDIR}/html ${FreeOrion_APIDOC_GIT_LOCAL}/${FreeOrion_CPP_APIDOC_PATH}
            COMMAND
                ${CMAKE_COMMAND} -E chdir ${FreeOrion_APIDOC_GIT_LOCAL} ${GIT_EXECUTABLE} add -A ${FreeOrion_CPP_APIDOC_PATH}
            COMMAND
                ${CMAKE_COMMAND} -E chdir ${FreeOrion_APIDOC_GIT_LOCAL} ${GIT_EXECUTABLE} commit -F ${CMAKE_CURRENT_SOURCE_DIR}/apidoc-commit-message
            COMMAND
                ${CMAKE_COMMAND} -E chdir ${FreeOrion_APIDOC_GIT_LOCAL} ${GIT_EXECUTABLE}  push ${FreeOrion_APIDOC_GIT_REMOTE} master:master
            DEPENDS cpp-apidoc freeorionversion
            COMMENT "Deploy FreeOrion API documentation to github.io" VERBATIM
        )
    endif()
endif()
