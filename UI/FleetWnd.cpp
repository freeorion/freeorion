#include "FleetWnd.h"

#include "CUIControls.h"
#include "SidePanel.h"
#include "IconTextBrowseWnd.h"
#include "MeterBrowseWnd.h"
#include "ClientUI.h"
#include "Sound.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/Order.h"
#include "../util/OptionsDB.h"
#include "../util/ScopedTimer.h"
#include "../client/human/HumanClientApp.h"
#include "../universe/Fleet.h"
#include "../universe/Planet.h"
#include "../universe/Predicates.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../universe/Enums.h"
#include "../universe/Pathfinder.h"
#include "../network/Message.h"
#include "../Empire/Empire.h"

#include <GG/DrawUtil.h>
#include <GG/Enum.h>
#include <GG/GUI.h>
#include <GG/Layout.h>
#include <GG/StaticGraphic.h>

#include <boost/cast.hpp>
#include <boost/unordered_map.hpp>
#include <tuple>

#include <unordered_set>


FO_COMMON_API extern const int INVALID_DESIGN_ID;

namespace {
    const GG::Pt        DataPanelIconSpace()
    { return GG::Pt(GG::X(ClientUI::Pts()*3), GG::Y(ClientUI::Pts()*2.5)); }
    GG::X               FLEET_WND_WIDTH = GG::X(360);
    GG::Y               FLEET_WND_HEIGHT = GG::Y(400);

    // how should ship and fleet icons be scaled and/or positioned in the reserved space
    const GG::Flags<GG::GraphicStyle>   DataPanelIconStyle()
    { return GG::GRAPHIC_CENTER | GG::GRAPHIC_VCENTER | GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE; }

    const GG::X         DATA_PANEL_TEXT_PAD = GG::X(4); // padding on the left and right of fleet/ship description
    const int           DATA_PANEL_BORDER = 1;          // how thick should the border around ship or fleet panel be
    const int           PAD = 4;
    const std::string   SHIP_DROP_TYPE_STRING = "FleetWnd ShipRow";
    const std::string   FLEET_DROP_TYPE_STRING = "FleetWnd FleetRow";
    const std::string   FLEET_WND_NAME = "map.fleet";

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
    { return std::max(DataPanelIconSpace().y, LabelHeight() + StatIconSize().y) + 2*DATA_PANEL_BORDER + PAD; }

    std::shared_ptr<GG::Texture> DamageIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "meter" / "damage.png", true); }

    std::shared_ptr<GG::Texture> TroopIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "meter" / "troops.png", true); }

    std::shared_ptr<GG::Texture> ColonyIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "meter" / "colony.png", true); }

    std::shared_ptr<GG::Texture> FightersIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "meter" / "fighters.png", true); }

    std::shared_ptr<GG::Texture> FleetCountIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "sitrep" / "fleet_arrived.png"); }

    std::shared_ptr<GG::Texture> IndustryIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "meter" / "industry.png"); }

    std::shared_ptr<GG::Texture> ResearchIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "meter" / "research.png"); }

    std::shared_ptr<GG::Texture> TradeIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "meter" / "trade.png"); }

    std::string FleetDestinationText(int fleet_id) {
        std::string retval = "";
        auto fleet = GetFleet(fleet_id);
        if (!fleet)
            return retval;

        int client_empire_id = HumanClientApp::GetApp()->EmpireID();

        std::shared_ptr<const System> dest_sys = GetSystem(fleet->FinalDestinationID());
        std::shared_ptr<const System> cur_sys = GetSystem(fleet->SystemID());
        bool returning_to_current_system = (dest_sys == cur_sys) && !fleet->TravelRoute().empty();
        if (dest_sys && (dest_sys != cur_sys || returning_to_current_system)) {
            std::pair<int, int> eta = fleet->ETA();       // .first is turns to final destination.  .second is turns to next system on route

            // name of final destination
            std::string dest_name = dest_sys->ApparentName(client_empire_id);
            if (GetOptionsDB().Get<bool>("ui.name.id.shown")) {
                dest_name = dest_name + " (" + std::to_string(dest_sys->ID()) + ")";
            }

            // next system on path
            std::string next_eta_text;
            if (eta.second == Fleet::ETA_UNKNOWN)
                next_eta_text = UserString("FW_FLEET_ETA_UNKNOWN");
            else if (eta.second == Fleet::ETA_NEVER)
                next_eta_text = UserString("FW_FLEET_ETA_NEVER");
            else if (eta.second == Fleet::ETA_OUT_OF_RANGE)
                next_eta_text = UserString("FW_FLEET_ETA_OUT_OF_RANGE");
            else
                next_eta_text = std::to_string(eta.second);

            // final destination
            std::string final_eta_text;
            if (eta.first == Fleet::ETA_UNKNOWN)
                final_eta_text = UserString("FW_FLEET_ETA_UNKNOWN");
            else if (eta.first == Fleet::ETA_NEVER)
                final_eta_text = UserString("FW_FLEET_ETA_NEVER");
            else if (eta.first == Fleet::ETA_OUT_OF_RANGE)
                final_eta_text = UserString("FW_FLEET_ETA_OUT_OF_RANGE");
            else
                final_eta_text = std::to_string(eta.first);

            if (ClientUI::GetClientUI()->GetMapWnd()->IsFleetExploring(fleet->ID()))
                retval = boost::io::str(FlexibleFormat(UserString("FW_FLEET_EXPLORING_TO")) %
                                        dest_name % final_eta_text % next_eta_text);
            else {
                // "FW_FLEET_MOVING_TO" userstring is currently truncated to drop ETA info
                // so as to fit better in a small fleet window
                std::string moving_key = GetOptionsDB().Get<bool>("ui.map.fleet.eta.shown")
                    ? UserString("FW_FLEET_MOVING_TO_ETA")
                    : UserString("FW_FLEET_MOVING_TO");
                retval = boost::io::str(FlexibleFormat(moving_key) %
                                        dest_name % final_eta_text % next_eta_text);
            }

        } else if (cur_sys) {
            std::string cur_system_name = cur_sys->ApparentName(client_empire_id);
            if (GetOptionsDB().Get<bool>("ui.name.id.shown")) {
                cur_system_name = cur_system_name + " (" + std::to_string(cur_sys->ID()) + ")";
            }

            if (ClientUI::GetClientUI()->GetMapWnd()->IsFleetExploring(fleet->ID())) {
                if (fleet->Fuel() == fleet->MaxFuel())
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

    bool ContainsArmedShips(const std::vector<int>& ship_ids) {
        for (int ship_id : ship_ids)
            if (auto ship = GetShip(ship_id))
                if (ship->IsArmed() || ship->HasFighters())
                    return true;
        return false;
    }

    bool AggressionForFleet(NewFleetAggression aggression_mode, const std::vector<int>& ship_ids) {
        if (aggression_mode == FLEET_AGGRESSIVE)
            return true;
        if (aggression_mode == FLEET_PASSIVE)
            return false;
        // auto aggression; examine ships to see if any are armed...
        return ContainsArmedShips(ship_ids);
    }

    void CreateNewFleetFromShips(const std::vector<int>& ship_ids,
                                 NewFleetAggression aggression)
    {
        if (ClientPlayerIsModerator())
            return; // todo: handle moderator actions for this...
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();
        if (client_empire_id == ALL_EMPIRES)
            return;

        // TODO: Should probably have the sound effect occur exactly once instead
        //       of not at all.
        Sound::TempUISoundDisabler sound_disabler;

        std::set<int> original_fleet_ids;           // ids of fleets from which ships were taken

        // validate ships in each group, and generate fleet names for those ships
        std::vector<std::shared_ptr<const Ship>> ships = Objects().FindObjects<const Ship>(ship_ids);
        if (ships.empty())
            return;

        std::shared_ptr<const Ship> first_ship = *ships.begin();
        auto system = GetSystem(first_ship->SystemID());
        if (!system)
            return;

        // validate that ships are in the same system and all owned by this
        // client's empire.
        // also record the fleets from which ships are taken
        for (auto& ship : ships) {
             if (ship->SystemID() != system->ID()) {
                 ErrorLogger() << "CreateNewFleetFromShips passed ships with inconsistent system ids";
                 continue;
             }
             if (!ship->OwnedBy(client_empire_id)) {
                 ErrorLogger() << "CreateNewFleetFromShips passed ships not owned by this client's empire";
                 return;
             }

             original_fleet_ids.insert(ship->FleetID());
        }

        // create new fleet with ships
        HumanClientApp::GetApp()->Orders().IssueOrder(
            std::make_shared<NewFleetOrder>(client_empire_id, "", ship_ids, AggressionForFleet(aggression, ship_ids)));
    }

    void CreateNewFleetFromShipsWithDesign(const std::set<int>& ship_ids,
                                           int design_id,
                                           NewFleetAggression aggression)
    {
        DebugLogger() << "CreateNewFleetFromShipsWithDesign with " << ship_ids.size()
                               << " ship ids and design id: " << design_id;
        if (ship_ids.empty() || design_id == INVALID_DESIGN_ID)
            return;
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();
        if (client_empire_id == ALL_EMPIRES && !ClientPlayerIsModerator())
            return;

        // select ships with the requested design id
        std::vector<int> ships_of_design_ids;
        for (auto& ship : Objects().FindObjects<Ship>(ship_ids)) {
            if (ship->DesignID() == design_id)
                ships_of_design_ids.push_back(ship->ID());
        }

        CreateNewFleetFromShips(ships_of_design_ids, aggression);
    }

    void CreateNewFleetsFromShipsForEachDesign(const std::set<int>& ship_ids,
                                               NewFleetAggression aggression)
    {
        DebugLogger() << "CreateNewFleetsFromShipsForEachDesign with "
                               << ship_ids.size() << " ship ids";
        if (ship_ids.empty())
            return;
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();
        if (client_empire_id == ALL_EMPIRES && !ClientPlayerIsModerator())
            return;

        // sort ships by ID into container, indexed by design id
        std::map<int, std::vector<int>> designs_ship_ids;
        for (auto& ship : Objects().FindObjects<Ship>(ship_ids)) {
            designs_ship_ids[ship->DesignID()].push_back(ship->ID());
        }

        // note that this will cause a UI update for each call to CreateNewFleetFromShips
        // we can re-evaluate this code if it presents a noticable performance problem
        for (const auto& entry : designs_ship_ids)
        { CreateNewFleetFromShips(entry.second, aggression); }
    }

    void MergeFleetsIntoFleet(int fleet_id) {
        if (ClientPlayerIsModerator())
            return; // todo: handle moderator actions for this...
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();
        if (client_empire_id == ALL_EMPIRES)
            return;

        auto target_fleet = GetFleet(fleet_id);
        if (!target_fleet) {
            ErrorLogger() << "MergeFleetsIntoFleet couldn't get a fleet with id " << fleet_id;
            return;
        }

        auto system = GetSystem(target_fleet->SystemID());
        if (!system) {
            ErrorLogger() << "MergeFleetsIntoFleet couldn't get system for the target fleet";
            return;
        }

        Sound::TempUISoundDisabler sound_disabler;

        // filter fleets in system to select just those owned by this client's
        // empire, and collect their ship ids
        std::vector<std::shared_ptr<Fleet>> all_system_fleets = Objects().FindObjects<Fleet>(system->FleetIDs());
        std::vector<int> empire_system_fleet_ids;
        empire_system_fleet_ids.reserve(all_system_fleets.size());
        std::vector<std::shared_ptr<Fleet>> empire_system_fleets;
        empire_system_fleets.reserve(all_system_fleets.size());
        std::vector<int> empire_system_ship_ids;

        for (auto& fleet : all_system_fleets) {
            if (!fleet->OwnedBy(client_empire_id))
                continue;
            if (fleet->ID() == target_fleet->ID() || fleet->ID() == INVALID_OBJECT_ID)
                continue;   // no need to do things to target fleet's contents

            empire_system_fleet_ids.push_back(fleet->ID());
            empire_system_fleets.push_back(fleet);

            const std::set<int>& fleet_ships = fleet->ShipIDs();
            std::copy(fleet_ships.begin(), fleet_ships.end(), std::back_inserter(empire_system_ship_ids));
        }


        // order ships moved into target fleet
        HumanClientApp::GetApp()->Orders().IssueOrder(
            std::make_shared<FleetTransferOrder>(client_empire_id, target_fleet->ID(), empire_system_ship_ids));
    }

   /** Returns map from object ID to issued colonize orders affecting it. */
    std::map<int, int> PendingScrapOrders() {
        std::map<int, int> retval;
        const ClientApp* app = ClientApp::GetApp();
        if (!app)
            return retval;
        for (const auto& id_and_order : app->Orders()) {
            if (std::shared_ptr<ScrapOrder> order = std::dynamic_pointer_cast<ScrapOrder>(id_and_order.second)) {
                retval[order->ObjectID()] = id_and_order.first;
            }
        }
        return retval;
    }

    void AddOptions(OptionsDB& db) {
        db.Add("ui.fleet.aggression", UserStringNop("OPTIONS_DB_FLEET_WND_AGGRESSION"), INVALID_FLEET_AGGRESSION, Validator<NewFleetAggression>());
        db.Add("ui.fleet.scanline.color", UserStringNop("OPTIONS_DB_UI_FLEET_WND_SCANLINE_CLR"), GG::Clr(24, 24, 24, 192), Validator<GG::Clr>());
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    NewFleetAggression NewFleetsAggressiveOptionSetting()
    { return GetOptionsDB().Get<NewFleetAggression>("ui.fleet.aggression"); }

    void SetNewFleetAggressiveOptionSetting(NewFleetAggression aggression)
    { GetOptionsDB().Set<NewFleetAggression>("ui.fleet.aggression", aggression); }

    std::shared_ptr<GG::Texture> FleetAggressiveIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "fleet_aggressive.png"); }
    std::shared_ptr<GG::Texture> FleetAggressiveMouseoverIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "fleet_aggressive_mouseover.png"); }
    std::shared_ptr<GG::Texture> FleetPassiveIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "fleet_passive.png"); }
    std::shared_ptr<GG::Texture> FleetPassiveMouseoverIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "fleet_passive_mouseover.png"); }
    std::shared_ptr<GG::Texture> FleetAutoIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "fleet_auto.png"); }
    std::shared_ptr<GG::Texture> FleetAutoMouseoverIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "fleet_auto_mouseover.png"); }
}

////////////////////////////////////////////////
// FleetUIManager                             //
////////////////////////////////////////////////
FleetUIManager::FleetUIManager() :
    m_order_issuing_enabled(true),
    m_active_fleet_wnd()
{}

FleetUIManager::iterator FleetUIManager::begin() const
{ return m_fleet_wnds.begin(); }

bool FleetUIManager::empty() const
{ return m_fleet_wnds.empty(); }

FleetUIManager::iterator FleetUIManager::end() const
{ return m_fleet_wnds.end(); }

FleetWnd* FleetUIManager::ActiveFleetWnd() const
{ return GG::LockAndResetIfExpired(m_active_fleet_wnd).get(); }

std::shared_ptr<FleetWnd> FleetUIManager::WndForFleetID(int fleet_id) const {
    std::shared_ptr<FleetWnd> retval = nullptr;
    GG::ProcessThenRemoveExpiredPtrs(
        m_fleet_wnds,
        [&retval, fleet_id](std::shared_ptr<FleetWnd>& wnd)
        {
            if (!retval && wnd->ContainsFleet(fleet_id))
                retval = wnd;
        });
    return retval;
}

std::shared_ptr<FleetWnd> FleetUIManager::WndForFleetIDs(const std::vector<int>& fleet_ids_) const {
    std::unordered_set<int> fleet_ids;
    for (const auto id : fleet_ids_)
        fleet_ids.insert(id);
    std::shared_ptr<FleetWnd> retval = nullptr;
    GG::ProcessThenRemoveExpiredPtrs(
        m_fleet_wnds,
        [&retval, fleet_ids](std::shared_ptr<FleetWnd>& wnd)
        {
            if (!retval && wnd->ContainsFleets(fleet_ids))
                retval = wnd;
        });
    return retval;
}

int FleetUIManager::SelectedShipID() const {
    const auto&& active_wnd = GG::LockAndResetIfExpired(m_active_fleet_wnd);
    if (!active_wnd)
        return INVALID_OBJECT_ID;

    std::set<int> selected_ship_ids = active_wnd->SelectedShipIDs();
    if (selected_ship_ids.size() != 1)
        return INVALID_OBJECT_ID;

    return *selected_ship_ids.begin();
}

std::set<int> FleetUIManager::SelectedShipIDs() const {
    const auto&& active_wnd = GG::LockAndResetIfExpired(m_active_fleet_wnd);
    if (!active_wnd)
        return std::set<int>();
    return active_wnd->SelectedShipIDs();
}

std::shared_ptr<FleetWnd> FleetUIManager::NewFleetWnd(
    const std::vector<int>& fleet_ids,
    double allowed_bounding_box_leeway /*= 0*/,
    int selected_fleet_id/* = INVALID_OBJECT_ID*/,
    GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | CLOSABLE | GG::RESIZABLE*/)
{
    std::string config_name = "";
    if (!GetOptionsDB().Get<bool>("ui.fleet.multiple.enabled")) {
        CloseAll();
        // Only write to OptionsDB if in single fleet window mode.
        config_name = FLEET_WND_NAME;
    }
    auto retval = GG::Wnd::Create<FleetWnd>(fleet_ids, m_order_issuing_enabled,
                                            allowed_bounding_box_leeway,
                                            selected_fleet_id, flags, config_name);

    m_fleet_wnds.insert(std::weak_ptr<FleetWnd>(retval));
    retval->ClosingSignal.connect(
        boost::bind(&FleetUIManager::FleetWndClosing, this, _1));
    retval->ClickedSignal.connect(
        boost::bind(&FleetUIManager::FleetWndClicked, this, _1));
    retval->FleetRightClickedSignal.connect(
        FleetRightClickedSignal);
    retval->ShipRightClickedSignal.connect(
        ShipRightClickedSignal);

    GG::GUI::GetGUI()->Register(retval);

    return retval;
}

