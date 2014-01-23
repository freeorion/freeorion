#include "FleetWnd.h"

#include "CUIControls.h"
#include "SidePanel.h"
#include "InfoPanels.h"
#include "ClientUI.h"
#include "Sound.h"
#include "ShaderProgram.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/Order.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "../util/ScopedTimer.h"
#include "../client/human/HumanClientApp.h"
#include "../universe/Fleet.h"
#include "../universe/Planet.h"
#include "../universe/Predicates.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../network/Message.h"
#include "../Empire/Empire.h"

#include <GG/DrawUtil.h>
#include <GG/Menu.h>
#include <GG/Layout.h>
#include <GG/StaticGraphic.h>
#include <GG/TextControl.h>

#include <boost/cast.hpp>

namespace {
    const GG::Pt        DATA_PANEL_ICON_SPACE = GG::Pt(GG::X(38), GG::Y(38));   // area reserved for ship or fleet icon in data panels (actual height can be bigger if the row expands due to font size)

    // how should ship and fleet icons be scaled and/or positioned in the reserved space
    const GG::Flags<GG::GraphicStyle>   DataPanelIconStyle()
    { return GG::GRAPHIC_CENTER | GG::GRAPHIC_VCENTER | GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE; }

    const GG::X         DATA_PANEL_TEXT_PAD = GG::X(4); // padding on the left and right of fleet/ship description
    const int           DATA_PANEL_BORDER = 1;          // how thick should the border around ship or fleet panel be
    const int           PAD = 4;
    const std::string   SHIP_DROP_TYPE_STRING = "FleetWnd ShipRow";
    const std::string   FLEET_DROP_TYPE_STRING = "FleetWnd FleetRow";

    const std::string   COUNT_STAT_STRING = "Count Stat";
    const std::string   DAMAGE_STAT_STRING = "Damage Stat";
    const std::string   SHIELD_STAT_STRING = "Sheild Stat";
    const std::string   STRUCTURE_STAT_STRING = "Structure Stat";
    const std::string   SPEED_STAT_STRING = "Speed Stat";
    const std::string   COLONY_CAPACITY_STAT_STRING = "Colony Capacity";
    const std::string   TROOP_CAPACITY_STAT_STRING = "Troop Capacity";
    const std::string   MeterStatString(MeterType meter_type) {
        std::string retval = GG::GetEnumMap<MeterType>().FromEnum(meter_type);
        //std::cout << "MeterStatString for meter type " << boost::lexical_cast<std::string>(meter_type) << " returning: " << retval << std::endl;
        return retval;
    }

    MeterType           MeterTypeFromStatString(const std::string& stat_name) {
        //std::cout << "MeterTypeFromStatString passed stat_name " << stat_name << std::endl;
        for (MeterType meter_type = MeterType(0); meter_type != NUM_METER_TYPES; meter_type = MeterType(meter_type + 1)) {
            std::string meter_name_string = MeterStatString(meter_type);
            //std::cout << " ... comparing to meter name string " << meter_name_string << " which was derived from meter type " << boost::lexical_cast<std::string>(meter_type) << std::endl;
            if (meter_name_string == stat_name) {
                //std::cout << " ... ... match! returning " << boost::lexical_cast<std::string>(meter_type) << std::endl;
                return meter_type;
            }
        }
        //std::cout << " ... didn't find match.  returning INVALID_METER_TYPE" << std::endl;
        return INVALID_METER_TYPE;
    }

    GG::Y LabelHeight()
    { return GG::Y(ClientUI::Pts()*3/2); }

    /** How big fleet and ship statistics icons should be relative to the
      * current font size.  Icons shouldn't scale below what they are for the
      * default, 12 pt, font size. */
    GG::Pt StatIconSize() {
        const int font_size = std::max(ClientUI::Pts(), 12);
        return GG::Pt(GG::X(font_size*11/3), GG::Y(font_size*4/3));
    }

    GG::Y ListRowHeight()
    { return std::max(DATA_PANEL_ICON_SPACE.y, LabelHeight() + StatIconSize().y) + 2*DATA_PANEL_BORDER + PAD; }

    boost::shared_ptr<GG::Texture> SpeedIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "meter" / "speed.png", true); }

    boost::shared_ptr<GG::Texture> DamageIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "meter" / "damage.png", true); }

    boost::shared_ptr<GG::Texture> FleetCountIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "sitrep" / "fleet_arrived.png"); }

    std::string FleetDestinationText(int fleet_id) {
        std::string retval = "";
        TemporaryPtr<const Fleet> fleet = GetFleet(fleet_id);
        if (!fleet)
            return retval;

        int empire_id = HumanClientApp::GetApp()->EmpireID();

        TemporaryPtr<const System> dest = GetSystem(fleet->FinalDestinationID());
        TemporaryPtr<const System> cur_sys = GetSystem(fleet->SystemID());
        if (dest && dest != cur_sys) {
            std::pair<int, int> eta = fleet->ETA();       // .first is turns to final destination.  .second is turns to next system on route

            // name of final destination
            const std::string& dest_name = dest->ApparentName(empire_id);

            // next system on path
            std::string next_eta_text;
            if (eta.second == Fleet::ETA_UNKNOWN)
                next_eta_text = UserString("FW_FLEET_ETA_UNKNOWN");
            else if (eta.second == Fleet::ETA_NEVER)
                next_eta_text = UserString("FW_FLEET_ETA_NEVER");
            else if (eta.second == Fleet::ETA_OUT_OF_RANGE)
                next_eta_text = UserString("FW_FLEET_ETA_OUT_OF_RANGE");
            else
                next_eta_text = boost::lexical_cast<std::string>(eta.second);

            // final destination
            std::string final_eta_text;
            if (eta.first == Fleet::ETA_UNKNOWN)
                final_eta_text = UserString("FW_FLEET_ETA_UNKNOWN");
            else if (eta.first == Fleet::ETA_NEVER)
                final_eta_text = UserString("FW_FLEET_ETA_NEVER");
            else if (eta.first == Fleet::ETA_OUT_OF_RANGE)
                final_eta_text = UserString("FW_FLEET_ETA_OUT_OF_RANGE");
            else
                final_eta_text = boost::lexical_cast<std::string>(eta.first);

            if(ClientUI::GetClientUI()->GetMapWnd()->IsFleetExploring(fleet->ID()))
                retval = boost::io::str(FlexibleFormat(UserString("FW_FLEET_EXPLORING_TO")) %
                                                dest_name % final_eta_text % next_eta_text);
            else
                retval = boost::io::str(FlexibleFormat(UserString("FW_FLEET_MOVING_TO")) % 
                                                dest_name % final_eta_text % next_eta_text);

        } else if (cur_sys) {
            const std::string& cur_system_name = cur_sys->ApparentName(empire_id);
            if(ClientUI::GetClientUI()->GetMapWnd()->IsFleetExploring(fleet->ID())){ 
                if(fleet->Fuel() == fleet->MaxFuel())
                    retval = boost::io::str(FlexibleFormat(UserString("FW_FLEET_EXPLORING_WAITING")));
                else
                    retval = boost::io::str(FlexibleFormat(UserString("FW_FLEET_EXPLORING_REFUEL")));
            } else {
                retval = boost::io::str(FlexibleFormat(UserString("FW_FLEET_HOLDING_AT")) % cur_system_name);
            }
        }
        return retval;
    }

    bool ClientPlayerIsModerator()
    { return HumanClientApp::GetApp()->GetClientType() == Networking::CLIENT_TYPE_HUMAN_MODERATOR; }

    void CreateNewFleetFromShips(const std::vector<int>& ship_ids, bool aggressive = false) {
        Logger().debugStream() << "CreateNewFleetFromShips with " << ship_ids.size()
                               << " ship ids (" << (aggressive ? "Aggressive" : "Passive") << ")";
        if (ship_ids.empty())
            return;
        if (ClientPlayerIsModerator())
            return; // todo: handle moderator actions for this...
        int empire_id = HumanClientApp::GetApp()->EmpireID();
        if (empire_id == ALL_EMPIRES)
            return;


        ScopedTimer new_fleet_creation_timer("CreateNewFleetFromShips with " + boost::lexical_cast<std::string>(ship_ids.size()) + " ship ids");
        Sound::TempUISoundDisabler sound_disabler;


        // get system where new fleet is to be created.
        int first_ship_id = *ship_ids.begin();
        TemporaryPtr<const Ship> first_ship = GetShip(first_ship_id);
        if (!first_ship) {
            Logger().errorStream() << "CreateNewFleetFromShips couldn't get ship with id " << first_ship_id;
            return;
        }
        int system_id = first_ship->SystemID();
        TemporaryPtr<const System> system = GetSystem(system_id); // may be null
        if (!system) {
            Logger().errorStream() << "CreateNewFleetFromShips couldn't get a valid system with system id " << system_id;
            return;
        }

        double X = first_ship->X(), Y = first_ship->Y();


        // verify that all fleets are at the same system and position and owned
        // by the same empire.  also collect all fleet ids from which ships are
        // being taken
        std::vector<int> original_fleet_ids;
        std::vector<TemporaryPtr<Ship> > ships = Objects().FindObjects<Ship>(ship_ids);
        for (std::vector<TemporaryPtr<Ship> >::const_iterator it = ships.begin();
            it != ships.end(); ++it)
        {
            TemporaryPtr<const Ship> ship = *it;

            if (ship->SystemID() != system_id) {
                Logger().errorStream() << "CreateNewFleetFromShips passed ships with inconsistent system ids";
                return;
            }
            if (ship->X() != X || ship->Y() != Y) {
                Logger().errorStream() << "CreateNewFleetFromShips passed ship and system with inconsistent locations";
                return;
            }
            if (!ship->OwnedBy(empire_id)) {
                Logger().errorStream() << "CreateNewFleetFromShips passed ships not owned by this client's empire";
                return;
            }
            int fleet_id = ship->FleetID();
            if (fleet_id != INVALID_OBJECT_ID)
                original_fleet_ids.push_back(fleet_id);
        }


        // get new fleet id
        int new_fleet_id = GetNewObjectID();
        if (new_fleet_id == INVALID_OBJECT_ID) {
            ClientUI::MessageBox(UserString("SERVER_TIMEOUT"), true);
            return;
        }


        // generate new fleet name
        std::string fleet_name = Fleet::GenerateFleetName(ship_ids, new_fleet_id);

        // create new fleet with ships
        HumanClientApp::GetApp()->Orders().IssueOrder(
            OrderPtr(new NewFleetOrder(empire_id, fleet_name, new_fleet_id, system_id, ship_ids)));

        // set aggression of new fleet, if not already what was requested
        if (TemporaryPtr<const Fleet> new_fleet = GetFleet(new_fleet_id))
            if (new_fleet->Aggressive() != aggressive)
                HumanClientApp::GetApp()->Orders().IssueOrder(
                    OrderPtr(new AggressiveOrder(empire_id, new_fleet_id, aggressive)));

        // delete empty fleets from which ships may have been taken
        std::vector<TemporaryPtr<Fleet> > fleets = Objects().FindObjects<Fleet>(original_fleet_ids);
        for (std::vector<TemporaryPtr<Fleet> >::iterator it = fleets.begin();
                it != fleets.end(); ++it)
        {
            TemporaryPtr<const Fleet> fleet = *it;
            if (fleet && fleet->Empty())
                HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(
                    new DeleteFleetOrder(empire_id, fleet->ID())));
        }
    }

    void CreateNewFleetFromShipsWithDesign(const std::set<int>& ship_ids,
                                           int design_id, bool aggressive = false)
    {
        Logger().debugStream() << "CreateNewFleetFromShipsWithDesign with " << ship_ids.size()
                               << " ship ids and design id: " << design_id;
        if (ship_ids.empty() || design_id == ShipDesign::INVALID_DESIGN_ID)
            return;
        if (ClientPlayerIsModerator())
            return; // todo: handle moderator actions for this...
        int empire_id = HumanClientApp::GetApp()->EmpireID();
        if (empire_id == ALL_EMPIRES)
            return;


        Sound::TempUISoundDisabler sound_disabler;


        // get system where new fleet is to be created.
        int first_ship_id = *ship_ids.begin();
        TemporaryPtr<const Ship> first_ship = GetShip(first_ship_id);
        if (!first_ship) {
            Logger().errorStream() << "CreateNewFleetFromShipsWithDesign couldn't get ship with id " << first_ship_id;
            return;
        }
        int system_id = first_ship->SystemID();
        TemporaryPtr<const System> system = GetSystem(system_id); // may be null
        if (!system) {
            Logger().errorStream() << "CreateNewFleetFromShipsWithDesign couldn't get a valid system with system id " << system_id;
            return;
        }

        double X = first_ship->X(), Y = first_ship->Y();

        // select ships with the requested design id
        // also verify that all fleets are at the same system and position and owned by the same empire.
        // also collect all fleet ids from which ships are being taken
        std::vector<int> original_fleet_ids;
        std::vector<int> ships_of_design_ids;
        std::vector<TemporaryPtr<Ship> > ships = Objects().FindObjects<Ship>(ship_ids);
        for (std::vector<TemporaryPtr<Ship> >::const_iterator it = ships.begin();
            it != ships.end(); ++it)
        {
            TemporaryPtr<const Ship> ship = *it;

            if (ship->DesignID() != design_id)
                continue;

            if (ship->SystemID() != system_id) {
                Logger().errorStream() << "CreateNewFleetFromShipsWithDesign passed ships with inconsistent system ids";
                return;
            }
            if (ship->X() != X || ship->Y() != Y) {
                Logger().errorStream() << "CreateNewFleetFromShipsWithDesign passed ship and system with inconsistent locations";
                return;
            }
            if (!ship->OwnedBy(empire_id)) {
                Logger().errorStream() << "CreateNewFleetFromShipsWithDesign passed ships not owned by this client's empire";
                return;
            }
            int fleet_id = ship->FleetID();
            if (fleet_id != INVALID_OBJECT_ID)
                original_fleet_ids.push_back(fleet_id);
            ships_of_design_ids.push_back(ship->ID());
        }

        if (ships_of_design_ids.empty())
            return;

        // get new fleet id
        int new_fleet_id = GetNewObjectID();
        if (new_fleet_id == INVALID_OBJECT_ID) {
            ClientUI::MessageBox(UserString("SERVER_TIMEOUT"), true);
            return;
        }


        // generate new fleet name
        std::string fleet_name = Fleet::GenerateFleetName(ships_of_design_ids, new_fleet_id);

        // create new fleet with ships
        HumanClientApp::GetApp()->Orders().IssueOrder(
            OrderPtr(new NewFleetOrder(empire_id, fleet_name, new_fleet_id, system_id, ships_of_design_ids)));

        // set aggression of new fleet, if not already what was requested
        if (TemporaryPtr<const Fleet> new_fleet = GetFleet(new_fleet_id))
            if (new_fleet->Aggressive() != aggressive)
                HumanClientApp::GetApp()->Orders().IssueOrder(
                    OrderPtr(new AggressiveOrder(empire_id, new_fleet_id, aggressive)));

        // delete empty fleets from which ships may have been taken
        std::vector<TemporaryPtr<Fleet> > fleets = Objects().FindObjects<Fleet>(original_fleet_ids);
        for (std::vector<TemporaryPtr<Fleet> >::iterator it = fleets.begin();
                it != fleets.end(); ++it)
        {
            TemporaryPtr<const Fleet> fleet = *it;
            if (fleet && fleet->Empty())
                HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(
                    new DeleteFleetOrder(empire_id, fleet->ID())));
        }
    }

    void CreateNewFleetsFromShipsForEachDesign(const std::set<int>& ship_ids,
                                               bool aggressive = false)
    {
        Logger().debugStream() << "CreateNewFleetsFromShipsForEachDesign with "
                               << ship_ids.size() << " ship ids";
        if (ship_ids.empty())
            return;
        if (ClientPlayerIsModerator())
            return; // todo: handle moderator actions for this...
        int empire_id = HumanClientApp::GetApp()->EmpireID();
        if (empire_id == ALL_EMPIRES)
            return;


        Sound::TempUISoundDisabler sound_disabler;

        // get system where new fleet is to be created.
        int first_ship_id = *ship_ids.begin();
        TemporaryPtr<const Ship> first_ship = GetShip(first_ship_id);
        if (!first_ship) {
            Logger().errorStream() << "CreateNewFleetsFromShipsForEachDesign couldn't get ship with id " << first_ship_id;
            return;
        }
        int system_id = first_ship->SystemID();
        TemporaryPtr<const System> system = GetSystem(system_id); // may be null
        if (!system) {
            Logger().errorStream() << "CreateNewFleetsFromShipsForEachDesign couldn't get a valid system with system id " << system_id;
            return;
        }

        double X = first_ship->X(), Y = first_ship->Y();

        // sort ships by ID into container, indexed by design id
        // also collect all fleet ids from which ships are being taken
        std::map<int, std::vector<int> > designs_ship_ids;
        std::vector<int> original_fleet_ids;
        std::vector<TemporaryPtr<Ship> > ships = Objects().FindObjects<Ship>(ship_ids);
        for (std::vector<TemporaryPtr<Ship> >::const_iterator it = ships.begin();
            it != ships.end(); ++it)
        {
            TemporaryPtr<const Ship> ship = *it;

            if (ship->SystemID() != system_id) {
                Logger().errorStream() << "CreateNewFleetsFromShipsForEachDesign passed ships with inconsistent system ids";
                return;
            }
            if (ship->X() != X || ship->Y() != Y) {
                Logger().errorStream() << "CreateNewFleetsFromShipsForEachDesign passed ship and system with inconsistent locations";
                return;
            }
            if (!ship->OwnedBy(empire_id)) {
                Logger().errorStream() << "CreateNewFleetsFromShipsForEachDesign passed ships not owned by this client's empire";
                return;
            }
            int fleet_id = ship->FleetID();
            if (fleet_id != INVALID_OBJECT_ID)
                original_fleet_ids.push_back(fleet_id);

            designs_ship_ids[ship->DesignID()].push_back(ship->ID());
        }

        // make new fleets for each group of ships, by design id
        for (std::map<int, std::vector<int> >::const_iterator it = designs_ship_ids.begin();
             it != designs_ship_ids.end(); ++it)
        {
            int design_id = it->first;
            const std::vector<int>& ship_ids = it->second;
            if (ship_ids.empty())
                continue;

            int new_fleet_id = GetNewObjectID();
            if (new_fleet_id == INVALID_OBJECT_ID) {
                ClientUI::MessageBox(UserString("SERVER_TIMEOUT"), true);
                return;
            }

            // generate new fleet name
            std::string fleet_name = Fleet::GenerateFleetName(ship_ids, new_fleet_id);

            // create new fleet with ships
            HumanClientApp::GetApp()->Orders().IssueOrder(
                OrderPtr(new NewFleetOrder(empire_id, fleet_name, new_fleet_id, system_id, ship_ids)));

            // set aggression of new fleet, if not already what was requested
            if (TemporaryPtr<const Fleet> new_fleet = GetFleet(new_fleet_id))
                if (new_fleet->Aggressive() != aggressive)
                    HumanClientApp::GetApp()->Orders().IssueOrder(
                        OrderPtr(new AggressiveOrder(empire_id, new_fleet_id, aggressive)));
        }

        // delete empty fleets from which ships may have been taken
        std::vector<TemporaryPtr<Fleet> > fleets = Objects().FindObjects<Fleet>(original_fleet_ids);
        for (std::vector<TemporaryPtr<Fleet> >::iterator it = fleets.begin();
                it != fleets.end(); ++it)
        {
            TemporaryPtr<const Fleet> fleet = *it;
            if (fleet && fleet->Empty())
                HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(
                    new DeleteFleetOrder(empire_id, fleet->ID())));
        }
    }

    void MergeFleetsIntoFleet(int fleet_id) {
        if (ClientPlayerIsModerator())
            return; // todo: handle moderator actions for this...
        int empire_id = HumanClientApp::GetApp()->EmpireID();
        if (empire_id == ALL_EMPIRES)
            return;

        TemporaryPtr<Fleet> target_fleet = GetFleet(fleet_id);
        if (!target_fleet) {
            Logger().errorStream() << "MergeFleetsIntoFleet couldn't get a fleet with id " << fleet_id;
            return;
        }

        TemporaryPtr<const System> system = GetSystem(target_fleet->SystemID());
        if (!system) {
            Logger().errorStream() << "MergeFleetsIntoFleet couldn't get system for the target fleet";
            return;
        }

        // filter fleets in system to select just those owned by this client's
        // empire, and then order their ships transferred into target fleet
        std::vector<TemporaryPtr<Fleet> > all_system_fleets = Objects().FindObjects<Fleet>(system->FleetIDs());
        std::vector<int> empire_system_fleet_ids;
        empire_system_fleet_ids.reserve(all_system_fleets.size());
        for (std::vector<TemporaryPtr<Fleet> >::iterator it = all_system_fleets.begin();
             it != all_system_fleets.end(); ++it)
        {
            TemporaryPtr<Fleet> fleet = *it;
            if (!fleet->OwnedBy(empire_id))
                continue;
            if (fleet->ID() == target_fleet->ID() || fleet->ID() == INVALID_OBJECT_ID)
                continue;   // no need to do things to target fleet's contents

            empire_system_fleet_ids.push_back(fleet->ID());

            // order ships moved into target fleet
            std::vector<int> ship_ids(fleet->ShipIDs().begin(), fleet->ShipIDs().end());
            HumanClientApp::GetApp()->Orders().IssueOrder(
                OrderPtr(new FleetTransferOrder(empire_id, fleet->ID(), target_fleet->ID(), ship_ids)));
        }

        // delete empty fleets from which ships may have been taken
        std::vector<TemporaryPtr<Fleet> > potentially_empty_fleets =
            Objects().FindObjects<Fleet>(empire_system_fleet_ids);
        for (std::vector<TemporaryPtr<Fleet> >::iterator it = potentially_empty_fleets.begin();
                it != potentially_empty_fleets.end(); ++it)
        {
            TemporaryPtr<const Fleet> fleet = *it;
            if (fleet && fleet->Empty())
                HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(
                    new DeleteFleetOrder(empire_id, fleet->ID())));
        }
    }

   /** Returns map from object ID to issued colonize orders affecting it. */
    std::map<int, int> PendingScrapOrders() {
        std::map<int, int> retval;
        const ClientApp* app = ClientApp::GetApp();
        if (!app)
            return retval;
        const OrderSet& orders = app->Orders();
        for (OrderSet::const_iterator it = orders.begin(); it != orders.end(); ++it) {
            if (boost::shared_ptr<ScrapOrder> order = boost::dynamic_pointer_cast<ScrapOrder>(it->second)) {
                retval[order->ObjectID()] = it->first;
            }
        }
        return retval;
    }

    const double TWO_PI = 2.0*3.14159;
    boost::shared_ptr<ShaderProgram> scanline_shader;
}

