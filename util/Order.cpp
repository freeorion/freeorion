#include "Order.h"

#include <fstream>
#include <vector>
#include <boost/algorithm/string/trim.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "AppInterface.h"
#include "i18n.h"
#include "Logger.h"
#include "OrderSet.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../universe/Building.h"
#include "../universe/Condition.h"
#include "../universe/Fleet.h"
#include "../universe/Pathfinder.h"
#include "../universe/Planet.h"
#include "../universe/ShipDesign.h"
#include "../universe/Ship.h"
#include "../universe/Species.h"
#include "../universe/System.h"
#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"


/////////////////////////////////////////////////////
// Order
/////////////////////////////////////////////////////
std::shared_ptr<Empire> Order::GetValidatedEmpire(ScriptingContext& context) const {
    auto empire = context.GetEmpire(EmpireID());
    if (!empire)
        throw std::runtime_error("Invalid empire ID specified for order.");
    return empire;
}

void Order::Execute(ScriptingContext& context) const {
    if (m_executed)
        return;
    m_executed = true;

    ExecuteImpl(context);
}

bool Order::Undo(ScriptingContext& context) const {
    auto undone =  UndoImpl(context);
    if (undone)
        m_executed = false;
    return undone;
}

namespace {
#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
    constexpr std::string EMPTY_STRING;
#else
    const std::string EMPTY_STRING;
#endif

    const std::string& ExecutedTag(const Order* order) {
        if (order && !order->Executed())
            return UserString("ORDER_UNEXECUTED");
        return EMPTY_STRING;
    }

    // checks for first and continuation bytes in allowed ranges for UTF8
    // doesn't check for overlong sequences
    constexpr bool IsValidUTF8(std::string_view sv) {
        if (sv.empty())
            return true;

        auto it = sv.begin();
        auto dist = std::distance(it, sv.end());

        while (true) {
            const uint8_t c = *it;
            const std::ptrdiff_t sequence_length =
                (c <= 0x7F) ? 1 :
                (c >= 0xC2 && c <= 0xDF) ? 2 :
                (c >= 0xE0 && c <= 0xEF) ? 3 :
                (c >= 0xF0) ? 4 : 0;

            if (dist < sequence_length || sequence_length == 0)
                return false;

            if (sequence_length == 1) {
                dist -= 1;

            } else if (sequence_length == 2) {
                const uint8_t c2 = *++it;
                if (c2 < 0x80 || c2 > 0xBF)
                    return false;
                dist -= 2;

            } else if (sequence_length == 3) {
                const uint8_t c2 = *++it;
                if (c2 < 0x80 || c2 > 0xBF)
                    return false;
                const uint8_t c3 = *++it;
                if (c3 < 0x80 || c3 > 0xBF)
                    return false;
                dist -= 3;

            }  else if (sequence_length == 4) {
                const uint8_t c2 = *++it;
                if (c2 < 0x80 || c2 > 0xBF)
                    return false;
                const uint8_t c3 = *++it;
                if (c3 < 0x80 || c3 > 0xBF)
                    return false;
                const uint8_t c4 = *++it;
                if (c4 < 0x80 || c4 > 0xBF)
                    return false;
                dist -= 4;
            }

            ++it;

            if (dist < 1)
                return true;
        }

        return true;
    };

    template <std::size_t N>
    constexpr bool IsValidUTF8(std::array<char, N> arr)
    { return IsValidUTF8(std::string_view{arr.data(), arr.size()}); }

    static_assert(IsValidUTF8("test string!"));

#if defined(__cpp_lib_char8_t)
    constexpr std::u8string_view long_chars = u8"αbåオーガитیای مجهو ";
    constexpr auto long_chars_arr = []() {
        std::array<std::string_view::value_type, long_chars.size()> retval{};
        for (std::size_t idx = 0; idx < retval.size(); ++idx)
            retval[idx] = long_chars[idx];
        return retval;
    }();
    constexpr std::string_view long_chars_sv(long_chars_arr.data(), long_chars_arr.size());
#else
    constexpr std::string_view long_chars_sv = u8"αbåオーガитیای مجهو";
#endif
    static_assert(IsValidUTF8(long_chars_sv));

    static_assert(IsValidUTF8(""));

    constexpr auto ch = [](const uint8_t u) -> char { return static_cast<char>(u); };

    constexpr std::array<char, 4> some_chars1{33, 53, ch(225), 90}; // two ascii chars, then a truncated sequence
    static_assert(!IsValidUTF8(some_chars1));

    constexpr std::array<char, 1> some_chars2{ch(239)}; // truncated 3 byte sequence
    static_assert(!IsValidUTF8(some_chars2));

    constexpr std::array<char, 4> some_chars3{ch(255), 1, 80, 80}; // 4 byte sequence with second byte that is not a continuation byte
    static_assert(!IsValidUTF8(some_chars3));

    constexpr std::array<char, 4> some_chars4{50, 60, ch(192), 80}; // two ascii chars, then an invalid first byte
    static_assert(!IsValidUTF8(some_chars4));

    constexpr bool IsControlChar(const uint8_t c) noexcept
    { return c < 0x20 || c == 0x7F || (c >= 0x81 && c <= 0x9F); }

    constexpr std::string_view formatting_chars = "<>;:,.@#$%&*(){}'\"/?\\`[]|\a\b\f\n\r\t\b";
    constexpr bool IsFormattingChar(char c)
    { return formatting_chars.find(c) != std::string_view::npos; }
}

////////////////////////////////////////////////
// RenameOrder
////////////////////////////////////////////////
RenameOrder::RenameOrder(int empire, int object, std::string name,
                         const ScriptingContext& context) :
    Order(empire),
    m_object(object),
    m_name(std::move(name))
{
    if (!Check(empire, object, m_name, context))
        m_object = INVALID_OBJECT_ID;
}

std::string RenameOrder::Dump() const
{ return boost::io::str(FlexibleFormat(UserString("ORDER_RENAME")) % m_object % m_name) + ExecutedTag(this); }

bool RenameOrder::Check(int empire, int object, std::string new_name,
                        const ScriptingContext& context)
{
    if (new_name.size() > 64) {
        ErrorLogger() << "RenameOrder::Check() : passed an overly long new_name of size: " << new_name.size();
        return false;
    }

#if defined(FREEORION_ANDROID)
    boost::trim(new_name);
#else
    auto& locale = GetLocale();
    boost::trim(new_name, locale);
#endif

    // disallow nameless objects
    if (new_name.empty()) {
        ErrorLogger() << "RenameOrder::Check() : passed an empty new_name.";
        return false;
    }

    // check UTF8 valididty
    if (!IsValidUTF8(new_name)) {
        ErrorLogger() << "RenameOrder::Check() : passed invalid UTF8 new_name.";
        return false;
    }

    // disallow control characters
    if (std::any_of(new_name.begin(), new_name.end(), IsControlChar)) {
        ErrorLogger() << "RenameOrder::Check : passed control character in name.";
        return false;
    }

    // disallow formatting characters
    if (std::any_of(new_name.begin(), new_name.end(), IsFormattingChar)) {
        ErrorLogger() << "RenameOrder::Check : passed formatting character in name.";
        return false;
    }

    auto obj = context.ContextObjects().get(object);

    if (!obj) {
        ErrorLogger() << "RenameOrder::Check() : passed an invalid object.";
        return false;
    }

    // verify that empire specified in order owns specified object
    if (!obj->OwnedBy(empire)) {
        ErrorLogger() << "RenameOrder::Check() : Object " << object << " is"
                      << " not owned by empire " << empire << ".";
        return false;
    }

    if (obj->Name() == new_name) {
        ErrorLogger() << "RenameOrder::Check() : Object " << object
                      << " should renamed to the same name.";
        return false;
    }

    return true;
}

void RenameOrder::ExecuteImpl(ScriptingContext& context) const {
    if (!Check(EmpireID(), m_object, m_name, context))
        return;

    GetValidatedEmpire(context);

    auto obj = context.ContextObjects().get(m_object);

    obj->Rename(m_name);
}

////////////////////////////////////////////////
// CreateFleetOrder
////////////////////////////////////////////////
NewFleetOrder::NewFleetOrder(int empire, std::string fleet_name,
                             std::vector<int> ship_ids,
                             const ScriptingContext& context,
                             bool aggressive, bool passive, bool defensive) :
    NewFleetOrder(empire, std::move(fleet_name), std::move(ship_ids),
                  aggressive ? FleetAggression::FLEET_AGGRESSIVE :
                  defensive ? FleetAggression::FLEET_DEFENSIVE :
                  passive ? FleetAggression::FLEET_PASSIVE :
                  FleetAggression::FLEET_OBSTRUCTIVE, 
                  context)
{}

NewFleetOrder::NewFleetOrder(int empire, std::string fleet_name,
                             std::vector<int> ship_ids, FleetAggression aggression,
                             const ScriptingContext& context) :
    Order(empire),
    m_fleet_name(std::move(fleet_name)),
    m_fleet_id(INVALID_OBJECT_ID),
    m_ship_ids(std::move(ship_ids)),
    m_aggression(aggression)
{
    if (!Check(empire, m_fleet_name, m_ship_ids, m_aggression, context))
        return;
}