void FleetUIManager::CullEmptyWnds() {
    // scan through FleetWnds, deleting those that have no fleets
    GG::ProcessThenRemoveExpiredPtrs(m_fleet_wnds,
                                 [](std::shared_ptr<FleetWnd>& wnd)
                                 {
                                     if (wnd->FleetIDs().empty())
                                         wnd->CloseClicked();
                                 });
}

void FleetUIManager::SetActiveFleetWnd(std::shared_ptr<FleetWnd> fleet_wnd) {
    const auto&& active_wnd = GG::LockAndResetIfExpired(m_active_fleet_wnd);
    if (fleet_wnd == active_wnd)
        return;

    // disconnect old active FleetWnd signals
    if (active_wnd) {
        for (boost::signals2::connection& con : m_active_fleet_wnd_signals)
            con.disconnect();
        m_active_fleet_wnd_signals.clear();
    }

    // set new active FleetWnd
    m_active_fleet_wnd = fleet_wnd;

    // connect new active FleetWnd selection changed signal
    m_active_fleet_wnd_signals.push_back(fleet_wnd->SelectedFleetsChangedSignal.connect(
        ActiveFleetWndSelectedFleetsChangedSignal));
    m_active_fleet_wnd_signals.push_back(fleet_wnd->SelectedShipsChangedSignal.connect(
        ActiveFleetWndSelectedShipsChangedSignal));

    ActiveFleetWndChangedSignal();
}

bool FleetUIManager::CloseAll() {
    bool retval = false;

    // closing a fleet window removes it from m_fleet_wnds
    GG::ProcessThenRemoveExpiredPtrs(m_fleet_wnds,
                                 [&retval](std::shared_ptr<FleetWnd>& wnd) {
                                     retval = true;
                                     wnd->CloseClicked();
                                 });

    m_active_fleet_wnd.reset();
    ActiveFleetWndChangedSignal();

    // send order changes could be made on fleets
    HumanClientApp::GetApp()->SendPartialOrders();

    return retval;
}

void FleetUIManager::RefreshAll() {
    GG::ProcessThenRemoveExpiredPtrs(m_fleet_wnds,
                                 [](std::shared_ptr<FleetWnd>& wnd)
                                 { wnd->Refresh(); });
}

FleetUIManager& FleetUIManager::GetFleetUIManager() {
    static FleetUIManager retval;
    return retval;
}

void FleetUIManager::FleetWndClosing(FleetWnd* fleet_wnd) {
    if (m_active_fleet_wnd.expired()) {
        m_active_fleet_wnd.reset();
        ActiveFleetWndChangedSignal();
    }

    // send order changes could be made on this fleet
    HumanClientApp::GetApp()->SendPartialOrders();
}

void FleetUIManager::FleetWndClicked(std::shared_ptr<FleetWnd> fleet_wnd) {
    if (fleet_wnd == GG::LockAndResetIfExpired(m_active_fleet_wnd))
        return;
    SetActiveFleetWnd(std::forward<std::shared_ptr<FleetWnd>>(fleet_wnd));
}

void FleetUIManager::EnableOrderIssuing(bool enable/* = true*/) {
    m_order_issuing_enabled = enable;
    GG::ProcessThenRemoveExpiredPtrs(m_fleet_wnds,
                                 [&enable](std::shared_ptr<FleetWnd>& wnd)
                                 { wnd->EnableOrderIssuing(enable); });
}

namespace {
    bool ValidShipTransfer(std::shared_ptr<const Ship> ship, std::shared_ptr<const Fleet> new_fleet) {
        if (!ship || !new_fleet)
            return false;   // can't transfer no ship or to no fleet

        std::shared_ptr<const Fleet> current_fleet = GetFleet(ship->FleetID());
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

    bool ValidFleetMerge(std::shared_ptr<const Fleet> fleet, std::shared_ptr<const Fleet> target_fleet) {
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
    // ShipDataPanel
    ////////////////////////////////////////////////
    /** Shows info about a single ship. */
    class ShipDataPanel : public GG::Control {
    public:
        /** \name Structors */ //@{
        ShipDataPanel(GG::X w, GG::Y h, int ship_id);
        ~ShipDataPanel();
        //@}

        //! \name Accessors //@{
        /** Excludes border from the client area. */
        GG::Pt ClientUpperLeft() const override;
        /** Excludes border from the client area. */
        GG::Pt ClientLowerRight() const override;
        //@}

        //! \name Mutators //@{
        /** Renders black panel background, border with color depending on the
         *current state and a background for the ship's name text. */
        void Render() override;
        void PreRender() override;

        void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

        void Select(bool b);

        /** Indicate ship data has changed and needs refresh. */
        void RequireRefresh();
        //@}

    private:
        double StatValue(MeterType stat_name) const;

        void SetShipIcon();
        void Refresh();
        void DoLayout();

        void Init();

        bool m_initialized   = false;;
        bool m_needs_refresh = true;

        int                                m_ship_id;
        std::shared_ptr<GG::StaticGraphic> m_ship_icon;
        /// An overlays for orders like scrap, colonize, invade, bombard destroy etc.
        std::vector<std::shared_ptr<GG::StaticGraphic>> m_ship_icon_overlays;
        std::shared_ptr<ScanlineControl>   m_scanline_control;
        std::shared_ptr<GG::Label>         m_ship_name_text;
        std::shared_ptr<GG::Label>         m_design_name_text;

        /// statistic icons and associated meter types
        std::vector<std::pair<MeterType, std::shared_ptr<StatisticIcon>>> m_stat_icons;

        bool                        m_selected;
        boost::signals2::connection m_ship_connection;
        boost::signals2::connection m_fleet_connection;
    };

    ShipDataPanel::ShipDataPanel(GG::X w, GG::Y h, int ship_id) :
        Control(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS),
        m_initialized(false),
        m_ship_id(ship_id),
        m_ship_icon(nullptr),
        m_ship_icon_overlays(),
        m_scanline_control(nullptr),
        m_ship_name_text(nullptr),
        m_design_name_text(nullptr),
        m_stat_icons(),
        m_selected(false)
    {
        SetChildClippingMode(ClipToClient);
        RequireRefresh();
    }

    ShipDataPanel::~ShipDataPanel() {
        m_ship_connection.disconnect();
        m_fleet_connection.disconnect();
    }

    GG::Pt ShipDataPanel::ClientUpperLeft() const
    { return UpperLeft() + GG::Pt(GG::X(DATA_PANEL_BORDER), GG::Y(DATA_PANEL_BORDER)); }

    GG::Pt ShipDataPanel::ClientLowerRight() const
    { return LowerRight() - GG::Pt(GG::X(DATA_PANEL_BORDER), GG::Y(DATA_PANEL_BORDER));  }

    void ShipDataPanel::RequireRefresh() {
        m_needs_refresh = true;
        RequirePreRender();
    }

    void ShipDataPanel::PreRender() {
        if (!m_initialized)
            Init();

        GG::Control::PreRender();
        if (m_needs_refresh)
            Refresh();

        DoLayout();
    }

    void ShipDataPanel::Render() {
        // main background position and colour
        const GG::Clr& background_colour = ClientUI::WndColor();
        const GG::Pt ul = UpperLeft(), lr = LowerRight(), cul = ClientUpperLeft();

        // title background colour and position
        const GG::Clr& unselected_colour = ClientUI::WndOuterBorderColor();
        const GG::Clr& selected_colour = ClientUI::WndInnerBorderColor();
        GG::Clr border_colour = m_selected ? selected_colour : unselected_colour;
        if (Disabled())
            border_colour = DisabledColor(border_colour);
        const GG::Pt text_ul = cul + GG::Pt(DataPanelIconSpace().x, GG::Y0);
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
            RequirePreRender();
    }

    void ShipDataPanel::SetShipIcon() {
        DetachChildAndReset(m_ship_icon);
        for (auto& overlay : m_ship_icon_overlays)
            DetachChildAndReset(overlay);
        m_ship_icon_overlays.clear();
        DetachChildAndReset(m_scanline_control);

        auto ship = GetShip(m_ship_id);
        if (!ship)
            return;

        std::shared_ptr<GG::Texture> icon;

        if (const ShipDesign* design = ship->Design())
            icon = ClientUI::ShipDesignIcon(design->ID());
        else
            icon = ClientUI::ShipDesignIcon(INVALID_OBJECT_ID);  // default icon

        m_ship_icon = GG::Wnd::Create<GG::StaticGraphic>(icon, DataPanelIconStyle());
        m_ship_icon->Resize(GG::Pt(DataPanelIconSpace().x, ClientHeight()));
        AttachChild(m_ship_icon);

        // Add the overlay
        auto add_overlay = [this](const std::string& file) {
            if (auto overlay_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / file, true)) {
                auto overlay = GG::Wnd::Create<GG::StaticGraphic>(overlay_texture, DataPanelIconStyle());
                overlay->Resize(GG::Pt(DataPanelIconSpace().x, ClientHeight()));
                AttachChild(overlay);
                m_ship_icon_overlays.push_back(overlay);
            }
        };

        if (ship->OrderedScrapped())
            add_overlay("scrapped.png");
        if (ship->OrderedColonizePlanet() != INVALID_OBJECT_ID)
            add_overlay("colonizing.png");
        if (ship->OrderedInvadePlanet() != INVALID_OBJECT_ID)
            add_overlay("invading.png");
        if (ship->OrderedBombardPlanet() != INVALID_OBJECT_ID)
            add_overlay("bombarding.png");

        int client_empire_id = HumanClientApp::GetApp()->EmpireID();
        if ((ship->GetVisibility(client_empire_id) < VIS_BASIC_VISIBILITY)
            && GetOptionsDB().Get<bool>("ui.map.scanlines.shown"))
        {
            m_scanline_control = GG::Wnd::Create<ScanlineControl>(GG::X0, GG::Y0, m_ship_icon->Width(), m_ship_icon->Height(), true,
                                                                  GetOptionsDB().Get<GG::Clr>("ui.fleet.scanline.color"));
            AttachChild(m_scanline_control);
        }
    }

    void ShipDataPanel::Refresh() {
        m_needs_refresh = false;

        SetShipIcon();

        auto ship = GetShip(m_ship_id);
        if (!ship) {
            // blank text and delete icons
            m_ship_name_text->SetText("");
            DetachChildAndReset(m_design_name_text);
            for (auto& type_and_icon: m_stat_icons)
                DetachChild(type_and_icon.second);
            m_stat_icons.clear();
            return;
        }


        int empire_id = HumanClientApp::GetApp()->EmpireID();


        // name and design name update
        const std::string& ship_name = ship->PublicName(empire_id);
        std::string id_name_part;
        if (GetOptionsDB().Get<bool>("ui.name.id.shown")) {
            id_name_part = " (" + std::to_string(m_ship_id) + ")";
        }
        if (!ship->Unowned() && ship_name == UserString("FW_FOREIGN_SHIP")) {
            const Empire* ship_owner_empire = GetEmpire(ship->Owner());
            const std::string& owner_name = (ship_owner_empire ? ship_owner_empire->Name() : UserString("FW_FOREIGN"));
            m_ship_name_text->SetText(boost::io::str(FlexibleFormat(UserString("FW_EMPIRE_SHIP")) % owner_name) + id_name_part);
        } else {
            m_ship_name_text->SetText(ship_name + id_name_part);
        }

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
        for (auto& entry : m_stat_icons) {
            entry.second->SetValue(StatValue(entry.first));

            entry.second->ClearBrowseInfoWnd();
            if (entry.first == METER_CAPACITY) {  // refers to damage
                entry.second->SetBrowseInfoWnd(GG::Wnd::Create<ShipDamageBrowseWnd>(
                                                   m_ship_id, entry.first));

            } else if (entry.first == METER_TROOPS) {
                entry.second->SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
                                                   TroopIcon(), UserString("SHIP_TROOPS_TITLE"),
                                                   UserString("SHIP_TROOPS_STAT")));

            } else if (entry.first == METER_SECONDARY_STAT) {
                entry.second->SetBrowseInfoWnd(GG::Wnd::Create<ShipFightersBrowseWnd>(
                                                   m_ship_id, entry.first));
                entry.second->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.extended.delay"), 1);
                entry.second->SetBrowseInfoWnd(GG::Wnd::Create<ShipFightersBrowseWnd>(
                                                   m_ship_id, entry.first, true), 1);

            } else if (entry.first == METER_POPULATION) {
                entry.second->SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
                                                   ColonyIcon(), UserString("SHIP_COLONY_TITLE"),
                                                   UserString("SHIP_COLONY_STAT")));

            } else {
                entry.second->SetBrowseInfoWnd(GG::Wnd::Create<MeterBrowseWnd>(
                                                   m_ship_id, entry.first, AssociatedMeterType(entry.first)));
            }
        }
    }

    double ShipDataPanel::StatValue(MeterType stat_name) const {
        if (auto ship = GetShip(m_ship_id)) {
            if (stat_name == METER_CAPACITY)
                return ship->TotalWeaponsDamage(0.0f, false);
            else if (stat_name == METER_TROOPS)
                return ship->TroopCapacity();
            else if (stat_name == METER_SECONDARY_STAT)
                return ship->FighterCount();
            else if (stat_name == METER_POPULATION)
                return ship->ColonyCapacity();
            else if (ship->UniverseObject::GetMeter(stat_name))
                return ship->InitialMeterValue(stat_name);

            ErrorLogger() << "ShipDataPanel::StatValue couldn't get stat of name: " << boost::lexical_cast<std::string>(stat_name);
        }
        return 0.0;
    }

    void ShipDataPanel::DoLayout() {
        // resize ship and scrap indicator icons, they can fit and position themselves in the space provided
        // client height should never be less than the height of the space resereved for the icon
        if (m_ship_icon)
            m_ship_icon->Resize(GG::Pt(DataPanelIconSpace().x, ClientHeight()));
        for (auto& overlay :m_ship_icon_overlays)
            overlay->Resize(GG::Pt(DataPanelIconSpace().x, ClientHeight()));

        // position ship name text at the top to the right of icons
        const GG::Pt name_ul = GG::Pt(DataPanelIconSpace().x + DATA_PANEL_TEXT_PAD, GG::Y0);
        const GG::Pt name_lr = GG::Pt(ClientWidth() - DATA_PANEL_TEXT_PAD,           LabelHeight());
        if (m_ship_name_text)
            m_ship_name_text->SizeMove(name_ul, name_lr);
        if (m_design_name_text)
            m_design_name_text->SizeMove(name_ul, name_lr);

        if (ClientWidth() < 250)
            DetachChild(m_ship_name_text);
        else
            AttachChild(m_ship_name_text);

        // position ship statistic icons one after another horizontally and centered vertically
        GG::Pt icon_ul = GG::Pt(name_ul.x, LabelHeight() + std::max(GG::Y0, (ClientHeight() - LabelHeight() - StatIconSize().y) / 2));
        for (auto& entry : m_stat_icons) {
            entry.second->SizeMove(icon_ul, icon_ul + StatIconSize());
            icon_ul.x += StatIconSize().x;
        }
    }

    void ShipDataPanel::Init() {
        if (m_initialized)
            return;
        m_initialized = true;

        // ship name text.  blank if no ship.
        auto ship = GetShip(m_ship_id);
        std::string ship_name;
        if (ship)
            ship_name = ship->Name();

        if (GetOptionsDB().Get<bool>("ui.name.id.shown")) {
            ship_name = ship_name + " (" + std::to_string(m_ship_id) + ")";
        }

        m_ship_name_text = GG::Wnd::Create<CUILabel>(ship_name, GG::FORMAT_LEFT);
        AttachChild(m_ship_name_text);


        // design name and statistic icons
        if (!ship)
            return;

        if (const ShipDesign* design = ship->Design()) {
            m_design_name_text = GG::Wnd::Create<CUILabel>(design->Name(), GG::FORMAT_RIGHT);
            AttachChild(m_design_name_text);
        }


        int tooltip_delay = GetOptionsDB().Get<int>("ui.tooltip.delay");

        std::vector<std::pair<MeterType, std::shared_ptr<GG::Texture>>> meters_icons;
        meters_icons.push_back({METER_STRUCTURE,          ClientUI::MeterIcon(METER_STRUCTURE)});
        if (ship->IsArmed())
            meters_icons.push_back({METER_CAPACITY,       DamageIcon()});
        if (ship->HasFighters())
            meters_icons.push_back({METER_SECONDARY_STAT, FightersIcon()});
        if (ship->HasTroops())
            meters_icons.push_back({METER_TROOPS,         TroopIcon()});
        if (ship->CanColonize())
            meters_icons.push_back({METER_POPULATION,     ColonyIcon()});
        if (ship->InitialMeterValue(METER_INDUSTRY) > 0.0f)
            meters_icons.push_back({METER_INDUSTRY,       IndustryIcon()});
        if (ship->InitialMeterValue(METER_RESEARCH) > 0.0f)
            meters_icons.push_back({METER_RESEARCH,       ResearchIcon()});
        if (ship->InitialMeterValue(METER_TRADE) > 0.0f)
            meters_icons.push_back({METER_TRADE,          TradeIcon()});

        for (auto& meter : {METER_SHIELD, METER_FUEL, METER_DETECTION,
                            METER_STEALTH, METER_SPEED})
        {
            meters_icons.push_back({meter, ClientUI::MeterIcon(meter)});
        }

        for (auto& entry : meters_icons) {
            auto icon = GG::Wnd::Create<StatisticIcon>(entry.second, 0, 0, false, StatIconSize().x, StatIconSize().y);
            m_stat_icons.push_back({entry.first, icon});
            AttachChild(icon);
            icon->SetBrowseModeTime(tooltip_delay);
        }

        // bookkeeping
        m_ship_connection = ship->StateChangedSignal.connect(
            boost::bind(&ShipDataPanel::RequireRefresh, this));

        if (auto fleet = GetFleet(ship->FleetID()))
            m_fleet_connection = fleet->StateChangedSignal.connect(
                boost::bind(&ShipDataPanel::RequireRefresh, this));
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
            m_panel(nullptr)
        {
            SetName("ShipRow");
            SetChildClippingMode(ClipToClient);
            if (GetShip(m_ship_id))
                SetDragDropDataType(SHIP_DROP_TYPE_STRING);
        }

        void CompleteConstruction() override {
            GG::ListBox::Row::CompleteConstruction();

            m_panel = GG::Wnd::Create<ShipDataPanel>(Width(), Height(), m_ship_id);
            push_back(m_panel);
        }

        void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
            const GG::Pt old_size = Size();
            GG::ListBox::Row::SizeMove(ul, lr);
            //std::cout << "ShipRow::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
            if (!empty() && old_size != Size() && m_panel)
                m_panel->Resize(Size());
        }

        int             ShipID() const {return m_ship_id;}
    private:
        int             m_ship_id;
        std::shared_ptr<ShipDataPanel>  m_panel;
    };
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
    ~FleetDataPanel();

    /** Upper left plus border insets. */
    GG::Pt ClientUpperLeft() const override;

    /** Lower right minus border insets. */
    GG::Pt ClientLowerRight() const override;

    void PreRender() override;

    void Render() override;

    void DragDropHere(const GG::Pt& pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
                      GG::Flags<GG::ModKey> mod_keys) override;

    void CheckDrops(const GG::Pt& pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
                    GG::Flags<GG::ModKey> mod_keys) override;

    void DragDropLeave() override;

    void AcceptDrops(const GG::Pt& pt, std::vector<std::shared_ptr<GG::Wnd>> wnds, GG::Flags<GG::ModKey> mod_keys) override;

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    bool                Selected() const;
    NewFleetAggression  GetNewFleetAggression() const;
    void                Select(bool b);
    void                SetSystemID(int id);

    /** Indicate fleet data has changed and needs refresh. */
    void RequireRefresh();

    mutable boost::signals2::signal<void (const std::vector<int>&)> NewFleetFromShipsSignal;