////////////////////////////////////////////////
// FleetUIManager                             //
////////////////////////////////////////////////
FleetUIManager::FleetUIManager() :
    m_order_issuing_enabled(true),
    m_active_fleet_wnd(0)
{}

FleetUIManager::iterator FleetUIManager::begin() const
{ return m_fleet_wnds.begin(); }

bool FleetUIManager::empty() const
{ return m_fleet_wnds.empty(); }

FleetUIManager::iterator FleetUIManager::end() const
{ return m_fleet_wnds.end(); }

FleetWnd* FleetUIManager::ActiveFleetWnd() const
{ return m_active_fleet_wnd; }

FleetWnd* FleetUIManager::WndForFleet(TemporaryPtr<const Fleet> fleet) const {
    assert(fleet);
    FleetWnd* retval = 0;
    for (std::set<FleetWnd*>::const_iterator it = m_fleet_wnds.begin(); it != m_fleet_wnds.end(); ++it) {
        if ((*it)->ContainsFleet(fleet->ID())) {
            retval = *it;
            break;
        }
    }
    return retval;
}

int FleetUIManager::SelectedShipID() const {
    if (!m_active_fleet_wnd)
        return INVALID_OBJECT_ID;

    std::set<int> selected_ship_ids = m_active_fleet_wnd->SelectedShipIDs();
    if (selected_ship_ids.size() != 1)
        return INVALID_OBJECT_ID;

    return *selected_ship_ids.begin();
}

std::set<int> FleetUIManager::SelectedShipIDs() const {
    if (!m_active_fleet_wnd)
        return std::set<int>();
    return m_active_fleet_wnd->SelectedShipIDs();
}

FleetWnd* FleetUIManager::NewFleetWnd(const std::vector<int>& fleet_ids,
                                      int selected_fleet_id/* = INVALID_OBJECT_ID*/,
                                      GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | CLOSABLE*/)
{
    if (!GetOptionsDB().Get<bool>("UI.multiple-fleet-windows"))
        CloseAll();
    FleetWnd* retval = new FleetWnd(fleet_ids, m_order_issuing_enabled, selected_fleet_id, flags);

    m_fleet_wnds.insert(retval);
    GG::Connect(retval->ClosingSignal,              &FleetUIManager::FleetWndClosing,   this);
    GG::Connect(retval->ClickedSignal,              &FleetUIManager::FleetWndClicked,   this);
    GG::Connect(retval->FleetRightClickedSignal,    FleetRightClickedSignal);
    GG::Connect(retval->ShipRightClickedSignal,     ShipRightClickedSignal);

    GG::GUI::GetGUI()->Register(retval);

    return retval;
}

FleetWnd* FleetUIManager::NewFleetWnd(int system_id, int empire_id,
                                      int selected_fleet_id/* = INVALID_OBJECT_ID*/,
                                      GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | CLOSABLE*/)
{
    if (!GetOptionsDB().Get<bool>("UI.multiple-fleet-windows"))
        CloseAll();
    FleetWnd* retval = new FleetWnd(system_id, empire_id, m_order_issuing_enabled, selected_fleet_id, flags);

    m_fleet_wnds.insert(retval);
    GG::Connect(retval->ClosingSignal,              &FleetUIManager::FleetWndClosing,           this);
    GG::Connect(retval->ClickedSignal,              &FleetUIManager::FleetWndClicked,           this);
    GG::Connect(retval->FleetRightClickedSignal,    FleetRightClickedSignal);
    GG::Connect(retval->ShipRightClickedSignal,     ShipRightClickedSignal);

    GG::GUI::GetGUI()->Register(retval);

    return retval;
}

void FleetUIManager::CullEmptyWnds() {
    // scan through FleetWnds, deleting those that have no fleets
    for (std::set<FleetWnd*>::iterator it = m_fleet_wnds.begin(); it != m_fleet_wnds.end(); ) {
        std::set<FleetWnd*>::iterator cur_wnd_it = it++;
        FleetWnd* cur_wnd = *cur_wnd_it;
        if (cur_wnd->FleetIDs().empty())
            delete cur_wnd;
    }
}

void FleetUIManager::SetActiveFleetWnd(FleetWnd* fleet_wnd) {
    if (fleet_wnd == m_active_fleet_wnd)
        return;

    // disconnect old active FleetWnd signals
    if (m_active_fleet_wnd) {
        for (std::vector<boost::signals::connection>::iterator it = m_active_fleet_wnd_signals.begin(); it != m_active_fleet_wnd_signals.end(); ++it)
            it->disconnect();
        m_active_fleet_wnd_signals.clear();
    }

    // set new active FleetWnd
    m_active_fleet_wnd = fleet_wnd;

    // connect new active FleetWnd selection changed signal
    m_active_fleet_wnd_signals.push_back(GG::Connect(m_active_fleet_wnd->SelectedFleetsChangedSignal,   ActiveFleetWndSelectedFleetsChangedSignal));
    m_active_fleet_wnd_signals.push_back(GG::Connect(m_active_fleet_wnd->SelectedShipsChangedSignal,    ActiveFleetWndSelectedShipsChangedSignal));

    ActiveFleetWndChangedSignal();
}

bool FleetUIManager::CloseAll() {
    bool retval = !m_fleet_wnds.empty();
    std::vector<FleetWnd*> vec(m_fleet_wnds.begin(), m_fleet_wnds.end());

    for (std::size_t i = 0; i < vec.size(); ++i)
        delete vec[i];

    m_active_fleet_wnd = 0;

    ActiveFleetWndChangedSignal();

    return retval;
}

void FleetUIManager::RefreshAll() {
    if (m_fleet_wnds.empty())
        return;

    std::vector<FleetWnd*> vec(m_fleet_wnds.begin(), m_fleet_wnds.end());
    for (std::size_t i = 0; i < vec.size(); ++i)
        vec[i]->Refresh();
}

FleetUIManager& FleetUIManager::GetFleetUIManager() {
    static FleetUIManager retval;
    return retval;
}

void FleetUIManager::FleetWndClosing(FleetWnd* fleet_wnd) {
    bool active_wnd_affected = false;
    if (fleet_wnd == m_active_fleet_wnd) {
        m_active_fleet_wnd = 0;
        active_wnd_affected = true;
    }
    m_fleet_wnds.erase(fleet_wnd);
    if (active_wnd_affected)
        ActiveFleetWndChangedSignal();  // let anything that cares know the active fleetwnd just closed
}

void FleetUIManager::FleetWndClicked(FleetWnd* fleet_wnd) {
    if (fleet_wnd == m_active_fleet_wnd)
        return;
    SetActiveFleetWnd(fleet_wnd);
}

void FleetUIManager::EnableOrderIssuing(bool enable/* = true*/) {
    m_order_issuing_enabled = enable;
    for (std::set<FleetWnd*>::iterator it = m_fleet_wnds.begin(); it != m_fleet_wnds.end(); ++it)
        (*it)->EnableOrderIssuing(m_order_issuing_enabled);
}

namespace {
    ////////////////////////////////////////////////
    // Free Functions
    ////////////////////////////////////////////////
    bool    ValidShipTransfer(TemporaryPtr<const Ship> ship, TemporaryPtr<const Fleet> new_fleet) {
        if (!ship || !new_fleet)
            return false;   // can't transfer no ship or to no fleet

        TemporaryPtr<const Fleet> current_fleet = GetFleet(ship->FleetID());
        if (current_fleet && current_fleet->ID() == new_fleet->ID())
            return false;   // can't transfer a fleet to a fleet it already is in

        if (ship->X() != new_fleet->X() || ship->Y() != new_fleet->Y())
            return false;   // can't move fleets during a transfer.  can only transfer fleet at same location as ship

        if (new_fleet->SystemID() == INVALID_OBJECT_ID)
            return false;   // not in a system

        if (ship->SystemID() != new_fleet->SystemID())
            return false;   // fleets need to be in same system.  probably redundant with checking position

        if (ship->Unowned() || new_fleet->Unowned())
            return false;   // need to own a ship to transfer it...

        if (ship->Owner() != new_fleet->Owner())
            return false;   // need to have same owner.

        // all tests passed.  can transfer
        return true;
    }

    bool    ValidFleetMerge(TemporaryPtr<const Fleet> fleet, TemporaryPtr<const Fleet> target_fleet) {
        if (!fleet || !target_fleet)
            return false;   // missing objects

        if (fleet->SystemID() != target_fleet->SystemID())
            return false;   // at different systems

        if (fleet->SystemID() == INVALID_OBJECT_ID)
            return false;   // not in a system

        if (fleet->X() != target_fleet->X() || fleet->Y() != target_fleet->Y())
            return false;   // at different locations.

        if (fleet->Unowned() || target_fleet->Unowned())
            return false;

        if (fleet->Owner() != target_fleet->Owner())
            return false;   // different owners

        // all tests passed; can merge fleets
        return true;
    }

    ////////////////////////////////////////////////
    // ShipRow
    ////////////////////////////////////////////////
    /** A ListBox::Row subclass used to represent ships in ShipListBoxes. */
    class ShipRow : public GG::ListBox::Row {
    public:
        ShipRow(GG::X w, GG::Y h, int ship_id) :
            GG::ListBox::Row(w, h, ""),
            m_ship_id(ship_id),
            m_panel(0)
        {
            SetName("ShipRow");
            SetChildClippingMode(ClipToClient);
            if (GetShip(m_ship_id))
                SetDragDropDataType(SHIP_DROP_TYPE_STRING);
            {
                //ScopedTimer timer("ShipRow Panel creation / pushing");
                m_panel = new ShipDataPanel(w, h, m_ship_id);
                push_back(m_panel);
            }
        }

        void            SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            const GG::Pt old_size = Size();
            GG::ListBox::Row::SizeMove(ul, lr);
            //std::cout << "ShipRow::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
            if (!empty() && old_size != Size() && m_panel)
                m_panel->Resize(Size());
        }

        int             ShipID() const {return m_ship_id;}
    private:
        int             m_ship_id;
        ShipDataPanel*  m_panel;
    };
}

/** Renders scanlines over its area. */
class ScanlineControl : public GG::Control {
public:
    ScanlineControl(GG::X x, GG::Y y, GG::X w, GG::Y h, bool square = false) :
        Control(x, y, w, h, GG::Flags<GG::WndFlag>()),
        m_square(square)
    {
        if (!scanline_shader && GetOptionsDB().Get<bool>("UI.system-fog-of-war")) {
            boost::filesystem::path shader_path = GetRootDataDir() / "default" / "shaders" / "scanlines.frag";
            std::string shader_text;
            ReadFile(shader_path, shader_text);
            scanline_shader = boost::shared_ptr<ShaderProgram>(
                ShaderProgram::shaderProgramFactory("", shader_text));
        }
    }

    virtual void Render() {
        if (!scanline_shader || !GetOptionsDB().Get<bool>("UI.system-fog-of-war"))
            return;

        float fog_scanline_spacing = static_cast<float>(GetOptionsDB().Get<double>("UI.system-fog-of-war-spacing"));
        scanline_shader->Use();
        scanline_shader->Bind("scanline_spacing", fog_scanline_spacing);

        if (m_square) {
            GG::FlatRectangle(  UpperLeft(),    LowerRight(), GG::CLR_WHITE, GG::CLR_WHITE, 0u);
        } else {
            CircleArc(          UpperLeft(),    LowerRight(), 0.0, TWO_PI, true);
        }
        scanline_shader->stopUse();
    }
private:
    bool m_square;
};

////////////////////////////////////////////////
// ShipDataPanel
////////////////////////////////////////////////
ShipDataPanel::ShipDataPanel(GG::X w, GG::Y h, int ship_id) :
    Control(GG::X0, GG::Y0, w, h, GG::Flags<GG::WndFlag>()),
    m_initialized(false),
    m_ship_id(ship_id),
    m_ship_icon(0),
    m_scrap_indicator(0),
    m_colonize_indicator(0),
    m_invade_indicator(0),
    m_bombard_indicator(0),
    m_gift_indicator(0),
    m_scanline_control(0),
    m_ship_name_text(0),
    m_design_name_text(0),
    m_stat_icons(),
    m_selected(false)
{
    SetChildClippingMode(ClipToClient);
}

ShipDataPanel::~ShipDataPanel() {
    m_ship_connection.disconnect();
    m_fleet_connection.disconnect();
}

GG::Pt ShipDataPanel::ClientUpperLeft() const
{ return UpperLeft() + GG::Pt(GG::X(DATA_PANEL_BORDER), GG::Y(DATA_PANEL_BORDER)); }

GG::Pt ShipDataPanel::ClientLowerRight() const
{ return LowerRight() - GG::Pt(GG::X(DATA_PANEL_BORDER), GG::Y(DATA_PANEL_BORDER));  }

void ShipDataPanel::Render() {
    if (!m_initialized)
        Init();

    // main background position and colour
    const GG::Clr& background_colour = ClientUI::WndColor();
    const GG::Pt ul = UpperLeft(), lr = LowerRight(), cul = ClientUpperLeft();

    // title background colour and position
    const GG::Clr& unselected_colour = ClientUI::WndOuterBorderColor();
    const GG::Clr& selected_colour = ClientUI::WndInnerBorderColor();
    GG::Clr border_colour = m_selected ? selected_colour : unselected_colour;
    if (Disabled())
        border_colour = DisabledColor(border_colour);
    const GG::Pt text_ul = cul + GG::Pt(DATA_PANEL_ICON_SPACE.x, GG::Y0);
    const GG::Pt text_lr = cul + GG::Pt(ClientWidth(),           LabelHeight());

    // render
    GG::FlatRectangle(ul,       lr,         background_colour,  border_colour, DATA_PANEL_BORDER);  // background and border
    GG::FlatRectangle(text_ul,  text_lr,    border_colour,      GG::CLR_ZERO,  0);                  // title background box
}

void ShipDataPanel::Select(bool b) {
    if (m_selected == b)
        return;
    m_selected = b;

    const GG::Clr& unselected_text_color = ClientUI::TextColor();
    const GG::Clr& selected_text_color = GG::CLR_BLACK;

    GG::Clr text_color_to_use = m_selected ? selected_text_color : unselected_text_color;

    if (Disabled())
        text_color_to_use = DisabledColor(text_color_to_use);

    if (m_ship_name_text)
        m_ship_name_text->SetTextColor(text_color_to_use);
    if (m_design_name_text)
        m_design_name_text->SetTextColor(text_color_to_use);
}

void ShipDataPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    GG::Control::SizeMove(ul, lr);
    //std::cout << "ShipDataPanel::SizeMove new size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
    if (old_size != Size())
        DoLayout();
}

