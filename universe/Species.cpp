#include "Species.h"

#include <iterator>
#include <boost/filesystem/fstream.hpp>
#include "Conditions.h"
#include "Effect.h"
#include "PopCenter.h"
#include "Planet.h"
#include "Ship.h"
#include "UniverseObject.h"
#include "ValueRefs.h"
#include "../util/AppInterface.h"
#include "../util/CheckSums.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/Random.h"
#include "../util/ScopedTimer.h"


/////////////////////////////////////////////////
// FocusType                                   //
/////////////////////////////////////////////////
FocusType::FocusType(std::string& name, std::string& description,
                     std::unique_ptr<Condition::Condition>&& location,
                     std::string& graphic) :
    m_name(std::move(name)),
    m_description(std::move(description)),
    m_location(std::move(location)),
    m_graphic(std::move(graphic))
{}

FocusType::~FocusType()
{}

std::string FocusType::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "FocusType\n";
    retval += DumpIndent(ntabs+1) + "name = \"" + m_name + "\"\n";
    retval += DumpIndent(ntabs+1) + "description = \"" + m_description + "\"\n";
    retval += DumpIndent(ntabs+1) + "location = \n";
    retval += m_location->Dump(ntabs+2);
    retval += DumpIndent(ntabs+1) + "graphic = \"" + m_graphic + "\"\n";
    return retval;
}

unsigned int FocusType::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_description);
    CheckSums::CheckSumCombine(retval, m_location);
    CheckSums::CheckSumCombine(retval, m_graphic);

    return retval;
}

/////////////////////////////////////////////////
// Species                                     //
/////////////////////////////////////////////////
namespace {
    std::string PlanetTypeToString(PlanetType type) {
        switch (type) {
        case PlanetType::PT_SWAMP:     return "Swamp";
        case PlanetType::PT_TOXIC:     return "Toxic";
        case PlanetType::PT_INFERNO:   return "Inferno";
        case PlanetType::PT_RADIATED:  return "Radiated";
        case PlanetType::PT_BARREN:    return "Barren";
        case PlanetType::PT_TUNDRA:    return "Tundra";
        case PlanetType::PT_DESERT:    return "Desert";
        case PlanetType::PT_TERRAN:    return "Terran";
        case PlanetType::PT_OCEAN:     return "Ocean";
        case PlanetType::PT_ASTEROIDS: return "Asteroids";
        case PlanetType::PT_GASGIANT:  return "GasGiant";
        default:                       return "?";
        }
    }
    std::string PlanetEnvironmentToString(PlanetEnvironment env) {
        switch (env) {
        case PlanetEnvironment::PE_UNINHABITABLE: return "Uninhabitable";
        case PlanetEnvironment::PE_HOSTILE:       return "Hostile";
        case PlanetEnvironment::PE_POOR:          return "Poor";
        case PlanetEnvironment::PE_ADEQUATE:      return "Adequate";
        case PlanetEnvironment::PE_GOOD:          return "Good";
        default:                                  return "?";
        }
    }
}

Species::Species(std::string&& name, std::string&& desc,
                 std::string&& gameplay_desc, std::vector<FocusType>&& foci,
                 std::string&& default_focus,
                 std::map<PlanetType, PlanetEnvironment>&& planet_environments,
                 std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
                 std::unique_ptr<Condition::Condition>&& combat_targets,
                 bool playable, bool native, bool can_colonize, bool can_produce_ships,
                 const std::set<std::string>& tags,
                 std::set<std::string>&& likes, std::set<std::string>&& dislikes,
                 std::string&& graphic,
                 double spawn_rate, int spawn_limit) :
    m_name(std::move(name)),
    m_description(std::move(desc)),
    m_gameplay_description(std::move(gameplay_desc)),
    m_foci(std::move(foci)),
    m_default_focus(std::move(default_focus)),
    m_planet_environments(std::move(planet_environments)),
    m_combat_targets(std::move(combat_targets)),
    m_playable(playable),
    m_native(native),
    m_can_colonize(can_colonize),
    m_can_produce_ships(can_produce_ships),
    m_spawn_rate(spawn_rate),
    m_spawn_limit(spawn_limit),
    m_likes(std::move(likes)),
    m_dislikes(std::move(dislikes)),
    m_graphic(std::move(graphic))
{
    for (auto&& effect : effects)
        m_effects.emplace_back(std::move(effect));

    Init();

    for (const std::string& tag : tags)
        m_tags.emplace(boost::to_upper_copy<std::string>(tag));
}

