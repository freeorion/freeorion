#ifndef _GalaxySetupData_h_
#define _GalaxySetupData_h_


#include <string>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <GG/Enum.h>
#include "Export.h"


//! Types of universe shapes during galaxy generation
GG_ENUM(Shape,
    INVALID_SHAPE = -1,
    SPIRAL_2,       ///< a two-armed spiral galaxy
    SPIRAL_3,       ///< a three-armed spiral galaxy
    SPIRAL_4,       ///< a four-armed spiral galaxy
    CLUSTER,        ///< a cluster galaxy
    ELLIPTICAL,     ///< an elliptical galaxy
    DISC,           ///< a disc shaped galaxy
    BOX,            ///< a rectangular shaped galaxy
    IRREGULAR,      ///< an irregular galaxy
    RING,           ///< a ring galaxy
    RANDOM,         ///< a random one of the other shapes
    GALAXY_SHAPES   ///< the number of shapes in this enum (leave this last)
)


//! General-use option for galaxy setup picks with "more" or "less" options.
GG_ENUM(GalaxySetupOption,
    INVALID_GALAXY_SETUP_OPTION = -1,
    GALAXY_SETUP_NONE,
    GALAXY_SETUP_LOW,
    GALAXY_SETUP_MEDIUM,
    GALAXY_SETUP_HIGH,
    GALAXY_SETUP_RANDOM,
    NUM_GALAXY_SETUP_OPTIONS
)


//! Levels of AI Aggression during galaxy generation
GG_ENUM(Aggression,
    INVALID_AGGRESSION = -1,
    BEGINNER,
    TURTLE,         ///< Very Defensive
    CAUTIOUS,       ///< Somewhat Defensive
    TYPICAL,        ///< Typical
    AGGRESSIVE,     ///< Aggressive
    MANIACAL,       ///< Very Aggressive
    NUM_AI_AGGRESSION_LEVELS
)


//! Returns a user readable string for a GalaxySetupOption
FO_COMMON_API const std::string& TextForGalaxySetupSetting(GalaxySetupOption gso);

//! Returns a user readable string for a Shape
FO_COMMON_API const std::string& TextForGalaxyShape(Shape shape);

//! Returns a user readable string for an Aggression
FO_COMMON_API const std::string& TextForAIAggression(Aggression a);


//! The data that represent the galaxy setup for a new game.
struct FO_COMMON_API GalaxySetupData {
    GalaxySetupData();

    GalaxySetupData(GalaxySetupData const&) = default;

    GalaxySetupData(GalaxySetupData&& base);

    GalaxySetupData& operator=(GalaxySetupData const&) = default;

    auto GetSeed() const -> std::string const&;
    auto GetSize() const -> int;
    auto GetShape() const -> Shape;
    auto GetAge() const -> GalaxySetupOption;
    auto GetStarlaneFreq() const -> GalaxySetupOption;
    auto GetPlanetDensity() const -> GalaxySetupOption;
    auto GetSpecialsFreq() const -> GalaxySetupOption;
    auto GetMonsterFreq() const -> GalaxySetupOption;
    auto GetNativeFreq() const -> GalaxySetupOption;
    auto GetAggression() const -> Aggression;
    auto GetGameRules() const -> std::map<std::string, std::string> const&;
    auto GetGameUID() const -> std::string const&;

    void SetSeed(std::string const& seed);
    void SetGameUID(std::string const& game_uid);

    std::string         seed;
    int                 size;
    Shape               shape;
    GalaxySetupOption   age;
    GalaxySetupOption   starlane_freq;
    GalaxySetupOption   planet_density;
    GalaxySetupOption   specials_freq;
    GalaxySetupOption   monster_freq;
    GalaxySetupOption   native_freq;
    Aggression          ai_aggr;
    std::map<std::string, std::string>
                        game_rules;
    std::string         game_uid;

    //! Used during serialization to globally set what empire knowledge to use
    //!
    //! HACK! This must be set to the encoding empire's id when serializing a
    //! GalaxySetupData, so that only the relevant parts of the galaxy data are
    //! serialized.  The use of this local field is done just so I don't
    //! have to rewrite any custom boost::serialization classes that implement
    //! empire-dependent visibility.
    int                 encoding_empire;
};


#endif //  _GalaxySetupData_h_
