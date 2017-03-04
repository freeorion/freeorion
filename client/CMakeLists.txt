add_subdirectory(AI)
add_subdirectory(human)

target_sources(freeorionca
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/ClientApp.h
        ${CMAKE_CURRENT_LIST_DIR}/ClientFSMEvents.h
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/ClientApp.cpp
        ${CMAKE_CURRENT_LIST_DIR}/ClientFSMEvents.cpp
)

target_sources(freeorion
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/ClientApp.h
        ${CMAKE_CURRENT_LIST_DIR}/ClientFSMEvents.h
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/ClientApp.cpp
        ${CMAKE_CURRENT_LIST_DIR}/ClientFSMEvents.cpp
)