bool NewFleetOrder::Aggressive() const noexcept
{ return m_aggression == FleetAggression::FLEET_AGGRESSIVE; }

std::string NewFleetOrder::Dump() const {
    const std::string& aggression_text =
        m_aggression == FleetAggression::FLEET_AGGRESSIVE ? UserString("FLEET_AGGRESSIVE") :
        m_aggression == FleetAggression::FLEET_OBSTRUCTIVE ? UserString("FLEET_OBSTRUCTIVE") :
        m_aggression == FleetAggression::FLEET_DEFENSIVE ? UserString("FLEET_DEFENSIVE") :
        m_aggression == FleetAggression::FLEET_PASSIVE ? UserString("FLEET_PASSIVE") :
        UserString("INVALID_FLEET_AGGRESSION");

    return boost::io::str(FlexibleFormat(UserString("ORDER_FLEET_NEW"))
                          % m_fleet_name
                          % std::to_string(m_ship_ids.size())
                          % aggression_text)
        + ExecutedTag(this);
}

bool NewFleetOrder::Check(int empire, const std::string& fleet_name, const std::vector<int>& ship_ids,
                          FleetAggression aggression, const ScriptingContext& context)
{
    if (ship_ids.empty()) {
        ErrorLogger() << "Empire " << empire << " attempted to create a new fleet (" << fleet_name << ") without ships";
        return false;
    }

    if (auto pos = fleet_name.find_first_of(formatting_chars); pos != std::string::npos) {
        ErrorLogger() << "New fleet name contains banned character: \"" << fleet_name[pos] << "\"";
        return false;
    }

    int system_id = INVALID_OBJECT_ID;

    std::set<int> arrival_starlane_ids;
    for (const auto* ship : context.ContextObjects().findRaw<Ship>(ship_ids)) {
        if (!ship) {
            ErrorLogger() << "Empire " << empire << " attempted to create a new fleet (" << fleet_name
                          << ") with an invalid ship";
            return false;
        }

        auto ship_fleet = context.ContextObjects().get<Fleet>(ship->FleetID());
        if (!ship_fleet)
            continue;   // OK
        auto arr_lane = ship_fleet->ArrivalStarlane();
        if (arr_lane == INVALID_OBJECT_ID) // newly produced fleets have invalid object ID as arrival lane for one turn. this lets them be merged with fleets that have had their arrival lane set to their system
            arr_lane = ship_fleet->SystemID();
        arrival_starlane_ids.insert(arr_lane);
    }
    if (arrival_starlane_ids.size() > 1) {
        ErrorLogger() << "Empire " << empire << " attempted to create a new fleet with ships from multiple arrival starlanes";
        return false;
    }


    for (const auto* ship : context.ContextObjects().findRaw<Ship>(ship_ids)) {
        // verify that empire is not trying to take ships from somebody else's fleet
        if (!ship->OwnedBy(empire)) {
            ErrorLogger() << "Empire " << empire << " attempted to create a new fleet (" << fleet_name
                          << ") with ships from another's (" << ship->Owner() << ") fleet.";
            return false;
        }
        if (ship->SystemID() == INVALID_OBJECT_ID) {
            ErrorLogger() << "Empire " << empire << " attempted to create a new fleet (" << fleet_name
                          << ") with ship (" << ship->ID() << ") not in a system";
            return false;
        }

        if (system_id == INVALID_OBJECT_ID)
            system_id = ship->SystemID();

        if (ship->SystemID() != system_id) {
            ErrorLogger() << "Empire " << empire << " attempted to make a new fleet (" << fleet_name
                          << ") from ship (" << ship->ID() << ") in the wrong system (" << ship->SystemID()
                          << " not " << system_id << ")";
            return false;
        }
    }

    if (system_id == INVALID_OBJECT_ID) {
        ErrorLogger() << "Empire " << empire << " attempted to create a new fleet (" << fleet_name
                      << ") outside a system";
        return false;
    }
    auto system = context.ContextObjects().get<System>(system_id);
    if (!system) {
        ErrorLogger() << "Empire " << empire << " attempted to create a new fleet (" << fleet_name
                      << ") in a nonexistant system (" << system_id << ")";
        return false;
    }

    return true;
}

void NewFleetOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);

    if (!Check(EmpireID(), m_fleet_name, m_ship_ids, m_aggression, context))
        return;

    Universe& u = context.ContextUniverse();
    ObjectMap& o = context.ContextObjects();
    const auto& ids_as_flatset{context.EmpireIDs()};
    const std::vector<int> empire_ids{ids_as_flatset.begin(), ids_as_flatset.end()};

    u.InhibitUniverseObjectSignals(true);

    // validate specified ships
    auto validated_ships = o.findRaw<Ship>(m_ship_ids);

    int system_id = validated_ships[0]->SystemID();
    auto system = o.get<System>(system_id);

    std::shared_ptr<Fleet> fleet;
    if (m_fleet_id == INVALID_OBJECT_ID) {
        // create fleet
        fleet = u.InsertNew<Fleet>(m_fleet_name, system->X(), system->Y(),
                                   EmpireID(), context.current_turn);
        m_fleet_id = fleet->ID();
    } else {
        fleet = u.InsertByEmpireWithID<Fleet>(EmpireID(), m_fleet_id, m_fleet_name,
                                              system->X(), system->Y(), EmpireID(),
                                              context.current_turn);
    }

    if (!fleet) {
        ErrorLogger() << "Unable to create fleet.";
        return;
    }

    fleet->GetMeter(MeterType::METER_STEALTH)->SetCurrent(Meter::LARGE_VALUE);
    fleet->SetAggression(m_aggression);

    // an ID is provided to ensure consistancy between server and client universes
    u.SetEmpireObjectVisibility(EmpireID(), fleet->ID(), Visibility::VIS_FULL_VISIBILITY);

    system->Insert(fleet, System::NO_ORBIT, context.current_turn, o);

    // new fleet will get same m_arrival_starlane as fleet of the first ship in the list.
    auto first_ship{validated_ships[0]};
    auto first_fleet = o.get<Fleet>(first_ship->FleetID());
    if (first_fleet)
        fleet->SetArrivalStarlane(first_fleet->ArrivalStarlane());

    std::vector<Fleet*> modified_fleets;
    int ordered_moved_turn = BEFORE_FIRST_TURN;
    // remove ships from old fleet(s) and add to new
    for (auto* ship : validated_ships) {
        if (auto* old_fleet = o.getRaw<Fleet>(ship->FleetID())) {
            ordered_moved_turn = std::max(ordered_moved_turn, old_fleet->LastTurnMoveOrdered());
            old_fleet->RemoveShips({ship->ID()});
            modified_fleets.push_back(old_fleet);
        }
        ship->SetFleetID(fleet->ID());
    }
    std::sort(modified_fleets.begin(), modified_fleets.end());
    modified_fleets.erase(std::unique(modified_fleets.begin(), modified_fleets.end()), modified_fleets.end());
    fleet->AddShips(m_ship_ids);
    fleet->SetMoveOrderedTurn(ordered_moved_turn);

    if (m_fleet_name.empty())
        fleet->Rename(fleet->GenerateFleetName(context));

    u.InhibitUniverseObjectSignals(false);

    system->FleetsInsertedSignal(std::vector<int>{fleet->ID()}, o);
    system->StateChangedSignal();

    // Signal changed state of modified fleets and remove any empty fleets.
    for (auto* modified_fleet : modified_fleets) {
        if (!modified_fleet->Empty()) {
            modified_fleet->StateChangedSignal();
        } else {
            if (auto modified_fleet_system = o.getRaw<System>(modified_fleet->SystemID()))
                modified_fleet_system->Remove(modified_fleet->ID());

            u.Destroy(modified_fleet->ID(), empire_ids);
        }
    }
}

////////////////////////////////////////////////
// FleetMoveOrder
////////////////////////////////////////////////
FleetMoveOrder::FleetMoveOrder(int empire_id, int fleet_id, int dest_system_id,
                               bool append, const ScriptingContext& context) :
    Order(empire_id),
    m_fleet(fleet_id),
    m_dest_system(dest_system_id),
    m_append(append)
{
    if (!Check(empire_id, fleet_id, dest_system_id, append, context))
        return;

    auto fleet = context.ContextObjects().getRaw<Fleet>(m_fleet);
    //if (!fleet) { // additional check not necessary after calling Check(...)
    //    ErrorLogger() << "FleetMoveOrder has invalid fleed id:" << m_fleet;
    //    return;
    //}

    int start_system = fleet->SystemID();
    if (start_system == INVALID_OBJECT_ID)
        start_system = fleet->NextSystemID();
    if (append && !fleet->TravelRoute().empty())
        start_system = fleet->TravelRoute().back();

    auto short_path = context.ContextUniverse().GetPathfinder().ShortestPath(
        start_system, m_dest_system, EmpireID(), context.ContextObjects()).first;
    if (short_path.empty()) {
        ErrorLogger() << "FleetMoveOrder generated empty shortest path between system " << start_system
                      << " and " << m_dest_system << " for empire " << EmpireID()
                      << " with fleet " << m_fleet;
        return;
    }

    // if in a system now, don't include it in the route
    if (short_path.front() == fleet->SystemID()) {
        DebugLogger() << "FleetMoveOrder removing fleet " << m_fleet
                      << " current system location " << fleet->SystemID()
                      << " from shortest path to system " << m_dest_system;
        short_path.erase(short_path.begin()); // pop_front();
    }

    m_route = std::move(short_path);

    // ensure a zero-length (invalid) route is not requested / sent to a fleet
    if (m_route.empty())
        m_route.push_back(start_system);
}