Species::~Species()
{}

void Species::Init() {
    for (auto& effect : m_effects)
        effect->SetTopLevelContent(m_name);

    if (!m_location) {
        // set up a Condition structure to match popcenters that have
        // (not uninhabitable) environment for this species

        std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetEnvironment>>> environments;
        environments.emplace_back(
            std::make_unique<ValueRef::Constant<PlanetEnvironment>>(PlanetEnvironment::PE_UNINHABITABLE));

        auto this_species_name_ref =
            std::make_unique<ValueRef::Constant<std::string>>(m_name);  // m_name specifies this species

        auto enviro_cond = std::unique_ptr<Condition::Condition>(
            std::make_unique<Condition::Not>(
                std::unique_ptr<Condition::Condition>(
                    std::make_unique<Condition::PlanetEnvironment>(
                        std::move(environments), std::move(this_species_name_ref)))));

        auto type_cond = std::make_unique<Condition::Type>(
            std::make_unique<ValueRef::Constant<UniverseObjectType>>(UniverseObjectType::OBJ_POP_CENTER));

        m_location = std::unique_ptr<Condition::Condition>(std::make_unique<Condition::And>(
            std::move(enviro_cond), std::move(type_cond)));
    }
    m_location->SetTopLevelContent(m_name);

    if (m_combat_targets)
        m_combat_targets->SetTopLevelContent(m_name);

    TraceLogger() << "Species::Init: " << Dump();
}

std::string Species::Dump(unsigned short ntabs) const {
    std::string retval = DumpIndent(ntabs) + "Species\n";
    retval += DumpIndent(ntabs+1) + "name = \"" + m_name + "\"\n";
    retval += DumpIndent(ntabs+1) + "description = \"" + m_description + "\"\n";
    retval += DumpIndent(ntabs+1) + "gameplay_description = \"" + m_gameplay_description + "\"\n";
    if (m_playable)
        retval += DumpIndent(ntabs+1) + "Playable\n";
    if (m_native)
        retval += DumpIndent(ntabs+1) + "Native\n";
    if (m_can_produce_ships)
        retval += DumpIndent(ntabs+1) + "CanProduceShips\n";
    if (m_can_colonize)
        retval += DumpIndent(ntabs+1) + "CanColonize\n";
    if (m_foci.size() == 1) {
        retval += DumpIndent(ntabs+1) + "foci =\n";
        m_foci.begin()->Dump(ntabs+1);
    } else {
        retval += DumpIndent(ntabs+1) + "foci = [\n";
        for (const FocusType& focus : m_foci)
            retval += focus.Dump(ntabs+2);
        retval += DumpIndent(ntabs+1) + "]\n";
    }
    if (m_effects.size() == 1) {
        retval += DumpIndent(ntabs+1) + "effectsgroups =\n";
        retval += m_effects[0]->Dump(ntabs+2);
    } else {
        retval += DumpIndent(ntabs+1) + "effectsgroups = [\n";
        for (auto& effect : m_effects)
            retval += effect->Dump(ntabs+2);
        retval += DumpIndent(ntabs+1) + "]\n";
    }
    if (m_combat_targets)
        retval += DumpIndent(ntabs+1) + "combatTargets = " + m_combat_targets->Dump(ntabs+2);
    if (m_planet_environments.size() == 1) {
        retval += DumpIndent(ntabs+1) + "environments =\n";
        retval += DumpIndent(ntabs+2) + "type = " + PlanetTypeToString(m_planet_environments.begin()->first)
            + " environment = " + PlanetEnvironmentToString(m_planet_environments.begin()->second)
            + "\n";
    } else {
        retval += DumpIndent(ntabs+1) + "environments = [\n";
        for (const auto& entry : m_planet_environments) {
            retval += DumpIndent(ntabs+2) + "type = " + PlanetTypeToString(entry.first)
                + " environment = " + PlanetEnvironmentToString(entry.second)
                + "\n";
        }
        retval += DumpIndent(ntabs+1) + "]\n";
    }

    retval += DumpIndent(ntabs+1) + "spawnrate = " + std::to_string(m_spawn_rate) + "\n"
           +  DumpIndent(ntabs+1) + "spawnlimit = " + std::to_string(m_spawn_limit) + "\n"
           +  DumpIndent(ntabs+1) + "graphic = \"" + m_graphic + "\"\n";
    return retval;
}

