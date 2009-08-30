#include "Special.h"

#include "../universe/ParserUtil.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"

#include <boost/filesystem/fstream.hpp>

std::string DumpIndent();

extern int g_indent;

namespace {
    struct store_special_impl
    {
        template <class T1, class T2, class T3, class T4>
        struct result {typedef void type;};
        template <class T>
        void operator()(std::map<std::string, Special*>& specials, std::set<std::string>& planet_special_names, bool planet_specials, const T& special) const
        {
            if (specials.find(special->Name()) != specials.end()) {
                std::string error_str = "ERROR: More than one special in specials.txt and planet_specials.txt has the name " + special->Name();
                throw std::runtime_error(error_str.c_str());
            }
            specials[special->Name()] = special;
            if (planet_specials)
                planet_special_names.insert(special->Name());
        }
    };

    const phoenix::function<store_special_impl> store_special_;

    class SpecialManager
    {
    public:
        SpecialManager()
        {
            ProcessSpecialsFile("specials.txt",         false);
            ProcessSpecialsFile("planet_specials.txt",  true);
        }

        ~SpecialManager()
        {
            for (std::map<std::string, Special*>::iterator it = m_specials.begin(); it != m_specials.end(); ++it) {
                delete it->second;
            }
        }

        const std::set<std::string>& PlanetSpecialNames() const
        {
            return m_planet_special_names;
        }

        Special* GetSpecial(const std::string& name) const
        {
            std::map<std::string, Special*>::const_iterator it = m_specials.find(name);
            return it != m_specials.end() ? it->second : 0;
        }

    private:
        void ProcessSpecialsFile(const std::string& file_name, bool planet_specials)
        {
            std::string input;

            boost::filesystem::ifstream ifs(GetResourceDir() / file_name);
            if (ifs) {
                std::getline(ifs, input, '\0');
                ifs.close();
            } else {
                Logger().errorStream() << "Unable to open data file " << file_name;
                return;
            }

            using namespace boost::spirit;
            using namespace phoenix;

            parse_info<const char*> result =
                parse(input.c_str(),
                      as_lower_d[*special_p[store_special_(var(m_specials), var(m_planet_special_names), val(planet_specials), arg1)]]
                      >> end_p,
                      skip_p);
            if (!result.full)
                ReportError(input.c_str(), result);
        }
        std::map<std::string, Special*> m_specials;
        std::set<std::string> m_planet_special_names;
    };

    const SpecialManager& GetSpecialManager()
    {
        static SpecialManager special_manager;
        return special_manager;
    }

}

Special::Special(const std::string& name, const std::string& description,
                 const std::vector<boost::shared_ptr<const Effect::EffectsGroup> > effects,
                 const std::string& graphic) :
    m_name(name),
    m_description(description),
    m_effects(effects),
    m_graphic(graphic)
{}

const std::string& Special::Name() const
{
    return m_name;
}

const std::string& Special::Description() const
{
    return m_description;
}

std::string Special::Dump() const
{
    std::string retval = DumpIndent() + "Special\n";
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

const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& Special::Effects() const
{
    return m_effects;
}

const std::string& Special::Graphic() const
{
    return m_graphic;
}

Special* GetSpecial(const std::string& name)
{
    return GetSpecialManager().GetSpecial(name);
}

const std::set<std::string>& PlanetSpecialNames()
{
    return GetSpecialManager().PlanetSpecialNames();
}