std::string FleetMoveOrder::Dump() const
{ return boost::io::str(FlexibleFormat(UserString("ORDER_FLEET_MOVE")) % m_fleet % m_dest_system) + ExecutedTag(this); }

bool FleetMoveOrder::Check(int empire_id, int fleet_id, int dest_system_id,
                           bool append, const ScriptingContext& context)
{
    auto fleet = context.ContextObjects().getRaw<Fleet>(fleet_id);
    if (!fleet) {
        ErrorLogger() << "Empire with id " << empire_id << " ordered fleet with id " << fleet_id << " to move, but no such fleet exists";
        return false;
    }

    if (!fleet->OwnedBy(empire_id) ) {
        ErrorLogger() << "Empire with id " << empire_id << " order to move but does not own fleet with id " << fleet_id;
        return false;
    }

    const auto& known_objs{AppEmpireID() == ALL_EMPIRES ?
        context.ContextUniverse().EmpireKnownObjects(empire_id) : context.ContextObjects()};
    auto dest_system = known_objs.getRaw<System>(dest_system_id);
    if (!dest_system) {
        ErrorLogger() << "Empire with id " << empire_id << " ordered fleet to move to system with id " << dest_system_id << " but no such system is known to that empire";
        return false;
    }

    return true;
}

void FleetMoveOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);

    if (!Check(EmpireID(), m_fleet, m_dest_system, m_append, context))
        return;

    // convert list of ids to list of System
    auto fleet = context.ContextObjects().get<Fleet>(FleetID());
    if (!fleet) {
        ErrorLogger() << "FleetMoveOrder::ExecuteImpl couldn't get fleet with ID: " << m_fleet;
        return;
    }
    using RouteListT = std::remove_const_t<std::remove_reference_t<decltype(fleet->TravelRoute())>>;
    RouteListT fleet_travel_route{m_append ? fleet->TravelRoute() : RouteListT{}};

    if (m_append && !fleet_travel_route.empty()) {
        DebugLogger() << "FleetMoveOrder::ExecuteImpl appending initial" << [&]() {
            std::stringstream ss;
            for (int waypoint : fleet_travel_route)
                ss << " " << waypoint;
            return ss.str();
        }() << "  with" << [&]() {
            std::stringstream ss;
            for (int waypoint : m_route)
                ss << " " << waypoint;
            return ss.str();
        }();
        fleet_travel_route.pop_back(); // remove last item as it should be the first in the appended route
    }

    std::copy(m_route.begin(), m_route.end(), std::back_inserter(fleet_travel_route));
    DebugLogger() << [&fleet, &fleet_travel_route]() {
        std::stringstream ss;
        ss << "FleetMoveOrder::ExecuteImpl Setting route of fleet " << fleet->ID() << " at system " << fleet->SystemID() << " to: ";
        if (fleet_travel_route.empty())
            return std::string("[empty route]");
        for (int waypoint : fleet_travel_route)
            ss << " " << std::to_string(waypoint);
        return ss.str();
    }();

    if (!fleet_travel_route.empty() && fleet_travel_route.front() == fleet->SystemID()) {
        DebugLogger() << "FleetMoveOrder::ExecuteImpl given route that starts with fleet " << fleet->ID()
                      << "'s current system (" << fleet_travel_route.front() << "); removing it";
        fleet_travel_route.erase(fleet_travel_route.begin()); // pop_front();
    }

    // check destination validity: disallow movement that's out of range
    const auto eta = fleet->ETA(fleet->MovePath(fleet_travel_route, false, context));
    if (eta.first == Fleet::ETA_NEVER || eta.first == Fleet::ETA_OUT_OF_RANGE) {
        DebugLogger() << "FleetMoveOrder::ExecuteImpl rejected out of range move order";
        return;
    }

    try {
        fleet->SetRoute(std::move(fleet_travel_route), context.ContextObjects());
        fleet->SetMoveOrderedTurn(context.current_turn);
        // todo: set last turn ordered moved
    } catch (const std::exception& e) {
        ErrorLogger() << "Caught exception setting fleet route while executing fleet move order: " << e.what();
    }
}

////////////////////////////////////////////////
// FleetTransferOrder
////////////////////////////////////////////////
FleetTransferOrder::FleetTransferOrder(int empire, int dest_fleet, std::vector<int> ships,
                                       const ScriptingContext& context) :
    Order(empire),
    m_dest_fleet(dest_fleet),
    m_add_ships(std::move(ships))
{
    if (!Check(empire, m_dest_fleet, m_add_ships, context))
        ErrorLogger() << "FleetTransferOrder constructor found problem...";
}

std::string FleetTransferOrder::Dump() const {
    std::string ships;
    if (!m_add_ships.empty()) {
        ships.reserve(15*m_add_ships.size()); // guesstimate
        for (auto s : m_add_ships)
            ships.append(std::to_string(s)).append(" ");
        ships.resize(ships.length() - 1);
    }
    return boost::io::str(FlexibleFormat(UserString("ORDER_FLEET_TRANSFER")) % ships % m_dest_fleet) + ExecutedTag(this);
}

bool FleetTransferOrder::Check(int empire_id, int dest_fleet_id, const std::vector<int>& ship_ids,
                               const ScriptingContext& context)
{
    const ObjectMap& objects{context.ContextObjects()};

    auto fleet = objects.get<Fleet>(dest_fleet_id);
    if (!fleet) {
        ErrorLogger() << "Empire attempted to move ships to a nonexistant fleet";
        return false;
    }
    // check that destination fleet is owned by empire
    if (!fleet->OwnedBy(empire_id)) {
        ErrorLogger() << "IssueFleetTransferOrder : passed fleet_id "<< dest_fleet_id << " of fleet not owned by player";
        return false;
    }

    if (fleet->SystemID() == INVALID_OBJECT_ID) {
        ErrorLogger() << "IssueFleetTransferOrder : new fleet is not in a system";
        return false;
    }

    bool invalid_ships{false};

    for (const auto& ship : objects.find<Ship>(ship_ids)) {
        if (!ship) {
            ErrorLogger() << "IssueFleetTransferOrder : passed an invalid ship_id";
            invalid_ships = true;
            break;
        }

        if (!ship->OwnedBy(empire_id)) {
            ErrorLogger() << "IssueFleetTransferOrder : passed ship_id of ship not owned by player";
            invalid_ships = true;
            break;
        }

        if (ship->SystemID() == INVALID_OBJECT_ID) {
            ErrorLogger() << "IssueFleetTransferOrder : ship is not in a system";
            invalid_ships = true;
            break;
        }

        if (ship->SystemID() != fleet->SystemID()) {
            ErrorLogger() << "IssueFleetTransferOrder : passed ship is not in the same system as the target fleet";
            invalid_ships = true;
            break;
        }

        if (ship->FleetID() == dest_fleet_id) {
            ErrorLogger() << "IssueFleetTransferOrder : passed ship that is already in the target fleet";
            invalid_ships = true;
            break;
        }

        if (auto original_fleet = context.ContextObjects().get<Fleet>(ship->FleetID())) {
            auto ship_old_fleet_arr_lane = original_fleet->ArrivalStarlane();
            if (ship_old_fleet_arr_lane == INVALID_OBJECT_ID) // deal with quirky case where new fleets have no arrival lane for one turn
                ship_old_fleet_arr_lane = original_fleet->SystemID();
            auto new_fleet_arr_lane = fleet->ArrivalStarlane();
            if (new_fleet_arr_lane == INVALID_OBJECT_ID)      // same quirky case can affect both source and destination fleets
                new_fleet_arr_lane = fleet->SystemID();

            if (ship_old_fleet_arr_lane != new_fleet_arr_lane) {
                ErrorLogger() << "IssueFleetTransferOrder : passed ship " << ship->ID()
                              << " that is in a fleet " << original_fleet->ID()
                              << " that has a different arrival starlane " << ship_old_fleet_arr_lane
                              << " than the destination fleet " << fleet->ID()
                              << " with arrival starlane " << new_fleet_arr_lane;
                invalid_ships = true;
                break;
            }
        }
    }

    return !invalid_ships;
}

void FleetTransferOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);

    if (!Check(EmpireID(), m_dest_fleet, m_add_ships, context))
        return;

    // look up the destination fleet
    auto target_fleet = context.ContextObjects().get<Fleet>(m_dest_fleet);

    // check that all ships are in the same system
    auto ships = context.ContextObjects().findRaw<Ship>(m_add_ships);

    context.ContextUniverse().InhibitUniverseObjectSignals(true);

    // remove from old fleet(s)
    std::set<Fleet*> modified_fleets;
    for (auto& ship : ships) {
        if (auto source_fleet = context.ContextObjects().getRaw<Fleet>(ship->FleetID())) {
            source_fleet->RemoveShips({ship->ID()});
            modified_fleets.insert(source_fleet);
        }
        ship->SetFleetID(target_fleet->ID());
    }

    // add to new fleet
    std::vector<int> validated_ship_ids;
    validated_ship_ids.reserve(m_add_ships.size());
    for (const auto& ship : ships)
        validated_ship_ids.push_back(ship->ID());

    target_fleet->AddShips(validated_ship_ids);

    context.ContextUniverse().InhibitUniverseObjectSignals(false);

    // signal change to fleet states
    modified_fleets.insert(target_fleet.get());
    const auto& ids_as_flatset{context.EmpireIDs()};
    const std::vector<int> empire_ids{ids_as_flatset.begin(), ids_as_flatset.end()};

    for (auto* modified_fleet : modified_fleets) {
        if (!modified_fleet) {
            continue;
        } else if (!modified_fleet->Empty()) {
            modified_fleet->StateChangedSignal();
        } else {
            if (auto system = context.ContextObjects().getRaw<System>(modified_fleet->SystemID()))
                system->Remove(modified_fleet->ID());

            context.ContextUniverse().Destroy(modified_fleet->ID(), empire_ids);
        }
    }
}

////////////////////////////////////////////////
// AnnexOrder
////////////////////////////////////////////////
AnnexOrder::AnnexOrder(int empire, int planet, const ScriptingContext& context) :
    Order(empire),
    m_planet(planet)
{ Check(empire, m_planet, context); }

std::string AnnexOrder::Dump() const
{ return boost::io::str(FlexibleFormat(UserString("ORDER_ANNEX")) % m_planet) + ExecutedTag(this); }

bool AnnexOrder::Check(int empire_id, int planet_id, const ScriptingContext& context) {
    const ObjectMap& o = context.ContextObjects();

    const auto* planet = o.getRaw<const Planet>(planet_id);
    if (!planet) {
        ErrorLogger() << "AnnexOrder couldn't get planet with id " << planet_id;
        return false;
    }

    if (empire_id == ALL_EMPIRES) {
        ErrorLogger() << "AnnexOrder given non-empire empire id: " << empire_id;
        return false;
    }

    const auto& planet_species_name = planet->SpeciesName();
    if (planet_species_name.empty()) {
        ErrorLogger() << "AnnexOrder given planet without a species: " << planet_id;
        return false;
    }
    const auto* planet_species = context.species.GetSpecies(planet_species_name);
    if (!planet_species) {
        ErrorLogger() << "AnnexOrder given planet with an unknown species: " << planet_species_name;
        return false;
    }
    const auto* annexation_condition = planet_species->AnnexationCondition();
    if (!annexation_condition) {
        ErrorLogger() << "AnnexOrder given planet with species with no annexation condition: " << planet_species_name;
        return false;
    }

    if (!context.source)
        ErrorLogger() << "AnnexOrder given context with no source... Context source should be an object owned by the order issuing empire";

    if (!context.source->OwnedBy(empire_id))
        ErrorLogger() << "AnnexOrder given context with source not owned by passed in empire id";

    if (!annexation_condition->EvalOne(context, planet)) {
        ErrorLogger() << "AnnexOrder given planet that does not meet its species annexation condition: " << planet_species_name;
        return false;
    }

    // TODO: check IP costs, like adopting policies

    return true;
}

void AnnexOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);

    if (!Check(EmpireID(), m_planet, context))
        return;

    ObjectMap& objects{context.ContextObjects()};
    if (auto* planet = objects.getRaw<Planet>(m_planet))
        planet->SetIsOrderAnnexedByEmpire(EmpireID());
}

bool AnnexOrder::UndoImpl(ScriptingContext& context) const {
    ObjectMap& objects{context.ContextObjects()};

    auto* planet = objects.getRaw<Planet>(m_planet);
    if (!planet) {
        ErrorLogger() << "AnnexOrder::UndoImpl couldn't get planet with id " << m_planet;
        return false;
    }

    planet->ResetBeingAnnxed();

    return true;
}

////////////////////////////////////////////////
// ColonizeOrder
////////////////////////////////////////////////
ColonizeOrder::ColonizeOrder(int empire, int ship, int planet, const ScriptingContext& context) :
    Order(empire),
    m_ship(ship),
    m_planet(planet)
{ Check(empire, m_ship, m_planet, context); }

std::string ColonizeOrder::Dump() const
{ return boost::io::str(FlexibleFormat(UserString("ORDER_COLONIZE")) % m_planet % m_ship) + ExecutedTag(this); }

bool ColonizeOrder::Check(int empire_id, int ship_id, int planet_id, const ScriptingContext& context) {
    const Universe& u = context.ContextUniverse();
    const ObjectMap& o = context.ContextObjects();
    const SpeciesManager& sm = context.species;

    if (empire_id == ALL_EMPIRES) {
        ErrorLogger() << "ColonizeOrder::Check() : empire " << empire_id << " is not an empire";
        return false;
    }

    auto ship = o.get<Ship>(ship_id);
    if (!ship) {
        ErrorLogger() << "ColonizeOrder::Check() : empire " << empire_id
                      << " passed an invalid ship_id: " << ship_id;
        return false;
    }
    auto fleet = o.get<Fleet>(ship->FleetID());
    if (!fleet) {
        ErrorLogger() << "ColonizeOrder::Check() : empire " << empire_id
                      << " passed ship (" << ship_id << ") with an invalid fleet_id: " << ship->FleetID();
        return false;
    }

    if (!fleet->OwnedBy(empire_id)) {
        ErrorLogger() << "ColonizeOrder::Check() : empire does not own fleet of passed ship";
        return 0;
    }
    if (!ship->OwnedBy(empire_id)) {
        ErrorLogger() << "ColonizeOrder::Check() : got ship that isn't owned by the order-issuing empire";
        return false;
    }

    if (!ship->CanColonize(u, sm)) { // verifies that species exists and can colonize and that ship can colonize
        ErrorLogger() << "ColonizeOrder::Check() : got ship that can't colonize";
        return false;
    }

    auto planet = o.get<Planet>(planet_id);
    float colonist_capacity = ship->ColonyCapacity(u);
    if (!planet) {
        ErrorLogger() << "ColonizeOrder::Check() : couldn't get planet with id " << planet_id;
        return false;
    }
    if (planet->GetMeter(MeterType::METER_POPULATION)->Initial() > 0.0f) {
        ErrorLogger() << "ColonizeOrder::Check() : given planet that already has population";
        return false;
    }
    if (!planet->Unowned() && planet->Owner() != empire_id) {
        ErrorLogger() << "ColonizeOrder::Check() : given planet that owned by another empire";
        return false;
    }
    if (planet->OwnedBy(empire_id) && colonist_capacity == 0.0f) {
        ErrorLogger() << "ColonizeOrder::Check() : given planet that is already owned by empire and colony ship with zero capcity";
        return false;
    }
    if (u.GetObjectVisibilityByEmpire(planet_id, empire_id) < Visibility::VIS_PARTIAL_VISIBILITY) {
        ErrorLogger() << "ColonizeOrder::Check() : given planet that empire has insufficient visibility of";
        return false;
    }
    if (colonist_capacity > 0.0f &&
        planet->EnvironmentForSpecies(context.species, ship->SpeciesName()) < PlanetEnvironment::PE_HOSTILE)
    {
        ErrorLogger() << "ColonizeOrder::Check() : nonzero colonist capacity, " << colonist_capacity
                      << ", and planet " << planet->Name() << " of type, " << planet->Type() << ", that ship's species, "
                      << ship->SpeciesName() << ", can't colonize";
        return false;
    }

    int ship_system_id = ship->SystemID();
    if (ship_system_id == INVALID_OBJECT_ID) {
        ErrorLogger() << "ColonizeOrder::Check() : given id of ship not in a system";
        return false;
    }
    int planet_system_id = planet->SystemID();
    if (ship_system_id != planet_system_id) {
        ErrorLogger() << "ColonizeOrder::Check() : given ids of ship and planet not in the same system";
        return false;
    }

    return true;
}

void ColonizeOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);

    if (!Check(EmpireID(), m_ship, m_planet, context))
        return;

    ObjectMap& objects{context.ContextObjects()};
    auto ship = objects.get<Ship>(m_ship);
    if (!ship)
        return;
    auto planet = objects.get<Planet>(m_planet);
    if (!planet)
        return;

    planet->SetIsAboutToBeColonized(true);
    ship->SetColonizePlanet(m_planet);

    if (auto fleet = objects.get<Fleet>(ship->FleetID()))
        fleet->StateChangedSignal();
}

bool ColonizeOrder::UndoImpl(ScriptingContext& context) const {
    ObjectMap& objects{context.ContextObjects()};

    auto planet = objects.get<Planet>(m_planet);
    if (!planet) {
        ErrorLogger() << "ColonizeOrder::UndoImpl couldn't get planet with id " << m_planet;
        return false;
    }
    if (!planet->IsAboutToBeColonized()) {
        ErrorLogger() << "ColonizeOrder::UndoImpl planet is not about to be colonized...";
        return false;
    }

    auto ship = objects.get<Ship>(m_ship);
    if (!ship) {
        ErrorLogger() << "ColonizeOrder::UndoImpl couldn't get ship with id " << m_ship;
        return false;
    }
    if (ship->OrderedColonizePlanet() != m_planet) {
        ErrorLogger() << "ColonizeOrder::UndoImpl ship is not about to colonize planet";
        return false;
    }

    planet->SetIsAboutToBeColonized(false);
    ship->ClearColonizePlanet();

    if (auto fleet = objects.get<Fleet>(ship->FleetID()))
        fleet->StateChangedSignal();

    return true;
}

