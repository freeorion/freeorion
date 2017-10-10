#include "Special.h"

#include "Condition.h"
#include "Effect.h"
#include "UniverseObject.h"
#include "ValueRef.h"
#include "../parse/Parse.h"
#include "../util/OptionsDB.h"
#include "../util/Logger.h"
#include "../util/AppInterface.h"
#include "../util/CheckSums.h"
#include "../util/ScopedTimer.h"

#include <boost/filesystem/fstream.hpp>

namespace {
    class SpecialManager {
    public:
        SpecialManager() {
            ScopedTimer timer("SpecialManager Init", true, std::chrono::milliseconds(1));

            try {
                m_specials = parse::specials();
            } catch (const std::exception& e) {
                ErrorLogger() << "Failed parsing specials: error: " << e.what();
                throw e;
            }

            TraceLogger() << "Specials:";
            for (const auto& entry : m_specials)
                TraceLogger() << " ... " << entry.first;
        }
        std::vector<std::string> SpecialNames() const {
            std::vector<std::string> retval;
            for (const auto& entry : m_specials) {
                retval.push_back(entry.first);
            }
            return retval;
        }
        const Special* GetSpecial(const std::string& name) const {
            auto it = m_specials.find(name);
            return it != m_specials.end() ? it->second.get() : nullptr;
        }
        unsigned int GetCheckSum() const {
            unsigned int retval{0};
            for (auto const& name_type_pair : m_specials)
                CheckSums::CheckSumCombine(retval, name_type_pair);
            CheckSums::CheckSumCombine(retval, m_specials.size());
            DebugLogger() << "SpecialManager checksum: " << retval;
            return retval;
        }
    private:
        std::map<std::string, std::unique_ptr<Special>> m_specials;
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

std::string Special::Description() const {
    std::stringstream result;

    result << UserString(m_description) << "\n";

    for (auto& effect : m_effects) {
        const std::string& description = effect->GetDescription();

        if (!description.empty()) {
            result << "\n" << UserString(description) << "\n";
        }
    }

    return result.str();
}

void Special::Init() {
    if (m_stealth)
        m_stealth->SetTopLevelContent(m_name);
    for (auto& effect : m_effects) {
        effect->SetTopLevelContent(m_name);
    }
    if (m_initial_capacity)
        m_initial_capacity->SetTopLevelContent(m_name);
    if (m_location)
        m_location->SetTopLevelContent(m_name);
}

std::string Special::Dump() const {
    std::string retval = DumpIndent() + "Special\n";
    ++g_indent;
    retval += DumpIndent() + "name = \"" + m_name + "\"\n";
    retval += DumpIndent() + "description = \"" + m_description + "\"\n";

    if (m_stealth)
        retval += DumpIndent() + "stealth = " + m_stealth->Dump() + "\n";

    retval += DumpIndent() + "spawnrate = " + std::to_string(m_spawn_rate) + "\n"
           +  DumpIndent() + "spawnlimit = " + std::to_string(m_spawn_limit) + "\n";

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
        for (auto& effect : m_effects) {
            retval += effect->Dump();
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

    auto obj = GetUniverseObject(object_id);
    if (!obj)
        return 0.0f;

    ScriptingContext context(obj);

    return m_initial_capacity->Eval(context);
}

unsigned int Special::GetCheckSum() const {
    unsigned int retval{0};

    CheckSums::CheckSumCombine(retval, m_name);
    CheckSums::CheckSumCombine(retval, m_description);
    CheckSums::CheckSumCombine(retval, m_stealth);
    CheckSums::CheckSumCombine(retval, m_effects);
    CheckSums::CheckSumCombine(retval, m_spawn_rate);
    CheckSums::CheckSumCombine(retval, m_spawn_limit);
    CheckSums::CheckSumCombine(retval, m_initial_capacity);
    CheckSums::CheckSumCombine(retval, m_location);
    CheckSums::CheckSumCombine(retval, m_graphic);

    return retval;
}

const Special* GetSpecial(const std::string& name)
{ return GetSpecialManager().GetSpecial(name); }

std::vector<std::string> SpecialNames()
{ return GetSpecialManager().SpecialNames(); }
