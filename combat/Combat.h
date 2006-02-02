// -*- C++ -*-
#ifndef _Combat_h_
#define _Combat_h_

#include <vector>
#include <string>

namespace GG {
  class TextControl;
  class StaticGraphic;
}
class XMLElement;

//! This struct is used to transmit combat turn update messages to players.
//! It's expected to change very frequently while coding goes toward v1.0
//! Generated in CombatSystem.cpp and used by CombatWnd.cpp
struct CombatUpdateMessage
{
  public:
    CombatUpdateMessage();
    ~CombatUpdateMessage();

    CombatUpdateMessage(const XMLElement&);
    XMLElement XMLEncode() const;
    
    //! Combat asserts info for one empire
    struct EmpireCombatInfo
    {
        EmpireCombatInfo();
        ~EmpireCombatInfo();
        EmpireCombatInfo(const XMLElement&);

        XMLElement XMLEncode() const;

        std::string empire;
        int         combat_ships;
        int         combat_ships_hitpoints;
        int         combat_ships_retreated;
        int         combat_ships_destroyed;

        int         non_combat_ships;
        int         non_combat_ships_hitpoints;
        int         non_combat_ships_retreated;
        int         non_combat_ships_destroyed;

        int         planets;
        int         planets_defence_bases;
        int         planets_lost;
        int         planets_defenseless;
    };

    std::string m_system; //! system were the combat take place
    std::vector<EmpireCombatInfo> m_opponents; //! empires which are involved into combat

};

inline std::string CombatRevision()
{return "$Id$";}

#endif // _Combat_h_
