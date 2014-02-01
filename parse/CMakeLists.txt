cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

add_definitions(-DNDEBUG -DFREEORION_BUILD_PARSE)

if (NOT BUILD_TESTS)
    if (CMAKE_COMPILER_IS_GNUCXX)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -O3")
    endif ()
endif ()

set (freeorionparse_HEADER
    ConditionParserImpl.h
    Double.h
    EffectParser.h
    EffectParserImpl.h
    EnumParser.h
    Int.h
    Label.h
    Lexer.h
    Parse.h
    ParseImpl.h
    ReportParseError.h
    ShipPartStatsParser.h
    Tokens.h
    ValueRefParser.h
    ValueRefParserImpl.h
)

set (freeorionparse_SOURCE
    AlignmentsParser.cpp
    BuildingsParser.cpp
    ConditionParser1.cpp
    ConditionParser2.cpp
    ConditionParser3.cpp
    ConditionParser4.cpp
    ConditionParser5.cpp
    ConditionParser6.cpp
    ConditionParser.cpp
    Double.cpp
    DoubleValueRefParser.cpp
    EffectParser1.cpp
    EffectParser2.cpp
    EffectParser3.cpp
    EffectParser4.cpp
    EffectParser.cpp
    EmpireStatsParser.cpp
    EncyclopediaParser.cpp
    EnumParser.cpp
    FieldsParser.cpp
    FleetPlansParser.cpp
    Int.cpp
    IntValueRefParser.cpp
    ItemsParser.cpp
    KeymapParser.cpp
    Label.cpp
    Lexer.cpp
    MonsterFleetPlansParser.cpp
    Parse.cpp
    PlanetEnvironmentValueRefParser.cpp
    PlanetSizeValueRefParser.cpp
    PlanetTypeValueRefParser.cpp
    ReportParseError.cpp
    ShipDesignsParser.cpp
    ShipHullsParser.cpp
    ShipPartsParser.cpp
    ShipPartStatsParser.cpp
    SpecialsParser.cpp
    SpeciesParser.cpp
    StarTypeValueRefParser.cpp
    StringValueRefParser.cpp
    TechsParser.cpp
    Tokens.cpp
    UniverseObjectTypeValueRefParser.cpp
    ValueRefParserImpl.cpp
    ../GG/src/adobe/name.cpp
    ../GG/src/adobe/string_pool.cpp
)

add_library(freeorionparse
    ${freeorionparse_HEADER}
    ${freeorionparse_SOURCE}
)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set_property(
        SOURCE
        ConditionParser1.cpp
        EffectParser1.cpp
        PROPERTY
        COMPILE_FLAGS -ftemplate-depth=512
    )
endif ()

install(
    TARGETS freeorionparse
    LIBRARY DESTINATION lib${LIB_SUFFIX}/freeorion
    COMPONENT COMPONENT_FREEORION
)

if (BUILD_TESTS)
    add_subdirectory(test)
endif ()
