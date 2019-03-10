#include "Species.h"

#include "Condition.h"
#include "Effect.h"
#include "PopCenter.h"
#include "Ship.h"
#include "UniverseObject.h"
#include "ValueRef.h"
#include "Enums.h"
#include "../util/OptionsDB.h"
#include "../util/Logger.h"
#include "../util/Random.h"
#include "../util/AppInterface.h"
#include "../util/CheckSums.h"
#include "../util/ScopedTimer.h"

#include <boost/filesystem/fstream.hpp>
//TODO: replace with std::make_unique when transitioning to C++14
#include <boost/smart_ptr/make_unique.hpp>

#include <iterator>


/////////////////////////////////////////////////
// FocusType                                   //
/////////////////////////////////////////////////
FocusType::FocusType(const std::string& name, const std::string& description,
                     std::unique_ptr<Condition::ConditionBase>&& location,
                     const std::string& graphic) :
    m_name(name),
    m_description(description),
    m_location(std::move(location)),
    m_graphic(graphic)
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
        case PT_SWAMP:      return "Swamp";
        case PT_TOXIC:      return "Toxic";
        case PT_INFERNO:    return "Inferno";
        case PT_RADIATED:   return "Radiated";
        case PT_BARREN:     return "Barren";
        case PT_TUNDRA:     return "Tundra";
        case PT_DESERT:     return "Desert";
        case PT_TERRAN:     return "Terran";
        case PT_OCEAN:      return "Ocean";
        case PT_ASTEROIDS:  return "Asteroids";
        case PT_GASGIANT:   return "GasGiant";
        default:            return "?";
        }
    }
    std::string PlanetEnvironmentToString(PlanetEnvironment env) {
        switch (env) {
        case PE_UNINHABITABLE:  return "Uninhabitable";
        case PE_HOSTILE:        return "Hostile";
        case PE_POOR:           return "Poor";
        case PE_ADEQUATE:       return "Adequate";
        case PE_GOOD:           return "Good";
        default:                return "?";
        }
    }
}

Species::Species(const SpeciesStrings& strings,
                 const std::vector<FocusType>& foci,
                 const std::string& preferred_focus,
                 const std::map<PlanetType, PlanetEnvironment>& planet_environments,
                 std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
                 std::unique_ptr<Condition::ConditionBase>&& combat_targets,
                 const SpeciesParams& params,
                 const std::set<std::string>& tags,
                 const std::string& graphic) :
    m_name(strings.name),
    m_description(strings.desc),
    m_gameplay_description(strings.gameplay_desc),
    m_foci(foci),
    m_preferred_focus(preferred_focus),
    m_planet_environments(planet_environments),
    m_combat_targets(std::move(combat_targets)),
    m_playable(params.playable),
    m_native(params.native),
    m_can_colonize(params.can_colonize),
    m_can_produce_ships(params.can_produce_ships),
    m_graphic(graphic)
{
    for (auto&& effect : effects)
        m_effects.emplace_back(std::move(effect));

    Init();

    for (const std::string& tag : tags)
        m_tags.insert(boost::to_upper_copy<std::string>(tag));
}

Species::~Species()
{}

