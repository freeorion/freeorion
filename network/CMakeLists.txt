target_sources(freeorioncommon
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/Message.h
        ${CMAKE_CURRENT_LIST_DIR}/MessageQueue.h
        ${CMAKE_CURRENT_LIST_DIR}/Networking.h
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/Message.cpp
        ${CMAKE_CURRENT_LIST_DIR}/MessageQueue.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Networking.cpp
)

target_sources(freeoriond
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/ServerNetworking.h
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/ServerNetworking.cpp
)

target_sources(freeorionca
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/ClientNetworking.h
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/ClientNetworking.cpp
)

if(NOT BUILD_HEADLESS)
    target_sources(freeorion
        PUBLIC
            ${CMAKE_CURRENT_LIST_DIR}/ClientNetworking.h
        PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/ClientNetworking.cpp
    )
endif()
