#include "AIInterface.h"

#include <map>
#include <string>

class Fleet;

class ReferenceAI : public AIBase
{
public:
    ReferenceAI();
    void GenerateOrders();
    void HandleChatMessage(int sender_id, const std::string& msg);

private:
    // utility order-generating functions
    void SplitFleet(Fleet* fleet);           ///< transfers ships after first ship in fleet into new single-ship fleets
    void Explore(Fleet* fleet);              ///< orders fleet to explore.  tries to find an unexplored system and goes there
    void ColonizeSomewhere(Fleet* fleet);    ///< orders fleet to find and colonize a planet / system

    // planning data / universe analysis intermediate results
    std::map<int, int> m_fleet_exploration_targets_map;  ///< map of (system_id, fleet_id) for systems that have had a fleet dispatched to explore them
};