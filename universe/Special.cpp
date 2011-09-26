#include "Special.h"

#include "ParserUtil.h"
#include "Effect.h"
#include "Condition.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"

#include <boost/filesystem/fstream.hpp>

std::string DumpIndent();

extern int g_indent;

namespace {
    struct store_special_impl {
        template <class T1, class T2>
        struct result {typedef void type;};
        template <class T>
        void operator()(std::map<std::string, Special*>& specials, const T& special) const {
            if (specials.find(special->Name()) != specials.end()) {
                std::string error_str = "ERROR: More than one special in specials.txt has the name " + special->Name();
                throw std::runtime_error(error_str.c_str());
            }
            specials[special->Name()] = special;
        }
    };
    const phoenix::function<store_special_impl> store_special_;

    class SpecialManager {
    public:
        SpecialManager() {
            std::string input, file_name("specials.txt");
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
                      as_lower_d[*special_p[store_special_(var(m_specials), arg1)]]
                      >> end_p,
                      skip_p);
            if (!result.full)
                ReportError(input.c_str(), result);

            //Logger().debugStream() << "Specials:";
            //for (std::map<std::string, Special*>::const_iterator it = m_specials.begin(); it != m_specials.end(); ++it)
            //    Logger().debugStream() << " ... " << it->second->Name() <<
            //                              " spawn rate: " << it->second->SpawnRate() <<
            //                              " spawn limit: " << it->second->SpawnLimit() <<
            //                              " location: " << (it->second->Location() ? it->second->Location()->Dump() : "none");
        }
        ~SpecialManager() {
            for (std::map<std::string, Special*>::iterator it = m_specials.begin();
                it != m_specials.end(); ++it)
            { delete it->second; }
        }
        std::vector<std::string> SpecialNames() const {
            std::vector<std::string> retval;
            for (std::map<std::string, Special*>::const_iterator it = m_specials.begin();
                it != m_specials.end(); ++it)
            { retval.push_back(it->first); }
            return retval;
        }
        const Special* GetSpecial(const std::string& name) const {
            std::map<std::string, Special*>::const_iterator it = m_specials.find(name);
            return it != m_specials.end() ? it->second : 0;
        }
    private:
        std::map<std::string, Special*> m_specials;
    };
    const SpecialManager& GetSpecialManager() {
        static SpecialManager special_manager;
        return special_manager;
    }
}

/////////////////////////////////////////////////
// Special                                     //
/////////////////////////////////////////////////
Special::Special(const std::string& name, const std::string& description,
                 const std::vector<boost::shared_ptr<const Effect::EffectsGroup> > effects,
                 double spawn_rate, int spawn_limit,
                 const Condition::ConditionBase* location,
                 const std::string& graphic) :
    m_name(name),
    m_description(description),
    m_effects(effects),
    m_spawn_rate(spawn_rate),
    m_spawn_limit(spawn_limit),
    m_location(location),
    m_graphic(graphic)
{}

Special::~Special()
{ delete m_location; }

const std::string& Special::Name() const
{ return m_name; }

const std::string& Special::Description() const
{ return m_description; }

std::string Special::Dump() const
{
    std::string retval = DumpIndent() + "Special\n";
    ++g_indent;
    retval += DumpIndent() + "name = \"" + m_name + "\"\n";
    retval += DumpIndent() + "description = \"" + m_description + "\"\n";
    retval += DumpIndent() + "spawnrate = " + boost::lexical_cast<std::string>(m_spawn_rate) + "\n"
           +  DumpIndent() + "spawnlimit = " + boost::lexical_cast<std::string>(m_spawn_limit) + "\n";
    retval += DumpIndent() + "location = \n";
    ++g_indent;
        retval += m_location->Dump();
    --g_indent;
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
{ return m_effects; }

double Special::SpawnRate() const
{ return m_spawn_rate; }

int Special::SpawnLimit() const
{ return m_spawn_limit; }

const Condition::ConditionBase* Special::Location() const
{ return m_location; }

const std::string& Special::Graphic() const
{ return m_graphic; }

const Special* GetSpecial(const std::string& name)
{ return GetSpecialManager().GetSpecial(name); }

std::vector<std::string> SpecialNames()
{ return GetSpecialManager().SpecialNames(); }