////////////////////////////////////////////////
// InvadeOrder
////////////////////////////////////////////////
InvadeOrder::InvadeOrder(int empire, int ship, int planet, const ScriptingContext& context) :
    Order(empire),
    m_ship(ship),
    m_planet(planet)
{ Check(empire, m_ship, m_planet, context); }

std::string InvadeOrder::Dump() const
{ return boost::io::str(FlexibleFormat(UserString("ORDER_INVADE")) % m_planet % m_ship) + ExecutedTag(this); }

bool InvadeOrder::Check(int empire_id, int ship_id, int planet_id, const ScriptingContext& context) {
    const Universe& u = context.ContextUniverse();
    const ObjectMap& o = context.ContextObjects();

    if (empire_id == ALL_EMPIRES) {
        ErrorLogger() << "InvadeOrder::Check() : empire " << empire_id << " is not an empire";
        return false;
    }

    // make sure ship_id is a ship...
    auto ship = o.get<Ship>(ship_id);
    if (!ship) {
        ErrorLogger() << "IssueInvadeOrder: passed an invalid ship_id";
        return false;
    }

    if (!ship->OwnedBy(empire_id)) {
        ErrorLogger() << "IssueInvadeOrder: empire does not own passed ship";
        return false;
    }
    if (!ship->HasTroops(u)) {
        ErrorLogger() << "InvadeOrder got ship that can't invade";
        return false;
    }

    // get fleet of ship
    auto fleet = o.get<Fleet>(ship->FleetID());
    if (!fleet) {
        ErrorLogger() << "IssueInvadeOrder: ship with passed ship_id has invalid fleet_id";
        return false;
    }

    // make sure player owns ship and its fleet
    if (!fleet->OwnedBy(empire_id)) {
        ErrorLogger() << "IssueInvadeOrder: empire does not own fleet of passed ship";
        return false;
    }

    auto planet = o.get<Planet>(planet_id);
    if (!planet) {
        ErrorLogger() << "InvadeOrder couldn't get planet with id " << planet_id;
        return false;
    }

    if (ship->SystemID() != planet->SystemID()) {
        ErrorLogger() << "InvadeOrder given ids of ship and planet not in the same system";
        return false;
    }

    if (u.GetObjectVisibilityByEmpire(planet_id, empire_id) < Visibility::VIS_BASIC_VISIBILITY) {
        ErrorLogger() << "InvadeOrder given planet that empire reportedly has insufficient visibility of";
        return false;
    }

    if (planet->OwnedBy(empire_id)) {
        ErrorLogger() << "InvadeOrder given planet that is already owned by the order-issuing empire";
        return false;
    }

    if (planet->Unowned() && planet->GetMeter(MeterType::METER_POPULATION)->Initial() == 0.0f) {
        ErrorLogger() << "InvadeOrder given unpopulated planet";
        return false;
    }

    if (planet->GetMeter(MeterType::METER_SHIELD)->Initial() > 0.0f) {
        ErrorLogger() << "InvadeOrder given planet with shield > 0";
        return false;
    }

    if (!planet->Unowned() && context.ContextDiploStatus(planet->Owner(), empire_id) !=
                              DiplomaticStatus::DIPLO_WAR)
    {
        ErrorLogger() << "InvadeOrder given planet owned by an empire not at war with order-issuing empire";
        return false;
    }

    return true;
}

void InvadeOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);

    if (!Check(EmpireID(), m_ship, m_planet, context))
        return;

    ObjectMap& objects{context.ContextObjects()};
    auto ship = objects.get<Ship>(m_ship);
    auto planet = objects.get<Planet>(m_planet);

    // note: multiple ships, from same or different empires, can invade the same planet on the same turn
    DebugLogger() << "InvadeOrder::ExecuteImpl set for ship " << m_ship << " "
                  << ship->Name() << " to invade planet " << m_planet << " " << planet->Name();
    planet->SetIsAboutToBeInvaded(true);
    ship->SetInvadePlanet(m_planet);

    if (auto fleet = objects.get<Fleet>(ship->FleetID()))
        fleet->StateChangedSignal();
}

bool InvadeOrder::UndoImpl(ScriptingContext& context) const {
    ObjectMap& objects{context.ContextObjects()};

    auto planet = objects.get<Planet>(m_planet);
    if (!planet) {
        ErrorLogger() << "InvadeOrder::UndoImpl couldn't get planet with id " << m_planet;
        return false;
    }

    auto ship = objects.get<Ship>(m_ship);
    if (!ship) {
        ErrorLogger() << "InvadeOrder::UndoImpl couldn't get ship with id " << m_ship;
        return false;
    }
    if (ship->OrderedInvadePlanet() != m_planet) {
        ErrorLogger() << "InvadeOrder::UndoImpl ship is not about to invade planet";
        return false;
    }

    planet->SetIsAboutToBeInvaded(false);
    ship->ClearInvadePlanet();

    if (auto fleet = objects.get<Fleet>(ship->FleetID()))
        fleet->StateChangedSignal();

    return true;
}

////////////////////////////////////////////////
// BombardOrder
////////////////////////////////////////////////
BombardOrder::BombardOrder(int empire, int ship, int planet, const ScriptingContext& context) :
    Order(empire),
    m_ship(ship),
    m_planet(planet)
{ Check(empire, m_ship, m_planet, context); }

std::string BombardOrder::Dump() const
{ return boost::io::str(FlexibleFormat(UserString("ORDER_BOMBARD")) % m_planet % m_ship) + ExecutedTag(this); }

bool BombardOrder::Check(int empire_id, int ship_id, int planet_id,
                         const ScriptingContext& context)
{
    const Universe& universe = context.ContextUniverse();
    const ObjectMap& objects = context.ContextObjects();

    if (empire_id == ALL_EMPIRES) {
        ErrorLogger() << "BombardOrder::Check() : empire " << empire_id << " is not an empire";
        return false;
    }

    auto ship = objects.get<Ship>(ship_id);
    if (!ship) {
        ErrorLogger() << "BombardOrder::ExecuteImpl couldn't get ship with id " << ship_id;
        return false;
    }
    if (!ship->CanBombard(universe)) {
        ErrorLogger() << "BombardOrder::ExecuteImpl got ship that can't bombard";
        return false;
    }
    if (!ship->OwnedBy(empire_id)) {
        ErrorLogger() << "BombardOrder::ExecuteImpl got ship that isn't owned by the order-issuing empire";
        return false;
    }

    auto planet = objects.get<Planet>(planet_id);
    if (!planet) {
        ErrorLogger() << "BombardOrder::ExecuteImpl couldn't get planet with id " << planet_id;
        return false;
    }
    if (planet->OwnedBy(empire_id)) {
        ErrorLogger() << "BombardOrder::ExecuteImpl given planet that is already owned by the order-issuing empire";
        return false;
    }
    if (!planet->Unowned() && context.ContextDiploStatus(planet->Owner(), empire_id) != DiplomaticStatus::DIPLO_WAR) {
        ErrorLogger() << "BombardOrder::ExecuteImpl given planet owned by an empire not at war with order-issuing empire";
        return false;
    }
    if (universe.GetObjectVisibilityByEmpire(planet_id, empire_id) < Visibility::VIS_BASIC_VISIBILITY) {
        ErrorLogger() << "BombardOrder::ExecuteImpl given planet that empire reportedly has insufficient visibility of, but will be allowed to proceed pending investigation";
    }

    int ship_system_id = ship->SystemID();
    if (ship_system_id == INVALID_OBJECT_ID) {
        ErrorLogger() << "BombardOrder::ExecuteImpl given id of ship not in a system";
        return false;
    }
    int planet_system_id = planet->SystemID();
    if (ship_system_id != planet_system_id) {
        ErrorLogger() << "BombardOrder::ExecuteImpl given ids of ship and planet not in the same system";
        return false;
    }

    return true;
}

void BombardOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);

    if (!Check(EmpireID(), m_ship, m_planet, context))
        return;

    ObjectMap& objects{context.ContextObjects()};
    auto ship = objects.get<Ship>(m_ship);
    auto planet = objects.get<Planet>(m_planet);

    // note: multiple ships, from same or different empires, can bombard the same planet on the same turn
    DebugLogger() << "BombardOrder::ExecuteImpl set for ship " << m_ship << " "
                  << ship->Name() << " to bombard planet " << m_planet << " "
                  << planet->Name();
    planet->SetIsAboutToBeBombarded(true);
    ship->SetBombardPlanet(m_planet);

    if (auto fleet = objects.get<Fleet>(ship->FleetID()))
        fleet->StateChangedSignal();
}