void ShipDataPanel::SetShipIcon() {
    delete m_ship_icon;
    m_ship_icon = 0;

    delete m_scrap_indicator;
    m_scrap_indicator = 0;

    delete m_colonize_indicator;
    m_colonize_indicator = 0;

    delete m_invade_indicator;
    m_invade_indicator = 0;

    delete m_bombard_indicator;
    m_bombard_indicator = 0;

    delete m_gift_indicator;
    m_gift_indicator = 0;

    delete m_scanline_control;
    m_scanline_control = 0;

    TemporaryPtr<const Ship> ship = GetShip(m_ship_id);
    if (!ship)
        return;

    boost::shared_ptr<GG::Texture> icon;

    if (const ShipDesign* design = ship->Design())
        icon = ClientUI::ShipDesignIcon(design->ID());
    else
        icon = ClientUI::ShipDesignIcon(INVALID_OBJECT_ID);  // default icon

    m_ship_icon = new GG::StaticGraphic(GG::X0, GG::Y0, DATA_PANEL_ICON_SPACE.x,
                                        ClientHeight(), icon, DataPanelIconStyle());
    AttachChild(m_ship_icon);

    if (ship->OrderedScrapped()) {
        boost::shared_ptr<GG::Texture> scrap_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "scrapped.png", true);
        m_scrap_indicator = new GG::StaticGraphic(GG::X0, GG::Y0, DATA_PANEL_ICON_SPACE.x, ClientHeight(), scrap_texture, DataPanelIconStyle());
        AttachChild(m_scrap_indicator);
    }
    if (ship->OrderedColonizePlanet() != INVALID_OBJECT_ID) {
        boost::shared_ptr<GG::Texture> colonize_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "colonizing.png", true);
        m_colonize_indicator = new GG::StaticGraphic(GG::X0, GG::Y0, DATA_PANEL_ICON_SPACE.x, ClientHeight(), colonize_texture, DataPanelIconStyle());
        AttachChild(m_colonize_indicator);
    }
    if (ship->OrderedInvadePlanet() != INVALID_OBJECT_ID) {
        boost::shared_ptr<GG::Texture> invade_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "invading.png", true);
        m_invade_indicator = new GG::StaticGraphic(GG::X0, GG::Y0, DATA_PANEL_ICON_SPACE.x, ClientHeight(), invade_texture, DataPanelIconStyle());
        AttachChild(m_invade_indicator);
    }
    if (ship->OrderedBombardPlanet() != INVALID_OBJECT_ID) {
        boost::shared_ptr<GG::Texture> bombard_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "bombarding.png", true);
        m_bombard_indicator = new GG::StaticGraphic(GG::X0, GG::Y0, DATA_PANEL_ICON_SPACE.x, ClientHeight(), bombard_texture, DataPanelIconStyle());
        AttachChild(m_bombard_indicator);
    }
    if (ship->OrderedGivenToEmpire() != ALL_EMPIRES) {
        boost::shared_ptr<GG::Texture> bombard_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "gifting.png", true);
        m_gift_indicator = new GG::StaticGraphic(GG::X0, GG::Y0, DATA_PANEL_ICON_SPACE.x, ClientHeight(), bombard_texture, DataPanelIconStyle());
        AttachChild(m_gift_indicator);
    }
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    if (ship->GetVisibility(client_empire_id) < VIS_BASIC_VISIBILITY) {
        m_scanline_control = new ScanlineControl(GG::X0, GG::Y0, m_ship_icon->Width(), m_ship_icon->Height(), true);
        AttachChild(m_scanline_control);
    }
}

void ShipDataPanel::Refresh() {
    SetShipIcon();

    TemporaryPtr<const Ship> ship = GetShip(m_ship_id);
    if (!ship) {
        // blank text and delete icons
        m_ship_name_text->SetText("");
        if (m_design_name_text) {
            delete m_design_name_text;
            m_design_name_text = 0;
        }
        for (std::vector<std::pair<std::string, StatisticIcon*> >::iterator it = m_stat_icons.begin(); it != m_stat_icons.end(); ++it)
            delete it->second;
        m_stat_icons.clear();
        return;
    }


    int empire_id = HumanClientApp::GetApp()->EmpireID();


    // name and design name update
    std::string ship_name = ship->PublicName(empire_id);
    m_ship_name_text->SetText(ship_name);

    if (m_design_name_text) {
        std::string design_name = UserString("FW_UNKNOWN_DESIGN_NAME");
        if (const ShipDesign* design = ship->Design())
            design_name = design->Name();
        const std::string& species_name = ship->SpeciesName();
        if (!species_name.empty()) {
            m_design_name_text->SetText(boost::io::str(FlexibleFormat(UserString("FW_SPECIES_SHIP_DESIGN_LABEL")) %
                                                        design_name %
                                                        UserString(species_name)));
        } else {
            m_design_name_text->SetText(design_name);
        }
    }


    // update stat icon values and browse wnds
    for (std::vector<std::pair<std::string, StatisticIcon*> >::const_iterator it = m_stat_icons.begin();
         it != m_stat_icons.end(); ++it)
    {
        //std::cout << "setting ship stat " << it->first << " to value: " << StatValue(it->first) << std::endl;
        it->second->SetValue(StatValue(it->first));

        it->second->ClearBrowseInfoWnd();
        if (it->first == DAMAGE_STAT_STRING) {
            boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd(new IconTextBrowseWnd(
                DamageIcon(), UserString("SHIP_DAMAGE_STAT_TITLE"),
                UserString("SHIP_DAMAGE_STAT_MAIN")));
            it->second->SetBrowseInfoWnd(browse_wnd);
        } else {
            MeterType meter_type = MeterTypeFromStatString(it->first);
            MeterType associated_meter_type = AssociatedMeterType(meter_type);
            boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd(new MeterBrowseWnd(
                m_ship_id, meter_type, associated_meter_type));
            it->second->SetBrowseInfoWnd(browse_wnd);
        }
    }

    DoLayout();
}

double ShipDataPanel::StatValue(const std::string& stat_name) const {
    if (TemporaryPtr<const Ship> ship = GetShip(m_ship_id)) {
        if (stat_name == DAMAGE_STAT_STRING)
            return ship->TotalWeaponsDamage();

        MeterType meter_type = MeterTypeFromStatString(stat_name);
        //std::cout << "got meter type " << boost::lexical_cast<std::string>(meter_type) << " from stat_name " << stat_name << std::endl;
        if (ship->UniverseObject::GetMeter(meter_type)) {
            //std::cout << " ... ship has meter! returning meter points value " << ship->CurrentMeterValue(meter_type) << std::endl;
            return ship->CurrentMeterValue(meter_type);
        }
        Logger().errorStream() << "ShipDataPanel::StatValue couldn't get stat of name: " << stat_name;
    }
    return 0.0;
}

void ShipDataPanel::DoLayout() {
    // resize ship and scrap indicator icons, they can fit and position themselves in the space provided
    // client height should never be less than the height of the space resereved for the icon
    if (m_ship_icon)
        m_ship_icon->Resize(GG::Pt(DATA_PANEL_ICON_SPACE.x, ClientHeight()));
    if (m_scrap_indicator)
        m_scrap_indicator->Resize(GG::Pt(DATA_PANEL_ICON_SPACE.x, ClientHeight()));
    if (m_colonize_indicator)
        m_colonize_indicator->Resize(GG::Pt(DATA_PANEL_ICON_SPACE.x, ClientHeight()));
    if (m_invade_indicator)
        m_invade_indicator->Resize(GG::Pt(DATA_PANEL_ICON_SPACE.x, ClientHeight()));
    if (m_bombard_indicator)
        m_bombard_indicator->Resize(GG::Pt(DATA_PANEL_ICON_SPACE.x, ClientHeight()));
    if (m_gift_indicator)
        m_gift_indicator->Resize(GG::Pt(DATA_PANEL_ICON_SPACE.x, ClientHeight()));
    if (m_scanline_control)
        m_scanline_control->Resize(GG::Pt(DATA_PANEL_ICON_SPACE.x, ClientHeight()));

    // position ship name text at the top to the right of icons
    const GG::Pt name_ul = GG::Pt(DATA_PANEL_ICON_SPACE.x + DATA_PANEL_TEXT_PAD, GG::Y0);
    const GG::Pt name_lr = GG::Pt(ClientWidth() - DATA_PANEL_TEXT_PAD,           LabelHeight());
    if (m_ship_name_text)
        m_ship_name_text->SizeMove(name_ul, name_lr);

    if (m_design_name_text)
        m_design_name_text->SizeMove(name_ul, name_lr);

    // position ship statistic icons one after another horizontally and centered vertically
    GG::Pt icon_ul = GG::Pt(name_ul.x, LabelHeight() + std::max(GG::Y0, (ClientHeight() - LabelHeight() - StatIconSize().y) / 2));
    for (std::vector<std::pair<std::string, StatisticIcon*> >::const_iterator it = m_stat_icons.begin(); it != m_stat_icons.end(); ++it) {
        it->second->SizeMove(icon_ul, icon_ul + StatIconSize());
        icon_ul.x += StatIconSize().x;
    }
}

void ShipDataPanel::Init() {
    if (m_initialized)
        return;
    m_initialized = true;

    // ship name text.  blank if no ship.
    TemporaryPtr<const Ship> ship = GetShip(m_ship_id);
    std::string ship_name;
    if (ship)
        ship_name = ship->Name();

    m_ship_name_text = new GG::TextControl(GG::X(Value(Height())), GG::Y0, GG::X1, LabelHeight(),
                                            ship_name, ClientUI::GetFont(),
                                            ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    AttachChild(m_ship_name_text);


    // design name and statistic icons
    if (!ship)
        return;

    if (const ShipDesign* design = ship->Design()) {
        m_design_name_text = new GG::TextControl(GG::X(Value(Height())), GG::Y0, GG::X1, LabelHeight(),
                                                    design->Name(), ClientUI::GetFont(),
                                                    ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
        AttachChild(m_design_name_text);
    }


    int tooltip_delay = GetOptionsDB().Get<int>("UI.tooltip-delay");


    // damage stat icon
    StatisticIcon* icon = new StatisticIcon(GG::X0, GG::Y0, StatIconSize().x, StatIconSize().y,
                                            DamageIcon(), 0, 0, false);
    m_stat_icons.push_back(std::make_pair(DAMAGE_STAT_STRING, icon));
    AttachChild(icon);
    icon->SetBrowseModeTime(tooltip_delay);

    // meter stat icons
    std::vector<MeterType> meters;
    meters.push_back(METER_STRUCTURE);  meters.push_back(METER_SHIELD);     meters.push_back(METER_FUEL);
    meters.push_back(METER_DETECTION);  meters.push_back(METER_STEALTH);    meters.push_back(METER_STARLANE_SPEED);
    for (std::vector<MeterType>::const_iterator it = meters.begin(); it != meters.end(); ++it) {
        StatisticIcon* icon = new StatisticIcon(GG::X0, GG::Y0, StatIconSize().x, StatIconSize().y,
                                                ClientUI::MeterIcon(*it), 0, 0, false);
        m_stat_icons.push_back(std::make_pair(MeterStatString(*it), icon));
        AttachChild(icon);
        icon->SetBrowseModeTime(tooltip_delay);
    }

    // bookkeeping
    m_ship_connection = GG::Connect(ship->StateChangedSignal, &ShipDataPanel::Refresh, this);

    if (TemporaryPtr<Fleet> fleet = GetFleet(ship->FleetID()))
        m_fleet_connection = GG::Connect(fleet->StateChangedSignal, &ShipDataPanel::Refresh, this);

    Refresh();
}

////////////////////////////////////////////////
// FleetDataPanel
////////////////////////////////////////////////
/** Represents a single fleet.  This class is used as the drop-target in
  * FleetWnd or as the sole Control in each FleetRow. */
class FleetDataPanel : public GG::Control {
public:
    FleetDataPanel(GG::X w, GG::Y h, int fleet_id);
    FleetDataPanel(GG::X w, GG::Y h, int system_id, bool new_fleet_drop_target);

    virtual GG::Pt      ClientUpperLeft() const;  ///< upper left plus border insets
    virtual GG::Pt      ClientLowerRight() const; ///< lower right minus border insets

    virtual void        DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last, const GG::Pt& pt) const;

    bool                Selected() const;
    bool                NewFleetAggression() const;

    virtual void        Render();
    virtual void        DragDropEnter(const GG::Pt& pt, const std::map<GG::Wnd*, GG::Pt>& drag_drop_wnds, GG::Flags<GG::ModKey> mod_keys);
    virtual void        DragDropLeave();
    virtual void        AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt);
    void                Select(bool b);
    virtual void        SizeMove(const GG::Pt& ul, const GG::Pt& lr);

    mutable boost::signal<void (const std::vector<int>&)> NewFleetFromShipsSignal;

private:
    void                AggressionToggleButtonPressed();

    void                Refresh();
    void                SetStatIconValues();
    void                UpdateAggressionToggle();
    double              StatValue(const std::string& stat_name) const;
    std::string         StatTooltip(const std::string& stat_name) const;
    void                DoLayout();

    const int           m_fleet_id;
    const int           m_system_id;
    const bool          m_new_fleet_drop_target;
    bool                m_new_fleet_aggression;

    boost::signals::connection  m_fleet_connection;

    GG::Control*        m_fleet_icon;
    GG::TextControl*    m_fleet_name_text;
    GG::TextControl*    m_fleet_destination_text;
    GG::Button*         m_aggression_toggle;
    ScanlineControl*    m_scanline_control;

    std::vector<std::pair<std::string, StatisticIcon*> >    m_stat_icons;   // statistic icons and associated meter types

    bool                m_selected;
};