protected:
    bool EventFilter(GG::Wnd* w, const GG::WndEvent& event) override;

    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const override;

private:
    void                ToggleAggression();

    void                Refresh();
    void                RefreshStateChangedSignals();

    void                SetStatIconValues();
    void                UpdateAggressionToggle();
    void                DoLayout();
    void                Init();
    void                ColorTextForSelect();

    const int           m_fleet_id;
    int                 m_system_id;
    const bool          m_is_new_fleet_drop_target;
    NewFleetAggression  m_new_fleet_aggression;

    bool                m_needs_refresh = true;

    boost::signals2::connection              m_fleet_connection;
    std::vector<boost::signals2::connection> m_ship_connections;

    std::shared_ptr<GG::Control>        m_fleet_icon = nullptr;
    std::shared_ptr<GG::Label>          m_fleet_name_text = nullptr;
    std::shared_ptr<GG::Label>          m_fleet_destination_text = nullptr;
    std::shared_ptr<GG::Button>         m_aggression_toggle = nullptr;
    std::vector<std::shared_ptr<GG::StaticGraphic>>  m_fleet_icon_overlays;
    std::shared_ptr<ScanlineControl>    m_scanline_control = nullptr;

    std::vector<std::pair<MeterType, std::shared_ptr<StatisticIcon>>>   m_stat_icons;   // statistic icons and associated meter types

    bool                m_selected = false;
    bool                m_initialized = false;
};

FleetDataPanel::FleetDataPanel(GG::X w, GG::Y h, int fleet_id) :
    Control(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS),
    m_fleet_id(fleet_id),
    m_system_id(INVALID_OBJECT_ID),
    m_is_new_fleet_drop_target(false),
    m_new_fleet_aggression(NewFleetsAggressiveOptionSetting()),
    m_fleet_icon_overlays()
{
    RequireRefresh();
    SetChildClippingMode(ClipToClient);
}

FleetDataPanel::FleetDataPanel(GG::X w, GG::Y h, int system_id, bool new_fleet_drop_target) :
    Control(GG::X0, GG::Y0, w, h, GG::INTERACTIVE),
    m_fleet_id(INVALID_OBJECT_ID),
    m_system_id(system_id),
    m_is_new_fleet_drop_target(new_fleet_drop_target),
    m_new_fleet_aggression(NewFleetsAggressiveOptionSetting()),
    m_fleet_icon_overlays()
{
    RequirePreRender();
    SetChildClippingMode(ClipToClient);
}

FleetDataPanel::~FleetDataPanel()
{}

GG::Pt FleetDataPanel::ClientUpperLeft() const
{ return UpperLeft() + GG::Pt(GG::X(DATA_PANEL_BORDER), GG::Y(DATA_PANEL_BORDER)); }

GG::Pt FleetDataPanel::ClientLowerRight() const
{ return LowerRight() - GG::Pt(GG::X(DATA_PANEL_BORDER), GG::Y(DATA_PANEL_BORDER));  }

bool FleetDataPanel::Selected() const
{ return m_selected; }

NewFleetAggression FleetDataPanel::GetNewFleetAggression() const
{ return m_new_fleet_aggression; }

void FleetDataPanel::RequireRefresh() {
    m_needs_refresh = true;
    RequirePreRender();
}

void FleetDataPanel::PreRender() {
    if (!m_initialized)
        Init();

    GG::Wnd::PreRender();
    if (m_needs_refresh)
        Refresh();
    DoLayout();
}

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
    const GG::Pt text_ul = cul + GG::Pt(DataPanelIconSpace().x, GG::Y0);
    const GG::Pt text_lr = cul + GG::Pt(ClientWidth(),           LabelHeight());

    // render
    GG::FlatRectangle(ul,       lr,         background_colour,  border_colour, DATA_PANEL_BORDER);  // background and border
    GG::FlatRectangle(text_ul,  text_lr,    border_colour,      GG::CLR_ZERO,  0);                  // title background box
}

void FleetDataPanel::DragDropHere(const GG::Pt& pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
                                  GG::Flags<GG::ModKey> mod_keys)
{
    if (!m_is_new_fleet_drop_target) {
        // normally the containing row (or the listbox that contains that) will
        // handle drag-drop related things
        //std::cout << "FleetDataPanel::DragDropHere forwarding to parent..." << std::endl << std::flush;
        ForwardEventToParent();
    }

    //std::cout << "FleetDataPanel::DragDropHere locally checking drops..." << std::endl << std::flush;
    DropsAcceptable(drop_wnds_acceptable.begin(), drop_wnds_acceptable.end(), pt, mod_keys);

    if (Disabled()) {
        Select(false);
        return;
    }

    // select panel if all dragged Wnds can be dropped here...

    Select(true);   // default

    // get whether each Wnd is dropable
    DropsAcceptable(drop_wnds_acceptable.begin(), drop_wnds_acceptable.end(), pt, mod_keys);

    // scan through wnds, looking for one that isn't dropable
    for (const auto& drop_wnd_acceptable : drop_wnds_acceptable) {
        if (!drop_wnd_acceptable.second) {
            // wnd can't be dropped
            Select(false);
            break;
        }
    }
}

void FleetDataPanel::CheckDrops(const GG::Pt& pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
                                GG::Flags<GG::ModKey> mod_keys)
{
    if (!m_is_new_fleet_drop_target) {
        // normally the containing row (or the listbox that contains that) will
        // handle drag-drop related things
        ForwardEventToParent();
    }
    Control::CheckDrops(pt, drop_wnds_acceptable, mod_keys);
}

void FleetDataPanel::DragDropLeave()
{
    //std::cout << "FleetDataPanel::DragDropLeave" << std::endl << std::flush;
    Select(false);
}

void FleetDataPanel::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                                     const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const
{
    if (!m_is_new_fleet_drop_target) {
        //std::cout << "FleetDataPanel::DropsAcceptable default rejecting all" << std::endl << std::flush;
        // reject all
        Wnd::DropsAcceptable(first, last, pt, mod_keys);
        return;
    }

    //std::cout << "FleetDataPanel::DropsAcceptable locally checking drops" << std::endl << std::flush;
    // only used when FleetDataPanel sets independently in the FleetWnd, not
    // in a FleetListBox

    int this_client_empire_id = HumanClientApp::GetApp()->EmpireID();
    std::shared_ptr<const Fleet> this_panel_fleet = GetFleet(m_fleet_id);

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
        auto ship = GetShip(ship_row->ShipID());
        if (!ship)
            continue;

        // reject drops if the ship is not owned by this client's empire
        if (!ship->OwnedBy(this_client_empire_id))
            continue;

        if (m_is_new_fleet_drop_target) {
            // only allows dropping ships?
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

void FleetDataPanel::AcceptDrops(const GG::Pt& pt, std::vector<std::shared_ptr<GG::Wnd>> wnds, GG::Flags<GG::ModKey> mod_keys) {
    if (!m_is_new_fleet_drop_target && Parent()) {
        // normally the containing row (or the listbox that contains that) will
        // handle drag-drops
        ForwardEventToParent();
    }

    // following only used when FleetDataPanel sets independently in the
    // FleetWnd, not in a FleetListBox

    DebugLogger() << "FleetWnd::AcceptDrops with " << wnds.size() << " wnds at pt: " << pt;
    std::vector<int> ship_ids;
    ship_ids.reserve(wnds.size());
    for (auto& wnd : wnds)
        if (const ShipRow* ship_row = boost::polymorphic_downcast<const ShipRow*>(wnd.get()))
            ship_ids.push_back(ship_row->ShipID());
    std::string id_list;
    for (int ship_id : ship_ids)
        id_list += std::to_string(ship_id) + " ";
    DebugLogger() << "FleetWnd::AcceptDrops found " << ship_ids.size() << " ship ids: " << id_list;

    if (ship_ids.empty())
        return;

    NewFleetFromShipsSignal(ship_ids);
}

void FleetDataPanel::Select(bool b) {
    if (m_selected == b)
        return;
    m_selected = b;

    ColorTextForSelect();
}

void FleetDataPanel::SetSystemID(int id)
{ m_system_id = id; }

void FleetDataPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    GG::Control::SizeMove(ul, lr);
    //std::cout << "FleetDataPanel::SizeMove new size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
    if (old_size != Size())
        DoLayout();
}

bool FleetDataPanel::EventFilter(GG::Wnd* w, const GG::WndEvent& event) {
    //std::cout << "FleetDataPanel::EventFilter " << EventTypeName(event) << std::endl << std::flush;

    if (w == this)
        return false;

    switch (event.Type()) {
    case GG::WndEvent::DragDropEnter:
    case GG::WndEvent::DragDropHere:
    case GG::WndEvent::CheckDrops:
    case GG::WndEvent::DragDropLeave:
    case GG::WndEvent::DragDroppedOn:
        if (w == this) {
            ErrorLogger() << "FleetDataPanel::EventFilter w == this";
            return false;
        }
        HandleEvent(event);
        return true;
        break;
    default:
        return false;
    }
}

void FleetDataPanel::ToggleAggression() {
    if (!m_aggression_toggle)
        return;
    auto fleet = GetFleet(m_fleet_id);
    if (fleet) {
        if (ClientPlayerIsModerator())
            return; // todo: handle moderator actions for this...
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();
        if (client_empire_id == ALL_EMPIRES)
            return;

        bool new_aggression_state = !fleet->Aggressive();

        // toggle fleet aggression status
        HumanClientApp::GetApp()->Orders().IssueOrder(
            std::make_shared<AggressiveOrder>(client_empire_id, m_fleet_id, new_aggression_state));
    } else if (m_is_new_fleet_drop_target) {
        // cycle new fleet aggression
        if (m_new_fleet_aggression == INVALID_FLEET_AGGRESSION)
            m_new_fleet_aggression = FLEET_AGGRESSIVE;
        else if (m_new_fleet_aggression == FLEET_AGGRESSIVE)
            m_new_fleet_aggression = FLEET_PASSIVE;
        else
            m_new_fleet_aggression = INVALID_FLEET_AGGRESSION;
        SetNewFleetAggressiveOptionSetting(m_new_fleet_aggression);
        UpdateAggressionToggle();
    }
}

void FleetDataPanel::Refresh() {
    m_needs_refresh = false;

    DetachChildAndReset(m_fleet_icon);
    DetachChildAndReset(m_scanline_control);
    for (auto& overlay : m_fleet_icon_overlays)
        DetachChildAndReset(overlay);
    m_fleet_icon_overlays.clear();

    if (m_is_new_fleet_drop_target) {
        m_fleet_name_text->SetText(UserString("FW_NEW_FLEET_LABEL"));
        m_fleet_destination_text->Clear();

        std::shared_ptr<GG::Texture> new_fleet_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "new_fleet.png", true);
        m_fleet_icon = GG::Wnd::Create<GG::StaticGraphic>(new_fleet_texture, DataPanelIconStyle());
        AttachChild(m_fleet_icon);

    } else if (auto fleet = GetFleet(m_fleet_id)) {
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();
        // set fleet name and destination text
        std::string public_fleet_name = fleet->PublicName(client_empire_id);
        if (!fleet->Unowned() && public_fleet_name == UserString("FW_FOREIGN_FLEET")) {
            const Empire* ship_owner_empire = GetEmpire(fleet->Owner());
            const std::string& owner_name = (ship_owner_empire ? ship_owner_empire->Name() : UserString("FW_FOREIGN"));
            std::string fleet_name = boost::io::str(FlexibleFormat(UserString("FW_EMPIRE_FLEET")) % owner_name);
            if (GetOptionsDB().Get<bool>("ui.name.id.shown")) {
                fleet_name = fleet_name + " (" + std::to_string(m_fleet_id) + ")";
            }
            m_fleet_name_text->SetText(fleet_name);
        } else {
            if (GetOptionsDB().Get<bool>("ui.name.id.shown")) {
                public_fleet_name = public_fleet_name + " (" + std::to_string(m_fleet_id) + ")";
            }
            m_fleet_name_text->SetText(public_fleet_name);
        }
        m_fleet_destination_text->SetText(FleetDestinationText(m_fleet_id));

        // set icons
        std::vector<std::shared_ptr<GG::Texture>>   icons;
        std::vector<GG::Flags<GG::GraphicStyle>>    styles;

        std::shared_ptr<GG::Texture> size_icon = FleetSizeIcon(fleet, FleetButton::SizeType::LARGE);
        icons.push_back(size_icon);
        styles.push_back(DataPanelIconStyle());

        std::vector<std::shared_ptr<GG::Texture>> head_icons = FleetHeadIcons(fleet, FleetButton::SizeType::LARGE);
        std::copy(head_icons.begin(), head_icons.end(), std::back_inserter(icons));
        for (size_t i = 0; i < head_icons.size(); ++i)
            styles.push_back(DataPanelIconStyle());

        m_fleet_icon = GG::Wnd::Create<MultiTextureStaticGraphic>(icons, styles);
        AttachChild(m_fleet_icon);

        if (Empire* empire = GetEmpire(fleet->Owner()))
            m_fleet_icon->SetColor(empire->Color());
        else if (fleet->Unowned() && fleet->HasMonsters())
            m_fleet_icon->SetColor(GG::CLR_RED);

        auto all_ships = [fleet](const std::function<bool(const std::shared_ptr<const Ship>&)>& pred) {
            // Searching for each Ship one at a time is faster than
            // FindObjects(ship_ids), because an early exit avoids searching the
            // remaining ids.
            return std::all_of(
                fleet->ShipIDs().begin(), fleet->ShipIDs().end(),
                [&pred](const int ship_id) {
                    const auto& ship = Objects().Object<const Ship>(ship_id);
                    if (!ship) {
                        WarnLogger() << "Object map is missing ship with expected id " << ship_id;
                        return false;
                    }
                    return pred(ship);
                });
        };

        // Add the overlay
        auto add_overlay = [this](const std::string& file) {
            if (auto overlay_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / file, true)) {
                auto overlay = GG::Wnd::Create<GG::StaticGraphic>(overlay_texture, DataPanelIconStyle());
                overlay->Resize(GG::Pt(DataPanelIconSpace().x, ClientHeight()));
                AttachChild(overlay);
                m_fleet_icon_overlays.push_back(overlay);
            }
        };

        // Add overlays for all ships colonizing, invading etc.
        std::shared_ptr<GG::Texture> overlay_texture;
        if (all_ships([](const std::shared_ptr<const Ship>& ship) { return ship->OrderedScrapped(); }))
            add_overlay("scrapped.png");
        if (all_ships(
                [](const std::shared_ptr<const Ship>& ship)
                {return ship->OrderedColonizePlanet() != INVALID_OBJECT_ID; })
           )
        {
            add_overlay("colonizing.png");
        }
        if (all_ships(
                [](const std::shared_ptr<const Ship>& ship)
                { return ship->OrderedInvadePlanet() != INVALID_OBJECT_ID; })
           )
        {
            add_overlay("invading.png");
        }
        if (all_ships(
                [](const std::shared_ptr<const Ship>& ship)
                { return ship->OrderedBombardPlanet() != INVALID_OBJECT_ID; })
           )
        {
            add_overlay("bombarding.png");
        }

        // Moving fleets can't be gifted.  The order will be automatically
        // cancelled on the server.  This make the UI appear to cancel the
        // order when the ship is moved without requiring the player to
        // re-order the gifting if the ship is stopped.
        if (fleet->OrderedGivenToEmpire() != ALL_EMPIRES && fleet->TravelRoute().empty())
            add_overlay("gifting.png");

        if ((fleet->GetVisibility(client_empire_id) < VIS_BASIC_VISIBILITY)
            && GetOptionsDB().Get<bool>("ui.map.scanlines.shown"))
        {
            m_scanline_control = GG::Wnd::Create<ScanlineControl>(GG::X0, GG::Y0, DataPanelIconSpace().x, ClientHeight(), true,
                                                                  GetOptionsDB().Get<GG::Clr>("ui.fleet.scanline.color"));
            AttachChild(m_scanline_control);
        }

        SetStatIconValues();

        RefreshStateChangedSignals();
    }

    UpdateAggressionToggle();
}

