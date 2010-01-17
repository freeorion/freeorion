#include "CombatSystem.h"

#include "../universe/Universe.h"
#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
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


////////////////////////////////////////////////
// CombatInfo
////////////////////////////////////////////////
CombatInfo::CombatInfo() :
    system_id(UniverseObject::INVALID_OBJECT_ID)
{}

CombatInfo::~CombatInfo() {
    objects.Clear();
    for (std::map<int, ObjectMap>::iterator it = empire_known_objects.begin(); it != empire_known_objects.end(); ++it)
        it->second.Clear();
    empire_known_objects.clear();
}



bool CombatAssetsOwner::operator==(const CombatAssetsOwner &ca) const
{
    return owner && ca.owner && (owner->EmpireID() == ca.owner->EmpireID());
}

namespace
{
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
    Ship* shp = GetObject<Ship>(nID);
    if (shp) {
        System* sys = GetObject<System>(shp->SystemID());
        if (sys)
            sys->Remove(shp->ID());

        Fleet* flt = GetObject<Fleet>(shp->FleetID());
        if (flt) {
            flt->RemoveShip(shp->ID());
            if (flt->Empty()) {
                if (sys = GetObject<System>(flt->SystemID()))
                    sys->Remove(flt->ID());
                GetUniverse().Destroy(flt->ID());
            }
        }
        GetUniverse().Destroy(shp->ID());
    }
}

#ifdef DEBUG_COMBAT
static void Debugout(std::vector<CombatAssetsHitPoints> &empire_combat_forces)
{
    unsigned int hit_points;
    std::string debug;

    debug = "\ncurrent forces\n";

    for (unsigned int i = 0; i < empire_combat_forces.size(); i++) {
        debug+= "  " + empire_combat_forces[i].owner->Name() + " "
            +" ships (cmb,non cmb,destr,retr): (";

        hit_points = 0;
        for(unsigned int j = 0; j < empire_combat_forces[i].combat_ships.size(); j++)
            hit_points+=empire_combat_forces[i].combat_ships[j].second;

        debug += "#" + boost::lexical_cast<std::string,int>(empire_combat_forces[i].combat_ships.size()) 
              + ":" + boost::lexical_cast<std::string,int>(hit_points);

        hit_points = 0;
        for (unsigned int j = 0; j < empire_combat_forces[i].non_combat_ships.size(); j++)
            hit_points += empire_combat_forces[i].non_combat_ships[j].second;

        debug += ",#" + boost::lexical_cast<std::string,int>(empire_combat_forces[i].non_combat_ships.size())
              +":" + boost::lexical_cast<std::string,int>(hit_points);

        debug += ",#" + boost::lexical_cast<std::string,int>(empire_combat_forces[i].destroyed_ships.size());
        debug += ",#" + boost::lexical_cast<std::string,int>(empire_combat_forces[i].retreated_ships.size()) + ")";

        hit_points = 0;
        for(unsigned int j = 0; j < empire_combat_forces[i].planets.size(); j++)
            hit_points += empire_combat_forces[i].planets[j].second;

        debug += "; planets : #" + boost::lexical_cast<std::string,int>(empire_combat_forces[i].planets.size()) 
              +":" + boost::lexical_cast<std::string,int>(hit_points);

        debug += "\n";
    }

    Logger().debugStream() << debug;
}
#endif