bool BombardOrder::UndoImpl(ScriptingContext& context) const {
    ObjectMap& objects{context.ContextObjects()};

    auto planet = objects.get<Planet>(m_planet);
    if (!planet) {
        ErrorLogger() << "BombardOrder::UndoImpl couldn't get planet with id " << m_planet;
        return false;
    }

    auto ship = objects.get<Ship>(m_ship);
    if (!ship) {
        ErrorLogger() << "BombardOrder::UndoImpl couldn't get ship with id " << m_ship;
        return false;
    }
    if (ship->OrderedBombardPlanet() != m_planet) {
        ErrorLogger() << "BombardOrder::UndoImpl ship is not about to bombard planet";
        return false;
    }

    planet->SetIsAboutToBeBombarded(false);
    ship->ClearBombardPlanet();

    if (auto fleet = objects.get<Fleet>(ship->FleetID()))
        fleet->StateChangedSignal();

    return true;
}

////////////////////////////////////////////////
// ChangeFocusOrder
////////////////////////////////////////////////
ChangeFocusOrder::ChangeFocusOrder(int empire, int planet, std::string focus, const ScriptingContext& context) :
    Order(empire),
    m_planet(planet),
    m_focus(std::move(focus))
{ Check(empire, m_planet, m_focus, context); }

std::string ChangeFocusOrder::Dump() const
{ return boost::io::str(FlexibleFormat(UserString("ORDER_FOCUS_CHANGE")) % m_planet % m_focus) + ExecutedTag(this); }

bool ChangeFocusOrder::Check(int empire_id, int planet_id, const std::string& focus,
                             const ScriptingContext& context)
{
    auto planet = context.ContextObjects().getRaw<Planet>(planet_id);

    if (!planet) {
        ErrorLogger() << "Invalid planet id " << planet_id << " specified in change planet focus order.";
        return false;
    }

    if (!planet->OwnedBy(empire_id)) {
        ErrorLogger() << "Empire " << empire_id
                      << " attempted to issue change planet focus to another's planet: " << planet_id;
        return false;
    }

    if (!planet->FocusAvailable(focus, context)) {
        ErrorLogger() << "IssueChangeFocusOrder : invalid focus (" << focus
                      << ") for specified for planet " << planet_id << " and empire " << empire_id;
        // TODO: further clarify why invalid? get species and check that it has the focus, and then
        //       if the location condition fails?
        return false;
    }

    return true;
}

void ChangeFocusOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);

    if (!Check(EmpireID(), m_planet, m_focus, context))
        return;

    auto planet = context.ContextObjects().getRaw<Planet>(m_planet);

    planet->SetFocus(m_focus, context);
}

////////////////////////////////////////////////
// PolicyOrder
////////////////////////////////////////////////
std::string PolicyOrder::Dump() const {
    const auto& template_str = m_adopt ? UserString("ORDER_POLICY_ADOPT") : UserString("ORDER_POLICY_ABANDON");
    return boost::io::str(FlexibleFormat(template_str)
                          % m_policy_name % m_category % m_slot) + ExecutedTag(this);
}

void PolicyOrder::ExecuteImpl(ScriptingContext& context) const {
    auto empire = GetValidatedEmpire(context);
    if (m_adopt) {
        DebugLogger() << "PolicyOrder adopt " << m_policy_name << " in category " << m_category
                      << " in slot " << m_slot;
        empire->AdoptPolicy(m_policy_name, m_category, context, m_slot);
    } else if (!m_revert) {
        DebugLogger() << "PolicyOrder de-adopt " << m_policy_name;
        empire->DeAdoptPolicy(m_policy_name);
    } else {
        empire->RevertPolicies();
    }
}

////////////////////////////////////////////////
// ResearchQueueOrder
////////////////////////////////////////////////
ResearchQueueOrder::ResearchQueueOrder(int empire, std::string tech_name) :
    Order(empire),
    m_tech_name(std::move(tech_name)),
    m_remove(true)
{}

ResearchQueueOrder::ResearchQueueOrder(int empire, std::string tech_name, int position) :
    Order(empire),
    m_tech_name(std::move(tech_name)),
    m_position(position)
{}

ResearchQueueOrder::ResearchQueueOrder(int empire, std::string tech_name, bool pause, float dummy) :
    Order(empire),
    m_tech_name(std::move(tech_name)),
    m_pause(pause ? PAUSE : RESUME)
{}

std::string ResearchQueueOrder::Dump() const {
    const auto& template_str = [this]() -> const auto& {
        if (m_remove)
            return UserString("ORDER_RESEARCH_REMOVE");
        else if (m_pause == PAUSE)
            return UserString("ORDER_RESEARCH_PAUSE");
        else if (m_pause == RESUME)
            return UserString("ORDER_RESEARCH_RESUME");
        else
            return UserString("ORDER_RESEARCH_ENQUEUE_AT");
    }();

    const auto& tech_name = UserStringExists(m_tech_name) ? UserString(m_tech_name) : m_tech_name;
    return boost::io::str(FlexibleFormat(template_str) % tech_name % m_position) + ExecutedTag(this);
}

void ResearchQueueOrder::ExecuteImpl(ScriptingContext& context) const {
    auto empire = GetValidatedEmpire(context);

    if (m_remove) {
        DebugLogger() << "ResearchQueueOrder::ExecuteImpl: removing from queue tech: " << m_tech_name;
        empire->RemoveTechFromQueue(m_tech_name);
    } else if (m_pause == PAUSE) {
        DebugLogger() << "ResearchQueueOrder::ExecuteImpl: pausing tech: " << m_tech_name;
        empire->PauseResearch(m_tech_name);
    } else if (m_pause == RESUME) {
        DebugLogger() << "ResearchQueueOrder::ExecuteImpl: unpausing tech: " << m_tech_name;
        empire->ResumeResearch(m_tech_name);
    } else if (m_position != INVALID_INDEX) {
        DebugLogger() << "ResearchQueueOrder::ExecuteImpl: adding tech to queue: " << m_tech_name;
        empire->PlaceTechInQueue(m_tech_name, m_position);
    } else {
        ErrorLogger() << "ResearchQueueOrder::ExecuteImpl: Malformed";
    }
}

////////////////////////////////////////////////
// ProductionQueueOrder
////////////////////////////////////////////////
ProductionQueueOrder::ProductionQueueOrder(ProdQueueOrderAction action, int empire,
                                           ProductionQueue::ProductionItem item,
                                           int number, int location, int pos) :
    Order(empire),
    m_item(std::move(item)),
    m_location(location),
    m_new_quantity(number),
    m_new_index(pos),
    m_uuid(boost::uuids::random_generator()()),
    m_action(action)
{
    if (action != ProdQueueOrderAction::PLACE_IN_QUEUE)
        ErrorLogger() << "ProductionQueueOrder called with parameters for placing in queue but with another action";
}

ProductionQueueOrder::ProductionQueueOrder(ProdQueueOrderAction action, int empire,
                                           boost::uuids::uuid uuid, int num1, int num2) :
    Order(empire),
    m_uuid(uuid),
    m_action(action)
{
    switch(m_action) {
    case ProdQueueOrderAction::REMOVE_FROM_QUEUE:
    case ProdQueueOrderAction::UNREMOVE_FROM_QUEUE:
        break;
    case ProdQueueOrderAction::SPLIT_INCOMPLETE:
    case ProdQueueOrderAction::DUPLICATE_ITEM:
        m_uuid2 = boost::uuids::random_generator()();
        break;
    case ProdQueueOrderAction::SET_QUANTITY_AND_BLOCK_SIZE:
        m_new_quantity = num1;
        m_new_blocksize = num2;
        break;
    case ProdQueueOrderAction::SET_QUANTITY:
        m_new_quantity = num1;
        break;
    case ProdQueueOrderAction::MOVE_ITEM_TO_INDEX:
        m_new_index = num1;
        break;
    case ProdQueueOrderAction::SET_RALLY_POINT:
        m_rally_point_id = num1;
        break;
    case ProdQueueOrderAction::PAUSE_PRODUCTION:
    case ProdQueueOrderAction::RESUME_PRODUCTION:
    case ProdQueueOrderAction::ALLOW_STOCKPILE_USE:
    case ProdQueueOrderAction::DISALLOW_STOCKPILE_USE:
        break;
    default:
        ErrorLogger() << "ProductionQueueOrder given unrecognized action!";
    }
}

std::string ProductionQueueOrder::Dump() const
{ return UserString("ORDER_PRODUCTION"); }