void FleetDataPanel::RefreshStateChangedSignals() {
    m_fleet_connection.disconnect();
    for (auto& connection : m_ship_connections)
        connection.disconnect();

    auto fleet = GetFleet(m_fleet_id);
    if (!fleet)
        return;

    m_fleet_connection = fleet->StateChangedSignal.connect(
        boost::bind(&FleetDataPanel::RequireRefresh, this));

    for (auto& ship : Objects().FindObjects<const Ship>(fleet->ShipIDs()))
        m_ship_connections.push_back(
            ship->StateChangedSignal.connect(
                boost::bind(&FleetDataPanel::RequireRefresh, this)));
}

void FleetDataPanel::SetStatIconValues() {
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    const std::set<int>& this_client_known_destroyed_objects = GetUniverse().EmpireKnownDestroyedObjectIDs(client_empire_id);
    const std::set<int>& this_client_stale_object_info = GetUniverse().EmpireStaleKnowledgeObjectIDs(client_empire_id);
    int ship_count =        0;
    float damage_tally =    0.0f;
    float fighters_tally  = 0.0f;
    float structure_tally = 0.0f;
    float shield_tally =    0.0f;
    float min_fuel =        0.0f;
    float min_speed =       0.0f;
    float troops_tally =    0.0f;
    float colony_tally =    0.0f;
    std::vector<float> fuels;
    std::vector<float> speeds;

    auto fleet = GetFleet(m_fleet_id);

    fuels.reserve(fleet->NumShips());
    speeds.reserve(fleet->NumShips());
    for (auto& ship : Objects().FindObjects<const Ship>(fleet->ShipIDs())) {
        int ship_id = ship->ID();
        // skip known destroyed and stale info objects
        if (this_client_known_destroyed_objects.count(ship_id))
            continue;
        if (this_client_stale_object_info.count(ship_id))
            continue;

        if (ship->Design()) {
            ship_count++;
            damage_tally += ship->TotalWeaponsDamage(0.0f, false);
            fighters_tally += ship->FighterCount();
            troops_tally += ship->TroopCapacity();
            colony_tally += ship->ColonyCapacity();
            structure_tally += ship->InitialMeterValue(METER_STRUCTURE);
            shield_tally += ship->InitialMeterValue(METER_SHIELD);
            fuels.push_back(ship->InitialMeterValue(METER_FUEL));
            speeds.push_back(ship->InitialMeterValue(METER_SPEED));
        }
    }
    if (!fuels.empty())
        min_fuel = *std::min_element(fuels.begin(), fuels.end());
    if (!speeds.empty())
        min_speed = *std::min_element(speeds.begin(), speeds.end());

    for (const auto& entry : m_stat_icons) {
        MeterType stat_name = entry.first;
        const auto& icon = entry.second;
        DetachChild(icon);
        switch (stat_name) {
        case METER_SIZE:
            icon->SetValue(ship_count);
            AttachChild(icon);
            break;
        case METER_CAPACITY:
            icon->SetValue(damage_tally);
            if (fleet->HasArmedShips())
                AttachChild(icon);
            break;
        case METER_SECONDARY_STAT:
            icon->SetValue(fighters_tally);
            if (fleet->HasFighterShips())
                AttachChild(icon);
            break;
        case METER_TROOPS:
            icon->SetValue(troops_tally);
            if (fleet->HasTroopShips())
                AttachChild(icon);
            break;
        case METER_POPULATION:
            icon->SetValue(colony_tally);
            if (fleet->HasColonyShips())
                AttachChild(icon);
            break;
        case METER_INDUSTRY: {
            const auto resource_output = fleet->ResourceOutput(RE_INDUSTRY);
            icon->SetValue(resource_output);
            if (resource_output > 0.0f)
                AttachChild(icon);
        }
            break;
        case METER_RESEARCH: {
            const auto resource_output = fleet->ResourceOutput(RE_RESEARCH);
            icon->SetValue(resource_output);
            if (resource_output > 0.0f)
                AttachChild(icon);
        }
            break;
        case METER_TRADE: {
            const auto resource_output = fleet->ResourceOutput(RE_TRADE);
            icon->SetValue(resource_output);
            if (resource_output > 0.0f)
                AttachChild(icon);
        }
            break;
        case METER_STRUCTURE:
            icon->SetValue(structure_tally);
            AttachChild(icon);
            break;
        case METER_SHIELD:
            icon->SetValue(shield_tally/ship_count);
            AttachChild(icon);
            break;
        case METER_FUEL:
            icon->SetValue(min_fuel);
            AttachChild(icon);
            break;
        case METER_SPEED:
            icon->SetValue(min_speed);
            AttachChild(icon);
            break;
        default:
            break;
        }
    }
}

void FleetDataPanel::UpdateAggressionToggle() {
    if (!m_aggression_toggle)
        return;
    int tooltip_delay = GetOptionsDB().Get<int>("ui.tooltip.delay");
    m_aggression_toggle->SetBrowseModeTime(tooltip_delay);

    NewFleetAggression aggression = FLEET_AGGRESSIVE;

    if (m_is_new_fleet_drop_target) {
        aggression = m_new_fleet_aggression;
    } else if (auto fleet = GetFleet(m_fleet_id)) {
        aggression = fleet->Aggressive() ? FLEET_AGGRESSIVE : FLEET_PASSIVE;
    } else {
        DetachChild(m_aggression_toggle);
        return;
    }

    if (aggression == FLEET_AGGRESSIVE) {
        m_aggression_toggle->SetUnpressedGraphic(GG::SubTexture(FleetAggressiveIcon()));
        m_aggression_toggle->SetPressedGraphic  (GG::SubTexture(FleetPassiveIcon()));
        m_aggression_toggle->SetRolloverGraphic (GG::SubTexture(FleetAggressiveMouseoverIcon()));
        m_aggression_toggle->SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
            FleetAggressiveIcon(), UserString("FW_AGGRESSIVE"), UserString("FW_AGGRESSIVE_DESC")));
    } else if (aggression == FLEET_PASSIVE) {
        m_aggression_toggle->SetUnpressedGraphic(GG::SubTexture(FleetPassiveIcon()));
        if (m_is_new_fleet_drop_target)
            m_aggression_toggle->SetPressedGraphic  (GG::SubTexture(FleetAutoIcon()));
        else
            m_aggression_toggle->SetPressedGraphic  (GG::SubTexture(FleetAggressiveIcon()));
        m_aggression_toggle->SetRolloverGraphic (GG::SubTexture(FleetPassiveMouseoverIcon()));
        m_aggression_toggle->SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
            FleetPassiveIcon(), UserString("FW_PASSIVE"), UserString("FW_PASSIVE_DESC")));
    } else {    // aggression == INVALID_FLEET_AGGRESSION
        m_aggression_toggle->SetUnpressedGraphic(GG::SubTexture(FleetAutoIcon()));
        m_aggression_toggle->SetPressedGraphic  (GG::SubTexture(FleetAggressiveIcon()));
        m_aggression_toggle->SetRolloverGraphic (GG::SubTexture(FleetAutoMouseoverIcon()));
        m_aggression_toggle->SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
            FleetAutoIcon(), UserString("FW_AUTO"), UserString("FW_AUTO_DESC")));
    }
}

void FleetDataPanel::DoLayout() {
    if (m_fleet_icon) {
        // fleet icon will scale and position itself in the provided space
        m_fleet_icon->Resize(GG::Pt(DataPanelIconSpace().x, ClientHeight()));
    }
    if (m_scanline_control)
        m_scanline_control->Resize(GG::Pt(DataPanelIconSpace().x, ClientHeight()));
    for (auto& overlay : m_fleet_icon_overlays)
        overlay->Resize(GG::Pt(DataPanelIconSpace().x, ClientHeight()));

    // position fleet name and destination texts
    const GG::Pt name_ul = GG::Pt(DataPanelIconSpace().x + DATA_PANEL_TEXT_PAD, GG::Y0);
    const GG::Pt name_lr = GG::Pt(ClientWidth() - DATA_PANEL_TEXT_PAD - GG::X(Value(LabelHeight())),    LabelHeight());
    if (m_fleet_name_text)
        m_fleet_name_text->SizeMove(name_ul, name_lr);
    if (m_fleet_destination_text)
        m_fleet_destination_text->SizeMove(name_ul, name_lr);

    if (ClientWidth() < 250)
        DetachChild(m_fleet_name_text);
    else
        AttachChild(m_fleet_name_text);

    // position stat icons, centering them vertically if there's more space than required
    GG::Pt icon_ul = GG::Pt(name_ul.x, LabelHeight() + std::max(GG::Y0, (ClientHeight() - LabelHeight() - StatIconSize().y) / 2));
    for (auto& entry : m_stat_icons) {
        if (entry.second->Parent().get() != this)
            continue;
        entry.second->SizeMove(icon_ul, icon_ul + StatIconSize());
        icon_ul.x += StatIconSize().x;
    }

    // position aggression toggle / indicator
    if (m_aggression_toggle) {
        GG::Pt toggle_size(GG::X(Value(LabelHeight())), LabelHeight());
        GG::Pt toggle_ul = GG::Pt(ClientWidth() - toggle_size.x, GG::Y0);
        m_aggression_toggle->SizeMove(toggle_ul, toggle_ul + toggle_size);
    }
}

void FleetDataPanel::Init() {
    m_initialized = true;

    m_fleet_name_text = GG::Wnd::Create<CUILabel>("", GG::FORMAT_LEFT);
    AttachChild(m_fleet_name_text);
    m_fleet_destination_text = GG::Wnd::Create<CUILabel>("", GG::FORMAT_RIGHT);
    AttachChild(m_fleet_destination_text);

    if (m_fleet_id == INVALID_OBJECT_ID) {
        m_aggression_toggle = Wnd::Create<CUIButton>(
            GG::SubTexture(FleetAggressiveIcon()),
            GG::SubTexture(FleetPassiveIcon()),
            GG::SubTexture(FleetAggressiveMouseoverIcon()));
        AttachChild(m_aggression_toggle);
        m_aggression_toggle->LeftClickedSignal.connect(
            boost::bind(&FleetDataPanel::ToggleAggression, this));

    } else if (auto fleet = GetFleet(m_fleet_id)) {
        int tooltip_delay = GetOptionsDB().Get<int>("ui.tooltip.delay");

        std::vector<std::tuple<MeterType, std::shared_ptr<GG::Texture>, std::string>> meters_icons_browsetext;
        meters_icons_browsetext.emplace_back(METER_SIZE, FleetCountIcon(), "FW_FLEET_COUNT_SUMMARY");
        meters_icons_browsetext.emplace_back(METER_CAPACITY, DamageIcon(), "FW_FLEET_DAMAGE_SUMMARY");
        meters_icons_browsetext.emplace_back(METER_SECONDARY_STAT, FightersIcon(), "FW_FLEET_FIGHTER_SUMMARY");
        meters_icons_browsetext.emplace_back(METER_TROOPS, TroopIcon(), "FW_FLEET_TROOP_SUMMARY");
        meters_icons_browsetext.emplace_back(METER_POPULATION, ColonyIcon(), "FW_FLEET_COLONY_SUMMARY");
        meters_icons_browsetext.emplace_back(METER_INDUSTRY, IndustryIcon(), "FW_FLEET_INDUSTRY_SUMMARY");
        meters_icons_browsetext.emplace_back(METER_RESEARCH, ResearchIcon(), "FW_FLEET_RESEARCH_SUMMARY");
        meters_icons_browsetext.emplace_back(METER_TRADE, TradeIcon(), "FW_FLEET_TRADE_SUMMARY");
        meters_icons_browsetext.emplace_back(METER_STRUCTURE, ClientUI::MeterIcon(METER_STRUCTURE), "FW_FLEET_STRUCTURE_SUMMARY");
        meters_icons_browsetext.emplace_back(METER_SHIELD, ClientUI::MeterIcon(METER_SHIELD), "FW_FLEET_SHIELD_SUMMARY");
        meters_icons_browsetext.emplace_back(METER_FUEL, ClientUI::MeterIcon(METER_FUEL), "FW_FLEET_FUEL_SUMMARY");
        meters_icons_browsetext.emplace_back(METER_SPEED, ClientUI::MeterIcon(METER_SPEED), "FW_FLEET_SPEED_SUMMARY");

        for (const std::tuple<MeterType, std::shared_ptr<GG::Texture>, std::string>& entry : meters_icons_browsetext) {
            auto icon = GG::Wnd::Create<StatisticIcon>(std::get<1>(entry), 0, 0, false, StatIconSize().x, StatIconSize().y);
            m_stat_icons.push_back({std::get<0>(entry), icon});
            icon->SetBrowseModeTime(tooltip_delay);
            icon->SetBrowseText(UserString(std::get<2>(entry)));
            icon->InstallEventFilter(shared_from_this());
            AttachChild(icon);
            icon->SetBrowseModeTime(tooltip_delay);
        }

        int client_empire_id = HumanClientApp::GetApp()->EmpireID();
        if (fleet->OwnedBy(client_empire_id) || fleet->GetVisibility(client_empire_id) >= VIS_FULL_VISIBILITY) {
            m_aggression_toggle = Wnd::Create<CUIButton>(
                GG::SubTexture(FleetAggressiveIcon()),
                GG::SubTexture(FleetPassiveIcon()),
                GG::SubTexture(FleetAggressiveMouseoverIcon()));
            AttachChild(m_aggression_toggle);
            m_aggression_toggle->LeftClickedSignal.connect(
                boost::bind(&FleetDataPanel::ToggleAggression, this));
        }

        ColorTextForSelect();
    }
}

void FleetDataPanel::ColorTextForSelect() {
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
            m_panel(nullptr)
        {
            SetName("FleetRow");
            SetChildClippingMode(ClipToClient);
        }

        void CompleteConstruction() override {
            GG::ListBox::Row::CompleteConstruction();

            m_panel = GG::Wnd::Create<FleetDataPanel>(Width(), Height(), m_fleet_id);
            push_back(m_panel);
        }

        void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
            const GG::Pt old_size = Size();
            GG::ListBox::Row::SizeMove(ul, lr);
            //std::cout << "FleetRow::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
            if (!empty() && old_size != Size() && m_panel)
                m_panel->Resize(Size());
        }

        int             FleetID() const {return m_fleet_id;}
    private:
        int             m_fleet_id;
        std::shared_ptr<FleetDataPanel> m_panel;
    };
}

////////////////////////////////////////////////
// FleetsListBox
////////////////////////////////////////////////
/** A CUIListBox subclass used to list all the fleets, and handle drag-and-drop
  * operations on them, in FleetWnd. */
class FleetsListBox : public CUIListBox {
public:
    FleetsListBox(bool order_issuing_enabled) :
        CUIListBox(),
        m_highlighted_row_it(end()),
        m_order_issuing_enabled(order_issuing_enabled)
    { InitRowSizes(); }

    void            EnableOrderIssuing(bool enable) {
        m_order_issuing_enabled = enable;
    }