FleetDataPanel::FleetDataPanel(GG::X w, GG::Y h, int fleet_id) :
    Control(GG::X0, GG::Y0, w, h, GG::Flags<GG::WndFlag>()),
    m_fleet_id(fleet_id),
    m_system_id(INVALID_OBJECT_ID),
    m_new_fleet_drop_target(false),
    m_new_fleet_aggression(false),
    m_fleet_icon(0),
    m_fleet_name_text(0),
    m_fleet_destination_text(0),
    m_aggression_toggle(0),
    m_scanline_control(0),
    m_stat_icons(),
    m_selected(false)
{
    SetChildClippingMode(ClipToClient);
    m_fleet_name_text = new GG::TextControl(GG::X0, GG::Y0, GG::X1, LabelHeight(), "", ClientUI::GetFont(),
                                            ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    AttachChild(m_fleet_name_text);
    m_fleet_destination_text = new GG::TextControl(GG::X0, GG::Y0, GG::X1, LabelHeight(), "", ClientUI::GetFont(),
                                                   ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
    AttachChild(m_fleet_destination_text);

    if (TemporaryPtr<const Fleet> fleet = GetFleet(m_fleet_id)) {
        int tooltip_delay = GetOptionsDB().Get<int>("UI.tooltip-delay");

        // stat icon for fleet count
        StatisticIcon* icon = new StatisticIcon(GG::X0, GG::Y0, StatIconSize().x, StatIconSize().y,
                                 FleetCountIcon(), 0, 0, false);
        m_stat_icons.push_back(std::make_pair(COUNT_STAT_STRING, icon));
        icon->SetBrowseModeTime(tooltip_delay);
        icon->SetBrowseText(StatTooltip(COUNT_STAT_STRING));
        AttachChild(icon);

        // stat icon for fleet damage
        icon = new StatisticIcon(GG::X0, GG::Y0, StatIconSize().x, StatIconSize().y,
                                 DamageIcon(), 0, 0, false);
        m_stat_icons.push_back(std::make_pair(DAMAGE_STAT_STRING, icon));
        icon->SetBrowseModeTime(tooltip_delay);
        icon->SetBrowseText(StatTooltip(DAMAGE_STAT_STRING));
        AttachChild(icon);

        // stat icon for fleet structure
        icon = new StatisticIcon(GG::X0, GG::Y0, StatIconSize().x, StatIconSize().y,
                                 ClientUI::MeterIcon(METER_STRUCTURE), 0, 0, false);
        m_stat_icons.push_back(std::make_pair(STRUCTURE_STAT_STRING, icon));
        icon->SetBrowseModeTime(tooltip_delay);
        icon->SetBrowseText(StatTooltip(STRUCTURE_STAT_STRING));
        AttachChild(icon);

        // stat icon for fleet shields
        icon = new StatisticIcon(GG::X0, GG::Y0, StatIconSize().x, StatIconSize().y,
                                 ClientUI::MeterIcon(METER_SHIELD), 0, 0, false);
        m_stat_icons.push_back(std::make_pair(SHIELD_STAT_STRING, icon));
        icon->SetBrowseModeTime(tooltip_delay);
        icon->SetBrowseText(StatTooltip(SHIELD_STAT_STRING));
        AttachChild(icon);

        // stat icon for fleet fuel
        icon = new StatisticIcon(GG::X0, GG::Y0, StatIconSize().x, StatIconSize().y,
                                                ClientUI::MeterIcon(METER_FUEL), 0, 0, false);
        m_stat_icons.push_back(std::make_pair(MeterStatString(METER_FUEL), icon));
        icon->SetBrowseModeTime(tooltip_delay);
        icon->SetBrowseText(StatTooltip(MeterStatString(METER_FUEL)));
        AttachChild(icon);

        // stat icon for fleet speed
        icon = new StatisticIcon(GG::X0, GG::Y0, StatIconSize().x, StatIconSize().y,
                                 SpeedIcon(), 0, 0, false);
        m_stat_icons.push_back(std::make_pair(SPEED_STAT_STRING, icon));
        icon->SetBrowseModeTime(tooltip_delay);
        icon->SetBrowseText(StatTooltip(SPEED_STAT_STRING));
        AttachChild(icon);


        m_fleet_connection = GG::Connect(fleet->StateChangedSignal, &FleetDataPanel::Refresh, this);

        int client_empire_id = HumanClientApp::GetApp()->EmpireID();
        if (fleet->OwnedBy(client_empire_id) || fleet->GetVisibility(client_empire_id) >= VIS_FULL_VISIBILITY) {
            m_aggression_toggle = new GG::Button(GG::X0, GG::Y0, GG::X(16), GG::Y(16), "", ClientUI::GetFont(),
                                                 GG::CLR_WHITE, GG::CLR_ZERO, GG::ONTOP | GG::INTERACTIVE);
            AttachChild(m_aggression_toggle);
            GG::Connect(m_aggression_toggle->LeftClickedSignal, &FleetDataPanel::AggressionToggleButtonPressed, this);
        }
    }

    Refresh();
}

FleetDataPanel::FleetDataPanel(GG::X w, GG::Y h, int system_id, bool new_fleet_drop_target) :
    Control(GG::X0, GG::Y0, w, h, GG::INTERACTIVE),
    m_fleet_id(INVALID_OBJECT_ID),
    m_system_id(system_id),
    m_new_fleet_drop_target(new_fleet_drop_target), // should be true?
    m_new_fleet_aggression(false),
    m_fleet_icon(0),
    m_fleet_name_text(0),
    m_fleet_destination_text(0),
    m_aggression_toggle(0),
    m_scanline_control(0),
    m_stat_icons(),
    m_selected(false)
{
    SetChildClippingMode(ClipToClient);
    m_fleet_name_text = new GG::TextControl(GG::X0, GG::Y0, GG::X1, LabelHeight(), "", ClientUI::GetFont(),
                                            ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    AttachChild(m_fleet_name_text);
    m_fleet_destination_text = new GG::TextControl(GG::X0, GG::Y0, GG::X1, LabelHeight(), "", ClientUI::GetFont(),
                                                   ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
    AttachChild(m_fleet_destination_text);
    m_aggression_toggle = new GG::Button(GG::X0, GG::Y0, GG::X(16), GG::Y(16), "", ClientUI::GetFont(),
                                         GG::CLR_WHITE, GG::CLR_ZERO, GG::ONTOP | GG::INTERACTIVE);
    AttachChild(m_aggression_toggle);
    GG::Connect(m_aggression_toggle->LeftClickedSignal, &FleetDataPanel::AggressionToggleButtonPressed, this);

    Refresh();
}

GG::Pt FleetDataPanel::ClientUpperLeft() const
{ return UpperLeft() + GG::Pt(GG::X(DATA_PANEL_BORDER), GG::Y(DATA_PANEL_BORDER)); }

GG::Pt FleetDataPanel::ClientLowerRight() const
{ return LowerRight() - GG::Pt(GG::X(DATA_PANEL_BORDER), GG::Y(DATA_PANEL_BORDER));  }

bool FleetDataPanel::Selected() const
{ return m_selected; }

bool FleetDataPanel::NewFleetAggression() const
{ return m_new_fleet_aggression; }

void FleetDataPanel::Render() {
    // main background position and colour
    const GG::Clr& background_colour = ClientUI::WndColor();
    const GG::Pt ul = UpperLeft(), lr = LowerRight(), cul = ClientUpperLeft();

    // title background colour and position
    const GG::Clr& unselected_colour = ClientUI::WndOuterBorderColor();
    const GG::Clr& selected_colour = ClientUI::WndInnerBorderColor();
    GG::Clr border_colour = m_selected ? selected_colour : unselected_colour;
    if (Disabled())
        border_colour = DisabledColor(border_colour);
    const GG::Pt text_ul = cul + GG::Pt(DATA_PANEL_ICON_SPACE.x, GG::Y0);
    const GG::Pt text_lr = cul + GG::Pt(ClientWidth(),           LabelHeight());

    // render
    GG::FlatRectangle(ul,       lr,         background_colour,  border_colour, DATA_PANEL_BORDER);  // background and border
    GG::FlatRectangle(text_ul,  text_lr,    border_colour,      GG::CLR_ZERO,  0);                  // title background box
}

void FleetDataPanel::DragDropEnter(const GG::Pt& pt, const std::map<GG::Wnd*, GG::Pt>& drag_drop_wnds, GG::Flags<GG::ModKey> mod_keys) {
    if (Disabled()) {
        Select(false);
        return;
    }

    // select panel if all dragged Wnds can be dropped here...

    Select(true);   // default

    // make map from Wnd to bool to store whether each dropped Wnd is acceptable to drop here
    std::map<const GG::Wnd*, bool> drops_acceptable_map;
    for (std::map<GG::Wnd*, GG::Pt>::const_iterator it = drag_drop_wnds.begin(); it != drag_drop_wnds.end(); ++it)
        drops_acceptable_map[it->first] = false;

    // get whether each Wnd is dropable
    DropsAcceptable(drops_acceptable_map.begin(), drops_acceptable_map.end(), pt);

    // scan through wnds, looking for one that isn't dropable
    for (DropsAcceptableIter it = drops_acceptable_map.begin(); it != drops_acceptable_map.end(); ++it) {
        if (!it->second) {
            // wnd can't be dropped
            Select(false);
            break;
        }
    }
}

void FleetDataPanel::DragDropLeave()
{ Select(false); }

void FleetDataPanel::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last, const GG::Pt& pt) const {
    int this_client_empire_id = HumanClientApp::GetApp()->EmpireID();
    TemporaryPtr<const Fleet> this_panel_fleet = GetFleet(m_fleet_id);

    // for every Wnd being dropped...
    for (DropsAcceptableIter it = first; it != last; ++it) {
        it->second = false; // default

        // reject drops if not enabled
        if (this->Disabled())
            continue;

        // reject drops if a dropped Wnd isn't a valid ShipRow
        if (it->first->DragDropDataType() != SHIP_DROP_TYPE_STRING)
            continue;

        // reject drops if a ship being dropped doesn't exist
        const ShipRow* ship_row = boost::polymorphic_downcast<const ShipRow*>(it->first);
        if (!ship_row)
            continue;
        TemporaryPtr<const Ship> ship = GetShip(ship_row->ShipID());
        if (!ship)
            continue;

        // reject drops if the ship is not owned by this client's empire
        if (!ship->OwnedBy(this_client_empire_id))
            continue;

        if (m_new_fleet_drop_target) {
            // reject drops of ships not located in the same system as this drop target
            if (ship->SystemID() != m_system_id || m_system_id == INVALID_OBJECT_ID)
                continue;
        } else {
            // reject drops if this panel represents a fleet, but this client's
            // empire does not own it.
            if (this_panel_fleet && !this_panel_fleet->OwnedBy(this_client_empire_id))
                continue;
        }

        // all tests passed; can drop
        it->second = true;
    }
}

void FleetDataPanel::AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt) {
    Logger().debugStream() << "FleetWnd::AcceptDrops with " << wnds.size() << " wnds at pt: " << pt;
    std::vector<int> ship_ids;
    ship_ids.reserve(wnds.size());
    for (std::vector<Wnd*>::const_iterator it = wnds.begin(); it != wnds.end(); ++it)
        if (const ShipRow* ship_row = boost::polymorphic_downcast<const ShipRow*>(*it))
            ship_ids.push_back(ship_row->ShipID());
    std::string id_list;
    for (std::vector<int>::const_iterator it = ship_ids.begin(); it != ship_ids.end(); ++it)
        id_list += boost::lexical_cast<std::string>(*it) + " ";
    Logger().debugStream() << "FleetWnd::AcceptDrops found " << ship_ids.size() << " ship ids: " << id_list;

    if (ship_ids.empty())
        return;

    NewFleetFromShipsSignal(ship_ids);
}

void FleetDataPanel::Select(bool b) {
    if (m_selected == b)
        return;
    m_selected = b;

    const GG::Clr& unselected_text_color = ClientUI::TextColor();
    const GG::Clr& selected_text_color = GG::CLR_BLACK;

    GG::Clr text_color_to_use = m_selected ? selected_text_color : unselected_text_color;

    if (Disabled())
        text_color_to_use = DisabledColor(text_color_to_use);
    if (m_fleet_name_text)
        m_fleet_name_text->SetTextColor(text_color_to_use);
    if (m_fleet_destination_text)
        m_fleet_destination_text->SetTextColor(text_color_to_use);
}

void FleetDataPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    GG::Control::SizeMove(ul, lr);
    //std::cout << "FleetDataPanel::SizeMove new size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
    if (old_size != Size())
        DoLayout();
}

void FleetDataPanel::AggressionToggleButtonPressed() {
    if (!m_aggression_toggle)
        return;
    TemporaryPtr<const Fleet> fleet = GetFleet(m_fleet_id);
    if (fleet) {
        if (!ClientPlayerIsModerator()) {
            int client_empire_id = HumanClientApp::GetApp()->EmpireID();
            if (client_empire_id == ALL_EMPIRES)
                return;

            bool new_aggression_State = !fleet->Aggressive();

            // toggle fleet aggression status
            HumanClientApp::GetApp()->Orders().IssueOrder(
                OrderPtr(new AggressiveOrder(client_empire_id, m_fleet_id, new_aggression_State)));
        } else {
            // TODO: moderator action to toggle fleet aggression
        }
    } else if (m_new_fleet_drop_target) {
        // toggle new fleet aggression
        m_new_fleet_aggression = !m_new_fleet_aggression;
        UpdateAggressionToggle();
    }
}

namespace {
    boost::shared_ptr<GG::Texture> FleetAggressiveIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "fleet_aggressive.png"); }
    boost::shared_ptr<GG::Texture> FleetAggressiveMouseoverIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "fleet_aggressive_mouseover.png"); }
    boost::shared_ptr<GG::Texture> FleetPassiveIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "fleet_passive.png"); }
    boost::shared_ptr<GG::Texture> FleetPassiveMouseoverIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "fleet_passive_mouseover.png"); }
}

void FleetDataPanel::Refresh() {
    delete m_fleet_icon;
    m_fleet_icon = 0;
    delete m_scanline_control;
    m_scanline_control = 0;

    if (m_new_fleet_drop_target) {
        m_fleet_name_text->SetText(UserString("FW_NEW_FLEET_LABEL"));
        m_fleet_destination_text->Clear();

    } else if (TemporaryPtr<const Fleet> fleet = GetFleet(m_fleet_id)) {
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();
        // set fleet name and destination text
        m_fleet_name_text->SetText(fleet->PublicName(client_empire_id));
        m_fleet_destination_text->SetText(FleetDestinationText(m_fleet_id));

        // set icons
        std::vector<boost::shared_ptr<GG::Texture> > icons;
        std::vector<GG::Flags<GG::GraphicStyle> > styles;

        boost::shared_ptr<GG::Texture> head_icon = FleetHeadIcon(fleet, FleetButton::FLEET_BUTTON_LARGE);
        icons.push_back(head_icon);
        styles.push_back(DataPanelIconStyle());

        boost::shared_ptr<GG::Texture> size_icon = FleetSizeIcon(fleet, FleetButton::FLEET_BUTTON_LARGE);
        icons.push_back(size_icon);
        styles.push_back(DataPanelIconStyle());

        m_fleet_icon = new MultiTextureStaticGraphic(GG::X0, GG::Y0, DATA_PANEL_ICON_SPACE.x, ClientHeight(), icons, styles);
        AttachChild(m_fleet_icon);

        if (Empire* empire = Empires().Lookup(fleet->Owner()))
            m_fleet_icon->SetColor(empire->Color());
        else if (fleet->Unowned() && fleet->HasMonsters())
            m_fleet_icon->SetColor(GG::CLR_RED);

        if (fleet->GetVisibility(client_empire_id) < VIS_BASIC_VISIBILITY) {
            m_scanline_control = new ScanlineControl(GG::X0, GG::Y0, DATA_PANEL_ICON_SPACE.x, ClientHeight(), true);
            AttachChild(m_scanline_control);
        }

        SetStatIconValues();
    }

    UpdateAggressionToggle();
    DoLayout();
}

void FleetDataPanel::SetStatIconValues() {
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    const std::set<int>& this_client_known_destroyed_objects = GetUniverse().EmpireKnownDestroyedObjectIDs(client_empire_id);
    const std::set<int>& this_client_stale_object_info = GetUniverse().EmpireStaleKnowledgeObjectIDs(client_empire_id);
    int ship_count =        0;
    float damage_tally =    0.0;
    float structure_tally = 0.0;
    float shield_tally =    0.0;
    float min_fuel =        0;
    float min_speed =       0;
    std::vector<float> fuels;
    std::vector<float> speeds;

    TemporaryPtr<const Fleet> fleet = GetFleet(m_fleet_id);

    fuels.reserve(fleet->NumShips());
    speeds.reserve(fleet->NumShips());
    std::vector<TemporaryPtr<const Ship> > ships = Objects().FindObjects<const Ship>(fleet->ShipIDs());
    for (std::vector<TemporaryPtr<const Ship> >::const_iterator it = ships.begin();
         it != ships.end(); ++it)
    {
        TemporaryPtr<const Ship> ship = *it;
        int ship_id = ship->ID();
        // skip known destroyed and stale info objects
        if (this_client_known_destroyed_objects.find(ship_id) != this_client_known_destroyed_objects.end())
            continue;
        if (this_client_stale_object_info.find(ship_id) != this_client_stale_object_info.end())
            continue;

        if (ship->Design()) {
            ship_count++;
            damage_tally += ship->TotalWeaponsDamage();
            structure_tally += ship->CurrentMeterValue(METER_STRUCTURE);
            shield_tally += ship->CurrentMeterValue(METER_SHIELD);
            fuels.push_back(ship->CurrentMeterValue(METER_FUEL));
            speeds.push_back(ship->CurrentMeterValue(METER_STARLANE_SPEED));
        }
    }
    if (!fuels.empty())
        min_fuel = *std::min_element(fuels.begin(), fuels.end());
    if (!speeds.empty())
        min_speed = *std::min_element(speeds.begin(), speeds.end());
    for (std::vector<std::pair<std::string, StatisticIcon*> >::const_iterator it =
        m_stat_icons.begin(); it != m_stat_icons.end(); ++it) 
    {
        const std::string stat_name = it->first;
        if (stat_name == SPEED_STAT_STRING)
            it->second->SetValue(min_speed); 
        else if (stat_name == MeterStatString(METER_FUEL))
            it->second->SetValue(min_fuel);
        else if (stat_name == SHIELD_STAT_STRING)
            it->second->SetValue(shield_tally/ship_count);
        else if (stat_name == STRUCTURE_STAT_STRING)
            it->second->SetValue(structure_tally);
        else if (stat_name == DAMAGE_STAT_STRING)
            it->second->SetValue(damage_tally);
        else if (stat_name == COUNT_STAT_STRING)
            it->second->SetValue(ship_count);
    }
}

void FleetDataPanel::UpdateAggressionToggle() {
    if (!m_aggression_toggle)
        return;
    int tooltip_delay = GetOptionsDB().Get<int>("UI.tooltip-delay");
    m_aggression_toggle->SetBrowseModeTime(tooltip_delay);

    bool aggressive = false;

    if (m_new_fleet_drop_target) {
        aggressive = m_new_fleet_aggression;
    } else if (TemporaryPtr<const Fleet> fleet = GetFleet(m_fleet_id)) {
        aggressive = fleet->Aggressive();
    } else {
        DetachChild(m_aggression_toggle);
        return;
    }

    if (aggressive) {
        m_aggression_toggle->SetUnpressedGraphic(GG::SubTexture(FleetAggressiveIcon(),          GG::X0, GG::Y0, GG::X(64), GG::Y(64)));
        m_aggression_toggle->SetPressedGraphic  (GG::SubTexture(FleetPassiveIcon(),             GG::X0, GG::Y0, GG::X(64), GG::Y(64)));
        m_aggression_toggle->SetRolloverGraphic (GG::SubTexture(FleetAggressiveMouseoverIcon(), GG::X0, GG::Y0, GG::X(64), GG::Y(64)));
        boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd(new IconTextBrowseWnd(
            FleetAggressiveIcon(), UserString("FW_AGGRESSIVE"), UserString("FW_AGGRESSIVE_DESC")));
        m_aggression_toggle->SetBrowseInfoWnd(browse_wnd);
    } else {
        m_aggression_toggle->SetUnpressedGraphic(GG::SubTexture(FleetPassiveIcon(),             GG::X0, GG::Y0, GG::X(64), GG::Y(64)));
        m_aggression_toggle->SetPressedGraphic  (GG::SubTexture(FleetAggressiveIcon(),          GG::X0, GG::Y0, GG::X(64), GG::Y(64)));
        m_aggression_toggle->SetRolloverGraphic (GG::SubTexture(FleetPassiveMouseoverIcon(),    GG::X0, GG::Y0, GG::X(64), GG::Y(64)));
        boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd(new IconTextBrowseWnd(
            FleetPassiveIcon(), UserString("FW_PASSIVE"), UserString("FW_PASSIVE_DESC")));
        m_aggression_toggle->SetBrowseInfoWnd(browse_wnd);
    }
}

std::string FleetDataPanel::StatTooltip(const std::string& stat_name) const {
    if (stat_name == SPEED_STAT_STRING)
        return UserString("FW_FLEET_SPEED_SUMMARY");
    else if (stat_name ==  MeterStatString(METER_FUEL))
        return UserString("FW_FLEET_FUEL_SUMMARY");
    else if (stat_name == SHIELD_STAT_STRING)
        return UserString("FW_FLEET_SHIELD_SUMMARY");
    else if (stat_name == STRUCTURE_STAT_STRING)
        return UserString("FW_FLEET_STRUCTURE_SUMMARY");
    else if (stat_name == DAMAGE_STAT_STRING) 
        return UserString("FW_FLEET_DAMAGE_SUMMARY");
    else if (stat_name == COUNT_STAT_STRING) 
        return UserString("FW_FLEET_COUNT_SUMMARY");
    else
        return "";
}

void FleetDataPanel::DoLayout() {
    if (m_fleet_icon) {
        // fleet icon will scale and position itself in the provided space
        m_fleet_icon->Resize(GG::Pt(DATA_PANEL_ICON_SPACE.x, ClientHeight()));
    }
    if (m_scanline_control)
        m_scanline_control->Resize(GG::Pt(DATA_PANEL_ICON_SPACE.x, ClientHeight()));

    // position fleet name and destination texts
    const GG::Pt name_ul = GG::Pt(DATA_PANEL_ICON_SPACE.x + DATA_PANEL_TEXT_PAD, GG::Y0);
    const GG::Pt name_lr = GG::Pt(ClientWidth() - DATA_PANEL_TEXT_PAD - GG::X(Value(LabelHeight())),    LabelHeight());
    m_fleet_name_text->SizeMove(name_ul, name_lr);
    m_fleet_destination_text->SizeMove(name_ul, name_lr);

    // position stat icons, centering them vertically if there's more space than required
    GG::Pt icon_ul = GG::Pt(name_ul.x, LabelHeight() + std::max(GG::Y0, (ClientHeight() - LabelHeight() - StatIconSize().y) / 2));
    for (std::vector<std::pair<std::string, StatisticIcon*> >::const_iterator it = m_stat_icons.begin(); it != m_stat_icons.end(); ++it) {
        it->second->SizeMove(icon_ul, icon_ul + StatIconSize());
        icon_ul.x += StatIconSize().x;
    }

    // position aggression toggle / indicator
    if (m_aggression_toggle) {
        GG::Pt toggle_size(GG::X(Value(LabelHeight())), LabelHeight());
        GG::Pt toggle_ul = GG::Pt(ClientWidth() - toggle_size.x, GG::Y0);
        m_aggression_toggle->SizeMove(toggle_ul, toggle_ul + toggle_size);
    }
}

