#include "Species.h"

#include "ParserUtil.h"
#include "Effect.h"
#include "Condition.h"
#include "../util/MultiplayerCommon.h"
#include "../util/Directories.h"

#include <boost/filesystem/fstream.hpp>

std::string DumpIndent();

extern int g_indent;

namespace {
    struct store_species_impl
    {
        template <class T1, class T2>
        struct result {typedef void type;};
        template <class T>
        void operator()(std::map<std::string, Species*>& species_map, const T& species) const
        {
            if (species_map.find(species->Name()) != species_map.end()) {
                std::string error_str = "ERROR: More than one building type in species.txt has the name " + species->Name();
                throw std::runtime_error(error_str.c_str());
            }
            species_map[species->Name()] = species;
        }
    };

    const phoenix::function<store_species_impl> store_species_;
}


/////////////////////////////////////////////////
// FocusType                                   //
/////////////////////////////////////////////////
FocusType::FocusType() :
    m_name(),
    m_description(),
    m_location(),
    m_graphic()
{}

FocusType::FocusType(const std::string& name, const std::string& description,
                     const Condition::ConditionBase* location, const std::string& graphic) :
    m_name(name),
    m_description(description),
    m_location(location),
    m_graphic(graphic)
{}

const std::string& FocusType::Name() const {
    return m_name;
}

const std::string& FocusType::Description() const {
    return m_description;
}

const Condition::ConditionBase* FocusType::Location() const {
    return m_location.get();
}

const std::string& FocusType::Graphic() const {
    return m_graphic;
}

std::string FocusType::Dump() const {
    std::string retval = DumpIndent() + "FocusType\n";
    ++g_indent;
    retval += DumpIndent() + "name = \"" + m_name + "\"\n";
    retval += DumpIndent() + "description = \"" + m_description + "\"\n";
    retval += DumpIndent() + "location = \n";
    ++g_indent;
    retval += m_location->Dump();
    --g_indent;
    retval += DumpIndent() + "graphic = \"" + m_graphic + "\"\n";
    --g_indent;
    return retval;
}

/////////////////////////////////////////////////
// Species                                     //
/////////////////////////////////////////////////
Species::Species(const std::string& name, const std::string& description,
                 const std::vector<FocusType>& foci,
                 const std::map<PlanetType, PlanetEnvironment>& planet_environments,
                 const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects,
                 const std::string& graphic) :
    m_name(name),
    m_description(description),
    m_foci(foci),
    m_planet_environments(planet_environments),
    m_effects(effects),
    m_graphic(graphic)
{}

const std::string& Species::Name() const {
    return m_name;
}

const std::string& Species::Description() const {
    return m_description;
}

const std::set<int>& Species::Homeworlds() const {
    return m_homeworlds;
}

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

std::string Species::Dump() const {
    std::string retval = DumpIndent() + "Species\n";
    ++g_indent;
    retval += DumpIndent() + "name = \"" + m_name + "\"\n";
    retval += DumpIndent() + "description = \"" + m_description + "\"\n";
    if (m_foci.size() == 1) {
        retval += DumpIndent() + "foci =\n";
        m_foci.begin()->Dump();
    } else {
        retval += DumpIndent() + "foci = [\n";
        ++g_indent;
        for (std::vector<FocusType>::const_iterator it = m_foci.begin(); it != m_foci.end(); ++it) {
            retval += it->Dump();
        }
        --g_indent;
        retval += DumpIndent() + "]\n";
    }
    if (m_effects.size() == 1) {
        retval += DumpIndent() + "effectsgroups =\n";
        ++g_indent;
        retval += m_effects[0]->Dump();
        --g_indent;
    } else {
        retval += DumpIndent() + "effectsgroups = [\n";
        ++g_indent;
        for (unsigned int i = 0; i < m_effects.size(); ++i) {
            retval += m_effects[i]->Dump();
        }
        --g_indent;
        retval += DumpIndent() + "]\n";
    }
    if (m_planet_environments.size() == 1) {
        retval += DumpIndent() + "environments =\n";
        ++g_indent;
        retval += DumpIndent() + "type = " + PlanetTypeToString(m_planet_environments.begin()->first)
                               + " environment = " + PlanetEnvironmentToString(m_planet_environments.begin()->second)
                               + "\n";
        --g_indent;
    } else {
        retval += DumpIndent() + "environments = [\n";
        ++g_indent;
        for (std::map<PlanetType, PlanetEnvironment>::const_iterator it = m_planet_environments.begin(); it != m_planet_environments.end(); ++it) {
            retval += DumpIndent() + "type = " + PlanetTypeToString(it->first)
                                   + " environment = " + PlanetEnvironmentToString(it->second)
                                   + "\n";
        }
        --g_indent;
        retval += DumpIndent() + "]\n";
    }
    retval += DumpIndent() + "graphic = \"" + m_graphic + "\"\n";
    --g_indent;
    return retval;
}

const std::vector<FocusType>& Species::Foci() const
{
    return m_foci;
}

const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& Species::Effects() const {
    return m_effects;
}