    void AcceptDrops(const GG::Pt& pt, std::vector<std::shared_ptr<GG::Wnd>> wnds, GG::Flags<GG::ModKey> mod_keys) override {
        //DebugLogger() << "FleetsListBox::AcceptDrops";
        if (wnds.empty()) {
            ErrorLogger() << "FleetsListBox::AcceptDrops dropped wnds empty";
            return;
        }
        if (!m_order_issuing_enabled) {
            //DebugLogger() << "... order issuing disabled, aborting";
            return;
        }

        iterator drop_target_row = RowUnderPt(pt);
        //DebugLogger() << "... drop pt: " << pt;
        if (drop_target_row == end()) {
            //DebugLogger() << "... drop row is end, aborting";
            return;
        }
        //DebugLogger() << "... drop row is in position: " << std::distance(begin(), drop_target_row);

        // get drop target fleet
        const FleetRow* drop_target_fleet_row = boost::polymorphic_downcast<FleetRow*>(drop_target_row->get());
        if (!drop_target_fleet_row) {
            ErrorLogger() << "FleetsListBox::AcceptDrops  drop target not a fleet row. aborting";
            return;
        }

        int target_fleet_id = drop_target_fleet_row->FleetID();
        auto target_fleet = GetFleet(target_fleet_id);
        if (!target_fleet) {
            ErrorLogger() << "FleetsListBox::AcceptDrops  unable to get target fleet with id: " << target_fleet_id;
            return;
        }


        // sort dropped Wnds to extract fleets or ships dropped.  (should only be one or the other in a given drop)
        std::vector<std::shared_ptr<Fleet>> dropped_fleets;
        std::vector<std::shared_ptr<Ship>> dropped_ships;

        //DebugLogger() << "... getting/sorting dropped fleets or ships...";
        for (const auto& wnd : wnds) {
            if (drop_target_fleet_row == wnd.get()) {
                ErrorLogger() << "FleetsListBox::AcceptDrops  dropped wnd is same as drop target?! skipping";
                continue;
            }

            if (wnd->DragDropDataType() == FLEET_DROP_TYPE_STRING) {
                const FleetRow* fleet_row = boost::polymorphic_downcast<const FleetRow*>(wnd.get());
                if (!fleet_row) {
                    ErrorLogger() << "FleetsListBox::AcceptDrops  unable to get fleet row from dropped wnd";
                    continue;
                }
                dropped_fleets.push_back(GetFleet(fleet_row->FleetID()));

            } else if (wnd->DragDropDataType() == SHIP_DROP_TYPE_STRING) {
                const ShipRow* ship_row = boost::polymorphic_downcast<const ShipRow*>(wnd.get());
                if (!ship_row) {
                    ErrorLogger() << "FleetsListBox::AcceptDrops  unable to get ship row from dropped wnd";
                    continue;
                }
                dropped_ships.push_back(GetShip(ship_row->ShipID()));
            }
        }

        if (dropped_ships.empty() && dropped_fleets.empty()) {
            ErrorLogger() << "FleetsListBox::AcceptDrops  no ships or fleets dropped... aborting";
            return;
        }

        if (dropped_ships.empty() == dropped_fleets.empty()) {
            ErrorLogger() << "FleetsListBox::AcceptDrops  dropped a mix of fleets and ships... aborting";
            return;
        }
        int empire_id = HumanClientApp::GetApp()->EmpireID();

        if (ClientPlayerIsModerator())
            return; // todo: handle moderator actions for this...

        // Collected ship ids for the transfer.
        std::vector<int> ship_ids;
        ship_ids.reserve(dropped_ships.size() + dropped_fleets.size());

        // Need ship ids in a vector for fleet transfer order
        for (const auto& dropped_fleet : dropped_fleets) {
            if (!dropped_fleet) {
                ErrorLogger() << "FleetsListBox::AcceptDrops  unable to get dropped fleet?";
                continue;
            }

            // get fleet's ships in a vector, as this is what FleetTransferOrder takes
            const auto& ship_ids_set = dropped_fleet->ShipIDs();
            std::copy(ship_ids_set.begin(), ship_ids_set.end(), std::back_inserter(ship_ids));
        }

        for (const auto& ship : dropped_ships) {
            if (!ship) {
                ErrorLogger() << "FleetsListBox::AcceptDrops  couldn't get dropped ship?";
                continue;
            }
            ship_ids.push_back(ship->ID());
        }

        // order the transfer
        if (!ship_ids.empty())
            HumanClientApp::GetApp()->Orders().IssueOrder(
                std::make_shared<FleetTransferOrder>(empire_id, target_fleet_id, ship_ids));
    }

    void DragDropHere(const GG::Pt& pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
                      GG::Flags<GG::ModKey> mod_keys) override
    {
        //std::cout << "FleetsListBox::DragDropHere" << std::endl << std::flush;
        CUIListBox::DragDropHere(pt, drop_wnds_acceptable, mod_keys);

        // default to removing highlighting of any row that has it.
        // used to check: if (m_highlighted_row_it != row_it) before doing this...
        //ClearHighlighting();

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
        auto& drop_target_row = *row_it;
        assert(drop_target_row);
        assert(!drop_target_row->empty());

        GG::Control* control = !drop_target_row->empty() ? drop_target_row->at(0) : nullptr;
        assert(control);

        FleetDataPanel* drop_target_data_panel = boost::polymorphic_downcast<FleetDataPanel*>(control);
        assert(drop_target_data_panel);

        if (drop_target_data_panel->Selected())
            return;

        FleetRow* drop_target_fleet_row = boost::polymorphic_downcast<FleetRow*>(drop_target_row.get());
        assert(drop_target_fleet_row);

        auto drop_target_fleet = GetFleet(drop_target_fleet_row->FleetID());
        assert(drop_target_fleet);


        // get whether each Wnd is dropable
        DropsAcceptable(drop_wnds_acceptable.begin(), drop_wnds_acceptable.end(), pt, mod_keys);


        // scan through results in drops_acceptable_map and decide whether overall
        // drop is acceptable.  to be acceptable, all wnds must individually be
        // acceptable for dropping, and there must not be a mix of ships and fleets
        // being dropped.
        bool fleets_seen = false;
        bool ships_seen = false;

        for (auto& drop_wnd_acceptable : drop_wnds_acceptable) {
            if (!drop_wnd_acceptable.second)
                return; // a row was an invalid drop. abort without highlighting drop target.

            const auto dropped_wnd = drop_wnd_acceptable.first;
            if (dropped_wnd->DragDropDataType() == FLEET_DROP_TYPE_STRING) {
                fleets_seen = true;
                if (ships_seen)
                    return; // can't drop both at once

                const FleetRow* fleet_row = boost::polymorphic_downcast<const FleetRow*>(dropped_wnd);
                assert(fleet_row);
                auto fleet = GetFleet(fleet_row->FleetID());

                if (!ValidFleetMerge(fleet, drop_target_fleet))
                    return; // not a valid drop

            } else if (dropped_wnd->DragDropDataType() == SHIP_DROP_TYPE_STRING) {
                ships_seen = true;
                if (fleets_seen)
                    return; // can't drop both at once

                const ShipRow* ship_row = boost::polymorphic_downcast<const ShipRow*>(dropped_wnd);
                assert(ship_row);
                std::shared_ptr<Ship> ship = GetShip(ship_row->ShipID());

                if (!ValidShipTransfer(ship, drop_target_fleet))
                    return; // not a valid drop
            }
        }

        // passed all checks.  drop is valid!
        HighlightRow(row_it);
    }

    void DragDropLeave() override {
        //std::cout << "FleetsListBox::DragDropLeave" << std::endl << std::flush;
        CUIListBox::DragDropLeave();
        ClearHighlighting();
        //std::cout << "FleetsListBox::DragDropLeave done" << std::endl << std::flush;
    }

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
        const GG::Pt old_size = Size();
        CUIListBox::SizeMove(ul, lr);
        //std::cout << "FleetListBox::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
        if (old_size != Size()) {
            const GG::Pt row_size = ListRowSize();
            //std::cout << "FleetListBox::SizeMove list row size: (" << Value(row_size.x) << ", " << Value(row_size.y) << ")" << std::endl;
            for (auto& row : *this)
                row->Resize(row_size);
        }
    }

    GG::Pt          ListRowSize() const
    { return GG::Pt(Width() - RightMargin() - 5, ListRowHeight()); }

protected:
    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const override
    {
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
        const FleetRow* target_fleet_row = boost::polymorphic_downcast<const FleetRow*>(row->get());
        std::shared_ptr<const Fleet> target_fleet;
        if (target_fleet_row)
            target_fleet = GetFleet(target_fleet_row->FleetID());

        // loop through dropped Wnds, checking if each is a valid ship or fleet.  this doesn't
        // consider whether there is a mixture of fleets and ships, as each row is considered
        // independently.  actual drops will probably only accept one or the other, not a mixture
        // of fleets and ships being dropped simultaneously.
        for (DropsAcceptableIter it = first; it != last; ++it) {
            if (it->first == target_fleet_row)
                continue;   // can't drop onto self

            // for either of fleet or ship being dropped, check if merge or transfer is valid.
            // if any of the nested if's fail, the default rejection of the drop will remain set
            if (it->first->DragDropDataType() == FLEET_DROP_TYPE_STRING) {
                if (const FleetRow* fleet_row = boost::polymorphic_downcast<const FleetRow*>(it->first))
                    if (auto fleet = GetFleet(fleet_row->FleetID()))
                        it->second = ValidFleetMerge(fleet, target_fleet);

            } else if (it->first->DragDropDataType() == SHIP_DROP_TYPE_STRING) {
                if (const ShipRow* ship_row = boost::polymorphic_downcast<const ShipRow*>(it->first))
                    if (auto ship = GetShip(ship_row->ShipID()))
                        it->second = ValidShipTransfer(ship, target_fleet);
            } else {
                // no valid drop type string
                ErrorLogger() << "FleetsListBox unrecognized drop type: " << it->first->DragDropDataType();
            }
        }
    }

private:
    void            HighlightRow(iterator row_it) {
        if (row_it == end())
            return;

        if (row_it == m_highlighted_row_it)
            return;

        // get FleetDataPanel of row pointed to by row_it
        auto& selected_row = *row_it;
        assert(selected_row);
        assert(!selected_row->empty());
        GG::Control* control = !selected_row->empty() ? selected_row->at(0) : nullptr;
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

        // check that m_highlighted_row_it points to a valid row.
        // have been getting intermittant crashes when dragging rows after
        // drops onto other rows that occur after attempting to use an
        // invalid iterator
        bool valid_highlight_row = false;
        for (iterator test_it = this->begin(); test_it != this->end(); ++test_it) {
            if (test_it == m_highlighted_row_it) {
                valid_highlight_row = true;
                break;
            }
        }
        if (!valid_highlight_row) {
            ErrorLogger() << "FleetsListBox::ClearHighlighting : m_highlighted_row not valid! aborting";
            m_highlighted_row_it = end();
            return;
        }

        // Do not un-highlight a row from the selection set since rows in the selection set and the
        // drop target use the same graphical highlight effect.
        if (Selected(m_highlighted_row_it)) {
            m_highlighted_row_it = end();
            return;
        }

        auto& selected_row = *m_highlighted_row_it;
        if (!selected_row) {
            ErrorLogger() << "FleetsListBox::ClearHighlighting : no selected row!";
            return;
        }
        if (selected_row->empty()) {
            ErrorLogger() << "FleetsListBox::ClearHighlighting : selected row empty!";
            return;
        }

        GG::Control* control = !selected_row->empty() ? selected_row->at(0) : nullptr;
        if (!control) {
            ErrorLogger() << "FleetsListBox::ClearHighlighting : null control in selected row!";
            return;
        }

        FleetDataPanel* data_panel = boost::polymorphic_downcast<FleetDataPanel*>(control);
        if (!data_panel) {
            ErrorLogger() << "FleetsListBox::ClearHighlighting : no data panel!";
            return;
        }

        data_panel->Select(false);
        m_highlighted_row_it = end();
    }

    void            InitRowSizes() {
        SetNumCols(1);
        ManuallyManageColProps();
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
    ShipsListBox(int fleet_id, bool order_issuing_enabled) :
        CUIListBox(),
        m_fleet_id(fleet_id),
        m_order_issuing_enabled(order_issuing_enabled)
    {}

    void CompleteConstruction() override {
        CUIListBox::CompleteConstruction();
        Refresh();
    }

    void            Refresh() {
        ScopedTimer timer("ShipsListBox::Refresh");

        auto fleet = GetFleet(m_fleet_id);
        if (!fleet) {
            Clear();
            return;
        }

        const GG::Pt row_size = ListRowSize();
        Clear();

        // repopulate list with ships in current fleet

        SetNumCols(1);
        ManuallyManageColProps();

        int this_client_empire_id = HumanClientApp::GetApp()->EmpireID();
        const std::set<int>& this_client_known_destroyed_objects =
            GetUniverse().EmpireKnownDestroyedObjectIDs(this_client_empire_id);
        const std::set<int>& this_client_stale_object_info =
            GetUniverse().EmpireStaleKnowledgeObjectIDs(this_client_empire_id);

        const std::set<int>& ship_ids = fleet->ShipIDs();
        std::vector<std::shared_ptr<GG::ListBox::Row>> rows;
        rows.reserve(ship_ids.size());
        for (int ship_id : ship_ids) {
            // skip known destroyed and stale info objects
            if (this_client_known_destroyed_objects.count(ship_id))
                continue;
            if (this_client_stale_object_info.count(ship_id))
                continue;

            auto row = GG::Wnd::Create<ShipRow>(GG::X1, row_size.y, ship_id);
            rows.push_back(row);
        }
        Insert(rows);
        for (auto& row : rows)
        { row->Resize(row_size); }

        SelRowsChangedSignal(this->Selections());
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

    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const override
    {
        for (DropsAcceptableIter it = first; it != last; ++it) {
            it->second = false; // default

            if (!m_order_issuing_enabled)
                continue;

            const auto& ship_row = dynamic_cast<const ShipRow*>(it->first);
            if (!ship_row)
                continue;

            auto ship = GetShip(ship_row->ShipID());
            if (!ship) {
                ErrorLogger() << "ShipsListBox::DropsAcceptable couldn't get ship for ship row";
                continue;
            }

            auto fleet = GetFleet(ship->FleetID());
            if (!fleet) {
                ErrorLogger() << "ShipsListBox::DropsAcceptable couldn't get fleet with id " << ship->FleetID();
                continue;
            }

            if (ship && ValidShipTransfer(ship, fleet))
                continue;   // leave false: ship transfer not valid

            // all tests passed; can drop
            it->second = true;
        }
    }

    void AcceptDrops(const GG::Pt& pt, std::vector<std::shared_ptr<GG::Wnd>> wnds, GG::Flags<GG::ModKey> mod_keys) override {
        if (wnds.empty())
            return;

        std::shared_ptr<Ship> ship_from_dropped_wnd;
        std::vector<int> ship_ids;
        for (const auto& wnd : wnds) {
            if (wnd->DragDropDataType() == SHIP_DROP_TYPE_STRING) {
                const ShipRow* ship_row = boost::polymorphic_downcast<const ShipRow*>(wnd.get());
                assert(ship_row);
                ship_ids.push_back(ship_row->ShipID());
                ship_from_dropped_wnd = GetShip(ship_row->ShipID());
            }
        }

        if (!ship_from_dropped_wnd)
            return;

        int empire_id = HumanClientApp::GetApp()->EmpireID();

        if (ClientPlayerIsModerator())
            return; // todo: handle moderator actions for this...

        HumanClientApp::GetApp()->Orders().IssueOrder(
            std::make_shared<FleetTransferOrder>(empire_id, m_fleet_id, ship_ids));
    }

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
        const GG::Pt old_size = Size();
        CUIListBox::SizeMove(ul, lr);
        //std::cout << "ShipsListBox::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
        if (old_size != Size()) {
            const GG::Pt row_size = ListRowSize();
            //std::cout << "ShipsListBox::SizeMove list row size: (" << Value(row_size.x) << ", " << Value(row_size.y) << ")" << std::endl;
            for (auto& row : *this)
                row->Resize(row_size);
        }
    }

    GG::Pt          ListRowSize() const
    { return GG::Pt(Width() - 5 - RightMargin(), ListRowHeight()); }

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
    FleetDetailPanel(GG::X w, GG::Y h, int fleet_id, bool order_issuing_enabled, GG::Flags<GG::WndFlag> flags = GG::NO_WND_FLAGS);

    void CompleteConstruction() override;
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    int             FleetID() const;
    std::set<int>   SelectedShipIDs() const;    ///< returns ids of ships selected in the detail panel's ShipsListBox

    void            SetFleet(int fleet_id);                         ///< sets the currently-displayed Fleet.  setting to INVALID_OBJECT_ID shows no fleet
    void            SelectShips(const std::set<int>& ship_ids);///< sets the currently-selected ships in the ships list

    void            Refresh();

    void            EnableOrderIssuing(bool enabled = true);

    /** emitted when the set of selected ships changes */
    mutable boost::signals2::signal<void (const ShipsListBox::SelectionSet&)> SelectedShipsChangedSignal;
    mutable boost::signals2::signal<void (int)>                               ShipRightClickedSignal;

private:
    int             GetShipIDOfListRow(GG::ListBox::iterator it) const; ///< returns the ID number of the ship in row \a row_idx of the ships listbox
    void            DoLayout();
    void            UniverseObjectDeleted(std::shared_ptr<const UniverseObject> obj);
    void            ShipSelectionChanged(const GG::ListBox::SelectionSet& rows);
    void            ShipRightClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys);
    int             ShipInRow(GG::ListBox::iterator it) const;

    int                         m_fleet_id;
    bool                        m_order_issuing_enabled;
    boost::signals2::connection m_fleet_connection;

    std::shared_ptr<ShipsListBox>               m_ships_lb;
};

FleetDetailPanel::FleetDetailPanel(GG::X w, GG::Y h, int fleet_id, bool order_issuing_enabled, GG::Flags<GG::WndFlag> flags/* = GG::NO_WND_FLAGS*/) :
    GG::Wnd(GG::X0, GG::Y0, w, h, flags),
    m_fleet_id(INVALID_OBJECT_ID),
    m_order_issuing_enabled(order_issuing_enabled),
    m_ships_lb(nullptr)
{
    SetName("FleetDetailPanel");
    SetChildClippingMode(ClipToClient);

    m_ships_lb = GG::Wnd::Create<ShipsListBox>(m_fleet_id, order_issuing_enabled);
    m_ships_lb->SetHiliteColor(GG::CLR_ZERO);

    SetFleet(fleet_id);

    if (!m_order_issuing_enabled) {
        m_ships_lb->SetStyle(GG::LIST_NOSEL | GG::LIST_BROWSEUPDATES);
    } else {
        m_ships_lb->SetStyle(GG::LIST_QUICKSEL | GG::LIST_BROWSEUPDATES);
        m_ships_lb->AllowDropType(SHIP_DROP_TYPE_STRING);
    }

    m_ships_lb->SelRowsChangedSignal.connect(
        boost::bind(&FleetDetailPanel::ShipSelectionChanged, this, _1));
    m_ships_lb->RightClickedRowSignal.connect(
        boost::bind(&FleetDetailPanel::ShipRightClicked, this, _1, _2, _3));
    GetUniverse().UniverseObjectDeleteSignal.connect(
        boost::bind(&FleetDetailPanel::UniverseObjectDeleted, this, _1));
}

void FleetDetailPanel::CompleteConstruction() {
    GG::Wnd::CompleteConstruction();

    AttachChild(m_ships_lb);
    DoLayout();
}

