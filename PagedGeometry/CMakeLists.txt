cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

find_package(OGRE REQUIRED)

include_directories(include ${OGRE_INCLUDE_DIR})

add_library(pagedgeometry STATIC
    source/BatchedGeometry.cpp
    source/BatchPage.cpp
    source/GrassLoader.cpp
    source/ImpostorPage.cpp
    source/PagedGeometry.cpp
    source/PropertyMaps.cpp
    source/StaticBillboardSet.cpp
    source/TreeLoader3D.cpp
    source/WindBatchedGeometry.cpp
    source/WindBatchPage.cpp
)