namespace {
    ////////////////////////////////////////////////
    // FleetRow
    ////////////////////////////////////////////////
    /** A ListBox::Row subclass used to represent fleets in FleetsListBoxes. */
    class FleetRow : public GG::ListBox::Row {
    public:
        FleetRow(int fleet_id, GG::X w, GG::Y h) :
            GG::ListBox::Row(w, h, GetFleet(fleet_id) ? FLEET_DROP_TYPE_STRING : ""),
            m_fleet_id(fleet_id),
            m_panel(0)
        {
            SetName("FleetRow");
            SetChildClippingMode(ClipToClient);
            m_panel = new FleetDataPanel(w, h, m_fleet_id);
            push_back(m_panel);
        }

        virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            const GG::Pt old_size = Size();
            GG::ListBox::Row::SizeMove(ul, lr);
            //std::cout << "FleetRow::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
            if (!empty() && old_size != Size() && m_panel)
                m_panel->Resize(Size());
        }

        int             FleetID() const {return m_fleet_id;}
    private:
        int             m_fleet_id;
        FleetDataPanel* m_panel;
    };
}

////////////////////////////////////////////////
// FleetsListBox
////////////////////////////////////////////////
/** A CUIListBox subclass used to list all the fleets, and handle drag-and-drop
  * operations on them, in FleetWnd. */
class FleetsListBox : public CUIListBox {
public:
    FleetsListBox(GG::X x, GG::Y y, GG::X w, GG::Y h, bool order_issuing_enabled) :
        CUIListBox(x, y, w, h),
        m_highlighted_row_it(end()),
        m_order_issuing_enabled(order_issuing_enabled)
    { InitRowSizes(); }

    FleetsListBox(bool order_issuing_enabled) :
        CUIListBox(GG::X0, GG::Y0, GG::X1, GG::Y1),
        m_highlighted_row_it(end()),
        m_order_issuing_enabled(order_issuing_enabled)
    { InitRowSizes(); }

    void            EnableOrderIssuing(bool enable) {
        m_order_issuing_enabled = enable;
    }

    virtual void    DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last, const GG::Pt& pt) const {
        // default result, possibly to be updated later: reject all drops
        for (DropsAcceptableIter it = first; it != last; ++it)
            it->second = false;

        // early termination check: if this FleetsListBox does not presently allow order
        // issuing, all drops are unacceptable
        if (!m_order_issuing_enabled)
            return;

        // semi-early termination if pt is not over a FleetRow, all drops are unacceptable.
        // this is because ships can't be dropped into an empty spot of the FleetsListBox;
        // ships must be dropped into existing fleet panels / rows, or onto the new fleet
        // drop target.
        // as well, there is presently no way to drop a FleetRow into a FleetsListBox that
        // the row isn't already in, so if the drop isn't onto another FleetRow, there's
        // no sense in doing such a drop.

        // get FleetRow under drop point
        iterator row = RowUnderPt(pt);

        // if drop point isn't over a FleetRow, reject all drops (default case)
        if (row == end())
            return;

        // extract drop target fleet from row under drop point
        TemporaryPtr<const Fleet> target_fleet;
        if (const FleetRow* fleet_row = boost::polymorphic_downcast<const FleetRow*>(*row))
            target_fleet = GetFleet(fleet_row->FleetID());

        // loop through dropped Wnds, checking if each is a valid ship or fleet.  this doesn't
        // consider whether there is a mixture of fleets and ships, as each row is considered
        // independently.  actual drops will probably only accept one or the other, not a mixture
        // of fleets and ships being dropped simultaneously.
        for (DropsAcceptableIter it = first; it != last; ++it) {
            // for either of fleet or ship being dropped, check if merge or transfer is valid.
            // if any of the nested if's fail, the default rejection of the drop will remain set
            if (it->first->DragDropDataType() == FLEET_DROP_TYPE_STRING) {
                if (const FleetRow* fleet_row = boost::polymorphic_downcast<const FleetRow*>(it->first))
                    if (TemporaryPtr<const Fleet> fleet = GetFleet(fleet_row->FleetID()))
                        it->second = ValidFleetMerge(fleet, target_fleet);

            } else if (it->first->DragDropDataType() == SHIP_DROP_TYPE_STRING) {
                if (const ShipRow* ship_row = boost::polymorphic_downcast<const ShipRow*>(it->first))
                    if (TemporaryPtr<const Ship> ship = GetShip(ship_row->ShipID()))
                        it->second = ValidShipTransfer(ship, target_fleet);
            } else {
                // no valid drop type string
                Logger().errorStream() << "FleetsListBox unrecognized drop type: " << it->first->DragDropDataType();
            }
        }
    }

    virtual void    AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt) {
        //std::cout << "FleetsListBox::AcceptDrops" << std::endl;
        assert(!wnds.empty());

        iterator drop_target_row = RowUnderPt(pt);
        assert(m_order_issuing_enabled && drop_target_row != end());


        // get drop target fleet
        const FleetRow* drop_target_fleet_row = boost::polymorphic_downcast<FleetRow*>(*drop_target_row);
        assert(drop_target_fleet_row);

        TemporaryPtr<Fleet> target_fleet = GetFleet(drop_target_fleet_row->FleetID());
        assert(target_fleet);

        int target_fleet_id = target_fleet->ID();


        // sort dropped Wnds to extract fleets or ships dropped.  (should only be one or the other in a given drop)
        std::vector<TemporaryPtr<Fleet> > dropped_fleets;
        std::vector<TemporaryPtr<Ship> > dropped_ships;

        for (std::vector<Wnd*>::const_iterator it = wnds.begin(); it != wnds.end(); ++it) {
            const GG::Wnd* wnd = *it;

            if (wnd->DragDropDataType() == FLEET_DROP_TYPE_STRING) {
                const FleetRow* fleet_row = boost::polymorphic_downcast<const FleetRow*>(wnd);
                assert(fleet_row);
                dropped_fleets.push_back(GetFleet(fleet_row->FleetID()));

            } else if (wnd->DragDropDataType() == SHIP_DROP_TYPE_STRING) {
                const ShipRow* ship_row = boost::polymorphic_downcast<const ShipRow*>(wnd);
                assert(ship_row);
                dropped_ships.push_back(GetShip(ship_row->ShipID()));
            }
        }

        assert(dropped_ships.empty() != dropped_fleets.empty());    // should only be dropping fleets or ships, not a mix of both
        int empire_id = HumanClientApp::GetApp()->EmpireID();

        if (!dropped_fleets.empty()) {
            // dropping fleets.  get each ships of all source fleets and transfer to the target fleet
            //std::cout << ".... dropped " << dropped_fleets.size() << " fleets" << std::endl;

            for (std::vector<TemporaryPtr<Fleet> >::const_iterator it = dropped_fleets.begin(); it != dropped_fleets.end(); ++it) {
                TemporaryPtr<const Fleet> dropped_fleet = *it;
                assert(dropped_fleet);
                int dropped_fleet_id = dropped_fleet->ID();

                // get fleet's ships in a vector, as this is what FleetTransferOrder takes
                const std::set<int>& ship_ids_set = dropped_fleet->ShipIDs();
                const std::vector<int> ship_ids_vec(ship_ids_set.begin(), ship_ids_set.end());

                if (!ClientPlayerIsModerator()) {
                    // order the transfer
                    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(
                        new FleetTransferOrder(empire_id, dropped_fleet_id, target_fleet_id, ship_ids_vec)));

                    // delete empty fleets
                    if (dropped_fleet->Empty())
                        HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(
                            new DeleteFleetOrder(empire_id, dropped_fleet_id)));
                } else {
                    // TODO: moderator action to transfer ships / remove empty fleets
                }
            }

        } else if (!dropped_ships.empty()) {
            // dropping ships.  transfer to target fleet.
            //std::cout << ".... dropped " << dropped_ships.size() << " ships" << std::endl;

            // get source fleet of ship(s).  assumes all ships are from the same source fleet.
            TemporaryPtr<const Ship> first_ship = dropped_ships[0];
            assert(first_ship);
            int fleet_id = first_ship->FleetID();

            // compile ship IDs into a vector, while also recording original fleets from which ships are being taken
            std::vector<int> ship_ids_vec;
            std::set<int> dropped_ships_fleets;
            for (std::vector<TemporaryPtr<Ship> >::const_iterator it = dropped_ships.begin(); it != dropped_ships.end(); ++it) {
                TemporaryPtr<const Ship> ship = *it;
                assert(ship);
                ship_ids_vec.push_back(ship->ID());
                dropped_ships_fleets.insert(ship->FleetID());
            }

            if (!ClientPlayerIsModerator()) {
                // order the transfer
                HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(
                    new FleetTransferOrder(empire_id, fleet_id, target_fleet_id, ship_ids_vec)));

                // delete empty fleets
                for (std::set<int>::const_iterator it = dropped_ships_fleets.begin(); it != dropped_ships_fleets.end(); ++it) {
                    TemporaryPtr<const Fleet> fleet = GetFleet(*it);
                    if (fleet && fleet->Empty())
                        HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(
                            new DeleteFleetOrder(empire_id, fleet->ID())));
                }
            } else {
                    // TODO: moderator action to transfer ships / remove empty fleets
            }
        }
    }

    virtual void    DragDropEnter(const GG::Pt& pt, const std::map<Wnd*, GG::Pt>& drag_drop_wnds, GG::Flags<GG::ModKey> mod_keys)
    { DragDropHere(pt, drag_drop_wnds, mod_keys); }

    virtual void    DragDropHere(const GG::Pt& pt, const std::map<Wnd*, GG::Pt>& drag_drop_wnds, GG::Flags<GG::ModKey> mod_keys) {
        CUIListBox::DragDropHere(pt, drag_drop_wnds, mod_keys);

        // default to removing highlighting of any row that has it.
        // used to check: if (m_highlighted_row_it != row_it) before doing this...
        ClearHighlighting();

        // abort if this FleetsListBox can't be manipulated
        if (!m_order_issuing_enabled)
            return;

        // get FleetRow under drop point
        iterator row_it = RowUnderPt(pt);
        if (row_it == end())
            return; // not over a valid row

        // check if row under drop point is already selected.  if it is, don't
        // need to highlight it, since as of this writing, the two are the same
        // thing, visually and internally
        GG::ListBox::Row* drop_target_row = *row_it;
        assert(drop_target_row);
        assert(!drop_target_row->empty());

        GG::Control* control = (*drop_target_row)[0];
        assert(control);

        FleetDataPanel* drop_target_data_panel = boost::polymorphic_downcast<FleetDataPanel*>(control);
        assert(drop_target_data_panel);

        if (drop_target_data_panel->Selected())
            return;

        FleetRow* drop_target_fleet_row = boost::polymorphic_downcast<FleetRow*>(drop_target_row);
        assert(drop_target_fleet_row);

        TemporaryPtr<Fleet> drop_target_fleet = GetFleet(drop_target_fleet_row->FleetID());
        assert(drop_target_fleet);

        // use DropsAcceptable to check whether wnds being dragged over can be dropped.

        // make map from Wnd to bool indicating whether it is acceptable to drop here
        std::map<const GG::Wnd*, bool> drops_acceptable_map;
        for (std::map<GG::Wnd*, GG::Pt>::const_iterator it = drag_drop_wnds.begin(); it != drag_drop_wnds.end(); ++it)
            drops_acceptable_map[it->first] = false;

        // get whether each Wnd is dropable
        DropsAcceptable(drops_acceptable_map.begin(), drops_acceptable_map.end(), pt);


        // scan through results in drops_acceptable_map and decide whether overall
        // drop is acceptable.  to be acceptable, all wnds must individually be
        // acceptable for dropping, and there must not be a mix of ships and fleets
        // being dropped.
        bool fleets_seen = false;
        bool ships_seen = false;

        for (DropsAcceptableIter it = drops_acceptable_map.begin(); it != drops_acceptable_map.end(); ++it) {
            if (!it->second)
                return; // a row was an invalid drop. abort without highlighting drop target.

            const GG::Wnd* dropped_wnd = it->first;
            if (dropped_wnd->DragDropDataType() == FLEET_DROP_TYPE_STRING) {
                fleets_seen = true;
                if (ships_seen)
                    return; // can't drop both at once

                const FleetRow* fleet_row = boost::polymorphic_downcast<const FleetRow*>(dropped_wnd);
                assert(fleet_row);
                TemporaryPtr<Fleet> fleet = GetFleet(fleet_row->FleetID());

                if (!ValidFleetMerge(fleet, drop_target_fleet))
                    return; // not a valid drop

            } else if (dropped_wnd->DragDropDataType() == SHIP_DROP_TYPE_STRING) {
                ships_seen = true;
                if (fleets_seen)
                    return; // can't drop both at once

                const ShipRow* ship_row = boost::polymorphic_downcast<const ShipRow*>(dropped_wnd);
                assert(ship_row);
                TemporaryPtr<Ship> ship = GetShip(ship_row->ShipID());

                if (!ValidShipTransfer(ship, drop_target_fleet))
                    return; // not a valid drop
            }
        }

        // passed all checks.  drop is valid!
        HighlightRow(row_it);
    }

    virtual void    DragDropLeave()
    { ClearHighlighting(); }

    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
        const GG::Pt old_size = Size();
        CUIListBox::SizeMove(ul, lr);
        //std::cout << "FleetListBox::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
        if (old_size != Size()) {
            const GG::Pt row_size = ListRowSize();
            //std::cout << "FleetListBox::SizeMove list row size: (" << Value(row_size.x) << ", " << Value(row_size.y) << ")" << std::endl;
            for (GG::ListBox::iterator it = begin(); it != end(); ++it)
                (*it)->Resize(row_size);
        }
    }

    GG::Pt          ListRowSize() const
    { return GG::Pt(Width() - ClientUI::ScrollWidth() - 5, ListRowHeight()); }

private:
    void            HighlightRow(iterator row_it) {
        if (row_it == end())
            return;

        if (row_it == m_highlighted_row_it)
            return;

        // get FleetDataPanel of row pointed to by row_it
        GG::ListBox::Row* selected_row = *row_it;
        assert(selected_row);
        assert(!selected_row->empty());
        GG::Control* control = (*selected_row)[0];
        FleetDataPanel* data_panel = boost::polymorphic_downcast<FleetDataPanel*>(control);
        assert(data_panel);

        // don't need to select and shouldn't store as highlighted if row is actually already selected in ListBox itself
        if (data_panel->Selected())
            return;

        // mark data panel selected, which indicates highlighting
        data_panel->Select(true);
        m_highlighted_row_it = row_it;
    }

    void            ClearHighlighting() {
        if (m_highlighted_row_it == end())
            return;

        GG::ListBox::Row* selected_row = *m_highlighted_row_it;
        assert(selected_row);
        assert(!selected_row->empty());
        GG::Control* control = (*selected_row)[0];
        FleetDataPanel* data_panel = boost::polymorphic_downcast<FleetDataPanel*>(control);
        assert(data_panel);

        data_panel->Select(false);
        m_highlighted_row_it = end();
    }

    void            InitRowSizes() {
        // preinitialize listbox/row column widths, because what
        // ListBox::Insert does on default is not suitable for this case
        SetNumCols(1);
        SetColWidth(0, GG::X0);
        LockColWidths();
    }

    iterator    m_highlighted_row_it;
    bool        m_order_issuing_enabled;
};

////////////////////////////////////////////////
// ShipsListBox
////////////////////////////////////////////////
/** A CUIListBox subclass used to list all the ships, and handle drag-and-drop
  * operations on them, in FleetDetailPanel. */
class ShipsListBox : public CUIListBox {
public:
    ShipsListBox(GG::X x, GG::Y y, GG::X w, GG::Y h, int fleet_id, bool order_issuing_enabled) :
        CUIListBox(x, y, w, h),
        m_fleet_id(fleet_id),
        m_order_issuing_enabled(order_issuing_enabled)
    {}

    ShipsListBox(int fleet_id, bool order_issuing_enabled) :
        CUIListBox(GG::X0, GG::Y0, GG::X1, GG::Y1),
        m_fleet_id(fleet_id),
        m_order_issuing_enabled(order_issuing_enabled)
    {}

    void            Refresh() {
        ScopedTimer timer("ShipsListBox::Refresh");

        TemporaryPtr<const Fleet> fleet = GetFleet(m_fleet_id);
        if (!fleet) {
            Clear();
            return;
        }

        const GG::Pt row_size = ListRowSize();
        Clear();

        // repopulate list with ships in current fleet

        // preinitialize listbox/row column widths, because what
        // ListBox::Insert does on default is not suitable for this case
        SetNumCols(1);
        SetColWidth(0, GG::X0);
        LockColWidths();

        int this_client_empire_id = HumanClientApp::GetApp()->EmpireID();
        const std::set<int>& this_client_known_destroyed_objects =
            GetUniverse().EmpireKnownDestroyedObjectIDs(this_client_empire_id);
        const std::set<int>& this_client_stale_object_info =
            GetUniverse().EmpireStaleKnowledgeObjectIDs(this_client_empire_id);

        const std::set<int> ship_ids = fleet->ShipIDs();
        for (std::set<int>::const_iterator it = ship_ids.begin(); it != ship_ids.end(); ++it) {
            int ship_id = *it;

            // skip known destroyed and stale info objects
            if (this_client_known_destroyed_objects.find(ship_id) != this_client_known_destroyed_objects.end())
                continue;
            if (this_client_stale_object_info.find(ship_id) != this_client_stale_object_info.end())
                continue;

            ShipRow* row = new ShipRow(GG::X1, row_size.y, ship_id);
            Insert(row); //ShipsListBox::iterator row_it = Insert(row);
            row->Resize(row_size);
        }

        SelChangedSignal(this->Selections());
    }

    void            SetFleet(int fleet_id) {
        if (m_fleet_id == fleet_id)
            return;

        m_fleet_id = fleet_id;
        Refresh();
    }

    void            EnableOrderIssuing(bool enable) {
        m_order_issuing_enabled = enable;
    }