std::string Species::GameplayDescription() const {
    std::stringstream result;

    result << UserString(m_gameplay_description);

    bool requires_separator = true;

    for (auto& effect : m_effects) {
        const std::string& description = effect->GetDescription();
        if (description.empty())
            continue;

        if (requires_separator) {
            result << "\n";
            requires_separator = false;
        }

        result << UserString(description) << "\n";
    }

    return result.str();
}

PlanetEnvironment Species::GetPlanetEnvironment(PlanetType planet_type) const {
    auto it = m_planet_environments.find(planet_type);
    if (it == m_planet_environments.end())
        return PlanetEnvironment::PE_UNINHABITABLE;
    else
        return it->second;
}

namespace {
    PlanetType RingNextPlanetType(PlanetType current_type) {
        PlanetType next(PlanetType(int(current_type)+1));
        if (next >= PlanetType::PT_ASTEROIDS)
            next = PlanetType::PT_SWAMP;
        return next;
    }
    PlanetType RingPreviousPlanetType(PlanetType current_type) {
        PlanetType next(PlanetType(int(current_type)-1));
        if (next <= PlanetType::INVALID_PLANET_TYPE)
            next = PlanetType::PT_OCEAN;
        return next;
    }
}

PlanetType Species::NextBetterPlanetType(PlanetType initial_planet_type) const {
    // some types can't be terraformed
    if (initial_planet_type == PlanetType::PT_GASGIANT)
        return PlanetType::PT_GASGIANT;
    if (initial_planet_type == PlanetType::PT_ASTEROIDS)
        return PlanetType::PT_ASTEROIDS;
    if (initial_planet_type == PlanetType::INVALID_PLANET_TYPE)
        return PlanetType::INVALID_PLANET_TYPE;
    if (initial_planet_type == PlanetType::NUM_PLANET_TYPES)
        return PlanetType::NUM_PLANET_TYPES;
    // and sometimes there's no variation data
    if (m_planet_environments.empty())
        return initial_planet_type;

    // determine which environment rating is the best available for this species,
    // excluding gas giants and asteroids
    PlanetEnvironment best_environment = PlanetEnvironment::PE_UNINHABITABLE;
    //std::set<PlanetType> best_types;
    for (const auto& entry : m_planet_environments) {
        if (entry.first < PlanetType::PT_ASTEROIDS) {
            if (entry.second == best_environment) {
                //best_types.insert(entry.first);
            } else if (entry.second > best_environment) {
                best_environment = entry.second;
                //best_types.clear();
                //best_types.insert(entry.first);
            }
        }
    }

    // if no improvement available, abort early
    PlanetEnvironment initial_environment = GetPlanetEnvironment(initial_planet_type);
    if (initial_environment >= best_environment)
        return initial_planet_type;

    // find which of the best types is closest to the current type
    int forward_steps_to_best = 0;
    for (PlanetType type = RingNextPlanetType(initial_planet_type); type != initial_planet_type; type = RingNextPlanetType(type)) {
        forward_steps_to_best++;
        if (GetPlanetEnvironment(type) == best_environment)
            break;
    }
    int backward_steps_to_best = 0;
    for (PlanetType type = RingPreviousPlanetType(initial_planet_type); type != initial_planet_type; type = RingPreviousPlanetType(type)) {
        backward_steps_to_best++;
        if (GetPlanetEnvironment(type) == best_environment)
            break;
    }
    if (forward_steps_to_best <= backward_steps_to_best)
        return RingNextPlanetType(initial_planet_type);
    else
        return RingPreviousPlanetType(initial_planet_type);
}

