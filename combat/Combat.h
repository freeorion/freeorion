// -*- C++ -*-
#ifndef _Combat_h_
#define _Combat_h_

#include <vector>
#include <string>

namespace GG {
  class TextControl;
  class StaticGraphic;

  class XMLElement;
}

struct CombatUpdateMessage
{
  public:
    CombatUpdateMessage();
    ~CombatUpdateMessage();

    CombatUpdateMessage(const GG::XMLElement&);
    GG::XMLElement XMLEncode() const;
    
    struct EmpireCombatInfo
    {
        EmpireCombatInfo();
        ~EmpireCombatInfo();
        EmpireCombatInfo(const GG::XMLElement&);

        GG::XMLElement XMLEncode() const;

        std::string empire;
        int         combat_ships;
        int         combat_ships_hitpoints;
        int         non_combat_ships;
        int         non_combat_ships_hitpoints;

        int         planets;
        int         planets_defence_bases;
        
        int         destroyed_ships_destroyed;
        int         retreated_ships;
        int         defenseless_planets;
    };

    std::string m_system;
    std::vector<EmpireCombatInfo> m_opponents;
};

#endif // _Combat_h_