void ProductionQueueOrder::ExecuteImpl(ScriptingContext& context) const {
    try {
        auto empire = GetValidatedEmpire(context);

        switch(m_action) {
        case ProdQueueOrderAction::PLACE_IN_QUEUE: {
            if (m_item.build_type == BuildType::BT_BUILDING ||
                m_item.build_type == BuildType::BT_SHIP ||
                m_item.build_type == BuildType::BT_STOCKPILE)
            {
                DebugLogger() << "ProductionQueueOrder place in queue: " << m_item.Dump()
                              << "  at index: " << m_new_index;
                empire->PlaceProductionOnQueue(m_item, m_uuid, context, m_new_quantity, 1, m_location, m_new_index);
            } else {
                ErrorLogger() << "ProductionQueueOrder tried to place invalid build type in queue!";
            }
            break;
        }
        case ProdQueueOrderAction::REMOVE_FROM_QUEUE: {
            const auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to remove invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder removing item at index: " << idx;
                empire->MarkToBeRemoved(idx);
            }
            break;
        }
        case ProdQueueOrderAction::UNREMOVE_FROM_QUEUE: {
            const auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to unremove invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder unremoving item at index: " << idx;
                empire->MarkNotToBeRemoved(idx);
            }
            break;
        }
        case ProdQueueOrderAction::SPLIT_INCOMPLETE: {
            auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to split invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder splitting incomplete from item";
                empire->SplitIncompleteProductionItem(idx, m_uuid2, context);
            }
            break;
        }
        case ProdQueueOrderAction::DUPLICATE_ITEM: {
            auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to duplicate invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder duplicating item";
                empire->DuplicateProductionItem(idx, m_uuid2, context);
            }
            break;
        }
        case ProdQueueOrderAction::SET_QUANTITY_AND_BLOCK_SIZE: {
            auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to set quantity and blocksize of invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder setting quantity and block size";
                empire->SetProductionQuantityAndBlocksize(idx, m_new_quantity, m_new_blocksize);
            }
            break;
        }
        case ProdQueueOrderAction::SET_QUANTITY: {
            auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to set quantity of invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder setting quantity " << m_new_quantity;
                empire->SetProductionQuantity(idx, m_new_quantity);
            }
            break;
        }
        case ProdQueueOrderAction::MOVE_ITEM_TO_INDEX: {
            auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to move invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder moving to index " << m_new_index;
                empire->MoveProductionWithinQueue(idx, m_new_index);
            }
            break;
        }
        case ProdQueueOrderAction::SET_RALLY_POINT: {
            auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to set rally point of invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder setting rally point to " << m_rally_point_id;
                empire->SetProductionRallyPoint(idx, m_rally_point_id);
            }
            break;
        }
        case ProdQueueOrderAction::PAUSE_PRODUCTION: {
            auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to pause invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder pausing";
                empire->PauseProduction(idx);
            }
            break;
        }
        case ProdQueueOrderAction::RESUME_PRODUCTION: {
            auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to resume invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder resuming";
                empire->ResumeProduction(idx);
            }
            break;
        }
        case ProdQueueOrderAction::ALLOW_STOCKPILE_USE: {
            auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to allow stockpiling on invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder allowing stockpile";
                empire->AllowUseImperialPP(idx, true);
            }
            break;
        }
        case ProdQueueOrderAction::DISALLOW_STOCKPILE_USE: {
            auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to disallow stockpiling on invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder disallowing stockpile";
                empire->AllowUseImperialPP(idx, false);
            }
            break;
        }
        default:
            ErrorLogger() << "ProductionQueueOrder::ExecuteImpl got invalid action";
        }
    } catch (const std::exception& e) {
        ErrorLogger() << "Production order execution threw exception: " << e.what();
        throw;
    }
}

////////////////////////////////////////////////
// ShipDesignOrder
////////////////////////////////////////////////
ShipDesignOrder::ShipDesignOrder(int empire, int existing_design_id_to_remember,
                                 const ScriptingContext& context) :
    Order(empire),
    m_design_id(existing_design_id_to_remember)
{ CheckRemember(empire, m_design_id, context); }

ShipDesignOrder::ShipDesignOrder(int empire, int design_id_to_erase, bool dummy,
                                 const ScriptingContext& context) :
    Order(empire),
    m_design_id(design_id_to_erase),
    m_delete_design_from_empire(true)
{ CheckErase(empire, m_design_id, m_delete_design_from_empire, context); }

ShipDesignOrder::ShipDesignOrder(int empire, const ShipDesign& ship_design,
                                 const ScriptingContext& context) :
    Order(empire),
    m_uuid(ship_design.UUID()),
    m_name(ship_design.Name(false)),
    m_description(ship_design.Description(false)),
    m_hull(ship_design.Hull()),
    m_parts(ship_design.Parts()),
    m_icon(ship_design.Icon()),
    m_3D_model(ship_design.Model()),
    m_design_id(INVALID_DESIGN_ID),
    m_designed_on_turn(ship_design.DesignedOnTurn()),
    m_create_new_design(true),
    m_is_monster(ship_design.IsMonster()),
    m_name_desc_in_stringtable(ship_design.LookupInStringtable())
{ CheckNew(empire, m_name, m_description, m_hull, m_parts, context); }

ShipDesignOrder::ShipDesignOrder(int empire, int existing_design_id,
                                 std::string new_name, std::string new_description,
                                 const ScriptingContext& context) :
    Order(empire),
    m_name(std::move(new_name)),
    m_description(std::move(new_description)),
    m_design_id(existing_design_id),
    m_update_name_or_description(true)
{ CheckRename(empire, m_design_id, m_name, m_description, context); }

std::string ShipDesignOrder::Dump() const
{ return UserString("ORDER_SHIP_DESIGN"); }

void ShipDesignOrder::ExecuteImpl(ScriptingContext& context) const {
    auto empire = GetValidatedEmpire(context);

    Universe& universe = context.ContextUniverse();

    if (m_delete_design_from_empire) {
        if (!CheckErase(EmpireID(), m_design_id, m_delete_design_from_empire, context))
            return;

        empire->RemoveShipDesign(m_design_id);

    } else if (m_create_new_design) {
        if (!CheckNew(EmpireID(), m_name, m_description, m_hull, m_parts, context))
            return;

        try {
            ShipDesign new_ship_design(std::invalid_argument(""), m_name, m_description,
                                       m_designed_on_turn, EmpireID(), m_hull, m_parts,
                                       m_icon, m_3D_model, m_name_desc_in_stringtable,
                                       m_is_monster, m_uuid);

            if (m_design_id == INVALID_DESIGN_ID) {
                // On the client create a new design id
                m_design_id = universe.InsertShipDesign(std::move(new_ship_design));
                DebugLogger() << "ShipDesignOrder::ExecuteImpl inserted new ship design ID " << m_design_id;

            } else {
                // On the server use the design id passed from the client
                const auto success = universe.InsertShipDesignID(std::move(new_ship_design), EmpireID(),
                                                                 m_design_id);
                if (!success) {
                    ErrorLogger() << "Couldn't insert ship design by ID " << m_design_id;
                    return;
                }
            }

            universe.SetEmpireKnowledgeOfShipDesign(m_design_id, EmpireID());
            empire->AddShipDesign(m_design_id, universe);


        } catch (const std::exception& e) {
            ErrorLogger() << "Couldn't create ship design: " << e.what();
            return;
        }

    } else if (m_update_name_or_description) {
        if (!CheckRename(EmpireID(), m_design_id, m_name, m_description, context))
            return;

        universe.RenameShipDesign(m_design_id, m_name, m_description);

    } else {
        // player is ordering empire to retain a particular design, so that is can
        // be used to construct ships by that empire.
        if (!CheckRemember(EmpireID(), m_design_id, context))
            return;
        empire->AddShipDesign(m_design_id, universe);

        // TODO: consider removing this order, so that an empire needs to use
        // espionage or influence to gain access to a ship design made by another
        // player
    }
}

bool ShipDesignOrder::CheckRemember(int empire_id, int existing_design_id_to_remember,
                                    const ScriptingContext& context)
{
    const auto empire = context.GetEmpire(empire_id);
    if (!empire) {
        ErrorLogger() << "ShipDesignOrder : given invalid empire id";
        return false;
    }

    // check if empire is already remembering the design
    if (empire->ShipDesignKept(existing_design_id_to_remember)) {
        ErrorLogger() << "Empire " << empire_id
                      << " tried to remember a ShipDesign id = " << existing_design_id_to_remember
                      << " that was already being remembered";
        return false;
    }

    // check if the empire can see any objects that have this design (thus enabling it to be copied)
    auto& empire_known_design_ids = context.ContextUniverse().EmpireKnownShipDesignIDs(empire_id);
    if (!empire_known_design_ids.contains(existing_design_id_to_remember)) {
        ErrorLogger() << "Empire " << empire_id
                      << " tried to remember a ShipDesign id = " << existing_design_id_to_remember
                      << " that this empire hasn't seen";
        return false;
    }

    return true;
}

bool ShipDesignOrder::CheckErase(int empire_id, int design_id_to_erase, bool dummy,
                                 const ScriptingContext& context)
{
    const auto empire = context.GetEmpire(empire_id);
    if (!empire) {
        ErrorLogger() << "ShipDesignOrder : given invalid empire id";
        return false;
    }

    // player is ordering empire to forget about a particular design
    if (!empire->ShipDesignKept(design_id_to_erase)) {
        ErrorLogger() << "Empire " << empire_id << " tried to remove a ShipDesign id = " << design_id_to_erase
                      << " that the empire wasn't remembering";
        return false;
    }

    return true;
}

bool ShipDesignOrder::CheckNew(int empire_id, const std::string& name, const std::string& desc,
                               const std::string& hull, const std::vector<std::string>& parts,
                               const ScriptingContext& context)
{
    const auto empire = context.GetEmpire(empire_id);
    if (!empire) {
        ErrorLogger() << "ShipDesignOrder : given invalid empire id";
        return false;
    }

    // TODO: check hull and parts

    return true;
}

