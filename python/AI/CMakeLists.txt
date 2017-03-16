target_sources(freeorionca
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/AIFramework.h
        ${CMAKE_CURRENT_LIST_DIR}/AIWrapper.h
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/AIFramework.cpp
        ${CMAKE_CURRENT_LIST_DIR}/AIWrapper.cpp
)
