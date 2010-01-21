# - Try to find GiGi
# Once done this will define
#
#  GIGI_FOUND - system has GiGi
#  GIGI_INCLUDE_DIR
#  GIGI_LIBRARIES
#
# $GIGIDIR is an environment variable that may be set to guide CMake in
# finding GiGi.

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(GIGI GiGi)
    if (GIGI_FOUND)
        set(GIGI_INCLUDE_DIR ${GIGI_INCLUDE_DIRS})
    endif ()
else ()
    set(GIGI_INCLUDE_DIRS ${GIGIDIR}/include)
    set(GIGI_LIBRARY_DIRS ${GIGIDIR}/lib)
endif ()

find_path(
    GIGI_INCLUDE_DIR
    NAMES GG/GUI.h
    HINTS ${GIGI_INCLUDE_DIRS}
)

find_library(
    GIGI_GIGI_LIBRARY
    NAMES GiGi
    HINTS ${GIGI_LIBRARY_DIRS}
)
find_library(
    GIGI_GIGISDL_LIBRARY
    NAMES GiGiSDL
    HINTS ${GIGI_LIBRARY_DIRS}
)
find_library(
    GIGI_GIGIOGRE_LIBRARY
    NAMES GiGiOgre
    HINTS ${GIGI_LIBRARY_DIRS}
)

if (GIGI_INCLUDE_DIR AND
    GIGI_GIGI_LIBRARY)
    set(GIGI_FOUND true)
    set(GIGI_LIBRARIES ${GIGI_GIGI_LIBRARY})
    if (GIGI_GIGISDL_LIBRARY)
        set(GIGI_LIBRARIES ${GIGI_LIBRARIES} ${GIGI_GIGISDL_LIBRARY})
    endif ()
    if (GIGI_GIGIOGRE_LIBRARY)
        set(GIGI_LIBRARIES ${GIGI_LIBRARIES} ${GIGI_GIGIOGRE_LIBRARY})
    endif ()
else ()
    set(GIGI_FOUND false)
endif ()
