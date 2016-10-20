include_directories(
    ${PYTHON_INCLUDE_PATH}
)

add_definitions(-DFREEORION_BUILD_AI)

set(freeorionca_LINK_LIBS
    freeorioncommon
    freeorionparse
    ${PYTHON_LIBRARIES}
    ${Boost_PYTHON_LIBRARY}
)

set (freeorionca_HEADER
    AIClientApp.h
    ../ClientApp.h
    ../ClientFSMEvents.h
)

set (freeorionca_SOURCE
    ../../AI/AIInterface.cpp
    AIClientApp.cpp
    camain.cpp
    ../ClientApp.cpp
    ../ClientFSMEvents.cpp
    ../../combat/CombatSystem.cpp
    ../../network/ClientNetworking.cpp
    ../../python/CommonFramework.cpp
    ../../python/AI/AIFramework.cpp
    ../../python/EmpireWrapper.cpp
    ../../python/EnumWrapper.cpp
    ../../python/LoggingWrapper.cpp
    ../../python/UniverseWrapper.cpp
    ../../python/AI/AIWrapper.cpp
    ../../util/DependencyVersions.cpp
)

add_executable(freeorionca
    ${freeorionca_HEADER}
    ${freeorionca_SOURCE}
)

target_compile_options(
    freeorionca PRIVATE
    $<$<CXX_COMPILER_ID:gnu>:-fvisibility=hidden>
)

target_link_libraries(freeorionca
    ${freeorionca_LINK_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
)

install(
    TARGETS freeorionca
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT COMPONENT_FREEORION
)
