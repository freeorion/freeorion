add_definitions(-DFREEORION_BUILD_SERVER)

set(freeoriond_LINK_LIBS
    freeorioncommon
    freeorionparse
    ${PYTHON_LIBRARIES}
    ${Boost_PYTHON_LIBRARY}
)

set (freeoriond_HEADER
    ../network/ServerNetworking.h
    SaveLoad.h
    ServerApp.h
    ServerFSM.h
)

set (freeoriond_SOURCE
    dmain.cpp
    SaveLoad.cpp
    ServerApp.cpp
    ServerFSM.cpp
    ../combat/CombatSystem.cpp
    ../network/ServerNetworking.cpp
    ../python/CommonFramework.cpp
    ../python/EmpireWrapper.cpp
    ../python/EnumWrapper.cpp
    ../python/LoggingWrapper.cpp
    ../python/UniverseWrapper.cpp
    ../python/server/ServerWrapper.cpp
    ../python/server/ServerFramework.cpp
    ../universe/UniverseGenerator.cpp
    ../util/DependencyVersions.cpp
)

add_executable(freeoriond
    ${freeoriond_HEADER}
    ${freeoriond_SOURCE}
)

target_compile_options(
    freeoriond PRIVATE
    $<$<CXX_COMPILER_ID:gnu>:-fvisibility=hidden>
)

target_include_directories(freeoriond SYSTEM
    PRIVATE
        ${PROJECT_SOURCE_DIR}/GG
        ${PYTHON_INCLUDE_PATH}
)


target_link_libraries(freeoriond
    ${freeoriond_LINK_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
)

install(
    TARGETS freeoriond
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT COMPONENT_FREEORION
)

