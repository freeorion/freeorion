#include "Special.h"

#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"

#include <fstream>


namespace {
    class SpecialManager
    {
    public:
        SpecialManager()
        {
            std::string settings_dir = GetOptionsDB().Get<std::string>("settings-dir");
            if (!settings_dir.empty() && settings_dir[settings_dir.size() - 1] != '/')
                settings_dir += '/';
            std::ifstream ifs((settings_dir + "specials.xml").c_str());
            GG::XMLDoc doc;
            doc.ReadDoc(ifs);
            for (GG::XMLElement::const_child_iterator it = doc.root_node.child_begin(); it != doc.root_node.child_end(); ++it) {
                if (it->Tag() != "Special")
                    throw std::runtime_error("ERROR: Encountered non-Special in specials.xml!");
                Special* special = 0;
                try {
                    special = new Special(*it);
                } catch (const std::runtime_error& e) {
                    std::stringstream stream;
                    it->WriteElement(stream);
                    throw std::runtime_error(std::string("ERROR: \"") + e.what() + "\" encountered when loading this Special XML code:\n" + stream.str());
                }
                if (m_specials.find(special->Name()) != m_specials.end())
                    throw std::runtime_error(("ERROR: More than one special in specials.xml has the name " + special->Name()).c_str());
                m_specials[special->Name()] = special;
            }
            ifs.close();
        }

        Special* GetSpecial(const std::string& name) const
        {
            std::map<std::string, Special*>::const_iterator it = m_specials.find(name);
            return it != m_specials.end() ? it->second : 0;
        }

    private:
        std::map<std::string, Special*> m_specials;
    };

    bool temp_header_bool = RecordHeaderFile(SpecialRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}

Special::Special(const std::string& name, const std::string& description) :
    m_name(name),
    m_description(description),
    m_effects()
{
}

Special::Special(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Special")
        throw std::invalid_argument("Attempted to construct a Special from an XMLElement that had a tag other than \"Special\"");

    m_name = elem.Child("name").Text();
    m_description = elem.Child("description").Text();
    for (GG::XMLElement::const_child_iterator it = elem.Child("effects").child_begin(); it != elem.Child("effects").child_end(); ++it) {
        m_effects.push_back(boost::shared_ptr<Effect::EffectsGroup>(new Effect::EffectsGroup(*it)));
    }
}

const std::string& Special::Name() const
{
    return m_name;
}

const std::string& Special::Description() const
{
    return m_description;
}

const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& Special::Effects() const
{
    return m_effects;
}

Special* GetSpecial(const std::string& name)
{
    static SpecialManager manager;
    return manager.GetSpecial(name);
}