void CombatSystem::ResolveCombat(int system_id,const std::vector<CombatAssets> &assets)
{
#ifdef DEBUG_COMBAT
    log4cpp::Category::getRoot().debugStream() << "COMBAT resolution!";
#endif
    const double base_chance_to_retreat = 0.25;

#ifdef FREEORION_RELEASE
    ClockSeed();
#endif
    SmallIntDistType small_int_dist = SmallIntDist(0,10000);

    std::vector<CombatAssetsHitPoints> empire_combat_forces;

    // index to empire_combat_forces
    std::vector<int> combat_assets;

    for (unsigned int e = 0; e < assets.size(); e++) {
        CombatAssetsHitPoints cahp;
        cahp.owner = assets[e].owner;

        for (unsigned int i = 0; i < assets[e].fleets.size(); i++) {
            Fleet *flt = assets[e].fleets[i];
            for (Fleet::iterator shp_it = flt->begin(); shp_it != flt->end(); ++shp_it) {
                Ship* shp = GetObject<Ship>(*shp_it);

                if (shp->IsArmed())
                    cahp.combat_ships.push_back(std::make_pair(shp, static_cast<unsigned int>(shp->Design()->Defense())));
                else
                    cahp.non_combat_ships.push_back(std::make_pair(shp, static_cast<unsigned int>(shp->Design()->Defense())));
            }
        }
        for (unsigned int i = 0; i < assets[e].planets.size(); i++) {
            Planet* plt = assets[e].planets[i];
            cahp.defenseless_planets.push_back(plt);
        }

        empire_combat_forces.push_back(cahp);
        if (cahp.CountAssets() > 0)
            combat_assets.push_back(empire_combat_forces.size() - 1);
    }

    // if only non combat forces meet, no battle take place
    unsigned int e;
    for (e = 0; e < combat_assets.size(); e++)
        if (empire_combat_forces[combat_assets[e]].combat_ships.size() > 0)
            break;
    if (e >= combat_assets.size())
        return;

    while (combat_assets.size() > 1) {
        // give all non combat shpis a base chance to retreat
        for (unsigned int e = 0; e < combat_assets.size(); e++)
            for (unsigned int i = 0; i < empire_combat_forces[combat_assets[e]].non_combat_ships.size(); i++)
                if ((small_int_dist() % 100) <= (int)(base_chance_to_retreat*100.0)) {
                    empire_combat_forces[combat_assets[e]].retreated_ships .push_back(empire_combat_forces[combat_assets[e]].non_combat_ships[i].first);
                    empire_combat_forces[combat_assets[e]].non_combat_ships.erase    (empire_combat_forces[combat_assets[e]].non_combat_ships.begin()+i);
                    i--;
                }

        // are there any armed combat forces left?
        int count_armed_combat_forces = combat_assets.size();
        for (unsigned int i=0;i<combat_assets.size();i++)
            if (empire_combat_forces[combat_assets[i]].CountArmedAssets() == 0)
                count_armed_combat_forces--;

        if (count_armed_combat_forces == 0)
            break;

#ifdef DEBUG_COMBAT
        Debugout(empire_combat_forces);
#endif
        std::vector<unsigned int> damage_done;
        damage_done.resize(empire_combat_forces.size());

        // calc damage each empire combat force causes
        for (unsigned int e = 0; e < empire_combat_forces.size(); e++) {
            for (unsigned int i = 0; i < empire_combat_forces[e].combat_ships.size(); i++) {
                Ship* shp = empire_combat_forces[e].combat_ships[i].first;

                damage_done[e] += small_int_dist() % (static_cast<int>(shp->Design()->Attack()) + 1);
            }

#ifdef DEBUG_COMBAT
            log4cpp::Category::getRoot().debugStream() 
                << "Damage done by " << empire_combat_forces[e].owner->Name() << " " << damage_done[e];
#endif
        }

        // apply damage each empire combat force has done
        for (int e = 0; e < static_cast<int>(damage_done.size()); e++)
            while (damage_done[e] > 0) { // any damage?
                // all or all other forces destroyed!!
                if ((combat_assets.size() == 0) ||((combat_assets.size() == 1) && (e == combat_assets[0])))
                    break;

                // calc damage target assets at random, but don't shoot at yourself (+1)
                int target_combat_assets = combat_assets.size()==1?0:small_int_dist()%(combat_assets.size()-1);
                if(combat_assets[target_combat_assets] == e)
                    target_combat_assets++;

                int target_empire_index = combat_assets[target_combat_assets];

                // first  - damage to planet defence bases
                for (unsigned int i = 0; damage_done[e] > 0 && i<empire_combat_forces[target_empire_index].planets.size(); i++) {
                    unsigned int defence_points_left = empire_combat_forces[target_empire_index].planets[i].second;

                    if (damage_done[e]>=defence_points_left) {
                        empire_combat_forces[target_empire_index].defenseless_planets.push_back(empire_combat_forces[target_empire_index].planets[i].first);
                        empire_combat_forces[target_empire_index].planets.erase(empire_combat_forces[target_empire_index].planets.begin()+i);
                        damage_done[e]-=defence_points_left;
                    } else {
                        empire_combat_forces[target_empire_index].planets[i].second -= damage_done[e];
                        damage_done[e] = 0;
                    }
                }

                // second - damage to armed ships
                for (unsigned int i = 0; damage_done[e] > 0 && i < empire_combat_forces[target_empire_index].combat_ships.size(); i++) {
                    unsigned int defence_points_left = empire_combat_forces[target_empire_index].combat_ships[i].second;

                    if(damage_done[e]>=defence_points_left) {
                        empire_combat_forces[target_empire_index].destroyed_ships.push_back(empire_combat_forces[target_empire_index].combat_ships[i].first);
                        empire_combat_forces[target_empire_index].combat_ships.erase(empire_combat_forces[target_empire_index].combat_ships.begin()+i);
                        damage_done[e]-=defence_points_left;
                        i--;
                    } else {
                        empire_combat_forces[target_empire_index].combat_ships[i].second -= damage_done[e];
                        damage_done[e] = 0;
                    }
                }

                // third  - damage to unarmed ships
                for (unsigned int i = 0; damage_done[e] > 0 && i < empire_combat_forces[target_empire_index].non_combat_ships.size(); i++) {
                    unsigned int defence_points_left = empire_combat_forces[target_empire_index].non_combat_ships[i].second;

                    if (damage_done[e]>=defence_points_left) {
                        empire_combat_forces[target_empire_index].destroyed_ships.push_back(empire_combat_forces[target_empire_index].non_combat_ships[i].first);
                        empire_combat_forces[target_empire_index].non_combat_ships.erase(empire_combat_forces[target_empire_index].non_combat_ships.begin()+i);
                        damage_done[e]-=defence_points_left;
                    } else {
                        empire_combat_forces[target_empire_index].non_combat_ships[i].second -= damage_done[e];
                        damage_done[e] = 0;
                    }
                }
                for (unsigned int i=0; i < combat_assets.size(); i++) {
                    if (empire_combat_forces[combat_assets[i]].CountAssets() == 0) {
                        combat_assets.erase(combat_assets.begin()+ i);
                        i--;
                    }
                }
            }
    }

#ifdef DEBUG_COMBAT
    Debugout(empire_combat_forces);
#endif
    // evaluation
    // victor: the empire which is the sole survivor and still has armed forces
    int victor = combat_assets.size() == 1 && empire_combat_forces[combat_assets[0]].CountArmedAssets() > 0
        ?combat_assets[0]:-1;

    for(int e=0;e<static_cast<int>(empire_combat_forces.size());e++)
    {
        // conquer planets if there is a victor
        if(victor!=-1 && victor!=e)
        {
            for(unsigned int i=0; i<empire_combat_forces[e].defenseless_planets.size(); i++)
                empire_combat_forces[e].defenseless_planets[i]->Conquer(empire_combat_forces[victor].owner->EmpireID());

            // shouldn't occur - only if defense bases are ignored
            for(unsigned int i=0; i<empire_combat_forces[e].planets.size(); i++)
                empire_combat_forces[e].planets[i].first->Conquer(empire_combat_forces[victor].owner->EmpireID());
        }

        // all retreating ships are destroyed
        if(victor!=-1 && victor!=e && empire_combat_forces[e].retreated_ships.size()>0)
        {
            for(unsigned int i=0; i<empire_combat_forces[e].retreated_ships.size(); i++)
                empire_combat_forces[e].destroyed_ships.push_back(empire_combat_forces[e].retreated_ships[i]);
            empire_combat_forces[e].retreated_ships.clear();
        }

        //remove destroyed ships
        for(unsigned int i=0; i<empire_combat_forces[e].destroyed_ships.size(); i++)
            RemoveShip(empire_combat_forces[e].destroyed_ships[i]->ID());

        // add some information to sitreport
        empire_combat_forces[e].owner->AddSitRepEntry(CreateCombatSitRep(empire_combat_forces[e].owner->EmpireID(),victor==-1?-1:empire_combat_forces[victor].owner->EmpireID(),system_id));
    }
}