int FleetDetailPanel::GetShipIDOfListRow(GG::ListBox::iterator it) const
{ return boost::polymorphic_downcast<ShipRow*>(it->get())->ShipID(); }

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
        auto fleet = GetFleet(m_fleet_id);
        if (fleet && !fleet->Empty()) {
            m_fleet_connection = fleet->StateChangedSignal.connect(
                boost::bind(&FleetDetailPanel::Refresh, this), boost::signals2::at_front);
        } else {
            ErrorLogger() << "FleetDetailPanel::SetFleet ignoring set to missing or empty fleet id (" << fleet_id << ")";
        }
    }
}

void FleetDetailPanel::SelectShips(const std::set<int>& ship_ids) {
    const GG::ListBox::SelectionSet initial_selections = m_ships_lb->Selections();

    m_ships_lb->DeselectAll();

    // loop through ships, selecting any indicated
    for (auto it = m_ships_lb->begin(); it != m_ships_lb->end(); ++it) {
        ShipRow* row = dynamic_cast<ShipRow*>(it->get());
        if (!row) {
            ErrorLogger() << "FleetDetailPanel::SelectShips couldn't cast a list row to ShipRow?";
            continue;
        }

        // if this row's ship should be selected, so so
        if (ship_ids.count(row->ShipID())) {
            m_ships_lb->SelectRow(it);
            m_ships_lb->BringRowIntoView(it);   // may cause earlier rows brought into view to be brought out of view... oh well
        }
    }

    if (initial_selections != m_ships_lb->Selections())
        ShipSelectionChanged(m_ships_lb->Selections());
}

int FleetDetailPanel::FleetID() const
{ return m_fleet_id; }

std::set<int> FleetDetailPanel::SelectedShipIDs() const {
    //std::cout << "FleetDetailPanel::SelectedShipIDs()" << std::endl;
    std::set<int> retval;

    for (const auto& selection : m_ships_lb->Selections()) {
        bool hasRow = false;
        for (auto& lb_row : *m_ships_lb) {
            if (lb_row == *selection) {
                hasRow=true;
                break;
            }
        }
        if (!hasRow) {
            ErrorLogger() << "FleetDetailPanel::SelectedShipIDs tried to get invalid ship row selection;";
            continue;
        }
        auto& row = *selection;
        ShipRow* ship_row = dynamic_cast<ShipRow*>(row.get());
        if (ship_row && (ship_row->ShipID() != INVALID_OBJECT_ID))
            retval.insert(ship_row->ShipID());
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

void FleetDetailPanel::UniverseObjectDeleted(std::shared_ptr<const UniverseObject> obj) {
    if (obj && obj->ID() == m_fleet_id)
        SetFleet(INVALID_OBJECT_ID);
}

void FleetDetailPanel::ShipSelectionChanged(const GG::ListBox::SelectionSet& rows) {
    for (auto it = m_ships_lb->begin(); it != m_ships_lb->end(); ++it) {
        try {
            ShipDataPanel* ship_panel = boost::polymorphic_downcast<ShipDataPanel*>(!(**it).empty() ? (**it).at(0) : nullptr);
            ship_panel->Select(rows.count(it));
        } catch (const std::exception& e) {
            ErrorLogger() << "FleetDetailPanel::ShipSelectionChanged caught exception: " << e.what();
            continue;
        }
    }

    SelectedShipsChangedSignal(rows);
}

void FleetDetailPanel::ShipRightClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) {
    // get ship that was clicked, aborting if problems arise doing so
    ShipRow* ship_row = dynamic_cast<ShipRow*>(it->get());
    if (!ship_row)
        return;

    std::shared_ptr<Ship> ship = GetShip(ship_row->ShipID());
    if (!ship)
        return;
    auto fleet = GetFleet(m_fleet_id);

    const auto& map_wnd = ClientUI::GetClientUI()->GetMapWnd();
    if (ClientPlayerIsModerator() && map_wnd->GetModeratorActionSetting() != MAS_NoAction) {
        ShipRightClickedSignal(ship->ID());  // response handled in MapWnd
        return;
    }

    const ShipDesign* design = GetShipDesign(ship->DesignID()); // may be null
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();

    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);

    // Zoom to design
    if (design) {
        std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % design->Name(true));
        auto zoom_to_design_action = [design]() { ClientUI::GetClientUI()->ZoomToShipDesign(design->ID()); };
        popup->AddMenuItem(GG::MenuItem(popup_label, false, false, zoom_to_design_action));
    }

    // Rename ship context item
    if (ship->OwnedBy(client_empire_id) || ClientPlayerIsModerator()) {
        auto rename_action = [ship, client_empire_id]() {
            auto edit_wnd = GG::Wnd::Create<CUIEditWnd>(GG::X(350), UserString("ENTER_NEW_NAME"), ship->Name());
            edit_wnd->Run();

            if (ClientPlayerIsModerator())
                // TODO: Moderator action for renaming ships
                return;

            if (!RenameOrder::Check(client_empire_id, ship->ID(), edit_wnd->Result()))
                return;

            HumanClientApp::GetApp()->Orders().IssueOrder(
                std::make_shared<RenameOrder>(client_empire_id, ship->ID(), edit_wnd->Result()));
        };
        popup->AddMenuItem(GG::MenuItem(UserString("RENAME"), false, false, rename_action));
    }

    // Scrap or unscrap ships
    if (!ClientPlayerIsModerator()
        && !ship->OrderedScrapped()
        && ship->OwnedBy(client_empire_id))
    {
        // create popup menu with "Scrap" option
        auto scrap_action = [ship, client_empire_id]() {
            HumanClientApp::GetApp()->Orders().IssueOrder(
                std::make_shared<ScrapOrder>(client_empire_id, ship->ID()));
        };
        popup->AddMenuItem(GG::MenuItem(UserString("ORDER_SHIP_SCRAP"), false, false, scrap_action));
    } else if (!ClientPlayerIsModerator()
               && ship->OwnedBy(client_empire_id))
    {
        auto unscrap_action = [ship]() {
            // find order to scrap this ship, and recind it
            auto pending_scrap_orders = PendingScrapOrders();
            auto pending_order_it = pending_scrap_orders.find(ship->ID());
            if (pending_order_it != pending_scrap_orders.end())
                HumanClientApp::GetApp()->Orders().RescindOrder(pending_order_it->second);
        };
        // create popup menu with "Cancel Scrap" option
        popup->AddMenuItem(GG::MenuItem(UserString("ORDER_CANCEL_SHIP_SCRAP"), false, false, unscrap_action));
    }

    // Split fleets
    if (ship->OwnedBy(client_empire_id)
        && !ClientPlayerIsModerator()
        && fleet)
    {
        auto split_one_design_action = [this, design, fleet]() {
            // split ships with same design as clicked ship into separate fleet
            const auto&& parent = this->Parent();
            if (!parent)
                return;
            const FleetWnd* parent_fleet_wnd = dynamic_cast<const FleetWnd*>(parent.get());
            if (!parent_fleet_wnd)
                return;
            if (!design)
                return;
            CreateNewFleetFromShipsWithDesign(fleet->ShipIDs(), design->ID(),
                                              parent_fleet_wnd->GetNewFleetAggression());
        };

        auto split_all_designs_action = [this, fleet]() {
            // split all ships into new fleets by ship design
            const auto&& parent = this->Parent();
            if (!parent)
                return;
            const FleetWnd* parent_fleet_wnd = dynamic_cast<const FleetWnd*>(parent.get());
            if (!parent_fleet_wnd)
                return;
            CreateNewFleetsFromShipsForEachDesign(fleet->ShipIDs(),
                                                  parent_fleet_wnd->GetNewFleetAggression());
        };

        if (design)
            popup->AddMenuItem( GG::MenuItem(UserString("FW_SPLIT_SHIPS_THIS_DESIGN"), false, false, split_one_design_action));
        popup->AddMenuItem(     GG::MenuItem(UserString("FW_SPLIT_SHIPS_ALL_DESIGNS"), false, false, split_all_designs_action));
    }

    // Allow dismissal of stale visibility information
    if (!ship->OwnedBy(client_empire_id) && fleet) {
        auto forget_ship_action = [ship]() {
            ClientUI::GetClientUI()->GetMapWnd()->ForgetObject(ship->ID());
        };

        auto visibility_turn_map =
            GetUniverse().GetObjectVisibilityTurnMapByEmpire(ship->ID(), client_empire_id);

        auto last_turn_visible_it = visibility_turn_map.find(VIS_BASIC_VISIBILITY);
        if (last_turn_visible_it != visibility_turn_map.end()
            && last_turn_visible_it->second < CurrentTurn())
        {
            popup->AddMenuItem(GG::MenuItem(UserString("FW_ORDER_DISMISS_SENSOR_GHOST"), false, false, forget_ship_action));
        }
    }

    popup->Run();
}

int FleetDetailPanel::ShipInRow(GG::ListBox::iterator it) const {
    if (it == m_ships_lb->end())
        return INVALID_OBJECT_ID;

    if (ShipRow* ship_row = dynamic_cast<ShipRow*>(it->get()))
        return ship_row->ShipID();

    return INVALID_OBJECT_ID;
}

////////////////////////////////////////////////
// FleetWnd
////////////////////////////////////////////////
namespace {
    /** \p create or grow a bounding \p box from \p pt. */
    GG::Rect CreateOrGrowBox(bool create, const GG::Rect box, const GG::Pt pt) {
        if (create)
            return GG::Rect(pt, pt);
        else
            return GG::Rect(
                std::min(box.Left(),    pt.x),
                std::min(box.Top(),     pt.y),
                std::max(box.Right(),   pt.x),
                std::max(box.Bottom(),  pt.y));
    }

    /** Is \p ll smaller or equal to the size of \p rr? */
    bool SmallerOrEqual(GG::Rect ll, GG::Rect rr) {
        return (ll.Width() <= rr.Width() && ll.Height() <= rr.Height());
    }
}

FleetWnd::FleetWnd(const std::vector<int>& fleet_ids, bool order_issuing_enabled,
                   double allowed_bounding_box_leeway /*= 0*/,
                   int selected_fleet_id/* = INVALID_OBJECT_ID*/,
                   GG::Flags<GG::WndFlag> flags/* = INTERACTIVE | DRAGABLE | ONTOP | CLOSABLE | RESIZABLE*/,
                   const std::string& config_name) :
    MapWndPopup("", flags | GG::RESIZABLE, config_name),
    m_fleet_ids(),
    m_empire_id(ALL_EMPIRES),
    m_system_id(INVALID_OBJECT_ID),
    m_order_issuing_enabled(order_issuing_enabled),
    m_fleets_lb(nullptr),
    m_new_fleet_drop_target(nullptr),
    m_fleet_detail_panel(nullptr),
    m_stat_icons()
{
    if (!fleet_ids.empty()) {
        if (auto fleet = GetFleet(*fleet_ids.begin()))
            m_empire_id = fleet->Owner();
    }

    for (int fleet_id : fleet_ids)
        m_fleet_ids.insert(fleet_id);

    // verify that the selected fleet id is valid.
    if (selected_fleet_id != INVALID_OBJECT_ID &&
        !m_fleet_ids.count(selected_fleet_id))
    {
        ErrorLogger() << "FleetWnd::FleetWnd couldn't find requested selected fleet with id " << selected_fleet_id;
        selected_fleet_id = INVALID_OBJECT_ID;
    }

    // Determine the size of the bounding box containing the fleets, plus the leeway
    bool is_first_fleet = true;
    for (int fleet_id : m_fleet_ids) {
        auto fleet = GetFleet(fleet_id);
        if (!fleet)
            continue;

        auto fleet_loc = GG::Pt(GG::X(fleet->X()), GG::Y(fleet->Y()));
        // Grow the fleets bounding box
        m_bounding_box = CreateOrGrowBox(is_first_fleet, m_bounding_box, fleet_loc);
        is_first_fleet = false;
    }
    m_bounding_box = GG::Rect(m_bounding_box.UpperLeft(),
                              m_bounding_box.LowerRight()
                              + GG::Pt(GG::X(allowed_bounding_box_leeway),
                                       GG::Y(allowed_bounding_box_leeway)));

    m_fleet_detail_panel = GG::Wnd::Create<FleetDetailPanel>(GG::X1, GG::Y1, selected_fleet_id, m_order_issuing_enabled);
}

void FleetWnd::CompleteConstruction() {
    Sound::TempUISoundDisabler sound_disabler;

    // add fleet aggregate stat icons
    int tooltip_delay = GetOptionsDB().Get<int>("ui.tooltip.delay");

    for (auto entry : {
            std::make_tuple(METER_SIZE, FleetCountIcon(), UserStringNop("FW_FLEET_COUNT_SUMMARY")),
            std::make_tuple(METER_CAPACITY, DamageIcon(), UserStringNop("FW_FLEET_DAMAGE_SUMMARY")),
            std::make_tuple(METER_SECONDARY_STAT, FightersIcon(), UserStringNop("FW_FLEET_FIGHTER_SUMMARY")),
            std::make_tuple(METER_STRUCTURE, ClientUI::MeterIcon(METER_STRUCTURE), UserStringNop("FW_FLEET_STRUCTURE_SUMMARY")),
            std::make_tuple(METER_SHIELD, ClientUI::MeterIcon(METER_SHIELD), UserStringNop("FW_FLEET_SHIELD_SUMMARY")),
            std::make_tuple(METER_TROOPS, TroopIcon(), UserStringNop("FW_FLEET_TROOP_SUMMARY")),
            std::make_tuple(METER_POPULATION, ColonyIcon(), UserStringNop("FW_FLEET_COLONY_SUMMARY")),
        })
    {
        auto icon = GG::Wnd::Create<StatisticIcon>(std::get<1>(entry), 0, 0, false, StatIconSize().x, StatIconSize().y);
        m_stat_icons.push_back({std::get<0>(entry), icon});
        icon->SetBrowseModeTime(tooltip_delay);
        icon->SetBrowseText(UserString(std::get<2>(entry)));
        AttachChild(icon);
    }

    // create fleet list box
    m_fleets_lb = GG::Wnd::Create<FleetsListBox>(m_order_issuing_enabled);
    m_fleets_lb->SetHiliteColor(GG::CLR_ZERO);
    m_fleets_lb->SelRowsChangedSignal.connect(
        boost::bind(&FleetWnd::FleetSelectionChanged, this, _1));
    m_fleets_lb->LeftClickedRowSignal.connect(
        boost::bind(&FleetWnd::FleetLeftClicked, this, _1, _2, _3));
    m_fleets_lb->RightClickedRowSignal.connect(
        boost::bind(&FleetWnd::FleetRightClicked, this, _1, _2, _3));
    m_fleets_lb->DoubleClickedRowSignal.connect(
        boost::bind(&FleetWnd::FleetDoubleClicked, this, _1, _2, _3));
    AttachChild(m_fleets_lb);
    m_fleets_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_BROWSEUPDATES);
    m_fleets_lb->AllowDropType(SHIP_DROP_TYPE_STRING);
    m_fleets_lb->AllowDropType(FLEET_DROP_TYPE_STRING);

    // create fleet detail panel
    m_fleet_detail_panel->SelectedShipsChangedSignal.connect(
        boost::bind(&FleetWnd::ShipSelectionChanged, this, _1));
    m_fleet_detail_panel->ShipRightClickedSignal.connect(
        ShipRightClickedSignal);
    AttachChild(m_fleet_detail_panel);

    // determine fleets to show and populate list
    Refresh();

    // create drop target
    m_new_fleet_drop_target = GG::Wnd::Create<FleetDataPanel>(GG::X1, ListRowHeight(), m_system_id, true);
    AttachChild(m_new_fleet_drop_target);
    m_new_fleet_drop_target->NewFleetFromShipsSignal.connect(
        boost::bind(&FleetWnd::CreateNewFleetFromDrops, this, _1));

    GetUniverse().UniverseObjectDeleteSignal.connect(
        boost::bind(&FleetWnd::UniverseObjectDeleted, this, _1));

    RefreshStateChangedSignals();

    ResetDefaultPosition();
    MapWndPopup::CompleteConstruction();

    SetMinSize(GG::Pt(CUIWnd::MinimizedSize().x, TopBorder() + INNER_BORDER_ANGLE_OFFSET + BORDER_BOTTOM +
                                                 ListRowHeight() + 2*GG::Y(PAD)));
    DoLayout();
    SaveDefaultedOptions();
    SaveOptions();
}

FleetWnd::~FleetWnd() {
    // FleetWnd is registered as a top level window, the same as ClientUI and MapWnd.
    // Consequently, when the GUI shutsdown either could be destroyed before this Wnd
    if (auto client = ClientUI::GetClientUI())
        if (auto mapwnd = client->GetMapWnd())
            mapwnd->ClearProjectedFleetMovementLines();
    ClosingSignal(this);
}

void FleetWnd::PreRender() {
    MapWndPopup::PreRender();

    if (m_needs_refresh)
        Refresh();
}

GG::Rect FleetWnd::CalculatePosition() const {
    GG::Pt ul(GG::X(5), GG::GUI::GetGUI()->AppHeight() - FLEET_WND_HEIGHT - 5);
    GG::Pt wh(FLEET_WND_WIDTH, FLEET_WND_HEIGHT);
    return GG::Rect(ul, ul + wh);
}

