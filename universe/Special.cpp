#include "Special.h"

#include "Effect.h"
#include "Condition.h"
#include "ValueRef.h"
#include "../parse/Parse.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "../util/Logger.h"
#include "../util/AppInterface.h"
#include "../universe/UniverseObject.h"

#include <boost/filesystem/fstream.hpp>

namespace {
    class SpecialManager {
    public:
        SpecialManager() {
            parse::specials(GetResourceDir() / "specials.txt", m_specials);
            if (GetOptionsDB().Get<bool>("verbose-logging")) {
                DebugLogger() << "Specials:";
                for (std::map<std::string, Special*>::iterator it = m_specials.begin();
                    it != m_specials.end(); ++it)
                { DebugLogger() << " ... " << it->first; }
            }
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
Special::~Special() {
    delete m_stealth;
    delete m_initial_capacity;
    delete m_location;
}

std::string Special::Dump() const {
    std::string retval = DumpIndent() + "Special\n";
    ++g_indent;
    retval += DumpIndent() + "name = \"" + m_name + "\"\n";
    retval += DumpIndent() + "description = \"" + m_description + "\"\n";

    if (m_stealth)
        retval += DumpIndent() + "stealth = " + m_stealth->Dump() + "\n";

    retval += DumpIndent() + "spawnrate = " + boost::lexical_cast<std::string>(m_spawn_rate) + "\n"
           +  DumpIndent() + "spawnlimit = " + boost::lexical_cast<std::string>(m_spawn_limit) + "\n";

    if (m_initial_capacity) {
        retval += DumpIndent() + "initialcapacity = ";
        ++g_indent;
            retval += m_initial_capacity->Dump();
        --g_indent;
    }

    if (m_location) {
        retval += DumpIndent() + "location =\n";
        ++g_indent;
            retval += m_location->Dump();
        --g_indent;
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
    retval += DumpIndent() + "graphic = \"" + m_graphic + "\"\n";
    --g_indent;
    return retval;
}

float Special::InitialCapacity(int object_id) const {
    if (!m_initial_capacity)
        return 0.0f;

    TemporaryPtr<const UniverseObject> obj = GetUniverseObject(object_id);
    if (!obj)
        return 0.0f;

    ScriptingContext context(obj);

    return m_initial_capacity->Eval(context);
}

const Special* GetSpecial(const std::string& name)
{ return GetSpecialManager().GetSpecial(name); }

std::vector<std::string> SpecialNames()
{ return GetSpecialManager().SpecialNames(); }
