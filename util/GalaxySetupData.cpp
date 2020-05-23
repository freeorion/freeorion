#include "GalaxySetupData.h"

#include "AppInterface.h"
#include "i18n.h"
#include "OptionsDB.h"
#include "Random.h"


namespace {
    // command-line options
    void AddOptions(OptionsDB& db) {
        // Common galaxy settings
        db.Add("setup.seed",                UserStringNop("OPTIONS_DB_GAMESETUP_SEED"),                         std::string("0"),       Validator<std::string>());
        db.Add("setup.star.count",          UserStringNop("OPTIONS_DB_GAMESETUP_STARS"),                        150,                    RangedValidator<int>(10, 5000));
        db.Add("setup.galaxy.shape",        UserStringNop("OPTIONS_DB_GAMESETUP_GALAXY_SHAPE"),                 DISC,                   RangedValidator<Shape>(SPIRAL_2, RANDOM));
        db.Add("setup.galaxy.age",          UserStringNop("OPTIONS_DB_GAMESETUP_GALAXY_AGE"),                   GALAXY_SETUP_MEDIUM,    RangedValidator<GalaxySetupOption>(GALAXY_SETUP_LOW, GALAXY_SETUP_RANDOM));
        db.Add("setup.planet.density",      UserStringNop("OPTIONS_DB_GAMESETUP_PLANET_DENSITY"),               GALAXY_SETUP_MEDIUM,    RangedValidator<GalaxySetupOption>(GALAXY_SETUP_LOW, GALAXY_SETUP_RANDOM));
        db.Add("setup.starlane.frequency",  UserStringNop("OPTIONS_DB_GAMESETUP_STARLANE_FREQUENCY"),           GALAXY_SETUP_MEDIUM,    RangedValidator<GalaxySetupOption>(GALAXY_SETUP_LOW, GALAXY_SETUP_RANDOM));
        db.Add("setup.specials.frequency",  UserStringNop("OPTIONS_DB_GAMESETUP_SPECIALS_FREQUENCY"),           GALAXY_SETUP_MEDIUM,    RangedValidator<GalaxySetupOption>(GALAXY_SETUP_NONE, GALAXY_SETUP_RANDOM));
        db.Add("setup.monster.frequency",   UserStringNop("OPTIONS_DB_GAMESETUP_MONSTER_FREQUENCY"),            GALAXY_SETUP_MEDIUM,    RangedValidator<GalaxySetupOption>(GALAXY_SETUP_NONE, GALAXY_SETUP_RANDOM));
        db.Add("setup.native.frequency",    UserStringNop("OPTIONS_DB_GAMESETUP_NATIVE_FREQUENCY"),             GALAXY_SETUP_MEDIUM,    RangedValidator<GalaxySetupOption>(GALAXY_SETUP_NONE, GALAXY_SETUP_RANDOM));
        db.Add("setup.ai.player.count",     UserStringNop("OPTIONS_DB_GAMESETUP_NUM_AI_PLAYERS"),               6,                      RangedValidator<int>(0, IApp::MAX_AI_PLAYERS()));
        db.Add("setup.ai.aggression",       UserStringNop("OPTIONS_DB_GAMESETUP_AI_MAX_AGGRESSION"),            MANIACAL,               RangedValidator<Aggression>(BEGINNER, MANIACAL));
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    // returns number in range 0 to one less than the interger representation of
    // enum_vals_count, determined by the random seed
    template <typename T1>
    int GetIdx(const T1& enum_vals_count, const std::string& seed) {
        DebugLogger() << "hashing seed: " << seed;
        // use probably-bad but adequate for this purpose hash function to
        // convert seed into a hash value
        int hash_value = 223;
        for (size_t i = 0; i < seed.length(); ++i) {
            //DebugLogger() << "hash value: " << hash_value << " char: " << static_cast<int>(seed[i]);
            hash_value += (seed[i] * 61);
            hash_value %= 191;
        }
        DebugLogger() << "final hash value: " << hash_value
                      << " and returning: " << hash_value % static_cast<int>(enum_vals_count)
                      << " from 0 to " << static_cast<int>(enum_vals_count) - 1;
        return hash_value % static_cast<int>(enum_vals_count);
    }

    static char alphanum[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
    const std::string EMPTY_STRING;
}


const std::string& TextForGalaxySetupSetting(GalaxySetupOption gso) {
    switch (gso) {
        case GALAXY_SETUP_NONE:     return UserString("GSETUP_NONE");
        case GALAXY_SETUP_LOW:      return UserString("GSETUP_LOW");
        case GALAXY_SETUP_MEDIUM:   return UserString("GSETUP_MEDIUM");
        case GALAXY_SETUP_HIGH:     return UserString("GSETUP_HIGH");
        case GALAXY_SETUP_RANDOM:   return UserString("GSETUP_RANDOM");
        default:                    return EMPTY_STRING;
    }
}

const std::string& TextForGalaxyShape(Shape shape) {
    switch (shape) {
        case SPIRAL_2:      return UserString("GSETUP_2ARM");
        case SPIRAL_3:      return UserString("GSETUP_3ARM");
        case SPIRAL_4:      return UserString("GSETUP_4ARM");
        case CLUSTER:       return UserString("GSETUP_CLUSTER");
        case ELLIPTICAL:    return UserString("GSETUP_ELLIPTICAL");
        case DISC:          return UserString("GSETUP_DISC");
        case BOX:           return UserString("GSETUP_BOX");
        case IRREGULAR:     return UserString("GSETUP_IRREGULAR");
        case RING:          return UserString("GSETUP_RING");
        case RANDOM:        return UserString("GSETUP_RANDOM");
        default:            return EMPTY_STRING;
    }
}

const std::string& TextForAIAggression(Aggression a) {
    switch (a) {
        case BEGINNER:      return UserString("GSETUP_BEGINNER");
        case TURTLE:        return UserString("GSETUP_TURTLE");
        case CAUTIOUS:      return UserString("GSETUP_CAUTIOUS");
        case TYPICAL:       return UserString("GSETUP_TYPICAL");
        case AGGRESSIVE:    return UserString("GSETUP_AGGRESSIVE");
        case MANIACAL:      return UserString("GSETUP_MANIACAL");
        default:            return EMPTY_STRING;
    }
}


GalaxySetupData::GalaxySetupData() :
    size(100),
    shape(SPIRAL_2),
    age(GALAXY_SETUP_MEDIUM),
    starlane_freq(GALAXY_SETUP_MEDIUM),
    planet_density(GALAXY_SETUP_MEDIUM),
    specials_freq(GALAXY_SETUP_MEDIUM),
    monster_freq(GALAXY_SETUP_MEDIUM),
    native_freq(GALAXY_SETUP_MEDIUM),
    ai_aggr(MANIACAL),
    encoding_empire(ALL_EMPIRES)
{}

GalaxySetupData::GalaxySetupData(GalaxySetupData&& base) :
    seed(std::move(base.seed)),
    size(base.size),
    shape(base.shape),
    age(base.age),
    starlane_freq(base.starlane_freq),
    planet_density(base.planet_density),
    specials_freq(base.specials_freq),
    monster_freq(base.monster_freq),
    native_freq(base.native_freq),
    ai_aggr(base.ai_aggr),
    game_rules(std::move(base.game_rules)),
    game_uid(std::move(base.game_uid)),
    encoding_empire(base.encoding_empire)
{ SetSeed(seed); }

auto GalaxySetupData::GetSeed() const -> std::string const&
{ return seed; }

auto GalaxySetupData::GetSize() const -> int
{ return size; }

auto GalaxySetupData::GetShape() const -> Shape
{
    if (shape != RANDOM)
        return shape;
    size_t num_shapes = static_cast<size_t>(GALAXY_SHAPES) - 1; // -1 so that RANDOM isn't counted
    return static_cast<Shape>(GetIdx(num_shapes, seed + "shape"));
}

auto GalaxySetupData::GetAge() const -> GalaxySetupOption
{
    if (age != GALAXY_SETUP_RANDOM)
        return age;
    return static_cast<GalaxySetupOption>(GetIdx(3, seed + "age") + 1);       // need range 1-3 for age
}

auto GalaxySetupData::GetStarlaneFreq() const -> GalaxySetupOption
{
    if (starlane_freq != GALAXY_SETUP_RANDOM)
        return starlane_freq;
    return static_cast<GalaxySetupOption>(GetIdx(3, seed + "lanes") + 1);     // need range 1-3 for starlane freq
}

auto GalaxySetupData::GetPlanetDensity() const -> GalaxySetupOption
{
    if (planet_density != GALAXY_SETUP_RANDOM)
        return planet_density;
    return static_cast<GalaxySetupOption>(GetIdx(3, seed + "planets") + 1);   // need range 1-3 for planet density
}

auto GalaxySetupData::GetSpecialsFreq() const -> GalaxySetupOption
{
    if (specials_freq != GALAXY_SETUP_RANDOM)
        return specials_freq;
    return static_cast<GalaxySetupOption>(GetIdx(4, seed + "specials"));      // need range 0-3 for planet density
}

auto GalaxySetupData::GetMonsterFreq() const -> GalaxySetupOption
{
    if (monster_freq != GALAXY_SETUP_RANDOM)
        return monster_freq;
    return static_cast<GalaxySetupOption>(GetIdx(4, seed + "monsters"));      // need range 0-3 for monster frequency
}

auto GalaxySetupData::GetNativeFreq() const -> GalaxySetupOption
{
    if (native_freq != GALAXY_SETUP_RANDOM)
        return native_freq;
    return static_cast<GalaxySetupOption>(GetIdx(4, seed + "natives"));       // need range 0-3 for native frequency
}

auto GalaxySetupData::GetAggression() const -> Aggression
{ return ai_aggr; }

auto GalaxySetupData::GetGameRules() const -> std::map<std::string, std::string> const&
{ return game_rules; }

auto GalaxySetupData::GetGameUID() const -> std::string const&
{ return game_uid; }

void GalaxySetupData::SetGameUID(std::string const& game_uid_)
{ game_uid = game_uid_; }

void GalaxySetupData::SetSeed(std::string const& seed_)
{
    std::string new_seed = seed_;
    if (new_seed.empty() || new_seed == "RANDOM") {
        ClockSeed();
        new_seed.clear();
        for (int i = 0; i < 8; ++i)
            new_seed += alphanum[RandInt(0, (sizeof(alphanum) - 2))];
        DebugLogger() << "Set empty or requested random seed to " << new_seed;
    }
    seed = std::move(new_seed);
}