    virtual void    DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last, const GG::Pt& pt) const {
        for (DropsAcceptableIter it = first; it != last; ++it) {
            it->second = false; // default

            if (!m_order_issuing_enabled)
                continue;

            const ShipRow* ship_row = dynamic_cast<const ShipRow*>(it->first);
            if (!ship_row)
                continue;

            TemporaryPtr<const Ship> ship = GetShip(ship_row->ShipID());
            if (!ship) {
                Logger().errorStream() << "ShipsListBox::DropsAcceptable couldn't get ship for ship row";
                continue;
            }

            TemporaryPtr<const Fleet> fleet = GetFleet(ship->FleetID());
            if (!fleet) {
                Logger().errorStream() << "ShipsListBox::DropsAcceptable couldn't get fleet with id " << ship->FleetID();
                continue;
            }

            if (ship && ValidShipTransfer(ship, fleet))
                continue;   // leave false: ship transfer not valid

            // all tests passed; can drop
            it->second = true;
        }
    }

    virtual void    AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt) {
        if (wnds.empty())
            return;

        TemporaryPtr<Ship> ship_from_dropped_wnd;
        std::vector<int> ship_ids;
        for (std::vector<GG::Wnd*>::const_iterator it = wnds.begin(); it != wnds.end(); ++it) {
            const GG::Wnd* wnd = *it;
            if (wnd->DragDropDataType() == SHIP_DROP_TYPE_STRING) {
                const ShipRow* ship_row = boost::polymorphic_downcast<const ShipRow*>(wnd);
                assert(ship_row);
                ship_ids.push_back(ship_row->ShipID());
                ship_from_dropped_wnd = GetShip(ship_row->ShipID());
            }
        }

        if (!ship_from_dropped_wnd)
            return;

        int dropped_ship_fleet_id = ship_from_dropped_wnd->FleetID();
        int empire_id = HumanClientApp::GetApp()->EmpireID();

        if (!ClientPlayerIsModerator()) {
            HumanClientApp::GetApp()->Orders().IssueOrder(
                OrderPtr(new FleetTransferOrder(empire_id, dropped_ship_fleet_id, m_fleet_id, ship_ids)));

            // delete old fleet if now empty
            const ObjectMap& objects = GetUniverse().Objects();
            if (TemporaryPtr<const Fleet> dropped_ship_old_fleet = objects.Object<Fleet>(dropped_ship_fleet_id))
                if (dropped_ship_old_fleet->Empty())
                    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(
                        new DeleteFleetOrder(empire_id, dropped_ship_fleet_id)));
        } else {
                    // TODO: moderator action to transfer ships / remove empty fleets
        }
    }

    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
        const GG::Pt old_size = Size();
        CUIListBox::SizeMove(ul, lr);
        //std::cout << "ShipsListBox::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
        if (old_size != Size()) {
            const GG::Pt row_size = ListRowSize();
            //std::cout << "ShipsListBox::SizeMove list row size: (" << Value(row_size.x) << ", " << Value(row_size.y) << ")" << std::endl;
            for (GG::ListBox::iterator it = begin(); it != end(); ++it)
                (*it)->Resize(row_size);
        }
    }

    GG::Pt          ListRowSize() const {
        return GG::Pt(Width() - ClientUI::ScrollWidth() - 5, ListRowHeight());
    }
private:
    int     m_fleet_id;
    bool    m_order_issuing_enabled;
};

////////////////////////////////////////////////
// FleetDetailPanel
////////////////////////////////////////////////
/** Used in lower half of FleetWnd to show the
  * ships in a fleet, and some basic info about the fleet. */
class FleetDetailPanel : public GG::Wnd {
public:
    FleetDetailPanel(GG::X w, GG::Y h, int fleet_id, bool order_issuing_enabled, GG::Flags<GG::WndFlag> flags = GG::Flags<GG::WndFlag>()); ///< ctor

    int             FleetID() const;
    std::set<int>   SelectedShipIDs() const;    ///< returns ids of ships selected in the detail panel's ShipsListBox

    void            SetFleet(int fleet_id);                         ///< sets the currently-displayed Fleet.  setting to INVALID_OBJECT_ID shows no fleet
    void            SetSelectedShips(const std::set<int>& ship_ids);///< sets the currently-selected ships in the ships list

    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);

    void            EnableOrderIssuing(bool enabled = true);

    mutable boost::signal<void (const ShipsListBox::SelectionSet&)> SelectedShipsChangedSignal; ///< emitted when the set of selected ships changes
    mutable boost::signal<void (int)>                               ShipRightClickedSignal;

private:
    int             GetShipIDOfListRow(GG::ListBox::iterator it) const; ///< returns the ID number of the ship in row \a row_idx of the ships listbox
    void            Refresh();
    void            DoLayout();
    void            UniverseObjectDeleted(TemporaryPtr<const UniverseObject> obj);
    void            ShipSelectionChanged(const GG::ListBox::SelectionSet& rows);
    void            ShipBrowsed(GG::ListBox::iterator it);
    void            ShipRightClicked(GG::ListBox::iterator it, const GG::Pt& pt);
    int             ShipInRow(GG::ListBox::iterator it) const;

    int                         m_fleet_id;
    bool                        m_order_issuing_enabled;
    boost::signals::connection  m_fleet_connection;

    ShipsListBox*               m_ships_lb;
};

FleetDetailPanel::FleetDetailPanel(GG::X w, GG::Y h, int fleet_id, bool order_issuing_enabled, GG::Flags<GG::WndFlag> flags/* = GG::Flags<GG::WndFlag>()*/) :
    GG::Wnd(GG::X0, GG::Y0, w, h, flags),
    m_fleet_id(INVALID_OBJECT_ID),
    m_order_issuing_enabled(order_issuing_enabled),
    m_ships_lb(0)
{
    SetName("FleetDetailPanel");
    SetChildClippingMode(ClipToClient);

    m_ships_lb = new ShipsListBox(INVALID_OBJECT_ID, order_issuing_enabled);
    AttachChild(m_ships_lb);
    m_ships_lb->SetHiliteColor(GG::CLR_ZERO);

    SetFleet(fleet_id);

    if (!m_order_issuing_enabled) {
        m_ships_lb->SetStyle(GG::LIST_NOSEL | GG::LIST_BROWSEUPDATES);
    } else {
        m_ships_lb->SetStyle(GG::LIST_QUICKSEL | GG::LIST_BROWSEUPDATES);
        m_ships_lb->AllowDropType(SHIP_DROP_TYPE_STRING);
    }

    GG::Connect(m_ships_lb->SelChangedSignal,               &FleetDetailPanel::ShipSelectionChanged,    this);
    GG::Connect(m_ships_lb->BrowsedSignal,                  &FleetDetailPanel::ShipBrowsed,             this);
    GG::Connect(m_ships_lb->RightClickedSignal,             &FleetDetailPanel::ShipRightClicked,        this);
    GG::Connect(GetUniverse().UniverseObjectDeleteSignal,   &FleetDetailPanel::UniverseObjectDeleted,   this);

    DoLayout();
}

int FleetDetailPanel::GetShipIDOfListRow(GG::ListBox::iterator it) const
{ return boost::polymorphic_downcast<ShipRow*>(*it)->ShipID(); }

void FleetDetailPanel::SetFleet(int fleet_id) {
    // save old fleet id and set to new id
    int old_fleet_id = m_fleet_id;
    m_fleet_id = fleet_id;

    // if set fleet changed, or if current fleet is no fleet, disconnect any 
    // signals related to that fleet
    if (m_fleet_id != old_fleet_id || m_fleet_id == INVALID_OBJECT_ID)
        m_fleet_connection.disconnect();

    // if set fleet unchanged, refresh ships list.  if set fleet changed, update ships list for new fleet
    if (m_fleet_id == old_fleet_id)
        m_ships_lb->Refresh();
    else
        m_ships_lb->SetFleet(m_fleet_id);

    // if set fleet changed, and new fleet exists, update state change signal connection
    if (m_fleet_id != old_fleet_id && m_fleet_id != INVALID_OBJECT_ID) {
        TemporaryPtr<const Fleet> fleet = GetFleet(m_fleet_id);
        if (fleet && !fleet->Empty()) {
            m_fleet_connection = GG::Connect(fleet->StateChangedSignal, &FleetDetailPanel::Refresh, this, boost::signals::at_front);
        } else {
            Logger().debugStream() << "FleetDetailPanel::SetFleet ignoring set to missing or empty fleet id (" << fleet_id << ")";
        }
    }
}

void FleetDetailPanel::SetSelectedShips(const std::set<int>& ship_ids) {
    const GG::ListBox::SelectionSet initial_selections = m_ships_lb->Selections();

    m_ships_lb->DeselectAll();

    // early exit if nothing to select, since everything has already been deselected above.
    if (ship_ids.empty()) {
        if (!initial_selections.empty())
            ShipSelectionChanged(m_ships_lb->Selections());
        return;
    }

    // loop through ships, selecting any indicated
    for (GG::ListBox::iterator it = m_ships_lb->begin(); it != m_ships_lb->end(); ++it) {
        ShipRow* row = dynamic_cast<ShipRow*>(*it);
        if (!row) {
            Logger().errorStream() << "FleetDetailPanel::SetSelectedShips couldn't cast a listbow row to ShipRow?";
            continue;
        }

        // if this row's ship should be selected, so so
        if (ship_ids.find(row->ShipID()) != ship_ids.end()) {
            m_ships_lb->SelectRow(it);
            m_ships_lb->BringRowIntoView(it);   // may cause earlier rows brought into view to be brought out of view... oh well
        }
    }

    ShipSelectionChanged(m_ships_lb->Selections());

    if (initial_selections != m_ships_lb->Selections())
        ShipSelectionChanged(m_ships_lb->Selections());
}

int FleetDetailPanel::FleetID() const
{ return m_fleet_id; }

std::set<int> FleetDetailPanel::SelectedShipIDs() const {
    //std::cout << "FleetDetailPanel::SelectedShipIDs()" << std::endl;
    std::set<int> retval;

    const GG::ListBox::SelectionSet& selections = m_ships_lb->Selections();
    //std::cout << " selections size: " << selections.size() << std::endl;
    for (GG::ListBox::SelectionSet::const_iterator sel_it = selections.begin();
         sel_it != selections.end(); ++sel_it)
    {
        std::list<GG::ListBox::Row*>::iterator starRow_it = *sel_it;
        bool hasRow = false;
        for (std::list<GG::ListBox::Row*>::iterator lb_it = m_ships_lb->begin(); lb_it != m_ships_lb->end(); lb_it++) {
            if (lb_it == starRow_it) {
                hasRow=true;
                break;
            }
        }
        if (!hasRow) {
            Logger().errorStream() << "FleetDetailPanel::SelectedShipIDs tried to get invalid ship row selection;";
            continue;
        }
        GG::ListBox::Row* row = **sel_it;
        ShipRow* ship_row = 0;
        try {   // casing rows here sometimes causes RTTI exceptions.  not sure why, but need to avoid crash.
            if (ship_row = dynamic_cast<ShipRow*>(row)) {
                if (ship_row->ShipID() != INVALID_OBJECT_ID)
                    retval.insert(ship_row->ShipID());
                //std::cout << " ship row for ship: " << ship_row->ShipID() << " is selected" << std::endl << std::flush;
            }
        } catch (...) {
            continue;
        }
    }
    return retval;
}

void FleetDetailPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    GG::Wnd::SizeMove(ul, lr);
    if (old_size != Size())
        DoLayout();
}

void FleetDetailPanel::EnableOrderIssuing(bool enabled/* = true*/) {
    m_order_issuing_enabled = enabled;
    m_ships_lb->EnableOrderIssuing(m_order_issuing_enabled);
}

void FleetDetailPanel::Refresh()
{ SetFleet(m_fleet_id); }

void FleetDetailPanel::DoLayout() {
    GG::X   LEFT = GG::X0;
    GG::X   RIGHT = ClientWidth();
    GG::Y   top = GG::Y0;
    GG::Y   bottom = ClientHeight();

    GG::Pt ul = GG::Pt(LEFT, top);
    GG::Pt lr = GG::Pt(RIGHT, top + LabelHeight());

    ul = GG::Pt(LEFT, top);
    lr = GG::Pt(RIGHT, bottom);
    m_ships_lb->SizeMove(ul, lr);
}

void FleetDetailPanel::UniverseObjectDeleted(TemporaryPtr<const UniverseObject> obj) {
    if (obj && obj->ID() == m_fleet_id)
        SetFleet(INVALID_OBJECT_ID);
}

void FleetDetailPanel::ShipSelectionChanged(const GG::ListBox::SelectionSet& rows) {
    for (GG::ListBox::iterator it = m_ships_lb->begin(); it != m_ships_lb->end(); ++it) {
        try {
            ShipDataPanel* ship_panel = boost::polymorphic_downcast<ShipDataPanel*>((**it)[0]);
            ship_panel->Select(rows.find(it) != rows.end());
        } catch (const std::exception& e) {
            Logger().errorStream() << "FleetDetailPanel::ShipSelectionChanged caught exception: " << e.what();
            continue;
        }
    }
    SelectedShipsChangedSignal(rows);
}

void FleetDetailPanel::ShipBrowsed(GG::ListBox::iterator it)
{}

void FleetDetailPanel::ShipRightClicked(GG::ListBox::iterator it, const GG::Pt& pt) {
    // get ship that was clicked, aborting if problems arise doing so
    ShipRow* ship_row = dynamic_cast<ShipRow*>(*it);
    if (!ship_row)
        return;

    TemporaryPtr<Ship> ship = GetShip(ship_row->ShipID());
    if (!ship)
        return;
    TemporaryPtr<Fleet> fleet = GetFleet(m_fleet_id);

    const MapWnd* map_wnd = ClientUI::GetClientUI()->GetMapWnd();
    if (ClientPlayerIsModerator() && map_wnd->GetModeratorActionSetting() != MAS_NoAction) {
        ShipRightClickedSignal(ship->ID());  // response handled in MapWnd
        return;
    }

    const ShipDesign* design = GetShipDesign(ship->DesignID()); // may be null
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();


    // create popup menu with a rename ship option in it.
    GG::MenuItem menu_contents;

    if (design) {
        std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % design->Name(true));
        menu_contents.next_level.push_back(GG::MenuItem(popup_label, 5, false, false));
    }

    if (ship->OwnedBy(client_empire_id)
        || ClientPlayerIsModerator())
    {
        menu_contents.next_level.push_back(GG::MenuItem(UserString("RENAME"), 1, false, false));
    }

    if (!ClientPlayerIsModerator()
        && !ship->OrderedScrapped()
        && ship->OwnedBy(client_empire_id))
    {
        // create popup menu with "Scrap" option
        menu_contents.next_level.push_back(GG::MenuItem(UserString("ORDER_SHIP_SCRAP"), 3, false, false));
    } else if (!ClientPlayerIsModerator()
               && ship->OwnedBy(client_empire_id))
    {
        // create popup menu with "Cancel Scrap" option
        menu_contents.next_level.push_back(GG::MenuItem(UserString("ORDER_CANCEL_SHIP_SCRAP"), 4, false, false));
    }

    if (ship->OwnedBy(client_empire_id)
        && !ClientPlayerIsModerator()
        && fleet)
    {
        if (design)
            menu_contents.next_level.push_back( GG::MenuItem(UserString("FW_SPLIT_SHIPS_THIS_DESIGN"), 7, false, false));
        menu_contents.next_level.push_back(     GG::MenuItem(UserString("FW_SPLIT_SHIPS_ALL_DESIGNS"), 8, false, false));
    }


    GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents, ClientUI::TextColor(),
                        ClientUI::WndOuterBorderColor(), ClientUI::WndColor());
    if (popup.Run()) {
        switch (popup.MenuID()) {
        case 1: { // rename ship
            std::string ship_name = ship->Name();
            CUIEditWnd edit_wnd(GG::X(350), UserString("ENTER_NEW_NAME"), ship_name);
            edit_wnd.Run();

            std::string new_name = edit_wnd.Result();

            if (!ClientPlayerIsModerator()) {
                if (new_name != "" && new_name != ship_name) {
                    HumanClientApp::GetApp()->Orders().IssueOrder(
                        OrderPtr(new RenameOrder(client_empire_id, ship->ID(), new_name)));
                }
            } else {
                // TODO: Moderator action for renaming ships
            }
            break;
        }

        case 3: { // scrap ship
            HumanClientApp::GetApp()->Orders().IssueOrder(
                OrderPtr(new ScrapOrder(client_empire_id, ship->ID())));
            break;
        }

        case 4: { // un-scrap ship
            // find order to scrap this ship, and recind it
            std::map<int, int> pending_scrap_orders = PendingScrapOrders();
            std::map<int, int>::const_iterator it = pending_scrap_orders.find(ship->ID());
            if (it != pending_scrap_orders.end())
                HumanClientApp::GetApp()->Orders().RecindOrder(it->second);
            break;
        }

        case 5: { // pedia lookup ship design
            if (design)
                ClientUI::GetClientUI()->ZoomToShipDesign(design->ID());
            break;
        }

        case 7: { // split ships with same design as clicked ship into separate fleet
            bool aggressive = false;
            const GG::Wnd* parent = this->Parent();
            if (!parent)
                return;
            const FleetWnd* parent_fleet_wnd = boost::dynamic_pointer_cast<const FleetWnd>(parent);
            if (!parent_fleet_wnd)
                return;
            CreateNewFleetFromShipsWithDesign(fleet->ShipIDs(), design->ID(),
                                              parent_fleet_wnd->NewFleetAggression());
            break;
        }

        case 8: { // split all ships into new fleets by ship design
            bool aggressive = false;
            const GG::Wnd* parent = this->Parent();
            if (!parent)
                return;
            const FleetWnd* parent_fleet_wnd = boost::dynamic_pointer_cast<const FleetWnd>(parent);
            if (!parent_fleet_wnd)
                return;
            CreateNewFleetsFromShipsForEachDesign(fleet->ShipIDs(),
                                                  parent_fleet_wnd->NewFleetAggression());
            break;
        }

        default:
            break;
        }
    }
}

int FleetDetailPanel::ShipInRow(GG::ListBox::iterator it) const {
    if (it == m_ships_lb->end())
        return INVALID_OBJECT_ID;

    if (ShipRow* ship_row = dynamic_cast<ShipRow*>(*it))
        return ship_row->ShipID();

    return INVALID_OBJECT_ID;
}

////////////////////////////////////////////////
// FleetWnd
////////////////////////////////////////////////
// static(s)
GG::Pt FleetWnd::s_last_position =  GG::Pt(GG::X0, GG::Y0);
GG::Pt FleetWnd::s_last_size =      GG::Pt(GG::X(360), GG::Y(400));

FleetWnd::FleetWnd(const std::vector<int>& fleet_ids, bool order_issuing_enabled,
         int selected_fleet_id/* = INVALID_OBJECT_ID*/,
         GG::Flags<GG::WndFlag> flags/* = INTERACTIVE | DRAGABLE | ONTOP | CLOSABLE*/) :
    MapWndPopup("", s_last_position.x, s_last_position.y, s_last_size.x, s_last_size.y, flags | GG::RESIZABLE),
    m_fleet_ids(),
    m_empire_id(ALL_EMPIRES),
    m_system_id(INVALID_OBJECT_ID),
    m_order_issuing_enabled(order_issuing_enabled),
    m_fleets_lb(0),
    m_new_fleet_drop_target(0),
    m_fleet_detail_panel(0),
    m_stat_icons()
{
    for (std::vector<int>::const_iterator it = fleet_ids.begin(); it != fleet_ids.end(); ++it)
        m_fleet_ids.insert(*it);
    Init(selected_fleet_id);
}