unsigned int Species::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_description);
    CheckSums::CheckSumCombine(retval, m_gameplay_description);
    // opinions and homeworlds are per-game specific, so not included in checksum
    CheckSums::CheckSumCombine(retval, m_foci);
    CheckSums::CheckSumCombine(retval, m_default_focus);
    CheckSums::CheckSumCombine(retval, m_planet_environments);
    CheckSums::CheckSumCombine(retval, m_combat_targets);
    CheckSums::CheckSumCombine(retval, m_effects);
    CheckSums::CheckSumCombine(retval, m_location);
    CheckSums::CheckSumCombine(retval, m_playable);
    CheckSums::CheckSumCombine(retval, m_native);
    CheckSums::CheckSumCombine(retval, m_can_colonize);
    CheckSums::CheckSumCombine(retval, m_can_produce_ships);
    CheckSums::CheckSumCombine(retval, m_spawn_rate);
    CheckSums::CheckSumCombine(retval, m_spawn_limit);
    CheckSums::CheckSumCombine(retval, m_tags);
    CheckSums::CheckSumCombine(retval, m_graphic);

    return retval;
}

/////////////////////////////////////////////////
// SpeciesManager                              //
/////////////////////////////////////////////////
namespace {
    boost::optional<Pending::Pending<
        std::pair<SpeciesManager::SpeciesTypeMap,
                  SpeciesManager::CensusOrder>>> s_pending_types;
    SpeciesManager::SpeciesTypeMap s_species;
    SpeciesManager::CensusOrder s_census_order;
}

bool SpeciesManager::PlayableSpecies::operator()(
    const std::map<std::string, std::unique_ptr<Species>>::value_type& species_entry) const
{ return species_entry.second->Playable(); }

bool SpeciesManager::NativeSpecies::operator()(
    const std::map<std::string, std::unique_ptr<Species>>::value_type& species_entry) const
{ return species_entry.second->Native(); }

const Species* SpeciesManager::GetSpecies(const std::string& name) const {
    CheckPendingSpeciesTypes();
    auto it = s_species.find(name);
    return it != s_species.end() ? it->second.get() : nullptr;
}

Species* SpeciesManager::GetSpecies(const std::string& name) {
    CheckPendingSpeciesTypes();
    auto it = s_species.find(name);
    return it != s_species.end() ? it->second.get() : nullptr;
}

namespace {
    std::mutex species_mutex;
}

void SpeciesManager::SetSpeciesTypes(Pending::Pending<std::pair<SpeciesTypeMap, CensusOrder>>&& future) {
    std::lock_guard<std::mutex> lock(species_mutex);
    s_pending_types = std::move(future);
}

void SpeciesManager::CheckPendingSpeciesTypes() {
    std::lock_guard<std::mutex> lock(species_mutex);

    if (!s_pending_types) {
        if (s_species.empty())
            throw;
        return;
    }

    decltype(s_pending_types)::value_type::result_type container;
    Pending::SwapPending(s_pending_types, container);

    s_species = std::move(container.first);
    s_census_order = std::move(container.second);
}

