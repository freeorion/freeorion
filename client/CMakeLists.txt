if(BUILD_AI)
    add_subdirectory(AI)
endif()
if(BUILD_CLIENT_GG)
    add_subdirectory(human)
endif()
if(BUILD_CLIENT_GODOT EQUAL "3")
    add_subdirectory(godot)
endif()

if(BUILD_AI)
    target_sources(freeorionca
        PUBLIC
            ${CMAKE_CURRENT_LIST_DIR}/ClientApp.h
            ${CMAKE_CURRENT_LIST_DIR}/ClientNetworking.h
        PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/ClientApp.cpp
            ${CMAKE_CURRENT_LIST_DIR}/ClientNetworking.cpp
    )
endif()

if(BUILD_CLIENT_GG)
    target_sources(freeorion
        PUBLIC
            ${CMAKE_CURRENT_LIST_DIR}/ClientApp.h
            ${CMAKE_CURRENT_LIST_DIR}/ClientNetworking.h
        PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/ClientApp.cpp
            ${CMAKE_CURRENT_LIST_DIR}/ClientNetworking.cpp
    )
endif()

if(BUILD_CLIENT_GODOT EQUAL "3")
    target_sources(freeoriongodot
        PUBLIC
            ${CMAKE_CURRENT_LIST_DIR}/ClientApp.h
            ${CMAKE_CURRENT_LIST_DIR}/ClientNetworking.h
        PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/ClientApp.cpp
            ${CMAKE_CURRENT_LIST_DIR}/ClientNetworking.cpp
    )
endif()

