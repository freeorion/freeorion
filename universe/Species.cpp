#include "Species.h"

#include "ParserUtil.h"
#include "Effect.h"
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
// Species                                     //
/////////////////////////////////////////////////
Species::Species(const std::string& name, const std::string& description,
                 const std::vector<boost::shared_ptr<const Effect::EffectsGroup> > effects,
                 const std::string& graphic) :
    m_name(name),
    m_description(description),
    m_effects(effects),
    m_graphic(graphic)
{}

const std::string& Species::Name() const {
    return m_name;
}

const std::string& Species::Description() const {
    return m_description;
}

std::string Species::Dump() const {
    std::string retval = DumpIndent() + "Species\n";
    ++g_indent;
    retval += DumpIndent() + "name = \"" + m_name + "\"\n";
    retval += DumpIndent() + "description = \"" + m_description + "\"\n";
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
    retval += DumpIndent() + "graphic = \"" + m_graphic + "\"\n";
    --g_indent;
    return retval;
}

const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& Species::Effects() const {
    return m_effects;
}

const std::string& Species::Graphic() const {
    return m_graphic;
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


///////////////////////////////////////////////////////////
// Free Functions                                        //
///////////////////////////////////////////////////////////
SpeciesManager& GetSpeciesManager() {
    return SpeciesManager::GetSpeciesManager();
}

const Species* GetSpecies(const std::string& name) {
    return GetSpeciesManager().GetSpecies(name);
}