SpeciesManager::iterator SpeciesManager::begin() const {
    CheckPendingSpeciesTypes();
    return s_species.begin();
}

SpeciesManager::iterator SpeciesManager::end() const {
    CheckPendingSpeciesTypes();
    return s_species.end();
}

SpeciesManager::playable_iterator SpeciesManager::playable_begin() const
{ return playable_iterator(PlayableSpecies(), begin(), end()); }

SpeciesManager::playable_iterator SpeciesManager::playable_end() const
{ return playable_iterator(PlayableSpecies(), end(), end()); }

SpeciesManager::native_iterator SpeciesManager::native_begin() const
{ return native_iterator(NativeSpecies(), begin(), end()); }

SpeciesManager::native_iterator SpeciesManager::native_end() const
{ return native_iterator(NativeSpecies(), end(), end()); }

const SpeciesManager::CensusOrder& SpeciesManager::census_order() const {
    CheckPendingSpeciesTypes();
    return s_census_order;
}

bool SpeciesManager::empty() const {
    CheckPendingSpeciesTypes();
    return s_species.empty();
}

int SpeciesManager::NumSpecies() const {
    CheckPendingSpeciesTypes();
    return s_species.size();
}

int SpeciesManager::NumPlayableSpecies() const
{ return std::distance(playable_begin(), playable_end()); }

int SpeciesManager::NumNativeSpecies() const
{ return std::distance(native_begin(), native_end()); }

namespace {
    const std::string EMPTY_STRING;
}

const std::string& SpeciesManager::RandomSpeciesName() const {
    CheckPendingSpeciesTypes();
    if (s_species.empty())
        return EMPTY_STRING;

    int species_idx = RandInt(0, static_cast<int>(s_species.size()) - 1);
    return std::next(begin(), species_idx)->first;
}

const std::string& SpeciesManager::RandomPlayableSpeciesName() const {
    if (NumPlayableSpecies() <= 0)
        return EMPTY_STRING;

    int species_idx = RandInt(0, NumPlayableSpecies() - 1);
    return std::next(playable_begin(), species_idx)->first;
}

const std::string& SpeciesManager::SequentialPlayableSpeciesName(int id) const {
    if (NumPlayableSpecies() <= 0)
        return EMPTY_STRING;

    int species_idx = id % NumPlayableSpecies();
    DebugLogger() << "SpeciesManager::SequentialPlayableSpeciesName has " << NumPlayableSpecies() << " and is given id " << id << " yielding index " << species_idx;
    return std::next(playable_begin(), species_idx)->first;
}

void SpeciesManager::SetSpeciesHomeworlds(std::map<std::string, std::set<int>>&& species_homeworld_ids)
{ m_species_homeworlds = std::move(species_homeworld_ids); }

void SpeciesManager::SetSpeciesEmpireOpinions(std::map<std::string, std::map<int, float>>&& species_empire_opinions)
{ m_species_empire_opinions = std::move(species_empire_opinions); }

void SpeciesManager::SetSpeciesEmpireOpinion(const std::string& species_name, int empire_id, float opinion)
{ m_species_empire_opinions[species_name][empire_id] = opinion; }

void SpeciesManager::SetSpeciesSpeciesOpinions(std::map<std::string,
                                               std::map<std::string, float>>&& species_species_opinions)
{ m_species_species_opinions = std::move(species_species_opinions); }

void SpeciesManager::SetSpeciesSpeciesOpinion(const std::string& opinionated_species,
                                              const std::string& rated_species, float opinion)
{ m_species_species_opinions[opinionated_species][rated_species] = opinion; }

std::map<std::string, std::set<int>> SpeciesManager::GetSpeciesHomeworldsMap(int encoding_empire/* = ALL_EMPIRES*/) const {
    if (encoding_empire == ALL_EMPIRES)
        return m_species_homeworlds;
    // TODO: filter, output only info about species an empire has observed...?
    return m_species_homeworlds;
}

