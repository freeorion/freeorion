# - Try to find GiGi
# Once done this will define
#
#  GiGi_FOUND - system has GiGi
#  GiGi_INCLUDE_DIR
#  GiGi_LIBRARIES
#
# $GIGIDIR is an environment variable that may be set to guide CMake in
# finding GiGi.

find_package(PkgConfig)
if (GIGIDIR)
    set(GiGi_INCLUDE_DIRS ${GIGIDIR})
    set(GiGi_LIBRARY_DIRS ${GIGIDIR})
else ()
    if (PKG_CONFIG_FOUND)
	    pkg_check_modules(GiGi GiGi)
    	if (GiGi_FOUND)
	        set(GiGi_INCLUDE_DIR ${GiGi_INCLUDE_DIRS})
    	endif ()
	endif ()
endif ()

find_path(
    GiGi_INCLUDE_DIR
    NAMES GG/GUI.h
    HINTS ${GiGi_INCLUDE_DIRS}
)

find_library(
    GiGi_GIGI_LIBRARY
    NAMES GiGi
    HINTS ${GiGi_LIBRARY_DIRS}
)
find_library(
    GiGi_GIGISDL_LIBRARY
    NAMES GiGiSDL
    HINTS ${GiGi_LIBRARY_DIRS}
)
find_library(
    GiGi_GIGIOGRE_LIBRARY
    NAMES GiGiOgre
    HINTS ${GiGi_LIBRARY_DIRS}
)
find_library(
    GiGi_GIGIOGREOIS_LIBRARY
    NAMES GiGiOgrePlugin_OIS
    HINTS ${GiGi_LIBRARY_DIRS}
)
if (GiGi_INCLUDE_DIR AND
    GiGi_GIGI_LIBRARY)
    set(GiGi_FOUND true)
    set(GiGi_LIBRARIES ${GiGi_GIGI_LIBRARY})
    if (GiGi_GIGISDL_LIBRARY)
        set(GiGi_LIBRARIES ${GiGi_LIBRARIES} ${GiGi_GIGISDL_LIBRARY})
    endif ()
    if (GiGi_GIGIOGRE_LIBRARY)
        set(GiGi_LIBRARIES ${GiGi_LIBRARIES} ${GiGi_GIGIOGRE_LIBRARY})
    endif ()
    if (GiGi_GIGIOGREOIS_LIBRARY)
        set(GiGi_LIBRARIES ${GiGi_LIBRARIES} ${GiGi_GIGIOGREOIS_LIBRARY})
    endif ()
else ()
    set(GiGi_FOUND false)
endif ()
