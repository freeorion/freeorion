cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

add_definitions(-DNDEBUG -DFREEORION_BUILD_PARSE)

if (NOT BUILD_TESTS)
    if (CMAKE_COMPILER_IS_GNUCXX)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -O3")
    endif ()
endif ()

add_library(freeorionparse
    Lexer.cpp
    ReportParseError.cpp
    Int.cpp
    Double.cpp
    Label.cpp
    EnumParser.cpp
    IntValueRefParser.cpp
    DoubleValueRefParser.cpp
    StringValueRefParser.cpp
    PlanetSizeValueRefParser.cpp
    PlanetTypeValueRefParser.cpp
    PlanetEnvironmentValueRefParser.cpp
    UniverseObjectTypeValueRefParser.cpp
    StarTypeValueRefParser.cpp
    ConditionParser.cpp
    ConditionParser1.cpp
    ConditionParser2.cpp
    ConditionParser3.cpp
    ConditionParser4.cpp
    EffectParser.cpp
    EffectParser1.cpp
    EffectParser2.cpp
    EffectParser3.cpp
    EffectParser4.cpp
    FieldsParser.cpp
    Parse.cpp
    BuildingsParser.cpp
    SpecialsParser.cpp
    SpeciesParser.cpp
    TechsParser.cpp
    ItemsParser.cpp
    ShipPartsParser.cpp
    ShipPartStatsParser.cpp
    ShipHullsParser.cpp
    ShipDesignsParser.cpp
    FleetPlansParser.cpp
    MonsterFleetPlansParser.cpp
    AlignmentsParser.cpp
    EncyclopediaParser.cpp
    KeymapParser.cpp
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

if (BUILD_TESTS)
    add_subdirectory(test)
endif ()
