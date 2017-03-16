target_sources(freeoriond
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/ServerWrapper.h
        ${CMAKE_CURRENT_LIST_DIR}/ServerFramework.h
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/ServerWrapper.cpp
        ${CMAKE_CURRENT_LIST_DIR}/ServerFramework.cpp
)
