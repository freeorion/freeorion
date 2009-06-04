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
    pkg_check_modules(GIGI GiGi GiGiSDL GiGiOgre)
    if (GIGI_FOUND)
        set(GIGI_INCLUDE_DIR ${GIGI_INCLUDE_DIRS})
    endif ()
else ()
    set(GIGI_INCLUDE_DIRS ${GIGIDIR}/include)
    set(GIGI_LIBRARY_DIRS ${GIGIDIR}/lib)
endif ()

find_path(
    GIGI_INCLUDE_DIR
    NAMES gvc.h
    PATHS ${GIGI_INCLUDE_DIRS}
)

find_library(
    GIGI_GIGI_LIBRARY
    NAMES GiGi 
    PATHS ${GIGI_LIBRARY_DIRS}
)
find_library(
    GIGI_GIGISDL_LIBRARY
    NAMES GiGiSDL 
    PATHS ${GIGI_LIBRARY_DIRS}
)
find_library(
    GIGI_GIGIOGRE_LIBRARY
    NAMES GiGiOgre 
    PATHS ${GIGI_LIBRARY_DIRS}
)

if (GIGI_INCLUDE_DIR AND
    GIGI_GIGI_LIBRARY AND
    GIGI_GIGISDL_LIBRARY AND
    GIGI_GIGIOGRE_LIBRARY)
    set(GIGI_FOUND true)
    set(GIGI_LIBRARIES
        ${GIGI_GIGI_LIBRARY}
        ${GIGI_GIGISDL_LIBRARY}
        ${GIGI_GIGIOGRE_LIBRARY})
else ()
    set(GIGI_FOUND false)
endif ()
