add_subdirectory(AI)
if(NOT BUILD_HEADLESS)
    add_subdirectory(human)
endif()

target_sources(freeorionca
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/ClientApp.h
        ${CMAKE_CURRENT_LIST_DIR}/ClientFSMEvents.h
        ${CMAKE_CURRENT_LIST_DIR}/ClientNetworking.h
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/ClientApp.cpp
        ${CMAKE_CURRENT_LIST_DIR}/ClientFSMEvents.cpp
        ${CMAKE_CURRENT_LIST_DIR}/ClientNetworking.cpp
)

if(NOT BUILD_HEADLESS)
    target_sources(freeorion
        PUBLIC
            ${CMAKE_CURRENT_LIST_DIR}/ClientApp.h
            ${CMAKE_CURRENT_LIST_DIR}/ClientFSMEvents.h
            ${CMAKE_CURRENT_LIST_DIR}/ClientNetworking.h
        PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/ClientApp.cpp
            ${CMAKE_CURRENT_LIST_DIR}/ClientFSMEvents.cpp
            ${CMAKE_CURRENT_LIST_DIR}/ClientNetworking.cpp
    )
endif()