const std::map<PlanetType, PlanetEnvironment>& Species::PlanetEnvironments() const {
    return m_planet_environments;
}

PlanetEnvironment Species::GetPlanetEnvironment(PlanetType planet_type) const {
    std::map<PlanetType, PlanetEnvironment>::const_iterator it = m_planet_environments.find(planet_type);
    if (it == m_planet_environments.end())
        return PE_UNINHABITABLE;
    else
        return it->second;
}

const std::string& Species::Graphic() const {
    return m_graphic;
}

void Species::AddHomeworld(int homeworld_id) {
    if (!GetMainObjectMap().Object(homeworld_id))
        Logger().debugStream() << "Species asked to add homeworld id " << homeworld_id << " but there is no such object in the Universe";
    if (m_homeworlds.find(homeworld_id) != m_homeworlds.end())
        return;
    m_homeworlds.insert(homeworld_id);
    // TODO if needed: StateChangedSignal();
}

void Species::RemoveHomeworld(int homeworld_id) {
    if (m_homeworlds.find(homeworld_id) == m_homeworlds.end()) {
        Logger().debugStream() << "Species asked to remove homeworld id " << homeworld_id << " but doesn't have that id as a homeworld";
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


/////////////////////////////////////////////////
// SpeciesManager                              //
/////////////////////////////////////////////////
// static(s)
SpeciesManager* SpeciesManager::s_instance = 0;

SpeciesManager::SpeciesManager() {
    if (s_instance)
        throw std::runtime_error("Attempted to create more than one SpeciesManager.");

    s_instance = this;

    std::string file_name = "species.txt";
    std::string input;

    boost::filesystem::ifstream ifs(GetResourceDir() / file_name);
    if (ifs) {
        std::getline(ifs, input, '\0');
        ifs.close();
    } else {
        Logger().errorStream() << "Unable to open data file " << file_name;
        return;
    }

    using namespace boost::spirit::classic;
    using namespace phoenix;
    parse_info<const char*> result =
        parse(input.c_str(),
              as_lower_d[*species_p[store_species_(var(m_species), arg1)]]
              >> end_p,
              skip_p);
    if (!result.full)
        ReportError(input.c_str(), result);
}

SpeciesManager::~SpeciesManager() {
    for (std::map<std::string, Species*>::iterator it = m_species.begin(); it != m_species.end(); ++it) {
        delete it->second;
    }
}

const Species* SpeciesManager::GetSpecies(const std::string& name) const {
    std::map<std::string, Species*>::const_iterator it = m_species.find(name);
    return it != m_species.end() ? it->second : 0;
}

Species* SpeciesManager::GetSpecies(const std::string& name) {
    std::map<std::string, Species*>::iterator it = m_species.find(name);
    return it != m_species.end() ? it->second : 0;
}

SpeciesManager& SpeciesManager::GetSpeciesManager() {
    static SpeciesManager manager;
    return manager;
}

SpeciesManager::iterator SpeciesManager::begin() const {
    return m_species.begin();
}

SpeciesManager::iterator SpeciesManager::end() const {
    return m_species.end();
}

bool SpeciesManager::empty() const {
    return m_species.empty();
}

int SpeciesManager::NumSpecies() const {
    return m_species.size();
}

void SpeciesManager::SetSpeciesHomeworlds(const std::map<std::string, std::set<int> >& species_homeworld_ids) {
    for (std::map<std::string, std::set<int> >::const_iterator it = species_homeworld_ids.begin(); it != species_homeworld_ids.end(); ++it) {
        const std::string& species_name = it->first;
        const std::set<int>& homeworlds = it->second;

        Species* species = 0;
        std::map<std::string, Species*>::iterator species_it = m_species.find(species_name);
        if (species_it != m_species.end())
            species = species_it->second;

        if (species) {
            species->SetHomeworlds(homeworlds);
        } else {
            Logger().errorStream() << "SpeciesManager::SetSpeciesHomeworlds couldn't find a species with name " << species_name << " to assign homeworlds to";
        }
    }
}

std::map<std::string, std::set<int> > SpeciesManager::GetSpeciesHomeworldsMap(int encoding_empire/* = ALL_EMPIRES*/) const {
    std::map<std::string, std::set<int> > retval;
    for (iterator it = begin(); it != end(); ++it) {
        const std::string species_name = it->first;
        const Species* species = it->second;
        if (!species) {
            Logger().errorStream() << "SpeciesManager::GetSpeciesHomeworldsMap found a null species pointer in SpeciesManager?!";
            continue;
        }
        const std::set<int>& homeworld_ids = species->Homeworlds();
        for (std::set<int>::const_iterator homeworlds_it = homeworld_ids.begin(); homeworlds_it != homeworld_ids.end(); ++homeworlds_it)
            retval[species_name].insert(*homeworlds_it);
    }
    return retval;
}


///////////////////////////////////////////////////////////
// Free Functions                                        //
///////////////////////////////////////////////////////////
SpeciesManager& GetSpeciesManager() {
    return SpeciesManager::GetSpeciesManager();
}

const Species* GetSpecies(const std::string& name) {
    return GetSpeciesManager().GetSpecies(name);
}
