#include "Special.h"

#include "../util/OptionsDB.h"


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
                m_specials[it->Child("name").Text()] = new Special(*it);
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

}

Special::Special(const std::string& name, const std::string& description, Effect::EffectsGroup* effects) :
    m_name(name),
    m_description(description),
    m_effects(effects)
{
}

Special::Special(const GG::XMLElement& elem)
{
    if (elem.Tag() != "Special")
        throw std::invalid_argument("Attempted to construct a Special from an XMLElement that had a tag other than \"Special\"");

    m_name = elem.Child("name").Text();
    m_description = elem.Child("description").Text();
    m_effects = new Effect::EffectsGroup(elem.Child("EffectsGroup"));
}

Special::~Special()
{
    delete m_effects;
}

const std::string& Special::Name() const
{
    return m_name;
}

const std::string& Special::Description() const
{
    return m_description;
}

const Effect::EffectsGroup* Special::Effects() const
{
    return m_effects;
}

void Special::Execute(int host_id) const
{
    m_effects->Execute(host_id);
}

Special* GetSpecial(const std::string& name)
{
    static SpecialManager manager;
    return manager.GetSpecial(name);
}