const std::map<std::string, std::map<int, float>>& SpeciesManager::GetSpeciesEmpireOpinionsMap(int encoding_empire/* = ALL_EMPIRES*/) const
{ return m_species_empire_opinions; }

const std::map<std::string, std::map<std::string, float>>& SpeciesManager::GetSpeciesSpeciesOpinionsMap(int encoding_empire/* = ALL_EMPIRES*/) const
{ return m_species_species_opinions; }

float SpeciesManager::SpeciesEmpireOpinion(const std::string& species_name, int empire_id) const {
    auto sp_it = m_species_empire_opinions.find(species_name);
    if (sp_it == m_species_empire_opinions.end())
        return 0.0f;
    const std::map<int, float>& emp_map = sp_it->second;
    auto emp_it = emp_map.find(empire_id);
    if (emp_it == emp_map.end())
        return 0.0f;
    return emp_it->second;
}

float SpeciesManager::SpeciesSpeciesOpinion(const std::string& opinionated_species_name,
                                            const std::string& rated_species_name) const
{
    auto sp_it = m_species_species_opinions.find(opinionated_species_name);
    if (sp_it == m_species_species_opinions.end())
        return 0.0f;
    const auto& ra_sp_map = sp_it->second;
    auto ra_sp_it = ra_sp_map.find(rated_species_name);
    if (ra_sp_it == ra_sp_map.end())
        return 0.0f;
    return ra_sp_it->second;
}

void SpeciesManager::ClearSpeciesOpinions() {
    m_species_empire_opinions.clear();
    m_species_species_opinions.clear();
}

void SpeciesManager::AddSpeciesHomeworld(std::string species, int homeworld_id) {
    if (homeworld_id == INVALID_OBJECT_ID)
        return;
    if (species.empty())
        return;
    m_species_homeworlds[std::move(species)].insert(homeworld_id);
}

void SpeciesManager::RemoveSpeciesHomeworld(const std::string& species, int homeworld_id) {
    if (homeworld_id == INVALID_OBJECT_ID)
        return;
    if (species.empty())
        return;
    auto it = m_species_homeworlds.find(species);
    if (it != m_species_homeworlds.end())
        it->second.erase(homeworld_id);
}

void SpeciesManager::ClearSpeciesHomeworlds()
{ m_species_homeworlds.clear(); }

void SpeciesManager::UpdatePopulationCounter() {
    // ships of each species and design
    m_species_object_populations.clear();
    for (const auto& entry : Objects().ExistingObjects()) {
        auto obj = entry.second;
        if (obj->ObjectType() != UniverseObjectType::OBJ_PLANET && obj->ObjectType() != UniverseObjectType::OBJ_POP_CENTER)
            continue;

        auto pop_center = std::dynamic_pointer_cast<const PopCenter>(obj);
        if (!pop_center)
            continue;

        const std::string& species = pop_center->SpeciesName();
        if (species.empty())
            continue;

        try {
            m_species_object_populations[species][obj->ID()] += obj->GetMeter(MeterType::METER_POPULATION)->Current();
        } catch (...) {
            continue;
        }
    }
}

std::map<std::string, std::map<int, float>>& SpeciesManager::SpeciesObjectPopulations(int encoding_empire)
{ return m_species_object_populations; }

std::map<std::string, std::map<std::string, int>>& SpeciesManager::SpeciesShipsDestroyed(int encoding_empire)
{ return m_species_species_ships_destroyed; }

unsigned int SpeciesManager::GetCheckSum() const {
    CheckPendingSpeciesTypes();
    unsigned int retval{0};
    for (auto const& name_type_pair : s_species)
        CheckSums::CheckSumCombine(retval, name_type_pair);
    CheckSums::CheckSumCombine(retval, s_species.size());

    DebugLogger() << "SpeciesManager checksum: " << retval;
    return retval;
}
