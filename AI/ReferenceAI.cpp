#include "ReferenceAI.h"

#include "../universe/Universe.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../Empire/Empire.h"

#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"

#include <set>

ReferenceAI::ReferenceAI()
{}

void ReferenceAI::GenerateOrders()
{
    Universe& universe = GetUniverse();
    ObjectMap& objects = universe.Objects();
    int empire_id = AIInterface::EmpireID();

    Fleet* fleet;

    // 1) Split stationary multi-ship fleets into multiple single-ship fleets
    std::vector<UniverseObject*> stat_fleets = objects.FindObjects(StationaryFleetVisitor(empire_id));
    std::vector<UniverseObject*>::iterator fleet_it;

    for (fleet_it = stat_fleets.begin(); fleet_it != stat_fleets.end(); ++fleet_it) {
        if(!(fleet = dynamic_cast<Fleet*>(*fleet_it))) continue;

        // split fleet into single-ship fleets if it presently has more than one ship
        if (fleet->NumShips() > 1) {
            SplitFleet(fleet);
        }
    }

    // 2) Give stationary fleets orders
    stat_fleets = objects.FindObjects(StationaryFleetVisitor(empire_id));  // redo to get any newly created fleets from above

    for (fleet_it = stat_fleets.begin(); fleet_it != stat_fleets.end(); ++fleet_it) {
        if(!(fleet = dynamic_cast<Fleet*>(*fleet_it))) continue;

        if (fleet->NumShips() < 1) continue;    // shouldn't be possible... but to be safe...

        // get ship, design
        Ship* ship = objects.Object<Ship>(*(fleet->begin()));  if (!ship) continue;
        const ShipDesign *design = ship->Design();

        // give orders according to type of ship in fleet
        if (design->Name() == "Scout") {
            Explore(fleet);
        } else if (design->Name() == "Colony Ship") {
            ColonizeSomewhere(fleet);
        }
    }

    AIInterface::DoneTurn();
}

void ReferenceAI::HandleChatMessage(int sender_id, const std::string& msg)
{}

void ReferenceAI::Explore(Fleet* fleet) {
    if (!fleet) return;

    Logger().debugStream() << "telling fleet to explore";

    Universe& universe = GetUniverse();
    ObjectMap& objects = universe.Objects();
    int empire_id = AIInterface::EmpireID();

    // ensure this player owns this fleet
    if (!fleet->OwnedBy(empire_id))
        return;

    const Empire* empire = AIInterface::GetEmpire();
    if (!empire) throw std::runtime_error("Couldn't get pointer to empire when telling fleet to Explore");


    int start_id = fleet->SystemID();   // system where fleet is presently

    Logger().debugStream() << "telling fleet to explore2";

    // attempt to find an unexplored system that can be explored (fleet can get to)
    std::vector<System*> systems = objects.FindObjects<System>();
    for (std::vector<System*>::const_iterator system_it = systems.begin(); system_it != systems.end(); ++system_it) {
        System* system = *system_it;
        int dest_id = system->ID();   // system to go to
        if (empire->HasExploredSystem(dest_id)) continue;   // already explored system
        if (m_fleet_exploration_targets_map.find(dest_id) != m_fleet_exploration_targets_map.end()) continue;   // another fleet has been dispatched

        Logger().debugStream() << "telling fleet to explore3";

        // get path to destination.  don't care that it's short, but just that it exists
        std::list<int> route = universe.ShortestPath(start_id, dest_id, empire_id).first;

        if (route.empty()) continue; // can't get to system (with present starlanes knowledge)

        Logger().debugStream() << "telling fleet to explore4";

        // order ship to go ot system
        AIInterface::IssueFleetMoveOrder(fleet->ID(), dest_id);

        Logger().debugStream() << "telling fleet to explore5";

        // mark system as targeted for exploration, so another ship isn't sent to it redundantly
        m_fleet_exploration_targets_map.insert(std::pair<int, int>(dest_id, fleet->ID()));

        return; // don't need to keep looping at this point
    }
}

void ReferenceAI::ColonizeSomewhere(Fleet* fleet)
{}

void ReferenceAI::SplitFleet(Fleet* fleet)
{
    if (!fleet) return; // no fleet to process...
    if (fleet->NumShips() < 2) return;    // can't split fleet with one (or no?) ships

    const ObjectMap& objects = GetUniverse().Objects();
    int empire_id = AIInterface::EmpireID();

    // ensure this player owns this fleet
    if (!fleet->OwnedBy(empire_id))
        return;

    // starting with second ship, pick ships to transfer to new fleets
    std::set<int> ship_ids_to_remove;
    for (Fleet::iterator ship_it = ++(fleet->begin()); ship_it != fleet->end(); ++ship_it) {

        const Ship* ship = objects.Object<Ship>(*ship_it);
        if (!ship->OwnedBy(empire_id))
            continue;

        ship_ids_to_remove.insert(*ship_it);
    }

    if (ship_ids_to_remove.empty()) return;  // nothing more to do

    // order transfers of ships from old fleet to new fleets 
    for (std::set<int>::iterator ship_it = ship_ids_to_remove.begin(); ship_it != ship_ids_to_remove.end(); ++ship_it) {
        std::vector<int> ship_ids;
        ship_ids.push_back(*ship_it);

        std::string fleet_name = UserString("FW_NEW_FLEET_NAME");

        AIInterface::IssueNewFleetOrder(fleet_name, ship_ids);
    }
}

