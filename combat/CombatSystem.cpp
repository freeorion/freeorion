#include "CombatSystem.h"
#include "../util/AppInterface.h"
#include "../universe/Predicates.h"

#include "../universe/Planet.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/System.h"
#include "../Empire/Empire.h"

#include <time.h>
#include <map>

#define DEBUG_COMBAT

#ifdef FREEORION_BUILD_SERVER
  #include "../server/ServerApp.h"
  Empire *LookupEmpire(int ID) {return ServerApp::Empires().Lookup(ID);}
#else
  #include "../client/ClientApp.h"
  Empire *LookupEmpire(int ID) {return ClientApp::Empires().Lookup(ID);}
#endif



void RemoveShip(int nID)
{
  Ship *shp = dynamic_cast<Ship*>(GetUniverse().Object(nID));
  if(shp!=NULL)
  {
    Fleet *flt = shp->GetFleet();
    flt->RemoveShip(shp->ID());
    if(flt->NumShips()==0)
    {
      for(std::set<int>::const_iterator own_it=flt->Owners().begin();own_it != flt->Owners().end();++own_it)
      {
        Empire *empire = LookupEmpire(*own_it);
        empire->RemoveFleet(flt->ID());
      }
      GetUniverse().Remove(flt->ID());
    }
    GetUniverse().Remove(shp->ID());
  }
}



