cmake_minimum_required(VERSION 2.6)

message("-- Configuring GiGi")


set(GiGi_LINK_LIBS ${Boost_LIBRARIES} ${OPENGL_LIBRARIES} ${FREETYPE_LIBRARIES})
if (USE_DEVIL)
    list(APPEND GiGi_LINK_LIBS ${IL_LIBRARY} ${ILUT_LIBRARY})
else ()
    if (JPEG_FOUND)
        list(APPEND GiGi_LINK_LIBS ${JPEG_LIBRARIES})
    endif ()
    if (PNG_FOUND)
        list(APPEND GiGi_LINK_LIBS ${PNG_LIBRARIES})
    endif ()
    if (TIFF_FOUND)
        list(APPEND GiGi_LINK_LIBS ${TIFF_LIBRARIES})
    endif ()
endif ()

if (WIN32)
    if (MSVC)
        add_definitions(/wd4146 /wd4099 /wd4251 /wd4800 /wd4267 /wd4275
                        /wd4244 /wd4101 /wd4258 /wd4351 /wd4996)
    endif ()
endif ()

add_library(GiGi
    AlignmentFlags.cpp
    Base.cpp
    BrowseInfoWnd.cpp
    Button.cpp
    ClrConstants.cpp
    Control.cpp
    Cursor.cpp
    DrawUtil.cpp
    DropDownList.cpp
    DynamicGraphic.cpp
    Edit.cpp
    EventPump.cpp
    Font.cpp
    GroupBox.cpp
    GUI.cpp
    Layout.cpp
    ListBox.cpp
    Menu.cpp
    MultiEdit.cpp
    PtRect.cpp
    Scroll.cpp
    StaticGraphic.cpp
    StyleFactory.cpp
    TabWnd.cpp
    TextControl.cpp
    Texture.cpp
    Timer.cpp
    UnicodeCharsets.cpp
    Wnd.cpp
    WndEvent.cpp
    ZList.cpp

    dialogs/ColorDlg.cpp
    dialogs/FileDlg.cpp
    dialogs/ThreeButtonDlg.cpp
)

if (USE_STATIC_LIBS)
    set_target_properties(GiGi
        PROPERTIES
        LINK_SEARCH_END_STATIC true
    )
endif ()

target_link_libraries(GiGi ${GiGi_LINK_LIBS})

install(
    TARGETS GiGi
    LIBRARY DESTINATION lib${LIB_SUFFIX} COMPONENT COMPONENT_GIGI
)

if (BUILD_DEVEL_PACKAGE)
    install(
        TARGETS GiGi
        ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT COMPONENT_GIGI_DEVEL
    )
endif ()

if (UNIX AND BUILD_DEVEL_PACKAGE)
    get_pkg_config_libs(pkg_config_libs ${GiGi_LINK_LIBS})

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/GiGi.pc.in
        ${CMAKE_BINARY_DIR}/GiGi.pc
        @ONLY
    )

    install(
        FILES ${CMAKE_BINARY_DIR}/GiGi.pc
        DESTINATION lib${LIB_SUFFIX}/pkgconfig
        COMPONENT COMPONENT_GIGI_DEVEL
        OPTIONAL
    )
endif ()

if (BUILD_SDL_DRIVER)
    add_subdirectory(SDL)
endif ()

if (BUILD_OGRE_DRIVER)
    add_subdirectory(Ogre)
endif ()
