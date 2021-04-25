
target_sources(freeoriongodot
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/gdfreeorion.h
        ${CMAKE_CURRENT_LIST_DIR}/OptionsDB.h
        ${CMAKE_CURRENT_LIST_DIR}/GodotClientApp.h
        ${CMAKE_CURRENT_LIST_DIR}/GodotNetworking.h
        ${CMAKE_CURRENT_LIST_DIR}/GodotSystem.h
        ${CMAKE_CURRENT_LIST_DIR}/GodotFleet.h
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/gdlibrary.cpp
        ${CMAKE_CURRENT_LIST_DIR}/gdfreeorion.cpp
        ${CMAKE_CURRENT_LIST_DIR}/OptionsDB.cpp
        ${CMAKE_CURRENT_LIST_DIR}/GodotClientApp.cpp
        ${CMAKE_CURRENT_LIST_DIR}/GodotNetworking.cpp
        ${CMAKE_CURRENT_LIST_DIR}/GodotSystem.cpp
        ${CMAKE_CURRENT_LIST_DIR}/GodotFleet.cpp
)

