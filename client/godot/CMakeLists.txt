target_sources(
  freeoriongodot
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/FreeOrionNode.h
          ${CMAKE_CURRENT_LIST_DIR}/GodotClientApp.h
          ${CMAKE_CURRENT_LIST_DIR}/GodotFleet.h
          ${CMAKE_CURRENT_LIST_DIR}/GodotSystem.h
          ${CMAKE_CURRENT_LIST_DIR}/cgmain.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FreeOrionNode.cpp
          ${CMAKE_CURRENT_LIST_DIR}/GodotClientApp.cpp
          ${CMAKE_CURRENT_LIST_DIR}/GodotFleet.cpp
          ${CMAKE_CURRENT_LIST_DIR}/GodotSystem.cpp
)
