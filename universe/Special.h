// -*- C++ -*-
#ifndef _Special_h_
#define _Special_h_

#include "Effect.h"

class Special
{
public:
    Special(const std::string& name, const std::string& description, Effect::EffectsGroup* effects);
    Special(const GG::XMLElement& elem);
    ~Special();

    const std::string&          Name() const;
    const std::string&          Description() const;
    const Effect::EffectsGroup* Effects() const;
    void                        Execute(int host_d) const;

private:
    std::string           m_name;
    std::string           m_description;
    Effect::EffectsGroup* m_effects;
};

class SpecialManager
{
public:
    SpecialManager();

    Special* GetSpecial(const std::string& name) const;

private:
    std::map<std::string, Special*> m_specials;
};

Special* GetSpecial(const std::string& name);

#endif // _Special_h_
