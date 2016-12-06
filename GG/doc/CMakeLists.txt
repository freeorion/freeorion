find_package(Doxygen REQUIRED)

configure_file(
    Doxyfile.in
    Doxyfile
    @ONLY
)

if(NOT TARGET doc)
    add_custom_target(doc)
endif()

add_custom_command (
    OUTPUT GG/index.html
    COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
    MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
            ${CMAKE_CURRENT_SOURCE_DIR}/GGDoc.txt
    COMMENT "Generating GiGi API documentation with Doxygen" VERBATIM
)

add_custom_target (GiGi-apidoc DEPENDS GG/index.html)
add_dependencies(doc GiGi-apidoc)

install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/GG
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    COMPONENT COMPONENT_GIGI_DEVEL
)
