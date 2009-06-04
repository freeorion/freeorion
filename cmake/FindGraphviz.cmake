# - Try to find Graphviz
# Once done this will define
#
#  GRAPHVIZ_FOUND - system has Graphviz
#  GRAPHVIZ_INCLUDE_DIR
#  GRAPHVIZ_LIBRARIES
#
# $GRAPHVIZDIR is an environment variable that may be set to guide CMake in
# finding Graphviz.

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(GRAPHVIZ libagraph libcdt libcgraph libgraph libgvc libpathplan)
    if (GRAPHVIZ_FOUND)
        set(GRAPHVIZ_INCLUDE_DIR ${GRAPHVIZ_INCLUDE_DIRS})
    endif ()
else ()
    set(GRAPHVIZ_INCLUDE_DIRS ${GRAPHVIZDIR}/include ${GRAPHVIZDIR}/graphviz)
    set(GRAPHVIZ_LIBRARY_DIRS ${GRAPHVIZDIR}/lib)
endif ()

find_path(
    GRAPHVIZ_INCLUDE_DIR
    NAMES gvc.h
    PATHS ${GRAPHVIZ_INCLUDE_DIRS}
)

find_library(
    GRAPHVIZ_AGRAPH_LIBRARY
    NAMES agraph 
    PATHS ${GRAPHVIZ_LIBRARY_DIRS}
)
find_library(
    GRAPHVIZ_CDT_LIBRARY
    NAMES cdt 
    PATHS ${GRAPHVIZ_LIBRARY_DIRS}
)
find_library(
    GRAPHVIZ_CGRAPH_LIBRARY
    NAMES cgraph 
    PATHS ${GRAPHVIZ_LIBRARY_DIRS}
)
find_library(
    GRAPHVIZ_GRAPH_LIBRARY
    NAMES graph 
    PATHS ${GRAPHVIZ_LIBRARY_DIRS}
)
find_library(
    GRAPHVIZ_GVC_LIBRARY
    NAMES gvc 
    PATHS ${GRAPHVIZ_LIBRARY_DIRS}
)
find_library(
    GRAPHVIZ_PATHPLAN_LIBRARY
    NAMES pathplan 
    PATHS ${GRAPHVIZ_LIBRARY_DIRS}
)

if (GRAPHVIZ_INCLUDE_DIR AND
    GRAPHVIZ_AGRAPH_LIBRARY AND
    GRAPHVIZ_CDT_LIBRARY AND
    GRAPHVIZ_CGRAPH_LIBRARY AND
    GRAPHVIZ_GRAPH_LIBRARY AND
    GRAPHVIZ_GVC_LIBRARY AND
    GRAPHVIZ_PATHPLAN_LIBRARY)
    set(GRAPHVIZ_FOUND true)
    set(GRAPHVIZ_LIBRARIES
        ${GRAPHVIZ_AGRAPH_LIBRARY}
        ${GRAPHVIZ_CDT_LIBRARY}
        ${GRAPHVIZ_CGRAPH_LIBRARY}
        ${GRAPHVIZ_GRAPH_LIBRARY}
        ${GRAPHVIZ_GVC_LIBRARY}
        ${GRAPHVIZ_PATHPLAN_LIBRARY})
else ()
    set(GRAPHVIZ_FOUND false)
endif ()