void FleetWnd::SetStatIconValues() {
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    const std::set<int>& this_client_known_destroyed_objects = GetUniverse().EmpireKnownDestroyedObjectIDs(client_empire_id);
    const std::set<int>& this_client_stale_object_info = GetUniverse().EmpireStaleKnowledgeObjectIDs(client_empire_id);
    int ship_count =        0;
    float damage_tally =    0.0f;
    float fighters_tally  = 0.0f;
    float structure_tally = 0.0f;
    float shield_tally =    0.0f;
    float troop_tally =     0.0f;
    float colony_tally =    0.0f;

    for (auto& fleet : Objects().FindObjects<const Fleet>(m_fleet_ids)) {
        if ( !(((m_empire_id == ALL_EMPIRES) && (fleet->Unowned())) || fleet->OwnedBy(m_empire_id)) )
            continue;

        for (auto& ship : Objects().FindObjects<const Ship>(fleet->ShipIDs())) {
            int ship_id = ship->ID();

            // skip known destroyed and stale info objects
            if (this_client_known_destroyed_objects.count(ship_id))
                continue;
            if (this_client_stale_object_info.count(ship_id))
                continue;

            if (ship->Design()) {
                ship_count++;
                damage_tally += ship->TotalWeaponsDamage(0.0f, false);
                fighters_tally += ship->FighterCount();
                structure_tally += ship->InitialMeterValue(METER_STRUCTURE);
                shield_tally += ship->InitialMeterValue(METER_SHIELD);
                troop_tally += ship->TroopCapacity();
                colony_tally += ship->ColonyCapacity();
            }
        }
    }

    for (auto& entry : m_stat_icons) {
        MeterType stat_name = entry.first;
        if (stat_name == METER_SHIELD)
            entry.second->SetValue(shield_tally/ship_count);
        else if (stat_name == METER_STRUCTURE)
            entry.second->SetValue(structure_tally);
        else if (stat_name == METER_CAPACITY)
            entry.second->SetValue(damage_tally);
        else if (stat_name == METER_SECONDARY_STAT)
            entry.second->SetValue(fighters_tally);
        else if (stat_name == METER_POPULATION)
            entry.second->SetValue(colony_tally);
        else if (stat_name == METER_SIZE)
            entry.second->SetValue(ship_count);
        else if (stat_name == METER_TROOPS)
            entry.second->SetValue(troop_tally);
    }
}

void FleetWnd::RefreshStateChangedSignals() {
    m_system_connection.disconnect();
    if (auto system = GetSystem(m_system_id))
        m_system_connection = system->StateChangedSignal.connect(
            boost::bind(&FleetWnd::RequireRefresh, this), boost::signals2::at_front);

    for (auto& fleet_connection : m_fleet_connections)
        fleet_connection.disconnect();
    m_fleet_connections.clear();

    for (auto fleet_id : m_fleet_ids) {
        if (auto fleet = GetFleet(fleet_id))
            m_fleet_connections.push_back(
                fleet->StateChangedSignal.connect(
                    boost::bind(&FleetWnd::RequireRefresh, this)));
    }
}

void FleetWnd::RequireRefresh() {
    m_needs_refresh = true;
    RequirePreRender();
}

void FleetWnd::Refresh() {
    m_needs_refresh = false;

    int this_client_empire_id = HumanClientApp::GetApp()->EmpireID();
    const auto& this_client_known_destroyed_objects = GetUniverse().EmpireKnownDestroyedObjectIDs(this_client_empire_id);
    const auto& this_client_stale_object_info = GetUniverse().EmpireStaleKnowledgeObjectIDs(this_client_empire_id);

    // save selected fleet(s) and ships(s)
    auto initially_selected_fleets = this->SelectedFleetIDs();
    auto initially_selected_ships = this->SelectedShipIDs();

    // remove existing fleet rows
    auto initial_fleet_ids = m_fleet_ids;
    m_fleet_ids.clear();

    std::multimap<std::pair<int, GG::Pt>, int> fleet_locations_ids;
    std::multimap<std::pair<int, GG::Pt>, int> selected_fleet_locations_ids;

    // Check all fleets in initial_fleet_ids and keep those that exist.
    std::unordered_set<int> fleets_that_exist;
    GG::Rect fleets_bounding_box;
    for (int fleet_id : initial_fleet_ids) {
        // skip known destroyed and stale info objects
        if (this_client_known_destroyed_objects.count(fleet_id))
            continue;
        if (this_client_stale_object_info.count(fleet_id))
            continue;

        auto fleet = GetFleet(fleet_id);
        if (!fleet)
            continue;

        auto fleet_loc = GG::Pt(GG::X(fleet->X()), GG::Y(fleet->Y()));
        // Grow the fleets bounding box
        fleets_bounding_box = CreateOrGrowBox(fleets_that_exist.empty(), fleets_bounding_box, fleet_loc);

        fleets_that_exist.insert(fleet_id);
        fleet_locations_ids.insert({{fleet->SystemID(), fleet_loc}, fleet_id});
    }

    auto bounding_box_center = GG::Pt(fleets_bounding_box.MidX(), fleets_bounding_box.MidY());


    // Filter initially selected fleets according to existing fleets
    GG::Rect selected_fleets_bounding_box;
    for (int fleet_id : initially_selected_fleets) {
        if (!fleets_that_exist.count(fleet_id))
            continue;

        auto fleet = GetFleet(fleet_id);
        if (!fleet)
            continue;

        auto fleet_loc = GG::Pt(GG::X(fleet->X()), GG::Y(fleet->Y()));

        // Grow the selected fleets bounding box
        selected_fleets_bounding_box = CreateOrGrowBox(selected_fleet_locations_ids.empty(),
                                                       selected_fleets_bounding_box, fleet_loc);
        selected_fleet_locations_ids.insert({{fleet->SystemID(), fleet_loc}, fleet_id});
    }
    auto selected_bounding_box_center = GG::Pt(selected_fleets_bounding_box.MidX(),
                                               selected_fleets_bounding_box.MidY());


    // Determine FleetWnd location

    // Are all fleets in one system?  Use that location.
    // Otherwise, are all selected fleets in one system?  Use that location.
    // Otherwise, are all the moving fleets clustered within m_bounding_box of each other?
    // Otherwise, are all the selected fleets clustered within m_bounding_box of each other?
    // Otherwise, is the current location a system?  Use that location.
    // Otherwise remove all fleets as all fleets have gone in separate directions.

    std::pair<int, GG::Pt> location{INVALID_OBJECT_ID, GG::Pt(GG::X0, GG::Y0)};
    if (!fleet_locations_ids.empty()
        && fleet_locations_ids.begin()->first.first != INVALID_OBJECT_ID
        && (fleet_locations_ids.count(fleet_locations_ids.begin()->first) == fleet_locations_ids.size()))
    {
        location = fleet_locations_ids.begin()->first;

    } else if (!selected_fleet_locations_ids.empty()
               && selected_fleet_locations_ids.begin()->first.first != INVALID_OBJECT_ID
               && (selected_fleet_locations_ids.count(selected_fleet_locations_ids.begin()->first)
                   == selected_fleet_locations_ids.size()))
    {
        location = selected_fleet_locations_ids.begin()->first;

    } else if (!fleet_locations_ids.empty()
               && SmallerOrEqual(fleets_bounding_box, m_bounding_box))
    {
        location = {INVALID_OBJECT_ID, bounding_box_center};
        std::multimap<std::pair<int, GG::Pt>, int> fleets_near_enough;
        for (const auto& loc_and_id: fleet_locations_ids)
            fleets_near_enough.insert({location, loc_and_id.second});
        fleet_locations_ids.swap(fleets_near_enough);

    } else if (!selected_fleet_locations_ids.empty()
               && SmallerOrEqual(selected_fleets_bounding_box, m_bounding_box))
    {
        location = {INVALID_OBJECT_ID, selected_bounding_box_center};
        std::multimap<std::pair<int, GG::Pt>, int> fleets_near_enough;
        // Center bounding box on selected fleets.
        m_bounding_box = m_bounding_box +
            GG::Pt(selected_fleets_bounding_box.MidX() - m_bounding_box.MidX(),
                   selected_fleets_bounding_box.MidY() - m_bounding_box.MidY());
        for (const auto& loc_and_id: fleet_locations_ids) {
            const auto& pos = loc_and_id.first.second;
            if (m_bounding_box.Contains(pos))
                fleets_near_enough.insert({location, loc_and_id.second});
        }
        fleet_locations_ids.swap(fleets_near_enough);

    } else if (auto system = GetSystem(m_system_id)) {
        location = {m_system_id, GG::Pt(GG::X(system->X()), GG::Y(system->Y()))};

    } else {
        fleet_locations_ids.clear();
        selected_fleet_locations_ids.clear();
    }


    // Use fleets that are at the determined location
    auto flt_at_loc = fleet_locations_ids.equal_range(location);
    for (auto it = flt_at_loc.first; it != flt_at_loc.second; ++it)
    { m_fleet_ids.insert(it->second); }

    m_system_id = location.first;

    if (m_new_fleet_drop_target)
        m_new_fleet_drop_target->SetSystemID(m_system_id);

    // If the location is a system add in any ships from m_empire_id that are in the system.
    if (auto system = GetSystem(m_system_id)) {
        m_fleet_ids.clear();
        // get fleets to show from system, based on required ownership
        for (auto& fleet : Objects().FindObjects<Fleet>(system->FleetIDs())) {
            int fleet_id = fleet->ID();

            // skip known destroyed and stale info objects
            if (this_client_known_destroyed_objects.count(fleet_id) ||
                    this_client_stale_object_info.count(fleet_id))
                continue;

            if ( ((m_empire_id == ALL_EMPIRES) && (fleet->Unowned())) || fleet->OwnedBy(m_empire_id) )
                m_fleet_ids.insert(fleet_id);
        }
    }


    // Add rows for the known good fleet_ids.
    m_fleets_lb->Clear();
    for (int fleet_id : m_fleet_ids)
        AddFleet(fleet_id);


    // Determine selected fleets that are at the determined location
    std::set<int> still_present_initially_selected_fleets;
    auto sel_flt_at_loc = selected_fleet_locations_ids.equal_range(location);
    for (auto it = sel_flt_at_loc.first; it != sel_flt_at_loc.second; ++it)
    { still_present_initially_selected_fleets.insert(it->second); }


    //std::cout << "Still present initially selected fleets: ";
    //for (auto entry : still_present_initially_selected_fleets)
    //    std::cout << entry << " ";
    //std::cout << std::endl;


    // Reselect previously-selected fleets, or default select first fleet in FleetWnd
    if (!still_present_initially_selected_fleets.empty()) {
        // reselect any previously-selected fleets
        SelectFleets(still_present_initially_selected_fleets);
        // reselect any previously-selected ships
        SelectShips(initially_selected_ships);

    } else if (!m_fleets_lb->Empty()) {
        // default select first fleet
        int first_fleet_id = FleetInRow(m_fleets_lb->begin());
        if (first_fleet_id != INVALID_OBJECT_ID) {
            std::set<int> fleet_id_set;
            fleet_id_set.insert(first_fleet_id);
            SelectFleets(fleet_id_set);
        }
    }

    SetName(TitleText());
    SetStatIconValues();
    RefreshStateChangedSignals();
}

void FleetWnd::CloseClicked()
{ CUIWnd::CloseClicked(); }

void FleetWnd::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    MapWndPopup::LClick(pt, mod_keys);
    ClickedSignal(std::static_pointer_cast<FleetWnd>(shared_from_this()));
}

void FleetWnd::DoLayout() {
    const GG::X TOTAL_WIDTH(ClientWidth());
    const GG::X LEFT(GG::X0);
    const GG::X RIGHT(TOTAL_WIDTH);

    const GG::Y TOTAL_HEIGHT(ClientHeight());
    const GG::Y FLEET_STAT_HEIGHT(StatIconSize().y + PAD);
    const GG::Y AVAILABLE_HEIGHT(TOTAL_HEIGHT - GG::Y(INNER_BORDER_ANGLE_OFFSET+PAD) - FLEET_STAT_HEIGHT );
    GG::Y top( GG::Y0 + GG::Y(PAD) );
    const GG::Y ROW_HEIGHT(m_fleets_lb->ListRowSize().y);

    // position fleet aggregate stat icons
    GG::Pt icon_ul = GG::Pt(GG::X0 + DATA_PANEL_TEXT_PAD, top);
    for (auto& stat_icon : m_stat_icons) {
        stat_icon.second->SizeMove(icon_ul, icon_ul + StatIconSize());
        icon_ul.x += StatIconSize().x;
    }
    top += FLEET_STAT_HEIGHT;

    // are there any fleets owned by this client's empire int his FleetWnd?
    bool this_client_owns_fleets_in_this_wnd(false);
    int this_client_empire_id = HumanClientApp::GetApp()->EmpireID();
    for (int fleet_id : m_fleet_ids) {
        auto fleet = GetFleet(fleet_id);
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
        m_new_fleet_drop_target->SizeMove(  GG::Pt(LEFT + GG::X(PAD), top), GG::Pt(RIGHT - GG::X(PAD), top + ROW_HEIGHT));
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
    auto fleet = GetFleet(fleet_id);
    if (!fleet /*|| fleet->Empty()*/)
        return;

    // verify that fleet is consistent
    const GG::Pt row_size = m_fleets_lb->ListRowSize();
    auto row = GG::Wnd::Create<FleetRow>(fleet_id, GG::X1, row_size.y);
    m_fleets_lb->Insert(row);
    row->Resize(row_size);
}

void FleetWnd::DeselectAllFleets() {
    m_fleets_lb->DeselectAll();
    FleetSelectionChanged(m_fleets_lb->Selections());
}

void FleetWnd::SelectFleet(int fleet_id) {
    if (fleet_id == INVALID_OBJECT_ID || !(GetFleet(fleet_id))) {
        ErrorLogger() << "FleetWnd::SelectFleet invalid id " << fleet_id;
        DeselectAllFleets();
        return;
    }

    for (auto it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        FleetRow* row = dynamic_cast<FleetRow*>(it->get());
        if (row && row->FleetID() == fleet_id) {
            m_fleets_lb->DeselectAll();
            m_fleets_lb->SelectRow(it);

            FleetSelectionChanged(m_fleets_lb->Selections());

            m_fleets_lb->BringRowIntoView(it);
            break;
        }
    }
}

void FleetWnd::SelectFleets(const std::set<int>& fleet_ids) {
    const GG::ListBox::SelectionSet initial_selections = m_fleets_lb->Selections();
    m_fleets_lb->DeselectAll();

    // loop through fleets, selecting any indicated
    for (auto it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        FleetRow* row = dynamic_cast<FleetRow*>(it->get());
        if (!row) {
            ErrorLogger() << "FleetWnd::SelectFleets couldn't cast a list row to FleetRow?";
            continue;
        }

        // if this row's fleet should be selected, so so
        if (fleet_ids.count(row->FleetID())) {
            m_fleets_lb->SelectRow(it);
            m_fleets_lb->BringRowIntoView(it);  // may cause earlier rows brought into view to be brought out of view... oh well
        }
    }

    const GG::ListBox::SelectionSet sels = m_fleets_lb->Selections();

    if (sels.empty() || initial_selections != sels)
        FleetSelectionChanged(sels);
}

void FleetWnd::SelectShips(const std::set<int>& ship_ids)
{ m_fleet_detail_panel->SelectShips(ship_ids); }

void FleetWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = Size();
    MapWndPopup::SizeMove(ul, lr);
    if (Size() != old_size)
        DoLayout();
}

int FleetWnd::SystemID() const
{ return m_system_id; }

int FleetWnd::EmpireID() const
{ return m_empire_id; }

bool FleetWnd::ContainsFleet(int fleet_id) const {
    for (auto it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        auto fleet = GetFleet(FleetInRow(it));
        if (fleet && fleet->ID() == fleet_id)
            return true;
    }
    return false;
}

template <typename Set>
bool FleetWnd::ContainsFleets(const Set& fleet_ids_) const {
    if (fleet_ids_.empty())
        return false;

    auto fleet_ids = fleet_ids_;

    // Remove found ids from fleet_ids.  If fleet_ids is empty, all have been found.
    for (auto it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        auto fleet = GetFleet(FleetInRow(it));
        if (fleet)
            fleet_ids.erase(fleet->ID());

        if (fleet_ids.empty())
            return true;
    }

    return fleet_ids.empty();
}

const std::set<int>& FleetWnd::FleetIDs() const
{ return m_fleet_ids; }

