add_subdirectory(AI)
add_subdirectory(server)

target_sources(freeorioncommon
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/SetWrapper.h
        ${CMAKE_CURRENT_LIST_DIR}/CommonWrappers.h
)

target_sources(freeoriond
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/CommonFramework.cpp
        ${CMAKE_CURRENT_LIST_DIR}/EmpireWrapper.cpp
        ${CMAKE_CURRENT_LIST_DIR}/EnumWrapper.cpp
        ${CMAKE_CURRENT_LIST_DIR}/LoggingWrapper.cpp
        ${CMAKE_CURRENT_LIST_DIR}/UniverseWrapper.cpp
)

target_sources(freeorionca
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/CommonFramework.cpp
        ${CMAKE_CURRENT_LIST_DIR}/EmpireWrapper.cpp
        ${CMAKE_CURRENT_LIST_DIR}/EnumWrapper.cpp
        ${CMAKE_CURRENT_LIST_DIR}/LoggingWrapper.cpp
        ${CMAKE_CURRENT_LIST_DIR}/UniverseWrapper.cpp
)
