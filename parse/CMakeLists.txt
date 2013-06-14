cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

add_definitions(-DNDEBUG)

if (CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
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

if (BUILD_PARSER_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()
