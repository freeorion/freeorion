#include "CombatSystem.h"

#include "../util/AppInterface.h"
#include "../universe/Predicates.h"

#include "../universe/Planet.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../Empire/Empire.h"

#include "../util/Random.h"
#include "../util/AppInterface.h"

#include "../server/ServerApp.h"
#include "../network/Message.h"

#include "Combat.h"

#include <time.h>
#include <map>

//#define DEBUG_COMBAT

bool CombatAssetsOwner::operator==(const CombatAssetsOwner &ca) const  
{
  return owner && ca.owner && (owner->EmpireID() == ca.owner->EmpireID());
}

namespace
{
  void SendMessageToAllPlayer(Message::MessageType msg_type,Message::ModuleType module, const GG::XMLDoc& doc)
  {
    for (std::map<int, PlayerInfo>::const_iterator it = ServerApp::GetApp()->NetworkCore().Players().begin(); it != ServerApp::GetApp()->NetworkCore().Players().end(); ++it)
      ServerApp::GetApp()->NetworkCore().SendMessage(Message(msg_type,-1,it->first,module,doc));
  }

  struct CombatAssetsHitPoints
  {
    Empire* owner;

    int CountArmedAssets() {return combat_ships.size();}
    int CountAssets() {return combat_ships.size()+non_combat_ships.size()+planets.size();}

    std::vector<std::pair<Ship  *,unsigned int> > combat_ships;
    std::vector<std::pair<Ship  *,unsigned int> > non_combat_ships;
    std::vector<std::pair<Planet*,unsigned int> > planets;

    std::vector<Ship  *> destroyed_ships;
    std::vector<Ship  *> retreated_ships;
    std::vector<Planet*> defenseless_planets;
  };
}

static void RemoveShip(int nID)
{
  Ship *shp = GetUniverse().Object<Ship>(nID);
  if(shp!=NULL)
  {
    Fleet *flt = shp->GetFleet();
    flt->RemoveShip(shp->ID());
    if(flt->NumShips()==0)
    {
      GetUniverse().Remove(flt->ID());
    }
    GetUniverse().Remove(shp->ID());
  }
}

#ifdef DEBUG_COMBAT
static void Debugout(std::vector<CombatAssetsHitPoints> &empire_combat_forces)
{
  unsigned int hit_points;
  std::string debug;

  debug = "\ncurrent forces\n";

  for(unsigned int i=0;i<empire_combat_forces.size();i++)
  {
    debug+= "  " + empire_combat_forces[i].owner->Name() + " "
          +" ships (cmb,non cmb,destr,retr): (";

    hit_points=0;
    for(unsigned int j=0;j<empire_combat_forces[i].combat_ships.size();j++)
      hit_points+=empire_combat_forces[i].combat_ships[j].second;

    debug+= "#" + boost::lexical_cast<std::string,int>(empire_combat_forces[i].combat_ships.size()) 
          +":" + boost::lexical_cast<std::string,int>(hit_points);

    hit_points=0;
    for(unsigned int j=0;j<empire_combat_forces[i].non_combat_ships.size();j++)
      hit_points+=empire_combat_forces[i].non_combat_ships[j].second;

    debug+= ",#" + boost::lexical_cast<std::string,int>(empire_combat_forces[i].non_combat_ships.size())
          +":" + boost::lexical_cast<std::string,int>(hit_points);
    
    debug+= ",#" + boost::lexical_cast<std::string,int>(empire_combat_forces[i].destroyed_ships.size());
    debug+= ",#" + boost::lexical_cast<std::string,int>(empire_combat_forces[i].retreated_ships.size()) + ")";

    hit_points=0;
    for(unsigned int j=0;j<empire_combat_forces[i].planets.size();j++)
      hit_points+=empire_combat_forces[i].planets[j].second;

    debug+= "; planets : #" + boost::lexical_cast<std::string,int>(empire_combat_forces[i].planets.size()) 
          +":" + boost::lexical_cast<std::string,int>(hit_points);

    debug+="\n";
  }
  log4cpp::Category::getRoot().debugStream() << debug;
}
#endif

