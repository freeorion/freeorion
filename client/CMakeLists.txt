add_subdirectory(AI)

target_sources(freeorionca
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/ClientApp.h
        ${CMAKE_CURRENT_LIST_DIR}/ClientFSMEvents.h
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/ClientApp.cpp
        ${CMAKE_CURRENT_LIST_DIR}/ClientFSMEvents.cpp
)
