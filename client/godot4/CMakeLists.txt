target_sources(freeoriongodot
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/FreeOrionNode.h
        ${CMAKE_CURRENT_LIST_DIR}/GodotClientApp.h
        ${CMAKE_CURRENT_LIST_DIR}/GodotI18n.h

        ${CMAKE_CURRENT_LIST_DIR}/cgmain.cpp
        ${CMAKE_CURRENT_LIST_DIR}/FreeOrionNode.cpp
        ${CMAKE_CURRENT_LIST_DIR}/GodotClientApp.cpp
        ${CMAKE_CURRENT_LIST_DIR}/GodotI18n.cpp
)