CombatUpdateMessage GenerateCombatUpdateMessage(int system_id,const std::vector<CombatAssetsHitPoints> &empire_combat_forces)
{
  CombatUpdateMessage cmb_upd_msg;
  
  cmb_upd_msg.m_system = GetUniverse().Object(system_id)->Name();
  for(unsigned int e=0;e<empire_combat_forces.size();e++)
  {
    CombatUpdateMessage::EmpireCombatInfo eci;
    eci.empire                    = empire_combat_forces[e].owner->Name();
    eci.combat_ships              = empire_combat_forces[e].combat_ships.size();
    eci.combat_ships_hitpoints    = 0;
    for(unsigned int i = 0;i<empire_combat_forces[e].combat_ships.size();i++)
      eci.combat_ships_hitpoints+=empire_combat_forces[e].combat_ships[i].second;

    eci.non_combat_ships          = empire_combat_forces[e].non_combat_ships.size();
    eci.non_combat_ships_hitpoints= 0;
    for(unsigned int i = 0;i<empire_combat_forces[e].non_combat_ships.size();i++)
      eci.non_combat_ships_hitpoints+=empire_combat_forces[e].non_combat_ships[i].second;

    eci.planets                   = empire_combat_forces[e].planets.size();
    eci.planets_defence_bases     = 0;
    for(unsigned int i = 0;i<empire_combat_forces[e].planets.size();i++)
      eci.planets_defence_bases+=empire_combat_forces[e].planets[i].second;

    eci.combat_ships_destroyed = 0;
    eci.non_combat_ships_destroyed = 0;

    for(unsigned int i = 0;i<empire_combat_forces[e].destroyed_ships.size();i++)
      if(empire_combat_forces[e].destroyed_ships[i]->Design()->attack>0)
        eci.combat_ships_destroyed++;
      else
        eci.non_combat_ships_destroyed++;

    for(unsigned int i = 0;i<empire_combat_forces[e].retreated_ships.size();i++)
      if(empire_combat_forces[e].retreated_ships[i]->Design()->attack>0)
        eci.combat_ships_retreated++;
      else
        eci.non_combat_ships_retreated++;

    eci.planets_lost       = empire_combat_forces[e].defenseless_planets.size();
    eci.planets_defenseless= 0;

    cmb_upd_msg.m_opponents.push_back(eci);
  }
  return cmb_upd_msg;
}