FleetWnd::FleetWnd(int system_id, int empire_id, bool order_issuing_enabled,
         int selected_fleet_id/* = INVALID_OBJECT_ID*/,
         GG::Flags<GG::WndFlag> flags/* = INTERACTIVE | DRAGABLE | ONTOP | CLOSABLE*/) :
    MapWndPopup("", s_last_position.x, s_last_position.y, s_last_size.x, s_last_size.y, flags | GG::RESIZABLE),
    m_fleet_ids(),
    m_empire_id(empire_id),
    m_system_id(system_id),
    m_order_issuing_enabled(order_issuing_enabled),
    m_fleets_lb(0),
    m_new_fleet_drop_target(0),
    m_fleet_detail_panel(0),
    m_stat_icons()
{ Init(selected_fleet_id); }

FleetWnd::~FleetWnd() {
    ClientUI::GetClientUI()->GetMapWnd()->ClearProjectedFleetMovementLines();
    ClosingSignal(this);
}

void FleetWnd::Init(int selected_fleet_id) {
    SetMinSize(GG::Pt(CUIWnd::MinimizedWidth(), BORDER_TOP + INNER_BORDER_ANGLE_OFFSET + BORDER_BOTTOM +
                                                ListRowHeight() + 2*GG::Y(PAD)));

    // ensure position is not off screen
    GG::Pt window_pos = UpperLeft();
    if (GG::GUI::GetGUI()->AppWidth() < LowerRight().x)
        window_pos.x = GG::GUI::GetGUI()->AppWidth() - Width();
    if (GG::GUI::GetGUI()->AppHeight() < LowerRight().y)
        window_pos.y = GG::GUI::GetGUI()->AppHeight() - Height();
    MoveTo(window_pos);


    Sound::TempUISoundDisabler sound_disabler;

    // add fleet aggregate stat icons
    int tooltip_delay = GetOptionsDB().Get<int>("UI.tooltip-delay");

    // stat icon for fleet count
    StatisticIcon* icon = new StatisticIcon(GG::X0, GG::Y0, StatIconSize().x, StatIconSize().y,
                                            FleetCountIcon(), 0, 0, false);
    m_stat_icons.push_back(std::make_pair(COUNT_STAT_STRING, icon));
    icon->SetBrowseModeTime(tooltip_delay);
    icon->SetBrowseText(StatTooltip(COUNT_STAT_STRING));
    AttachChild(icon);

    // stat icon for fleet damage
    icon = new StatisticIcon(GG::X0, GG::Y0, StatIconSize().x, StatIconSize().y,
                             DamageIcon(), 0, 0, false);
    m_stat_icons.push_back(std::make_pair(DAMAGE_STAT_STRING, icon));
    icon->SetBrowseModeTime(tooltip_delay);
    icon->SetBrowseText(StatTooltip(DAMAGE_STAT_STRING));
    AttachChild(icon);

    // stat icon for fleet structure
    icon = new StatisticIcon(GG::X0, GG::Y0, StatIconSize().x, StatIconSize().y,
                             ClientUI::MeterIcon(METER_STRUCTURE), 0, 0, false);
    m_stat_icons.push_back(std::make_pair(STRUCTURE_STAT_STRING, icon));
    icon->SetBrowseModeTime(tooltip_delay);
    icon->SetBrowseText(StatTooltip(STRUCTURE_STAT_STRING));
    AttachChild(icon);

    // stat icon for fleet shields
    icon = new StatisticIcon(GG::X0, GG::Y0, StatIconSize().x, StatIconSize().y,
                             ClientUI::MeterIcon(METER_SHIELD), 0, 0, false);
    m_stat_icons.push_back(std::make_pair(SHIELD_STAT_STRING, icon));
    icon->SetBrowseModeTime(tooltip_delay);
    icon->SetBrowseText(StatTooltip(SHIELD_STAT_STRING));
    AttachChild(icon);

    // create fleet list box
    m_fleets_lb = new FleetsListBox(m_order_issuing_enabled);
    m_fleets_lb->SetHiliteColor(GG::CLR_ZERO);
    GG::Connect(m_fleets_lb->SelChangedSignal,                      &FleetWnd::FleetSelectionChanged,   this);
    GG::Connect(m_fleets_lb->LeftClickedSignal,                     &FleetWnd::FleetLeftClicked,        this);
    GG::Connect(m_fleets_lb->RightClickedSignal,                    &FleetWnd::FleetRightClicked,       this);
    GG::Connect(m_fleets_lb->DoubleClickedSignal,                   &FleetWnd::FleetDoubleClicked,      this);
    AttachChild(m_fleets_lb);
    m_fleets_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_BROWSEUPDATES);
    m_fleets_lb->AllowDropType(SHIP_DROP_TYPE_STRING);
    m_fleets_lb->AllowDropType(FLEET_DROP_TYPE_STRING);

    // create fleet detail panel
    m_fleet_detail_panel = new FleetDetailPanel(GG::X1, GG::Y1, INVALID_OBJECT_ID, m_order_issuing_enabled);
    GG::Connect(m_fleet_detail_panel->SelectedShipsChangedSignal,   &FleetWnd::ShipSelectionChanged,    this);
    GG::Connect(m_fleet_detail_panel->ShipRightClickedSignal,       ShipRightClickedSignal);
    AttachChild(m_fleet_detail_panel);

    // determine fleets to show and populate list
    Refresh();

    // create drop target
    m_new_fleet_drop_target = new FleetDataPanel(GG::X1, ListRowHeight(), m_system_id, true);
    AttachChild(m_new_fleet_drop_target);
    GG::Connect(m_new_fleet_drop_target->NewFleetFromShipsSignal,   &FleetWnd::CreateNewFleetFromDrops, this);

    GG::Connect(GetUniverse().UniverseObjectDeleteSignal,           &FleetWnd::UniverseObjectDeleted,   this);

    RefreshStateChangedSignals();

    SetName(TitleText());

    // verify that the selected fleet id is valid.
    if (selected_fleet_id != INVALID_OBJECT_ID &&
        m_fleet_ids.find(selected_fleet_id) == m_fleet_ids.end())
    {
        Logger().errorStream() << "FleetWnd::Init couldn't find requested selected fleet with id " << selected_fleet_id;
        selected_fleet_id = INVALID_OBJECT_ID;
    }

    DoLayout();
}

void FleetWnd::SetStatIconValues() {
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    const std::set<int>& this_client_known_destroyed_objects = GetUniverse().EmpireKnownDestroyedObjectIDs(client_empire_id);
    const std::set<int>& this_client_stale_object_info = GetUniverse().EmpireStaleKnowledgeObjectIDs(client_empire_id);
    int ship_count =        0;
    float damage_tally =    0.0;
    float structure_tally = 0.0;
    float shield_tally =    0.0;

    std::vector<TemporaryPtr<const Fleet> > fleets = Objects().FindObjects<const Fleet>(m_fleet_ids);
    for (std::vector<TemporaryPtr<const Fleet> >::const_iterator fleet_it = fleets.begin();
         fleet_it != fleets.end(); ++fleet_it)
    {
        TemporaryPtr<const Fleet> fleet = *fleet_it;

        if ( !(m_empire_id == ALL_EMPIRES || fleet->OwnedBy(m_empire_id)) )
            continue;

        std::vector<TemporaryPtr<const Ship> > ships = Objects().FindObjects<const Ship>(fleet->ShipIDs());
        for (std::vector<TemporaryPtr<const Ship> >::const_iterator ship_it = ships.begin();
             ship_it != ships.end(); ++ship_it)
        {
            TemporaryPtr<const Ship> ship = *ship_it;
            int ship_id = ship->ID();

            // skip known destroyed and stale info objects
            if (this_client_known_destroyed_objects.find(ship_id) != this_client_known_destroyed_objects.end())
                continue;
            if (this_client_stale_object_info.find(ship_id) != this_client_stale_object_info.end())
                continue;

            if (ship->Design()) {
                ship_count++;
                damage_tally += ship->TotalWeaponsDamage();
                structure_tally += ship->CurrentMeterValue(METER_STRUCTURE);
                shield_tally += ship->CurrentMeterValue(METER_SHIELD);
            }
        }
    }
    
    for (std::vector<std::pair<std::string, StatisticIcon*> >::const_iterator it =
        m_stat_icons.begin(); it != m_stat_icons.end(); ++it) 
    {
        const std::string stat_name = it->first;
        if (stat_name == SHIELD_STAT_STRING)
            it->second->SetValue(shield_tally/ship_count);
        else if (stat_name == STRUCTURE_STAT_STRING)
            it->second->SetValue(structure_tally);
        else if (stat_name == DAMAGE_STAT_STRING)
            it->second->SetValue(damage_tally);
        else if (stat_name == COUNT_STAT_STRING)
            it->second->SetValue(ship_count);
    }
}

std::string FleetWnd::StatTooltip(const std::string& stat_name) const {
    if (stat_name == SPEED_STAT_STRING)
        return UserString("FW_FLEET_SPEED_SUMMARY");
    else if (stat_name ==  MeterStatString(METER_FUEL))
        return UserString("FW_FLEET_FUEL_SUMMARY");
    else if (stat_name == SHIELD_STAT_STRING)
        return UserString("FW_FLEET_SHIELD_SUMMARY");
    else if (stat_name == STRUCTURE_STAT_STRING)
        return UserString("FW_FLEET_STRUCTURE_SUMMARY");
    else if (stat_name == DAMAGE_STAT_STRING) 
        return UserString("FW_FLEET_DAMAGE_SUMMARY");
    else if (stat_name == COUNT_STAT_STRING) 
        return UserString("FW_FLEET_COUNT_SUMMARY");
    else
        return "";
}

void FleetWnd::RefreshStateChangedSignals() {
    m_system_connection.disconnect();
    if (TemporaryPtr<const System> system = GetSystem(m_system_id))
        m_system_connection = GG::Connect(system->StateChangedSignal, &FleetWnd::SystemChangedSlot, this, boost::signals::at_front);
}

void FleetWnd::Refresh() {
    int this_client_empire_id = HumanClientApp::GetApp()->EmpireID();
    const std::set<int>& this_client_known_destroyed_objects = GetUniverse().EmpireKnownDestroyedObjectIDs(this_client_empire_id);
    const std::set<int>& this_client_stale_object_info = GetUniverse().EmpireStaleKnowledgeObjectIDs(this_client_empire_id);

    // save selected fleet(s) and ships(s)
    std::set<int> initially_selected_fleets = this->SelectedFleetIDs();
    std::set<int> initially_selected_ships = this->SelectedShipIDs();

    // remove existing fleet rows
    m_fleets_lb->Clear();   // deletes rows when removing; they don't need to be manually deleted
    std::set<int> initial_fleet_ids = m_fleet_ids;
    m_fleet_ids.clear();

    // skip nonexistant systems
    if (m_system_id != INVALID_OBJECT_ID && (
        this_client_known_destroyed_objects.find(m_system_id) != this_client_known_destroyed_objects.end() ||
        this_client_stale_object_info.find(m_system_id) != this_client_stale_object_info.end()))
    {
        m_system_connection.disconnect();
        m_fleet_detail_panel->SetFleet(INVALID_OBJECT_ID);
        return;
    }

    // repopulate m_fleet_ids according to FleetWnd settings
    if (GetSystem(m_system_id)) {
        // get fleets to show from system, based on required ownership
        const ObjectMap& objects = Objects();
        std::vector<TemporaryPtr<const Fleet> > all_fleets = objects.FindObjects<Fleet>();
        for (std::vector<TemporaryPtr<const Fleet> >::const_iterator it = all_fleets.begin(); it != all_fleets.end(); ++it) {
            TemporaryPtr<const Fleet> fleet = *it;
            int fleet_id = fleet->ID();

            // skip fleets in wrong system
            if (fleet->SystemID() != m_system_id)
                continue;
            // skip empty fleets
            //if (fleet->Empty())
            //    continue;

            // skip known destroyed and stale info objects
            if (this_client_known_destroyed_objects.find(fleet_id) != this_client_known_destroyed_objects.end() ||
                this_client_stale_object_info.find(fleet_id) != this_client_stale_object_info.end())
            { continue; }

            // skip fleets outside systems
            if (fleet->SystemID() == INVALID_OBJECT_ID)
                continue;

            if (m_empire_id == ALL_EMPIRES || fleet->OwnedBy(m_empire_id)) {
                m_fleet_ids.insert(fleet_id);
                AddFleet(fleet_id);
            }
        }
    } else {
        // check all fleets whose IDs are in initial_fleet_ids, adding back those that still exist
        for (std::set<int>::const_iterator it = initial_fleet_ids.begin(); it != initial_fleet_ids.end(); ++it) {
            int fleet_id = *it;

            // skip known destroyed and stale info objects
            if (this_client_known_destroyed_objects.find(fleet_id) != this_client_known_destroyed_objects.end())
                continue;
            if (this_client_stale_object_info.find(fleet_id) != this_client_stale_object_info.end())
                continue;

            if (GetFleet(fleet_id)) {
                m_fleet_ids.insert(fleet_id);
                AddFleet(fleet_id);
            }
        }
    }

    // filter initially selected fleets according to present fleets
    std::set<int> still_present_initially_selected_fleets;
    for (std::set<int>::const_iterator it = initially_selected_fleets.begin();
         it != initially_selected_fleets.end(); ++it)
    {
        int fleet_id =  *it;
        if (m_fleet_ids.find(fleet_id) != m_fleet_ids.end())
            still_present_initially_selected_fleets.insert(fleet_id);
    }

    if (!still_present_initially_selected_fleets.empty()) {
        // reselect any previously-selected fleets
        this->SetSelectedFleets(still_present_initially_selected_fleets);
        // reselect any previously-selected ships
        this->SetSelectedShips(initially_selected_ships);
    } else if (!m_fleets_lb->Empty()) {
        // default select first fleet
        int first_fleet_id = FleetInRow(m_fleets_lb->begin());
        if (first_fleet_id != INVALID_OBJECT_ID) {
            std::set<int> fleet_id_set;
            fleet_id_set.insert(first_fleet_id);
            this->SetSelectedFleets(fleet_id_set);
        }
    }

    SetStatIconValues();

    RefreshStateChangedSignals();
}

void FleetWnd::CloseClicked() {
    s_last_position = UpperLeft();
    CUIWnd::CloseClicked();
    delete this;
}

void FleetWnd::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    MapWndPopup::LClick(pt, mod_keys);
    ClickedSignal(this);
}

void FleetWnd::DoLayout() {
    const GG::X TOTAL_WIDTH(ClientWidth());
    const GG::X LEFT(GG::X0);
    const GG::X RIGHT(TOTAL_WIDTH);

    const GG::Y TOTAL_HEIGHT(ClientHeight());
    const GG::Y FLEET_STAT_HEIGHT(StatIconSize().y + PAD);
    const GG::Y AVAILABLE_HEIGHT(TOTAL_HEIGHT - GG::Y(INNER_BORDER_ANGLE_OFFSET+PAD) - FLEET_STAT_HEIGHT );
    GG::Y top( GG::Y0 + GG::Y(PAD) );
    const GG::Y BOTTOM(TOTAL_HEIGHT - GG::Y(INNER_BORDER_ANGLE_OFFSET));
    const GG::Y ROW_HEIGHT(m_fleets_lb->ListRowSize().y);
    
    // position fleet aggregate stat icons
    GG::Pt icon_ul = GG::Pt(GG::X0 + DATA_PANEL_TEXT_PAD, top);
    for (std::vector<std::pair<std::string, StatisticIcon*> >::const_iterator it = m_stat_icons.begin(); it != m_stat_icons.end(); ++it) {
        it->second->SizeMove(icon_ul, icon_ul + StatIconSize());
        icon_ul.x += StatIconSize().x;
    }
    top += FLEET_STAT_HEIGHT;
    
    // are there any fleets owned by this client's empire int his FleetWnd?
    bool this_client_owns_fleets_in_this_wnd(false);
    int this_client_empire_id = HumanClientApp::GetApp()->EmpireID();
    for (std::set<int>::const_iterator it = m_fleet_ids.begin(); it != m_fleet_ids.end(); ++it) {
        TemporaryPtr<const Fleet> fleet = GetFleet(*it);
        if (!fleet)
            continue;
        if (fleet->OwnedBy(this_client_empire_id)) {
            this_client_owns_fleets_in_this_wnd = true;
            break;
        }
    }

    // what parts of FleetWnd to show?
    bool show_new_fleet_drop_target(true);
    if (!m_new_fleet_drop_target || AVAILABLE_HEIGHT < 5*ROW_HEIGHT || !this_client_owns_fleets_in_this_wnd)
        show_new_fleet_drop_target = false;

    bool show_fleet_detail_panel(true);
    if (AVAILABLE_HEIGHT < 3*ROW_HEIGHT)
        show_fleet_detail_panel = false;


    // how tall to make fleets list?  subtract height for other panels from available height.
    GG::Y fleets_list_height(AVAILABLE_HEIGHT);
    if (show_fleet_detail_panel)
        fleets_list_height *= 0.5;
    if (show_new_fleet_drop_target)
        fleets_list_height -= (ROW_HEIGHT + GG::Y(PAD));

    // how tall to make ships list, if present?
    GG::Y ship_list_height(AVAILABLE_HEIGHT - fleets_list_height - GG::Y(PAD));
    if (show_new_fleet_drop_target)
        ship_list_height -= (ROW_HEIGHT + GG::Y(PAD));


    // position controls
    m_fleets_lb->SizeMove(                  GG::Pt(LEFT, top),              GG::Pt(RIGHT, top + fleets_list_height));
    top += fleets_list_height + GG::Y(PAD);

    if (show_new_fleet_drop_target) {
        AttachChild(m_new_fleet_drop_target);
        m_new_fleet_drop_target->SizeMove(  GG::Pt(LEFT + GG::X(PAD), top), GG::Pt(RIGHT - ClientUI::ScrollWidth() - GG::X(PAD), top + ROW_HEIGHT));
        top += ROW_HEIGHT + GG::Y(PAD);
    } else {
        if (m_new_fleet_drop_target)
            DetachChild(m_new_fleet_drop_target);
    }

    if (show_fleet_detail_panel) {
        AttachChild(m_fleet_detail_panel);
        m_fleet_detail_panel->SizeMove(     GG::Pt(LEFT, top),              GG::Pt(RIGHT, top + ship_list_height));
        top += ship_list_height + GG::Y(PAD);
    } else {
        if (m_fleet_detail_panel)
            DetachChild(m_fleet_detail_panel);
    }
}

void FleetWnd::AddFleet(int fleet_id) {
    TemporaryPtr<const Fleet> fleet = GetFleet(fleet_id);
    if (!fleet /*|| fleet->Empty()*/)
        return;

    // verify that fleet is consistent
    const GG::Pt row_size = m_fleets_lb->ListRowSize();
    FleetRow* row = new FleetRow(fleet_id, GG::X1, row_size.y);
    m_fleets_lb->Insert(row);
    row->Resize(row_size);
}

void FleetWnd::RemoveFleet(int fleet_id)
{}