std::set<int> FleetWnd::SelectedFleetIDs() const {
    std::set<int> retval;
    for (auto it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
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

NewFleetAggression FleetWnd::GetNewFleetAggression() const {
    if (m_new_fleet_drop_target)
        return m_new_fleet_drop_target->GetNewFleetAggression();
    return INVALID_FLEET_AGGRESSION;
}

void FleetWnd::FleetSelectionChanged(const GG::ListBox::SelectionSet& rows) {
    // show appropriate fleet in detail panel.  if one fleet is selected, show
    // its ships.  if more than one fleet is selected or no fleets are selected
    // then show no ships.
    if (rows.size() == 1) {
        // find selected row and fleet
        bool found_row = false;
        for (auto it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
            if (rows.count(it)) {
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


    for (auto it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        try {
            if (auto* fleet_panel = boost::polymorphic_downcast<FleetDataPanel*>(!(**it).empty() ? (**it).at(0) : nullptr))
                fleet_panel->Select(rows.count(it));
        } catch (const std::exception& e) {
            ErrorLogger() << "FleetWnd::FleetSelectionChanged caught exception: " << e.what();
            continue;
        }
    }

    m_fleet_detail_panel->Refresh();
    SelectedFleetsChangedSignal();
}

void FleetWnd::FleetRightClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) {
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();

    auto fleet = GetFleet(FleetInRow(it));
    if (!fleet)
        return;

    auto system = GetSystem(fleet->SystemID());
    std::set<int> ship_ids_set = fleet->ShipIDs();

    // find damaged ships
    std::vector<int> damaged_ship_ids;
    damaged_ship_ids.reserve(ship_ids_set.size());
    for (int ship_id : ship_ids_set) {
        auto ship = GetShip(ship_id);
        if (ship->InitialMeterValue(METER_STRUCTURE) < ship->InitialMeterValue(METER_MAX_STRUCTURE))
            damaged_ship_ids.push_back(ship_id);
    }

    // find ships with no remaining fuel
    std::vector<int> unfueled_ship_ids;
    unfueled_ship_ids.reserve(ship_ids_set.size());
    for (int ship_id : ship_ids_set) {
        auto ship = GetShip(ship_id);
        if (!ship)
            continue;
        if (ship->InitialMeterValue(METER_FUEL) < 1)
            unfueled_ship_ids.push_back(ship_id);
    }

    // find ships that can carry fighters but dont have a full complement of them
    std::vector<int> not_full_fighters_ship_ids;
    not_full_fighters_ship_ids.reserve(ship_ids_set.size());
    for (int ship_id : ship_ids_set) {
        auto ship = GetShip(ship_id);
        if (!ship)
            continue;
        if (!ship->HasFighters())
            continue;
        if (ship->FighterCount() < ship->FighterMax())
            not_full_fighters_ship_ids.push_back(ship_id);
    }


    // determine which other empires are at peace with client empire and have
    // an owned object in this fleet's system
    std::set<int> peaceful_empires_in_system;
    if (system) {
        for (auto& obj : Objects().FindObjects<const UniverseObject>(system->ObjectIDs())) {
            if (obj->GetVisibility(client_empire_id) < VIS_PARTIAL_VISIBILITY)
                continue;
            if (obj->Owner() == client_empire_id || obj->Unowned())
                continue;
            if (peaceful_empires_in_system.count(obj->Owner()))
                continue;
            if (Empires().GetDiplomaticStatus(client_empire_id, obj->Owner()) != DIPLO_PEACE &&
                Empires().GetDiplomaticStatus(client_empire_id, obj->Owner()) != DIPLO_ALLIED)
            { continue; }
            peaceful_empires_in_system.insert(obj->Owner());
        }
    }


    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);

    // add a fleet popup command to send the fleet exploring, and stop it from exploring
    if (system
        && !ClientUI::GetClientUI()->GetMapWnd()->IsFleetExploring(fleet->ID())
        && !ClientPlayerIsModerator()
        && fleet->OwnedBy(client_empire_id))
    {
        auto explore_action = [fleet]() { ClientUI::GetClientUI()->GetMapWnd()->SetFleetExploring(fleet->ID()); };

        popup->AddMenuItem(GG::MenuItem(UserString("ORDER_FLEET_EXPLORE"),       false, false, explore_action));
        popup->AddMenuItem(GG::MenuItem(true));
    }
    else if (system
             && !ClientPlayerIsModerator()
             && fleet->OwnedBy(client_empire_id))
    {
        auto stop_explore_action = [fleet]() { ClientUI::GetClientUI()->GetMapWnd()->StopFleetExploring(fleet->ID()); };
        popup->AddMenuItem(GG::MenuItem(UserString("ORDER_CANCEL_FLEET_EXPLORE"), false, false, stop_explore_action));
        popup->AddMenuItem(GG::MenuItem(true));
    }

    // Merge fleets
    if (system
        && fleet->OwnedBy(client_empire_id)
        && !ClientPlayerIsModerator()
       )
    {
        auto merge_action = [fleet]() { MergeFleetsIntoFleet(fleet->ID()); };
        popup->AddMenuItem(GG::MenuItem(UserString("FW_MERGE_SYSTEM_FLEETS"),   false, false, merge_action));
        popup->AddMenuItem(GG::MenuItem(true));
    }

    // Split damaged ships - need some, but not all, ships damaged, and need to be in a system
    if (system
        && ship_ids_set.size() > 1
        && !damaged_ship_ids.empty()
        && damaged_ship_ids.size() != ship_ids_set.size()
        && fleet->OwnedBy(client_empire_id)
        && !ClientPlayerIsModerator()
       )
    {
        auto split_damage_action = [&damaged_ship_ids]() { CreateNewFleetFromShips(damaged_ship_ids, FLEET_PASSIVE); };
        popup->AddMenuItem(GG::MenuItem(UserString("FW_SPLIT_DAMAGED_FLEET"),     false, false, split_damage_action));
    }

    // Split unfueled ships - need some, but not all, ships unfueled, and need to be in a system
    if (system
        && ship_ids_set.size() > 1
        && unfueled_ship_ids.size() > 0
        && unfueled_ship_ids.size() < ship_ids_set.size()
        && fleet->OwnedBy(client_empire_id)
        && !ClientPlayerIsModerator()
       )
    {
        auto split_unfueled_action = [fleet, &unfueled_ship_ids]() {
            NewFleetAggression nfa = fleet->Aggressive() ? FLEET_AGGRESSIVE : FLEET_PASSIVE;
            CreateNewFleetFromShips(unfueled_ship_ids, nfa);
        };

        popup->AddMenuItem(GG::MenuItem(UserString("FW_SPLIT_UNFUELED_FLEET"),    false, false, split_unfueled_action));
    }

    // Split ships with not as many fighters as they could contain
    if (system
        && ship_ids_set.size() > 1
        && not_full_fighters_ship_ids.size() > 0
        && not_full_fighters_ship_ids.size() < ship_ids_set.size()
        && fleet->OwnedBy(client_empire_id)
        && !ClientPlayerIsModerator()
       )
    {
        auto split_not_full_fighters_action = [fleet, &not_full_fighters_ship_ids]() {
            NewFleetAggression nfa = fleet->Aggressive() ? FLEET_AGGRESSIVE : FLEET_PASSIVE;
            CreateNewFleetFromShips(not_full_fighters_ship_ids, nfa);
        };
        popup->AddMenuItem(GG::MenuItem(UserString("FW_SPLIT_NOT_FULL_FIGHTERS_FLEET"), false, false, split_not_full_fighters_action));
    }

    // Split fleet - can't split fleets without more than one ship, or which are not in a system
    if (system
        && ship_ids_set.size() > 1
        && (fleet->OwnedBy(client_empire_id))
        && !ClientPlayerIsModerator()
       )
    {
        auto split_action = [this, &ship_ids_set]() {
            ScopedTimer split_fleet_timer("FleetWnd::SplitFleet", true);
            // remove first ship from set, so it stays in its existing fleet
            auto ship_id_it = ship_ids_set.begin();
            ship_ids_set.erase(ship_id_it);

            NewFleetAggression new_aggression_setting = INVALID_FLEET_AGGRESSION;
            if (m_new_fleet_drop_target)
                new_aggression_setting = m_new_fleet_drop_target->GetNewFleetAggression();

            // assemble container of containers of ids of fleets to create.
            // one ship id per vector
            for (int ship_id : ship_ids_set) {
                CreateNewFleetFromShips(
                    std::vector<int>{ship_id},
                    new_aggression_setting);
            }
        };

        auto split_per_design_action = [this, fleet]() {
            NewFleetAggression new_aggression_setting = INVALID_FLEET_AGGRESSION;
            if (m_new_fleet_drop_target)
                new_aggression_setting = m_new_fleet_drop_target->GetNewFleetAggression();

            CreateNewFleetsFromShipsForEachDesign(fleet->ShipIDs(), new_aggression_setting);
        };

        popup->AddMenuItem(GG::MenuItem(UserString("FW_SPLIT_FLEET"),             false, false, split_action));
        popup->AddMenuItem(GG::MenuItem(UserString("FW_SPLIT_SHIPS_ALL_DESIGNS"), false, false, split_per_design_action));
        popup->AddMenuItem(GG::MenuItem(true));
    }

    // Rename fleet
    if (fleet->OwnedBy(client_empire_id)
        || ClientPlayerIsModerator())
    {
        auto rename_action = [fleet, client_empire_id]() {
            auto edit_wnd = GG::Wnd::Create<CUIEditWnd>(GG::X(350), UserString("ENTER_NEW_NAME"), fleet->Name());
            edit_wnd->Run();

            if (ClientPlayerIsModerator())
                // TODO: handle moderator actions for this...
                return;

            if (!RenameOrder::Check(client_empire_id, fleet->ID(), edit_wnd->Result()))
                return;

            HumanClientApp::GetApp()->Orders().IssueOrder(
                std::make_shared<RenameOrder>(client_empire_id, fleet->ID(), edit_wnd->Result()));
        };
        popup->AddMenuItem(GG::MenuItem(UserString("RENAME"),                       false, false, rename_action));
        popup->AddMenuItem(GG::MenuItem(true));
    }

    bool post_scrap_bar = false;

    // add a fleet popup command to order all ships in the fleet scrapped
    if (system
        && fleet->HasShipsWithoutScrapOrders()
        && !ClientPlayerIsModerator()
        && fleet->OwnedBy(client_empire_id))
    {
        auto scrap_action = [fleet, client_empire_id]() {
            std::set<int> ship_ids = fleet->ShipIDs();
            for (int ship_id : ship_ids) {
                HumanClientApp::GetApp()->Orders().IssueOrder(
                    std::make_shared<ScrapOrder>(client_empire_id, ship_id));
            }
        };

        popup->AddMenuItem(GG::MenuItem(UserString("ORDER_FLEET_SCRAP"),          false, false, scrap_action));
        post_scrap_bar = true;
    }

    // add a fleet popup command to cancel all scrap orders on ships in this fleet
    if (system
        && fleet->HasShipsOrderedScrapped()
        && !ClientPlayerIsModerator())
    {
        auto unscrap_action = [fleet]() {
            const OrderSet orders = HumanClientApp::GetApp()->Orders();
            for (int ship_id : fleet->ShipIDs()) {
                for (const auto& id_and_order : orders) {
                    if (std::shared_ptr<ScrapOrder> order = std::dynamic_pointer_cast<ScrapOrder>(id_and_order.second)) {
                        if (order->ObjectID() == ship_id) {
                            HumanClientApp::GetApp()->Orders().RescindOrder(id_and_order.first);
                            // could break here, but won't to ensure there are no problems with doubled orders
                        }
                    }
                }
            }
        };

        popup->AddMenuItem(GG::MenuItem(UserString("ORDER_CANCEL_FLEET_SCRAP"),   false, false, unscrap_action));
        post_scrap_bar = true;
    }

    if (fleet->OwnedBy(client_empire_id)
        && fleet->TravelRoute().empty()
        && !peaceful_empires_in_system.empty()
        && !ClientPlayerIsModerator())
    {
        if (post_scrap_bar)
            popup->AddMenuItem(GG::MenuItem(true));

        // submenus for each available recipient empire
        GG::MenuItem give_away_menu(UserString("ORDER_GIVE_FLEET_TO_EMPIRE"), false, false);
        for (auto& entry : Empires()) {
            int recipient_empire_id = entry.first;
            if (!peaceful_empires_in_system.count(recipient_empire_id))
                continue;
            auto gift_action = [recipient_empire_id, fleet, client_empire_id]() {
                HumanClientApp::GetApp()->Orders().IssueOrder(
                    std::make_shared<GiveObjectToEmpireOrder>(client_empire_id, fleet->ID(), recipient_empire_id));
            };
            give_away_menu.next_level.push_back(GG::MenuItem(entry.second->Name(), false, false, gift_action));
        }
        popup->AddMenuItem(std::move(give_away_menu));

        if (fleet->OrderedGivenToEmpire() != ALL_EMPIRES) {
            auto ungift_action = [fleet]() {
                for (const auto& id_and_order : HumanClientApp::GetApp()->Orders()) {
                    if (std::shared_ptr<GiveObjectToEmpireOrder> order =
                        std::dynamic_pointer_cast<GiveObjectToEmpireOrder>(id_and_order.second))
                    {
                        if (order->ObjectID() == fleet->ID()) {
                            HumanClientApp::GetApp()->Orders().RescindOrder(id_and_order.first);
                            // could break here, but won't to ensure there are no problems with doubled orders
                        }
                    }
                }
            };
            GG::MenuItem cancel_give_away_menu(UserString("ORDER_CANCEL_GIVE_FLEET"), false, false, ungift_action);
            popup->AddMenuItem(std::move(cancel_give_away_menu));
        }
    }


    // Allow dismissal of stale visibility information
    if (!fleet->OwnedBy(client_empire_id)) {
        auto forget_fleet_action = [fleet]() {
            ClientUI::GetClientUI()->GetMapWnd()->ForgetObject(fleet->ID());
        };
        auto visibility_turn_map =
            GetUniverse().GetObjectVisibilityTurnMapByEmpire(fleet->ID(), client_empire_id);
        auto last_turn_visible_it = visibility_turn_map.find(VIS_BASIC_VISIBILITY);
        if (last_turn_visible_it != visibility_turn_map.end()
            && last_turn_visible_it->second < CurrentTurn())
        {
            popup->AddMenuItem(GG::MenuItem(UserString("FW_ORDER_DISMISS_SENSOR_GHOST"), false, false, forget_fleet_action));
        }
    }

    popup->Run();
}

void FleetWnd::FleetLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys)
{ ClickedSignal(std::static_pointer_cast<FleetWnd>(shared_from_this())); }

void FleetWnd::FleetDoubleClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys)
{ ClickedSignal(std::static_pointer_cast<FleetWnd>(shared_from_this())); }

int FleetWnd::FleetInRow(GG::ListBox::iterator it) const {
    if (it == m_fleets_lb->end())
        return INVALID_OBJECT_ID;

    try {
        //DebugLogger() << "FleetWnd::FleetInRow casting iterator to fleet row";
        if (FleetRow* fleet_row = dynamic_cast<FleetRow*>(it->get())) {
            return fleet_row->FleetID();
        }
    } catch (const std::exception& e) {
        ErrorLogger() << "FleetInRow caught exception: " << e.what();
    }

    return INVALID_OBJECT_ID;
}

namespace {
    std::string SystemNameNearestToFleet(int client_empire_id, int fleet_id) {
        auto fleet = GetFleet(fleet_id);
        if (!fleet)
            return "";

        int nearest_system_id(GetPathfinder()->NearestSystemTo(fleet->X(), fleet->Y()));
        if (auto system = GetSystem(nearest_system_id)) {
            const std::string& sys_name = system->ApparentName(client_empire_id);
            return sys_name;
        }
        return "";
    }
}

std::string FleetWnd::TitleText() const {
    // if no fleets available, default to indicating no fleets
    if (m_fleet_ids.empty())
        return UserString("FW_NO_FLEET");

    int client_empire_id = HumanClientApp::GetApp()->EmpireID();

    // at least one fleet is available, so show appropriate title this
    // FleetWnd's empire and system
    const Empire* empire = GetEmpire(m_empire_id);

    if (auto system = GetSystem(m_system_id)) {
        const std::string& sys_name = system->ApparentName(client_empire_id);
        return (empire
                ? boost::io::str(FlexibleFormat(UserString("FW_EMPIRE_FLEETS_AT_SYSTEM")) %
                                 empire->Name() % sys_name)
                : boost::io::str(FlexibleFormat(UserString("FW_GENERIC_FLEETS_AT_SYSTEM")) %
                                 sys_name));
    }

    const std::string sys_name = SystemNameNearestToFleet(client_empire_id, *m_fleet_ids.begin());
    if (!sys_name.empty()) {
        return (empire
                ? boost::io::str(FlexibleFormat(UserString("FW_EMPIRE_FLEETS_NEAR_SYSTEM")) %
                                 empire->Name() % sys_name)
                : boost::io::str(FlexibleFormat(UserString("FW_GENERIC_FLEETS_NEAR_SYSTEM")) %
                                 sys_name));
    }

    return (empire
            ? boost::io::str(FlexibleFormat(UserString("FW_EMPIRE_FLEETS")) %
                             empire->Name())
            : boost::io::str(FlexibleFormat(UserString("FW_GENERIC_FLEETS"))));
}

void FleetWnd::CreateNewFleetFromDrops(const std::vector<int>& ship_ids) {
    DebugLogger() << "FleetWnd::CreateNewFleetFromDrops with " << ship_ids.size() << " ship ids";

    if (ship_ids.empty())
        return;

    NewFleetAggression aggression = INVALID_FLEET_AGGRESSION;
    if (m_new_fleet_drop_target)
        aggression = m_new_fleet_drop_target->GetNewFleetAggression();

    // deselect all ships so that response to fleet rearrangement doesn't attempt
    // to get the selected ships that are no longer in their old fleet.
    m_fleet_detail_panel->SelectShips(std::set<int>());

    CreateNewFleetFromShips(ship_ids, aggression);
}

void FleetWnd::ShipSelectionChanged(const GG::ListBox::SelectionSet& rows)
{ SelectedShipsChangedSignal(); }

void FleetWnd::UniverseObjectDeleted(std::shared_ptr<const UniverseObject> obj) {
    // check if deleted object was a fleet.  The universe signals for all
    // object types, not just fleets.
    std::shared_ptr<const Fleet> deleted_fleet = std::dynamic_pointer_cast<const Fleet>(obj);
    if (!deleted_fleet)
        return;

    // if detail panel is showing the deleted fleet, reset to show nothing
    if (GetFleet(m_fleet_detail_panel->FleetID()) == deleted_fleet)
        m_fleet_detail_panel->SetFleet(INVALID_OBJECT_ID);

    const ObjectMap& objects = GetUniverse().Objects();

    // remove deleted fleet's row
    for (auto it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        int row_fleet_id = FleetInRow(it);
        if (objects.Object<Fleet>(row_fleet_id) == deleted_fleet) {
            m_fleets_lb->Erase(it);
            break;
        }
    }
}

void FleetWnd::EnableOrderIssuing(bool enable/* = true*/) {
    m_order_issuing_enabled = enable;
    if (m_new_fleet_drop_target)
        m_new_fleet_drop_target->Disable(!m_order_issuing_enabled);
    if (m_fleets_lb)
        m_fleets_lb->EnableOrderIssuing(m_order_issuing_enabled);
    if (m_fleet_detail_panel)
        m_fleet_detail_panel->EnableOrderIssuing(m_order_issuing_enabled);
}
