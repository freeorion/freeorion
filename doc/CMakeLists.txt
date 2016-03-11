cmake_minimum_required(VERSION 2.8.5)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        @ONLY
    )

    if(NOT TARGET doc)
        add_custom_target(doc)
    endif()

    add_custom_target(FreeOrion-apidoc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${FreeOrion_SOURCE_DIR}
        COMMENT "Generating FreeOrion API documentation with Doxygen" VERBATIM
    )
    add_dependencies(doc FreeOrion-apidoc)
endif()