void Species::Init() {
    for (auto& effect : m_effects) {
        effect->SetTopLevelContent(m_name);
    }

    if (!m_location) {
        // set up a Condition structure to match popcenters that have
        // (not uninhabitable) environment for this species
        std::vector<std::unique_ptr<ValueRef::ValueRefBase< ::PlanetEnvironment>>> environments_vec;
        environments_vec.push_back(
            boost::make_unique<ValueRef::Constant<PlanetEnvironment>>( ::PE_UNINHABITABLE));
        auto this_species_name_ref =
            boost::make_unique<ValueRef::Constant<std::string>>(m_name);  // m_name specifies this species
        auto enviro_cond = std::unique_ptr<Condition::ConditionBase>(
            boost::make_unique<Condition::Not>(
                std::unique_ptr<Condition::ConditionBase>(
                    boost::make_unique<Condition::PlanetEnvironment>(
                        std::move(environments_vec), std::move(this_species_name_ref)))));

        auto type_cond = std::unique_ptr<Condition::ConditionBase>(boost::make_unique<Condition::Type>(
            boost::make_unique<ValueRef::Constant<UniverseObjectType>>( ::OBJ_POP_CENTER)));

        std::vector<std::unique_ptr<Condition::ConditionBase>> operands;
        operands.push_back(std::move(enviro_cond));
        operands.push_back(std::move(type_cond));

        m_location = std::unique_ptr<Condition::ConditionBase>(boost::make_unique<Condition::And>(std::move(operands)));
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
    retval += DumpIndent(ntabs+1) + "graphic = \"" + m_graphic + "\"\n";
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
        return PE_UNINHABITABLE;
    else
        return it->second;
}

namespace {
    PlanetType RingNextPlanetType(PlanetType current_type) {
        PlanetType next(PlanetType(int(current_type)+1));
        if (next >= PT_ASTEROIDS)
            next = PT_SWAMP;
        return next;
    }
    PlanetType RingPreviousPlanetType(PlanetType current_type) {
        PlanetType next(PlanetType(int(current_type)-1));
        if (next <= INVALID_PLANET_TYPE)
            next = PT_OCEAN;
        return next;
    }
}

PlanetType Species::NextBetterPlanetType(PlanetType initial_planet_type) const {
    // some types can't be terraformed
    if (initial_planet_type == PT_GASGIANT)
        return PT_GASGIANT;
    if (initial_planet_type == PT_ASTEROIDS)
        return PT_ASTEROIDS;
    if (initial_planet_type == INVALID_PLANET_TYPE)
        return INVALID_PLANET_TYPE;
    if (initial_planet_type == NUM_PLANET_TYPES)
        return NUM_PLANET_TYPES;
    // and sometimes there's no variation data
    if (m_planet_environments.empty())
        return initial_planet_type;

    // determine which environment rating is the best available for this species
    PlanetEnvironment best_environment = PE_UNINHABITABLE;
    //std::set<PlanetType> best_types;
    for (const auto& entry : m_planet_environments) {
        if (entry.second == best_environment) {
            //best_types.insert(entry.first);
        } else if (entry.second > best_environment) {
            best_environment = entry.second;
            //best_types.clear();
            //best_types.insert(entry.first);
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

void Species::AddHomeworld(int homeworld_id) {
    if (!GetUniverseObject(homeworld_id))
        DebugLogger() << "Species asked to add homeworld id " << homeworld_id << " but there is no such object in the Universe";
    if (m_homeworlds.count(homeworld_id))
        return;
    m_homeworlds.insert(homeworld_id);
    // TODO if needed: StateChangedSignal();
}

void Species::RemoveHomeworld(int homeworld_id) {
    if (!m_homeworlds.count(homeworld_id)) {
        DebugLogger() << "Species asked to remove homeworld id " << homeworld_id << " but doesn't have that id as a homeworld";
        return;
    }
    m_homeworlds.erase(homeworld_id);
    // TODO if needed: StateChangedSignal();
}

void Species::SetHomeworlds(const std::set<int>& homeworld_ids) {
    if (m_homeworlds == homeworld_ids)
        return;
    m_homeworlds = homeworld_ids;
    // TODO if needed: StateChangedSignal();
}

void Species::SetEmpireOpinions(const std::map<int, double>& opinions)
{}

void Species::SetEmpireOpinion(int empire_id, double opinion)
{}

void Species::SetOtherSpeciesOpinions(const std::map<std::string, double>& opinions)
{}

void Species::SetOtherSpeciesOpinion(const std::string& species_name, double opinion)
{}

unsigned int Species::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_description);
    CheckSums::CheckSumCombine(retval, m_gameplay_description);
    // opinions and homeworlds are per-game specific, so not included in checksum
    CheckSums::CheckSumCombine(retval, m_foci);
    CheckSums::CheckSumCombine(retval, m_preferred_focus);
    CheckSums::CheckSumCombine(retval, m_planet_environments);
    CheckSums::CheckSumCombine(retval, m_combat_targets);
    CheckSums::CheckSumCombine(retval, m_effects);
    CheckSums::CheckSumCombine(retval, m_location);
    CheckSums::CheckSumCombine(retval, m_playable);
    CheckSums::CheckSumCombine(retval, m_native);
    CheckSums::CheckSumCombine(retval, m_can_colonize);
    CheckSums::CheckSumCombine(retval, m_can_produce_ships);
    CheckSums::CheckSumCombine(retval, m_tags);
    CheckSums::CheckSumCombine(retval, m_graphic);

    return retval;
}

/////////////////////////////////////////////////
// SpeciesManager                              //
/////////////////////////////////////////////////
// static(s)
SpeciesManager* SpeciesManager::s_instance = nullptr;

bool SpeciesManager::PlayableSpecies::operator()(
    const std::map<std::string, std::unique_ptr<Species>>::value_type& species_entry) const
{ return species_entry.second->Playable(); }

bool SpeciesManager::NativeSpecies::operator()(
    const std::map<std::string, std::unique_ptr<Species>>::value_type& species_entry) const
{ return species_entry.second->Native(); }

SpeciesManager::SpeciesManager() {
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one SpeciesManager.");

    // Only update the global pointer on sucessful construction.
    s_instance = this;
}

const Species* SpeciesManager::GetSpecies(const std::string& name) const {
    CheckPendingSpeciesTypes();
    auto it = m_species.find(name);
    return it != m_species.end() ? it->second.get() : nullptr;
}

Species* SpeciesManager::GetSpecies(const std::string& name) {
    CheckPendingSpeciesTypes();
    auto it = m_species.find(name);
    return it != m_species.end() ? it->second.get() : nullptr;
}

int SpeciesManager::GetSpeciesID(const std::string& name) const {
    CheckPendingSpeciesTypes();
    auto it = m_species.find(name);
    if (it == m_species.end())
        return -1;
    return std::distance(m_species.begin(), it);
}

SpeciesManager& SpeciesManager::GetSpeciesManager() {
    static SpeciesManager manager;
    return manager;
}

void SpeciesManager::SetSpeciesTypes(Pending::Pending<std::pair<SpeciesTypeMap, CensusOrder>>&& future)
{ m_pending_types = std::move(future); }

void SpeciesManager::CheckPendingSpeciesTypes() const {
    if (!m_pending_types) {
        if (m_species.empty())
            throw;
        return;
    }

    auto container = std::make_pair(std::move(m_species), m_census_order);

    Pending::SwapPending(m_pending_types, container);

    m_species = std::move(container.first);
    m_census_order = std::move(container.second);
}

SpeciesManager::iterator SpeciesManager::begin() const {
    CheckPendingSpeciesTypes();
    return m_species.begin();
}

SpeciesManager::iterator SpeciesManager::end() const {
    CheckPendingSpeciesTypes();
    return m_species.end();
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
    return m_census_order;
}

bool SpeciesManager::empty() const {
    CheckPendingSpeciesTypes();
    return m_species.empty();
}

int SpeciesManager::NumSpecies() const {
    CheckPendingSpeciesTypes();
    return m_species.size();
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
    if (m_species.empty())
        return EMPTY_STRING;

    int species_idx = RandSmallInt(0, static_cast<int>(m_species.size()) - 1);
    return std::next(begin(), species_idx)->first;
}

const std::string& SpeciesManager::RandomPlayableSpeciesName() const {
    if (NumPlayableSpecies() <= 0)
        return EMPTY_STRING;

    int species_idx = RandSmallInt(0, NumPlayableSpecies() - 1);
    return std::next(playable_begin(), species_idx)->first;
}

const std::string& SpeciesManager::SequentialPlayableSpeciesName(int id) const {
    if (NumPlayableSpecies() <= 0)
        return EMPTY_STRING;

    int species_idx = id % NumPlayableSpecies();
    DebugLogger() << "SpeciesManager::SequentialPlayableSpeciesName has " << NumPlayableSpecies() << " and is given id " << id << " yielding index " << species_idx;
    return std::next(playable_begin(), species_idx)->first;
}

void SpeciesManager::ClearSpeciesHomeworlds() {
    CheckPendingSpeciesTypes();
    for (auto& entry : m_species)
        entry.second->SetHomeworlds(std::set<int>());
}

void SpeciesManager::SetSpeciesHomeworlds(const std::map<std::string, std::set<int>>& species_homeworld_ids) {
    CheckPendingSpeciesTypes();
    ClearSpeciesHomeworlds();
    for (auto& entry : species_homeworld_ids) {
        const std::string& species_name = entry.first;
        const std::set<int>& homeworlds = entry.second;

        Species* species = nullptr;
        auto species_it = m_species.find(species_name);
        if (species_it != end())
            species = species_it->second.get();

        if (species) {
            species->SetHomeworlds(homeworlds);
        } else {
            ErrorLogger() << "SpeciesManager::SetSpeciesHomeworlds couldn't find a species with name " << species_name << " to assign homeworlds to";
        }
    }
}

void SpeciesManager::SetSpeciesEmpireOpinions(const std::map<std::string, std::map<int, float>>& species_empire_opinions)
{ m_species_empire_opinions = species_empire_opinions; }

void SpeciesManager::SetSpeciesEmpireOpinion(const std::string& species_name, int empire_id, float opinion)
{ m_species_empire_opinions[species_name][empire_id] = opinion; }

void SpeciesManager::SetSpeciesSpeciesOpinions(const std::map<std::string, std::map<std::string, float>>& species_species_opinions)
{ m_species_species_opinions = species_species_opinions; }

void SpeciesManager::SetSpeciesSpeciesOpinion(const std::string& opinionated_species, const std::string& rated_species, float opinion)
{ m_species_species_opinions[opinionated_species][rated_species] = opinion; }

std::map<std::string, std::set<int>> SpeciesManager::GetSpeciesHomeworldsMap(int encoding_empire/* = ALL_EMPIRES*/) const {
    CheckPendingSpeciesTypes();
    std::map<std::string, std::set<int>> retval;
    for (const auto& entry : m_species) {
        const std::string species_name = entry.first;
        const Species* species = entry.second.get();
        if (!species) {
            ErrorLogger() << "SpeciesManager::GetSpeciesHomeworldsMap found a null species pointer in SpeciesManager?!";
            continue;
        }
        for (int homeworld_id : species->Homeworlds())
            retval[species_name].insert(homeworld_id);
    }
    return retval;
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

float SpeciesManager::SpeciesSpeciesOpinion(const std::string& opinionated_species_name, const std::string& rated_species_name) const {
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

void SpeciesManager::UpdatePopulationCounter() {
    // ships of each species and design
    m_species_object_populations.clear();
    for (const auto& entry : Objects().ExistingObjects()) {
        auto obj = entry.second;
        if (obj->ObjectType() != OBJ_PLANET && obj->ObjectType() != OBJ_POP_CENTER)
            continue;

        auto pop_center = std::dynamic_pointer_cast<PopCenter>(obj);
        if (!pop_center)
            continue;

        const std::string& species = pop_center->SpeciesName();
        if (species.empty())
            continue;

        try {
            m_species_object_populations[species][obj->ID()] += obj->CurrentMeterValue(METER_POPULATION);
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
    for (auto const& name_type_pair : m_species)
        CheckSums::CheckSumCombine(retval, name_type_pair);
    CheckSums::CheckSumCombine(retval, m_species.size());

    DebugLogger() << "SpeciesManager checksum: " << retval;
    return retval;
}

///////////////////////////////////////////////////////////
// Free Functions                                        //
///////////////////////////////////////////////////////////
SpeciesManager& GetSpeciesManager()
{ return SpeciesManager::GetSpeciesManager(); }

const Species* GetSpecies(const std::string& name)
{ return SpeciesManager::GetSpeciesManager().GetSpecies(name); }