bool CombatAssetsOwner::operator==(const CombatAssetsOwner &ca) const  
{
  return owner && ca.owner && (owner->EmpireID() == ca.owner->EmpireID());
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

#ifdef DEBUG_COMBAT
void Debugout(std::vector<CombatAssetsHitPoints> &empire_combat_forces)
{
  unsigned int hit_points;
  std::string debug; char buffer[20];

  debug = "\ncurrent forces\n";

  for(unsigned int i=0;i<empire_combat_forces.size();i++)
  {
    debug+= "  " + empire_combat_forces[i].owner->Name() + " "
           +" ships (cmb,non cmb,destr,retr): (";

    hit_points=0;
    for(unsigned int j=0;j<empire_combat_forces[i].combat_ships.size();j++)
      hit_points+=empire_combat_forces[i].combat_ships[j].second;

    //debug+= "#" + (std::string)itoa(empire_combat_forces[i].combat_ships.size(),buffer,10) 
      //     +":" + (std::string)itoa(hit_points,buffer,10);

    hit_points=0;
    for(unsigned int j=0;j<empire_combat_forces[i].non_combat_ships.size();j++)
      hit_points+=empire_combat_forces[i].non_combat_ships[j].second;

    //debug+= ",#" + (std::string)itoa(empire_combat_forces[i].non_combat_ships.size(),buffer,10)
     //      +":" + (std::string)itoa(hit_points,buffer,10);
    
    //debug+= ",#" + (std::string)itoa(empire_combat_forces[i].destroyed_ships.size(),buffer,10);
    //debug+= ",#" + (std::string)itoa(empire_combat_forces[i].retreated_ships.size(),buffer,10) + ")";

    hit_points=0;
    for(unsigned int j=0;j<empire_combat_forces[i].planets.size();j++)
      hit_points+=empire_combat_forces[i].planets[j].second;

    //debug+= "; planets : #" + (std::string)itoa(empire_combat_forces[i].planets.size(),buffer,10) 
      //     +":" + (std::string)itoa(hit_points,buffer,10);

    debug+="\n";
  }
  //ServerApp::GetApp()->Logger().debugStream() << debug;
}
#endif

void CombatSystem::ResolveCombat(const std::vector<CombatAssets> &assets)
{
#ifdef DEBUG_COMBAT
  //ServerApp::GetApp()->Logger().debugStream() << "COMBAT resolution!";
#endif
  const double base_chance_to_retreat = 0.25;
  const int    defence_base_hit_points= 3;

  srand((unsigned int)time(NULL));

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
        Ship *shp = dynamic_cast<Ship*>(GetUniverse().Object(*shp_it));
        
        if(shp->IsArmed())
          cahp.combat_ships    .push_back(std::pair<Ship*,unsigned int> (shp,shp->Design().defense));
        else   
          cahp.non_combat_ships.push_back(std::pair<Ship*,unsigned int> (shp,shp->Design().defense));
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

  while(combat_assets.size()>1)
  {
    // give all non combat shpis a base chance to retreat
    for(unsigned int e=0;e<combat_assets.size();e++)
      for(unsigned int i=0; i<empire_combat_forces[combat_assets[e]].non_combat_ships.size(); i++)
        if((rand()%100)<=(int)(base_chance_to_retreat*100.0))
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

        damage_done[e] += rand()%(shp->Design().attack+1);
      }

#ifdef DEBUG_COMBAT
      //ServerApp::GetApp()->Logger().debugStream() 
        //<< "Damage done by " << empire_combat_forces[e].owner->Name() << " " << damage_done[e];
#endif
    }

    // apply damage each empire combat force has done
    for(unsigned int e=0; e < damage_done.size(); e++)
      while(damage_done[e]>0) // any damage?
      {
        // all or all other forces destroyed!!
        if(   (combat_assets.size() == 0)
           ||((combat_assets.size() == 1) && (e == combat_assets[0])))
          break;

        // calc damage target assets at random, but don't shoot at yourself (+1)
        int target_combat_assets = combat_assets.size()==1?0:rand()%(combat_assets.size()-1);
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
    }

#ifdef DEBUG_COMBAT
    Debugout(empire_combat_forces);
#endif
  // evaluation
  // victor: the empire which is the sole survivor and still has armed forces
  int victor = combat_assets.size()==1 && empire_combat_forces[combat_assets[0]].CountArmedAssets()>0
              ?combat_assets[0]:-1;

  for(unsigned int e=0;e<empire_combat_forces.size();e++)
  {   
    //remove defense bases of defenseless planets
    for(unsigned int i=0; i<empire_combat_forces[e].defenseless_planets.size(); i++)
      empire_combat_forces[e].defenseless_planets[i]->AdjustDefBases(0);
    
    //adjust defense bases of planets
    for(unsigned int i=0; i<empire_combat_forces[e].planets.size(); i++)
    {
      unsigned int defense_bases_left = defence_base_hit_points==0
                                       ?0
                                       : ( empire_combat_forces[e].planets[i].first->DefBases()*defence_base_hit_points
                                          -empire_combat_forces[e].planets[i].second
                                          +defence_base_hit_points-1)
                                        / defence_base_hit_points;
      empire_combat_forces[e].planets[i].first->AdjustDefBases(defense_bases_left);
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
      const Fleet  *flt=empire_combat_forces[e].retreated_ships[0]->GetFleet();
      const System *sys=NULL; double range = 0.0;

      // retreat to nearest system
      for(Empire::ConstPlanetIDItr plt_it=empire_combat_forces[e].owner->PlanetBegin();plt_it != empire_combat_forces[e].owner->PlanetEnd();++plt_it)
      {
        const System *s=dynamic_cast<const Planet*>(GetUniverse().Object(*plt_it))->GetSystem();
        double r = sqrt((s->X()-flt->X())*(s->X()-flt->X()) + (s->Y()-flt->Y())*(s->Y()-flt->Y()));

        if(sys==s)
          continue;

        if(sys==NULL || r<range)
          {sys = s; range = r;}
      }

      if(sys==NULL) 
      {
        for(unsigned int i=0; i<empire_combat_forces[e].retreated_ships.size(); i++)
          empire_combat_forces[e].destroyed_ships.push_back(empire_combat_forces[e].retreated_ships[i]);
        empire_combat_forces[e].retreated_ships.clear();
      }
      else
        for(unsigned int i=0; i<empire_combat_forces[e].retreated_ships.size(); i++)
        {
          Fleet *flt = empire_combat_forces[e].retreated_ships[i]->GetFleet();
          System *current_system = dynamic_cast<System *>(GetUniverse().Object(flt->SystemID()));

          current_system->Remove(flt->ID());// set flt-SystemId() to INVALID_OBJECT_ID
          flt->MoveTo(current_system->X(),current_system->Y());
          flt->SetDestination(sys->ID());
        }
    }

    //remove destroyed ships
    for(unsigned int i=0; i<empire_combat_forces[e].destroyed_ships.size(); i++)
      RemoveShip(empire_combat_forces[e].destroyed_ships[i]->ID());

    // TODO: add some information to sitreport
  }
}

