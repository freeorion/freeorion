#include "Combat.h"

#include "XMLDoc.h"

////////////////////////////////////////////////
// CombatUpdateMessage
////////////////////////////////////////////////
// class CombatUpdateMessage::EmpireCombatInfo
CombatUpdateMessage::EmpireCombatInfo::EmpireCombatInfo()
: empire                   (""),combat_ships         (0),
  combat_ships_hitpoints    (0),non_combat_ships      (0),
  non_combat_ships_hitpoints(0),planets               (0),
  planets_defence_bases     (0),destroyed_ships_destroyed(0),
  retreated_ships           (0),defenseless_planets   (0)
{}

CombatUpdateMessage::EmpireCombatInfo::~EmpireCombatInfo()
{}

CombatUpdateMessage::EmpireCombatInfo::EmpireCombatInfo(const GG::XMLElement &e)
: empire                    (                         e.Child("empire"                    ).Text()),
  combat_ships              (boost::lexical_cast<int>(e.Child("combat_ships"              ).Text())),
  combat_ships_hitpoints    (boost::lexical_cast<int>(e.Child("combat_ships_hitpoints"    ).Text())),
  non_combat_ships          (boost::lexical_cast<int>(e.Child("non_combat_ships"          ).Text())),
  non_combat_ships_hitpoints(boost::lexical_cast<int>(e.Child("non_combat_ships_hitpoints").Text())),
  planets                   (boost::lexical_cast<int>(e.Child("planets"                   ).Text())),
  planets_defence_bases     (boost::lexical_cast<int>(e.Child("planets_defence_bases"     ).Text())),
  destroyed_ships_destroyed (boost::lexical_cast<int>(e.Child("destroyed_ships_destroyed" ).Text())),
  retreated_ships           (boost::lexical_cast<int>(e.Child("retreated_ships"           ).Text())),
  defenseless_planets       (boost::lexical_cast<int>(e.Child("defenseless_planets"       ).Text()))
{}

GG::XMLElement CombatUpdateMessage::EmpireCombatInfo::XMLEncode() const
{
  GG::XMLElement e("empire-combat-info");
  
  e.AppendChild(GG::XMLElement("empire", empire));
  e.AppendChild(GG::XMLElement("combat_ships"              , boost::lexical_cast<std::string>(combat_ships              )));
  e.AppendChild(GG::XMLElement("combat_ships_hitpoints"    , boost::lexical_cast<std::string>(combat_ships_hitpoints    )));
  e.AppendChild(GG::XMLElement("non_combat_ships"          , boost::lexical_cast<std::string>(non_combat_ships          )));
  e.AppendChild(GG::XMLElement("non_combat_ships_hitpoints", boost::lexical_cast<std::string>(non_combat_ships_hitpoints)));
  e.AppendChild(GG::XMLElement("planets"                   , boost::lexical_cast<std::string>(planets                   )));
  e.AppendChild(GG::XMLElement("planets_defence_bases"     , boost::lexical_cast<std::string>(planets_defence_bases     )));
  e.AppendChild(GG::XMLElement("destroyed_ships_destroyed" , boost::lexical_cast<std::string>(destroyed_ships_destroyed )));
  e.AppendChild(GG::XMLElement("retreated_ships"           , boost::lexical_cast<std::string>(retreated_ships           )));
  e.AppendChild(GG::XMLElement("defenseless_planets"       , boost::lexical_cast<std::string>(defenseless_planets       )));

  return e;
}

// class CombatUpdateMessage::EmpireCombatInfo
CombatUpdateMessage::CombatUpdateMessage()
: m_system    (""),
  m_opponents ()
{}

CombatUpdateMessage::~CombatUpdateMessage()
{}

CombatUpdateMessage::CombatUpdateMessage(const GG::XMLElement &e)
: m_system    (e.Child("system").Text()),
  m_opponents ()
{
  GG::XMLElement o(e.Child("opponents"));

  for(int i=0;i<o.NumChildren();i++)
    m_opponents.push_back(EmpireCombatInfo(o.Child(i)));
}

GG::XMLElement CombatUpdateMessage::XMLEncode() const
{
  GG::XMLElement e("combat-update-message"),o("opponents");
  
  e.AppendChild(GG::XMLElement("system", m_system));

  for(unsigned int i=0;i<m_opponents.size();i++)
    o.AppendChild(m_opponents[i].XMLEncode());

  e.AppendChild(o);
  return e;
}

