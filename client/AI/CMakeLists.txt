target_sources(freeorionca
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/AIClientApp.h
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/AIClientApp.cpp
        ${CMAKE_CURRENT_LIST_DIR}/camain.cpp
)