void FleetWnd::SelectFleet(int fleet_id) {
    if (fleet_id == INVALID_OBJECT_ID || !(GetFleet(fleet_id))) {
        m_fleets_lb->DeselectAll();
        FleetSelectionChanged(m_fleets_lb->Selections());
        return;
    }

    //std::cout << "FleetWnd::SelectFleet " << fleet->Name() << " (" << fleet->ID() << ")" << std::endl;

    for (GG::ListBox::iterator it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        FleetRow* row = dynamic_cast<FleetRow*>(*it);
        if (row && row->FleetID() == fleet_id) {
            m_fleets_lb->DeselectAll();
            m_fleets_lb->SelectRow(it);

            FleetSelectionChanged(m_fleets_lb->Selections());

            m_fleets_lb->BringRowIntoView(it);
            break;
        }
    }
}

void FleetWnd::SetSelectedFleets(const std::set<int>& fleet_ids) {
    const GG::ListBox::SelectionSet initial_selections = m_fleets_lb->Selections();
    m_fleets_lb->DeselectAll();

    // early exit if nothing to select
    if (fleet_ids.empty()) {
        const GG::ListBox::SelectionSet sels = m_fleets_lb->Selections();
        FleetSelectionChanged(sels);
        return;
    }

    // loop through fleets, selecting any indicated
    for (GG::ListBox::iterator it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        FleetRow* row = dynamic_cast<FleetRow*>(*it);
        if (!row) {
            Logger().errorStream() << "FleetWnd::SetSelectedFleets couldn't cast a listbow row to FleetRow?";
            continue;
        }

        // if this row's fleet should be selected, so so
        if (fleet_ids.find(row->FleetID()) != fleet_ids.end()) {
            m_fleets_lb->SelectRow(it);
            m_fleets_lb->BringRowIntoView(it);  // may cause earlier rows brought into view to be brought out of view... oh well
        }
    }

    const GG::ListBox::SelectionSet sels = m_fleets_lb->Selections();

    if (sels.empty() || initial_selections != sels)
        FleetSelectionChanged(sels);
}

void FleetWnd::SetSelectedShips(const std::set<int>& ship_ids)
{ m_fleet_detail_panel->SetSelectedShips(ship_ids); }

void FleetWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = Size();
    MapWndPopup::SizeMove(ul, lr);
    s_last_position = ul;
    s_last_size = Size();
    if (s_last_size != old_size)
        DoLayout();
}

int FleetWnd::SystemID() const
{ return m_system_id; }

int FleetWnd::EmpireID() const
{ return m_empire_id; }

bool FleetWnd::ContainsFleet(int fleet_id) const {
    for (GG::ListBox::iterator it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        TemporaryPtr<Fleet> fleet = GetFleet(FleetInRow(it));
        if (fleet && fleet->ID() == fleet_id)
            return true;
    }
    return false;
}

const std::set<int>& FleetWnd::FleetIDs() const
{ return m_fleet_ids; }

std::set<int> FleetWnd::SelectedFleetIDs() const {
    std::set<int> retval;
    for (GG::ListBox::iterator it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        if (!m_fleets_lb->Selected(it))
            continue;

        int selected_fleet_id = FleetInRow(it);
        if (selected_fleet_id != INVALID_OBJECT_ID)
            retval.insert(selected_fleet_id);
    }
    return retval;
}

std::set<int> FleetWnd::SelectedShipIDs() const
{ return m_fleet_detail_panel->SelectedShipIDs(); }

bool FleetWnd::NewFleetAggression() const {
    if (m_new_fleet_drop_target)
        return m_new_fleet_drop_target->NewFleetAggression();
    return false;
}

void FleetWnd::FleetSelectionChanged(const GG::ListBox::SelectionSet& rows) {
    // show appropriate fleet in detail panel.  if one fleet is selected, show
    // its ships.  if more than one fleet is selected or no fleets are selected
    // then show no ships.
    if (rows.size() == 1) {
        // find selected row and fleet
        bool found_row = false;
        for (GG::ListBox::iterator it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
            if (rows.find(it) != rows.end()) {
                m_fleet_detail_panel->SetFleet(FleetInRow(it));
                found_row = true;
                break;
            }
        }
        if (!found_row)
            m_fleet_detail_panel->SetFleet(INVALID_OBJECT_ID);
    } else {
        m_fleet_detail_panel->SetFleet(INVALID_OBJECT_ID);
    }

    if (!rows.empty()) {
        // mark as selected all FleetDataPanel that are in \a rows and mark as not
        // selected all FleetDataPanel that aren't in \a rows
        for (GG::ListBox::iterator it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
            bool select_this_row = (rows.find(it) != rows.end());

            GG::ListBox::Row* row = *it;
            if (!row) {
                Logger().errorStream() << "FleetWnd::FleetSelectionChanged couldn't get row";
                continue;
            }
            GG::Control* control = (*row)[0];
            if (!control) {
                Logger().errorStream() << "FleetWnd::FleetSelectionChanged couldn't get control from row";
                continue;
            }
            FleetDataPanel* data_panel = dynamic_cast<FleetDataPanel*>(control);
            if (!data_panel) {
                Logger().errorStream() << "FleetWnd::FleetSelectionChanged couldn't get FleetDataPanel from control";
                continue;
            }
            data_panel->Select(select_this_row);
        }
    }

    //ClickedSignal(this);
    SelectedFleetsChangedSignal();
}

void FleetWnd::FleetRightClicked(GG::ListBox::iterator it, const GG::Pt& pt) {
    int empire_id = HumanClientApp::GetApp()->EmpireID();

    TemporaryPtr<Fleet> fleet = GetFleet(FleetInRow(it));
    if (!fleet)
        return;

    TemporaryPtr<const System> system = GetSystem(fleet->SystemID());    // may be null
    std::set<int> ship_ids_set = fleet->ShipIDs();

    // find damaged ships
    std::vector<int> damaged_ship_ids;
    damaged_ship_ids.reserve(ship_ids_set.size());
    for (std::set<int>::const_iterator it = ship_ids_set.begin(); it != ship_ids_set.end(); ++it) {
        TemporaryPtr<const Ship> ship = GetShip(*it);
        if (ship->CurrentMeterValue(METER_STRUCTURE) < ship->CurrentMeterValue(METER_MAX_STRUCTURE))
            damaged_ship_ids.push_back(*it);
    }

    // find ships with no remaining fuel
    std::vector<int> unfueled_ship_ids;
    unfueled_ship_ids.reserve(ship_ids_set.size());
    for (std::set<int>::const_iterator it = ship_ids_set.begin(); it != ship_ids_set.end(); ++it) {
        TemporaryPtr<const Ship> ship = GetShip(*it);
        if (!ship)
            continue;
        if (ship->CurrentMeterValue(METER_FUEL) < 1)
            unfueled_ship_ids.push_back(*it);
    }

    GG::MenuItem menu_contents;

    // add a fleet popup command to send the fleet exploring, and stop it from exploring
    if (system
        && !ClientUI::GetClientUI()->GetMapWnd()->IsFleetExploring(fleet->ID())
        && !ClientPlayerIsModerator()
        && fleet->OwnedBy(empire_id))
    {
        menu_contents.next_level.push_back(GG::MenuItem(UserString("ORDER_FLEET_EXPLORE"),        7, false, false));
    }
    else if (system
             && !ClientPlayerIsModerator()
             && fleet->OwnedBy(empire_id))
    {
        menu_contents.next_level.push_back(GG::MenuItem(UserString("ORDER_CANCEL_FLEET_EXPLORE"), 8, false, false));
    }

    // Merge fleets
    if (system
        && fleet->OwnedBy(empire_id)
        && !ClientPlayerIsModerator()
       )
    {
        menu_contents.next_level.push_back(GG::MenuItem(UserString("FW_MERGE_SYSTEM_FLEETS"),   10, false, false));
    }

    // Split damaged ships - need some, but not all, ships damaged, and need to be in a system
    if (system
        && ship_ids_set.size() > 1
        && !damaged_ship_ids.empty()
        && damaged_ship_ids.size() != ship_ids_set.size()
        && fleet->OwnedBy(empire_id)
        && !ClientPlayerIsModerator()
       )
    {
        menu_contents.next_level.push_back(GG::MenuItem(UserString("FW_SPLIT_DAMAGED_FLEET"),     2, false, false));
    }

    // Split unfueled ships - need some, but not all, ships unfueled, and need to be in a system
    if (system
        && ship_ids_set.size() > 1
        && unfueled_ship_ids.size() > 0
        && unfueled_ship_ids.size() < ship_ids_set.size()
        && fleet->OwnedBy(empire_id)
        && !ClientPlayerIsModerator()

       )
    {
        menu_contents.next_level.push_back(GG::MenuItem(UserString("FW_SPLIT_UNFUELED_FLEET"),    3, false, false));
    }

    // Split fleet - can't split fleets without more than one ship, or which are not in a system
    if (system
        && ship_ids_set.size() > 1
        && (fleet->OwnedBy(empire_id))
        && !ClientPlayerIsModerator()
       )
    {
        menu_contents.next_level.push_back(GG::MenuItem(UserString("FW_SPLIT_FLEET"),             4, false, false));
        menu_contents.next_level.push_back(GG::MenuItem(UserString("FW_SPLIT_SHIPS_ALL_DESIGNS"), 9, false, false));
    }

    // Rename fleet
    menu_contents.next_level.push_back(GG::MenuItem(UserString("RENAME"),                         1, false, false));

    // add a fleet popup command to order all ships in the fleet scrapped
    if (system
        && fleet->HasShipsWithoutScrapOrders()
        && !ClientPlayerIsModerator()
        && fleet->OwnedBy(empire_id))
    {
        menu_contents.next_level.push_back(GG::MenuItem(UserString("ORDER_FLEET_SCRAP"),          5, false, false));
    }

    // add a fleet popup command to cancel all scrap orders on ships in this fleet
    if (system
        && fleet->HasShipsOrderedScrapped()
        && !ClientPlayerIsModerator())
    {
        menu_contents.next_level.push_back(GG::MenuItem(UserString("ORDER_CANCEL_FLEET_SCRAP"),   6, false, false));
    }

    GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents, ClientUI::TextColor(),
                        ClientUI::WndOuterBorderColor(), ClientUI::WndColor());

    if (popup.Run()) {
        switch (popup.MenuID()) {
        case 1: {   // rename fleet
            std::string fleet_name = fleet->Name();
            CUIEditWnd edit_wnd(GG::X(350), UserString("ENTER_NEW_NAME"), fleet_name);
            edit_wnd.Run();

            std::string new_name = edit_wnd.Result();

            if (!ClientPlayerIsModerator()) {
                if (!new_name.empty() && new_name != fleet_name) {
                    HumanClientApp::GetApp()->Orders().IssueOrder(
                        OrderPtr(new RenameOrder(empire_id, fleet->ID(), new_name)));
                }
            } else {
                // TODO: Moderator action for renaming fleet
            }
            break;
        }

        case 10:{ // merge fleets
            MergeFleetsIntoFleet(fleet->ID());
            break;
        }

        case 2: { // split damaged
            bool aggressive = false;    // damaged ships want to avoid fighting more
            CreateNewFleetFromShips(damaged_ship_ids, aggressive);
            break;
        }

        case 3: { // split unfueled
            CreateNewFleetFromShips(unfueled_ship_ids, fleet->Aggressive());
            break;
        }

        case 4: {   // split
            // remove first ship from set
            std::set<int>::iterator it = ship_ids_set.begin();
            ship_ids_set.erase(it);

            // put remaining ships into their own fleets
            for (it = ship_ids_set.begin(); it != ship_ids_set.end(); ++it) {
                std::vector<int> ship_ids_vec;
                ship_ids_vec.push_back(*it);
                bool aggressive = false;
                if (m_new_fleet_drop_target)
                    aggressive = m_new_fleet_drop_target->NewFleetAggression();
                CreateNewFleetFromShips(ship_ids_vec, aggressive);
            }
            break;
        }

        case 9: { // split fleet into separate fleets for each design
            bool aggressive = false;
            if (m_new_fleet_drop_target)
                aggressive = m_new_fleet_drop_target->NewFleetAggression();
            CreateNewFleetsFromShipsForEachDesign(fleet->ShipIDs(), aggressive);
        }

        case 5: { // issue scrap orders to all ships in this fleet
            std::set<int> ship_ids_set = fleet->ShipIDs();
            for (std::set<int>::iterator it = ship_ids_set.begin(); it != ship_ids_set.end(); ++it) {
                    int ship_id = *it;
                    HumanClientApp::GetApp()->Orders().IssueOrder(
                        OrderPtr(new ScrapOrder(empire_id, ship_id))
                    );
                }
            break;
        }

        case 6: { // cancel scrap orders for all ships in this fleet
            const OrderSet orders = HumanClientApp::GetApp()->Orders();
            std::set<int> ship_ids_set = fleet->ShipIDs();
            for (std::set<int>::iterator it = ship_ids_set.begin(); it != ship_ids_set.end(); ++it) {
                int ship_id = *it;
                for (OrderSet::const_iterator it = orders.begin(); it != orders.end(); ++it) {
                    if (boost::shared_ptr<ScrapOrder> order = boost::dynamic_pointer_cast<ScrapOrder>(it->second)) {
                        if (order->ObjectID() == ship_id) {
                            HumanClientApp::GetApp()->Orders().RecindOrder(it->first);
                            // could break here, but won't to ensure there are no problems with doubled orders
                        }
                    }
                }
            }

            break;
        }

        case 7: { // send order to explore to the fleet
            if (!ClientPlayerIsModerator())
                ClientUI::GetClientUI()->GetMapWnd()->SetFleetExploring(fleet->ID());
            break;
        }

        case 8: { // send order to stop exploring to the fleet
            if (!ClientPlayerIsModerator())
                ClientUI::GetClientUI()->GetMapWnd()->StopFleetExploring(fleet->ID());
            break;
        }

        default:
            break;
        }
    }
}

void FleetWnd::FleetLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt)
{ ClickedSignal(this); }

void FleetWnd::FleetDoubleClicked(GG::ListBox::iterator it)
{ ClickedSignal(this); }

int FleetWnd::FleetInRow(GG::ListBox::iterator it) const {
    if (it == m_fleets_lb->end())
        return INVALID_OBJECT_ID;

    try {
        //Logger().debugStream() << "FleetWnd::FleetInRow casting iterator to fleet row";
        if (FleetRow* fleet_row = dynamic_cast<FleetRow*>(*it)) {
            return fleet_row->FleetID();
        }
    } catch (const std::exception& e) {
        Logger().errorStream() << "FleetInRow caught exception: " << e.what();
    }

    return INVALID_OBJECT_ID;
}

std::string FleetWnd::TitleText() const {
    // if no fleets available, default to indicating no fleets
    if (m_fleet_ids.empty())
        return UserString("FW_NO_FLEET");

    int client_empire_id = HumanClientApp::GetApp()->EmpireID();

    // at least one fleet is available, so show appropriate title this
    // FleetWnd's empire and system
    if (const Empire* empire = Empires().Lookup(m_empire_id)) {
        if (TemporaryPtr<const System> system = GetSystem(m_system_id)) {
            const std::string& sys_name = system->ApparentName(client_empire_id);
            return boost::io::str(FlexibleFormat(UserString("FW_EMPIRE_FLEETS_AT_SYSTEM")) %
                                  empire->Name() % sys_name);
        } else {
            return boost::io::str(FlexibleFormat(UserString("FW_EMPIRE_FLEETS")) %
                                  empire->Name());
        }
    } else {
        if (TemporaryPtr<const System> system = GetSystem(m_system_id)) {
            const std::string& sys_name = system->ApparentName(client_empire_id);
            return boost::io::str(FlexibleFormat(UserString("FW_GENERIC_FLEETS_AT_SYSTEM")) %
                                  sys_name);
        } else {
            return boost::io::str(FlexibleFormat(UserString("FW_GENERIC_FLEETS")));
        }
    }
}

void FleetWnd::CreateNewFleetFromDrops(const std::vector<int>& ship_ids) {
    Logger().debugStream() << "FleetWnd::CreateNewFleetFromDrops with " << ship_ids.size() << " ship ids";

    if (ship_ids.empty())
        return;

    bool aggressive = false;
    if (m_new_fleet_drop_target)
        aggressive = m_new_fleet_drop_target->NewFleetAggression();

    // deselect all ships so that response to fleet rearrangement doesn't attempt
    // to get the selected ships that are no longer in their old fleet.
    m_fleet_detail_panel->SetSelectedShips(std::set<int>());

    CreateNewFleetFromShips(ship_ids, aggressive);
}

void FleetWnd::ShipSelectionChanged(const GG::ListBox::SelectionSet& rows)
{ SelectedShipsChangedSignal(); }

void FleetWnd::UniverseObjectDeleted(TemporaryPtr<const UniverseObject> obj) {
    Logger().debugStream() << "FleetWnd::UniverseObjectDeleted";
    Logger().debugStream().flush();

    // check if deleted object was a fleet.  if not, abort.
    TemporaryPtr<const Fleet> deleted_fleet = universe_object_ptr_cast<const Fleet>(obj);
    if (!deleted_fleet)
        return; // TODO: Why exactly does this function not just take a Fleet instead of a UniverseObject?  Maybe rename to FleetDeleted?

    // if detail panel is showing the deleted fleet, reset to show nothing
    if (GetFleet(m_fleet_detail_panel->FleetID()) == deleted_fleet)
        m_fleet_detail_panel->SetFleet(INVALID_OBJECT_ID);

    const ObjectMap& objects = GetUniverse().Objects();

    // remove deleted fleet's row
    for (GG::ListBox::iterator it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        int row_fleet_id = FleetInRow(it);
        if (objects.Object<Fleet>(row_fleet_id) == deleted_fleet) {
            delete m_fleets_lb->Erase(it);
            break;
        }
    }
    Logger().debugStream() << "FleetWnd::UniverseObjectDeleted done";
    Logger().debugStream().flush();
}

void FleetWnd::SystemChangedSlot() {
    TemporaryPtr<const System> system = GetSystem(m_system_id);
    if (!system) {
        Logger().errorStream() << "FleetWnd::SystemChangedSlot called but couldn't get System with id " << m_system_id;
        return;
    }

    Refresh();
}

const GG::Pt& FleetWnd::LastPosition()
{ return s_last_position; }

const GG::Pt& FleetWnd::LastSize()
{ return s_last_size; }

void FleetWnd::EnableOrderIssuing(bool enable/* = true*/) {
    m_order_issuing_enabled = enable;
    if (m_new_fleet_drop_target)
        m_new_fleet_drop_target->Disable(!m_order_issuing_enabled);
    if (m_fleets_lb)
        m_fleets_lb->EnableOrderIssuing(m_order_issuing_enabled);
    if (m_fleet_detail_panel)
        m_fleet_detail_panel->EnableOrderIssuing(m_order_issuing_enabled);
}
