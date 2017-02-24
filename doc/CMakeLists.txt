find_package(Doxygen)

if(DOXYGEN_FOUND)
    set(FreeOrion_CPP_APIDOC_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/cpp-apidoc)

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        @ONLY
    )

    if(NOT TARGET doc)
        add_custom_target(doc)
    endif()

    add_custom_target(cpp-apidoc
        ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tag_parser.py --template ${CMAKE_CURRENT_SOURCE_DIR}/all_tags.dox.in --match ${FreeOrion_SOURCE_DIR}/default -f "*.py" "*.focs.txt" "*.macros" -o ${FreeOrion_CPP_APIDOC_OUTDIR}/all_tags.dox --link_source ${FreeOrion_SOURCE_DIR} "https://github.com/freeorion/freeorion/blob/master/" "#L" &&
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${FreeOrion_SOURCE_DIR}
        COMMENT "Generating FreeOrion API documentation with Doxygen" VERBATIM
    )

    add_dependencies(doc cpp-apidoc)
endif()
