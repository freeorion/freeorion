add_subdirectory(ui)

target_sources(freeorion
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/HumanClientApp.h
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/chmain.h
        ${CMAKE_CURRENT_LIST_DIR}/HumanClientFSM.h
        ${CMAKE_CURRENT_LIST_DIR}/chmain.cpp
        ${CMAKE_CURRENT_LIST_DIR}/HumanClientApp.cpp
        ${CMAKE_CURRENT_LIST_DIR}/HumanClientFSM.cpp
        # Add icon resource file to freeorion.exe
        $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_LIST_DIR}/FreeOrion.rc>
        $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_LIST_DIR}/FreeOrion.ico>

        $<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_LIST_DIR}/chmain.mm>
        $<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_LIST_DIR}/GUIController.mm>
        # Add icon and menu resource files to FreeOrion.app
        $<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_LIST_DIR}/main.xib>
        $<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_LIST_DIR}/FreeOrion.icns>
)

set_property(
    SOURCE
        ${CMAKE_CURRENT_LIST_DIR}/main.xib
        ${CMAKE_CURRENT_LIST_DIR}/FreeOrion.icns
    PROPERTY
        MACOSX_PACKAGE_LOCATION "Resources/"
)