bool ShipDesignOrder::CheckRename(int empire_id, int existing_design_id, const std::string& new_name,
                                  const std::string& new_description, const ScriptingContext& context)
{
    const auto empire = context.GetEmpire(empire_id);
    if (!empire) {
        ErrorLogger() << "ShipDesignOrder : given invalid empire id";
        return false;
    }

    const auto& universe = context.ContextUniverse();
    // check if a design with this ID exists
    auto existing = universe.GetShipDesign(existing_design_id);
    if (!existing) {
        ErrorLogger() << "Empire " << empire_id
                      << " tried to rename a ShipDesign with an id, " << existing_design_id
                      << " that does not exist";
        return false;
    }

    // player is ordering empire to rename a design
    const auto& empire_known_design_ids = universe.EmpireKnownShipDesignIDs(empire_id);
    auto design_it = empire_known_design_ids.find(existing_design_id);
    if (design_it == empire_known_design_ids.end()) {
        ErrorLogger() << "Empire " << empire_id
                      << " tried to rename/redescribe a ShipDesign id = " << existing_design_id
                      << " that this empire hasn't seen";
        return false;
    }
    const ShipDesign* design = universe.GetShipDesign(*design_it);
    if (!design) {
        ErrorLogger() << "Empire " << empire_id
                      << " tried to rename/redescribe a ShipDesign id = " << existing_design_id
                      << " that doesn't exist (but this empire has seen it)!";
        return false;
    }
    if (design->DesignedByEmpire() != empire_id) {
        ErrorLogger() << "Empire " << empire_id
                      << " tried to rename/redescribe a ShipDesign id = " << existing_design_id
                      << " that isn't owned by this empire!";
        return false;
    }

    return true;
}

////////////////////////////////////////////////
// ScrapOrder
////////////////////////////////////////////////
ScrapOrder::ScrapOrder(int empire, int object_id, const ScriptingContext& context) :
    Order(empire),
    m_object_id(object_id)
{ Check(empire, object_id, context); }

std::string ScrapOrder::Dump() const
{ return UserString("ORDER_SCRAP"); }

bool ScrapOrder::Check(int empire_id, int object_id, const ScriptingContext& context) {
    auto obj = context.ContextObjects().get(object_id);

    if (!obj) {
        ErrorLogger() << "IssueScrapOrder : passed an invalid object_id";
        return false;
    }

    if (!obj->OwnedBy(empire_id)) {
        ErrorLogger() << "IssueScrapOrder : passed object_id of object not owned by player";
        return false;
    }

    if (obj->ObjectType() != UniverseObjectType::OBJ_SHIP && obj->ObjectType() != UniverseObjectType::OBJ_BUILDING) {
        ErrorLogger() << "ScrapOrder::Check : passed object that is not a ship or building";
        return false;
    }

    auto ship = context.ContextObjects().get<Ship>(object_id);
    if (ship && ship->SystemID() == INVALID_OBJECT_ID)
        ErrorLogger() << "ScrapOrder::Check : can scrap a traveling ship";

    return true;
}

void ScrapOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);

    if (!Check(EmpireID(), m_object_id, context))
        return;

    ObjectMap& objects{context.ContextObjects()};

    if (auto ship = objects.get<Ship>(m_object_id)) {
        ship->SetOrderedScrapped(true);
    } else if (auto building = objects.get<Building>(m_object_id)) {
        building->SetOrderedScrapped(true);
    }
}

bool ScrapOrder::UndoImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);
    int empire_id = EmpireID();

    ObjectMap& objects{context.ContextObjects()};

    if (auto ship = objects.get<Ship>(m_object_id)) {
        if (ship->OwnedBy(empire_id))
            ship->SetOrderedScrapped(false);
    } else if (auto building = objects.get<Building>(m_object_id)) {
        if (building->OwnedBy(empire_id))
            building->SetOrderedScrapped(false);
    } else {
        return false;
    }
    return true;
}

////////////////////////////////////////////////
// AggressiveOrder
////////////////////////////////////////////////
AggressiveOrder::AggressiveOrder(int empire, int object_id, FleetAggression aggression,
                                 const ScriptingContext& context) :
    Order(empire),
    m_object_id(object_id),
    m_aggression(aggression)
{ Check(empire, object_id, m_aggression, context); }

std::string AggressiveOrder::Dump() const
{ return UserString("ORDER_FLEET_AGGRESSION"); }

bool AggressiveOrder::Check(int empire_id, int object_id, FleetAggression aggression,
                            const ScriptingContext& context)
{
    const ObjectMap& objects{context.ContextObjects()};

    auto fleet = objects.get<Fleet>(object_id);
    if (!fleet) {
        ErrorLogger() << "IssueAggressionOrder : no fleet with passed id";
        return false;
    }

    if (!fleet->OwnedBy(empire_id)) {
        ErrorLogger() << "IssueAggressionOrder : passed object_id of object not owned by player";
        return false;
    }

    return true;
}

void AggressiveOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);

    if (!Check(EmpireID(), m_object_id, m_aggression, context))
        return;

    if (auto fleet = context.ContextObjects().get<Fleet>(m_object_id))
        fleet->SetAggression(m_aggression);
    else
        ErrorLogger() << "AggressiveOrder::ExecuteImpl couldn't find fleet with id " << m_object_id;
}

/////////////////////////////////////////////////////
// GiveObjectToEmpireOrder
/////////////////////////////////////////////////////
GiveObjectToEmpireOrder::GiveObjectToEmpireOrder(int empire, int object_id, int recipient,
                                                 const ScriptingContext& context) :
    Order(empire),
    m_object_id(object_id),
    m_recipient_empire_id(recipient)
{ Check(empire, object_id, recipient, context); }

std::string GiveObjectToEmpireOrder::Dump() const
{ return UserString("ORDER_GIVE_TO_EMPIRE"); }

bool GiveObjectToEmpireOrder::Check(int empire_id, int object_id, int recipient_empire_id,
                                    const ScriptingContext& context)
{
    if (!context.GetEmpire(recipient_empire_id)) {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : given invalid recipient empire id";
        return false;
    }
    const auto giver_empire = context.GetEmpire(empire_id);
    if (!giver_empire) {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : given invalid giver empire id";
        return false;
    }
    if (giver_empire->CapitalID() == object_id) {
        ErrorLogger() << "IssueGiverObjectToEmpireOrder : giving away capital not allowed";
        return false;
    }


    const auto dip = context.ContextDiploStatus(empire_id, recipient_empire_id);
    if (dip < DiplomaticStatus::DIPLO_PEACE) {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : attempting to give to empire not at peace";
        return false;
    }

    const ObjectMap& objects{context.ContextObjects()};

    const auto obj = objects.get(object_id);
    if (!obj) {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : passed invalid object id";
        return false;
    }

    if (!obj->OwnedBy(empire_id)) {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : passed object not owned by player";
        return false;
    }

    const auto system = objects.get<System>(obj->SystemID());
    if (!system) {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : couldn't get system of object";
        return false;
    }

    if (obj->ObjectType() != UniverseObjectType::OBJ_FLEET &&
        obj->ObjectType() != UniverseObjectType::OBJ_PLANET)
    {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : passed object that is not a fleet or planet";
        return false;
    }

    const auto system_objects = objects.findRaw<const UniverseObject>(system->ObjectIDs());
    if (!std::any_of(system_objects.begin(), system_objects.end(),
                     [recipient_empire_id](const auto& o){ return o->Owner() == recipient_empire_id; }))
    {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : recipient empire has nothing in system";
        return false;
    }

    return true;
}

void GiveObjectToEmpireOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);

    if (!Check(EmpireID(), m_object_id, m_recipient_empire_id, context))
        return;

    if (auto fleet = context.ContextObjects().get<Fleet>(m_object_id))
        fleet->SetGiveToEmpire(m_recipient_empire_id);
    else if (auto planet = context.ContextObjects().get<Planet>(m_object_id))
        planet->SetGiveToEmpire(m_recipient_empire_id);
}

bool GiveObjectToEmpireOrder::UndoImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);
    int empire_id = EmpireID();

    ObjectMap& objects{context.ContextObjects()};

    if (auto fleet = objects.get<Fleet>(m_object_id)) {
        if (fleet->OwnedBy(empire_id)) {
            fleet->ClearGiveToEmpire();
            return true;
        }
    } else if (auto planet = objects.get<Planet>(m_object_id)) {
        if (planet->OwnedBy(empire_id)) {
            planet->ClearGiveToEmpire();
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////
// ForgetOrder
////////////////////////////////////////////////
ForgetOrder::ForgetOrder(int empire, int object_id) :
    Order(empire),
    m_object_id(object_id)
{}

std::string ForgetOrder::Dump() const
{ return UserString("ORDER_FORGET"); }

void ForgetOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);
    int empire_id = EmpireID();

    DebugLogger() << "ForgetOrder::ExecuteImpl empire: " << empire_id
                  << " for object: " << m_object_id;

    context.ContextUniverse().ForgetKnownObject(empire_id, m_object_id);
}