void CombatSystem::ResolveCombat(const int system_id,const std::vector<CombatAssets> &assets)
{
#ifdef DEBUG_COMBAT
  log4cpp::Category::getRoot().debugStream() << "COMBAT resolution!";
#endif
  const double base_chance_to_retreat = 0.25;
  const int    defence_base_hit_points= 3;


  ClockSeed();
  SmallIntDistType small_int_dist = SmallIntDist(0,10000);

  std::vector<CombatAssetsHitPoints> empire_combat_forces;

  // index to empire_combat_forces
  std::vector<int> combat_assets;
    
  for(unsigned int e=0; e < assets.size(); e++)
  {
    CombatAssetsHitPoints cahp;
    cahp.owner           = assets[e].owner;

    for(unsigned int i=0; i<assets[e].fleets.size(); i++)
    {
      Fleet *flt = assets[e].fleets[i];
      for(Fleet::iterator shp_it = flt->begin(); shp_it != flt->end(); ++shp_it)
      {
        Ship *shp = GetUniverse().Object<Ship>(*shp_it);
        
        if(shp->IsArmed())
          cahp.combat_ships    .push_back(std::pair<Ship*,unsigned int> (shp,shp->Design()->defense));
        else   
          cahp.non_combat_ships.push_back(std::pair<Ship*,unsigned int> (shp,shp->Design()->defense));
      }
    }
    for(unsigned int i=0; i<assets[e].planets.size(); i++)
    {
      Planet *plt = assets[e].planets[i];

      if(plt->DefBases()>0)
        // static defense bonus of >defence_base_hit_points< points per defense base
        cahp.planets.push_back(std::pair<Planet*,unsigned int>(plt,plt->DefBases() * defence_base_hit_points));
      else
        cahp.defenseless_planets.push_back(plt);
    }

    empire_combat_forces.push_back(cahp);
    if(cahp.CountAssets()>0)
      combat_assets.push_back(empire_combat_forces.size()-1);
  }

  // if only non combat forces meet, no battle take place
  unsigned int e;
  for(e=0;e<combat_assets.size();e++)
    if(empire_combat_forces[combat_assets[e]].combat_ships.size()>0)
      break;
  if(e>=combat_assets.size())
    return;

  GG::XMLDoc msg;
  msg.root_node.AppendChild(GenerateCombatUpdateMessage(system_id,empire_combat_forces).XMLEncode());
  SendMessageToAllPlayer(Message::COMBAT_START,Message::CORE,msg);

  while(combat_assets.size()>1)
  {
    // give all non combat shpis a base chance to retreat
    for(unsigned int e=0;e<combat_assets.size();e++)
      for(unsigned int i=0; i<empire_combat_forces[combat_assets[e]].non_combat_ships.size(); i++)
        if((small_int_dist()%100)<=(int)(base_chance_to_retreat*100.0))
        {
          empire_combat_forces[combat_assets[e]].retreated_ships .push_back(empire_combat_forces[combat_assets[e]].non_combat_ships[i].first);
          empire_combat_forces[combat_assets[e]].non_combat_ships.erase    (empire_combat_forces[combat_assets[e]].non_combat_ships.begin()+i);
          i--;
        }

    // are there any armed combat forces left?
    int count_armed_combat_forces = combat_assets.size();
    for(unsigned int i=0;i<combat_assets.size();i++)
      if(empire_combat_forces[combat_assets[i]].CountArmedAssets()==0)
        count_armed_combat_forces--;

    if(count_armed_combat_forces==0)
      break;

#ifdef DEBUG_COMBAT
    Debugout(empire_combat_forces);
#endif
    std::vector<unsigned int> damage_done;
    damage_done.resize(empire_combat_forces.size());

    // calc damage each empire combat force causes
    for(unsigned int e=0; e < empire_combat_forces.size(); e++)
    {
      for(unsigned int i=0; i<empire_combat_forces[e].combat_ships.size(); i++)
      {
        Ship *shp = empire_combat_forces[e].combat_ships[i].first;

        damage_done[e] += small_int_dist()%(shp->Design()->attack+1);
      }

#ifdef DEBUG_COMBAT
      log4cpp::Category::getRoot().debugStream() 
        << "Damage done by " << empire_combat_forces[e].owner->Name() << " " << damage_done[e];
#endif
    }

    // apply damage each empire combat force has done
    for(int e=0; e < static_cast<int>(damage_done.size()); e++)
      while(damage_done[e]>0) // any damage?
      {
        // all or all other forces destroyed!!
        if(   (combat_assets.size() == 0)
           ||((combat_assets.size() == 1) && (e == combat_assets[0])))
          break;

        // calc damage target assets at random, but don't shoot at yourself (+1)
        int target_combat_assets = combat_assets.size()==1?0:small_int_dist()%(combat_assets.size()-1);
        if(combat_assets[target_combat_assets] == e)
          target_combat_assets++;

        int target_empire_index = combat_assets[target_combat_assets];

        // first  - damage to planet defence bases
        for(unsigned int i=0; damage_done[e]>0 && i<empire_combat_forces[target_empire_index].planets.size(); i++)
        {
          unsigned int defence_points_left = empire_combat_forces[target_empire_index].planets[i].second;

          if(damage_done[e]>=defence_points_left)
          {
            empire_combat_forces[target_empire_index].defenseless_planets.push_back(empire_combat_forces[target_empire_index].planets[i].first);
            empire_combat_forces[target_empire_index].planets.erase(empire_combat_forces[target_empire_index].planets.begin()+i);
            damage_done[e]-=defence_points_left;
          }
          else
          {
            empire_combat_forces[target_empire_index].planets[i].second -= damage_done[e];
            damage_done[e] = 0;
          }
        }

        // second - damage to armed ships
        for(unsigned int i=0; damage_done[e]>0 && i<empire_combat_forces[target_empire_index].combat_ships.size(); i++)
        {
          unsigned int defence_points_left = empire_combat_forces[target_empire_index].combat_ships[i].second;

          if(damage_done[e]>=defence_points_left)
          {
            empire_combat_forces[target_empire_index].destroyed_ships.push_back(empire_combat_forces[target_empire_index].combat_ships[i].first);
            empire_combat_forces[target_empire_index].combat_ships.erase(empire_combat_forces[target_empire_index].combat_ships.begin()+i);
            damage_done[e]-=defence_points_left;
            i--;
          }
          else
          {
            empire_combat_forces[target_empire_index].combat_ships[i].second -= damage_done[e];
            damage_done[e] = 0;
          }
        }

        // third  - damage to unarmed ships
        for(unsigned int i=0; damage_done[e]>0 && i<empire_combat_forces[target_empire_index].non_combat_ships.size(); i++)
        {
          unsigned int defence_points_left = empire_combat_forces[target_empire_index].non_combat_ships[i].second;

          if(damage_done[e]>=defence_points_left)
          {
            empire_combat_forces[target_empire_index].destroyed_ships.push_back(empire_combat_forces[target_empire_index].non_combat_ships[i].first);
            empire_combat_forces[target_empire_index].non_combat_ships.erase(empire_combat_forces[target_empire_index].non_combat_ships.begin()+i);
            damage_done[e]-=defence_points_left;
          }
          else
          {
            empire_combat_forces[target_empire_index].non_combat_ships[i].second -= damage_done[e];
            damage_done[e] = 0;
          }
        }
        for(unsigned int i=0; i < combat_assets.size(); i++)
          if(empire_combat_forces[combat_assets[i]].CountAssets() == 0)
          {
            combat_assets.erase(combat_assets.begin()+ i);
            i--;
          }
      }
      msg = GG::XMLDoc();
      msg.root_node.AppendChild(GenerateCombatUpdateMessage(system_id,empire_combat_forces).XMLEncode());
      SendMessageToAllPlayer(Message::COMBAT_ROUND_UPDATE,Message::CORE,msg);
      //SDL_Delay(1000); maybe we add an delay to illustrate players a ongoing combat
    }

#ifdef DEBUG_COMBAT
    Debugout(empire_combat_forces);
#endif
  // evaluation
  // victor: the empire which is the sole survivor and still has armed forces
  int victor = combat_assets.size()==1 && empire_combat_forces[combat_assets[0]].CountArmedAssets()>0
              ?combat_assets[0]:-1;

  for(int e=0;e<static_cast<int>(empire_combat_forces.size());e++)
  {   
    //remove defense bases of defenseless planets
    for(unsigned int i=0; i<empire_combat_forces[e].defenseless_planets.size(); i++)
      empire_combat_forces[e].defenseless_planets[i]->AdjustDefBases(-empire_combat_forces[e].defenseless_planets[i]->DefBases());
    
    //adjust defense bases of planets
    for(unsigned int i=0; i<empire_combat_forces[e].planets.size(); i++)
    {
      unsigned int defense_bases_left = defence_base_hit_points==0
                                       ?0
                                       : ( empire_combat_forces[e].planets[i].first->DefBases()*defence_base_hit_points
                                          -empire_combat_forces[e].planets[i].second
                                          +defence_base_hit_points-1)
                                        / defence_base_hit_points;
      empire_combat_forces[e].planets[i].first->AdjustDefBases(defense_bases_left-empire_combat_forces[e].planets[i].first->DefBases());
    }

    // conquer planets if there is a victor
    if(victor!=-1 && victor!=e)
    {
      for(unsigned int i=0; i<empire_combat_forces[e].defenseless_planets.size(); i++)
        empire_combat_forces[e].defenseless_planets[i]->Conquer(empire_combat_forces[victor].owner->EmpireID());

      // shouldn't occur - only if defense bases are ignored
      for(unsigned int i=0; i<empire_combat_forces[e].planets.size(); i++)
        empire_combat_forces[e].planets[i].first->Conquer(empire_combat_forces[victor].owner->EmpireID());
    }
    // set target system of retreating ships (only if a victor exists)
    // if there is not target system all retreating ships are destroyed
    if(victor!=-1 && victor!=e && empire_combat_forces[e].retreated_ships.size()>0)
    {
      const System *sys=NULL;

      // retreat to nearest non-hostile system
      std::map<double, System*> neighbors = GetUniverse().ImmediateNeighbors(system_id);
      for (std::map<double, System*>::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
          if (it->second->Owners().empty() || it->second->Owners().find(empire_combat_forces[e].owner->EmpireID()) != it->second->Owners().end()) {
              sys = it->second;
              break;
          }
      }
      if(sys==NULL) 
      {
        for(unsigned int i=0; i<empire_combat_forces[e].retreated_ships.size(); i++)
          empire_combat_forces[e].destroyed_ships.push_back(empire_combat_forces[e].retreated_ships[i]);
        empire_combat_forces[e].retreated_ships.clear();
      }
      else
      {
        std::set<Fleet*> flt_set;
        for(unsigned int i=0; i<empire_combat_forces[e].retreated_ships.size(); i++)
          flt_set.insert(empire_combat_forces[e].retreated_ships[i]->GetFleet());

        for(std::set<Fleet*>::iterator it = flt_set.begin(); it != flt_set.end(); ++it)
        {
          std::pair<std::list<System*>, double> route = GetUniverse().ShortestPath(system_id, sys->ID());
          (*it)->SetRoute(route.first, route.second);
          (*it)->GetSystem()->Remove((*it)->ID());
        }
      }
    }

    //remove destroyed ships
    for(unsigned int i=0; i<empire_combat_forces[e].destroyed_ships.size(); i++)
      RemoveShip(empire_combat_forces[e].destroyed_ships[i]->ID());

    // add some information to sitreport
    empire_combat_forces[e].owner->AddSitRepEntry(CreateCombatSitRep(empire_combat_forces[e].owner->EmpireID(),victor==-1?-1:empire_combat_forces[victor].owner->EmpireID(),system_id));
  }

  msg = GG::XMLDoc();
  msg.root_node.AppendChild(GenerateCombatUpdateMessage(system_id,empire_combat_forces).XMLEncode());
  SendMessageToAllPlayer(Message::COMBAT_END,Message::CORE,msg);
}

