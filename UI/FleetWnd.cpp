#include "FleetWnd.h"

#include "CUIControls.h"
#include "SidePanel.h"
#include "IconTextBrowseWnd.h"
#include "MeterBrowseWnd.h"
#include "ModeratorActionsWnd.h"
#include "ClientUI.h"
#include "Sound.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/Order.h"
#include "../util/OptionsDB.h"
#include "../util/ScopedTimer.h"
#include "../client/human/GGHumanClientApp.h"
#include "../universe/Fleet.h"
#include "../universe/Planet.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../universe/Pathfinder.h"
#include "../network/Message.h"
#include "../Empire/Empire.h"

#include <GG/Enum.h>
#include <GG/GUI.h>
#include <GG/Layout.h>
#include <GG/StaticGraphic.h>

#include <tuple>
#include <unordered_set>


namespace {
    const GG::Pt DataPanelIconSpace()
    { return GG::Pt(GG::X(ClientUI::Pts()*3), GG::Y(ClientUI::Pts()*2.5)); }
    constexpr GG::X FLEET_WND_WIDTH = GG::X(360);
    constexpr GG::Y FLEET_WND_HEIGHT = GG::Y(400);

    // how should ship and fleet icons be scaled and/or positioned in the reserved space
    constexpr auto DATA_PANEL_ICON_STYLE = 
        GG::GRAPHIC_CENTER | GG::GRAPHIC_VCENTER | GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE;

    constexpr GG::X            DATA_PANEL_TEXT_PAD{4}; // padding on the left and right of fleet/ship description
    constexpr int              DATA_PANEL_BORDER = 1;  // how thick should the border around ship or fleet panel be
    constexpr int              PAD = 4;
    constexpr std::string_view SHIP_DROP_TYPE_STRING = "FleetWnd ShipRow";
    constexpr std::string_view FLEET_DROP_TYPE_STRING = "FleetWnd FleetRow";
    constexpr std::string_view FLEET_WND_NAME = "map.fleet";

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

    std::shared_ptr<GG::Texture> DestroyIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "meter" / "ammo.png", true); }

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

    std::shared_ptr<GG::Texture> InfluenceIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "meter" / "influence.png"); }

    std::string FleetDestinationText(int fleet_id, const ScriptingContext& context) {
        std::string retval;
        auto fleet = context.ContextObjects().get<Fleet>(fleet_id);
        if (!fleet)
            return retval;

        const int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();
        const auto map_wnd = ClientUI::GetClientUI()->GetMapWndConst();
        const bool fleet_is_exploring = map_wnd && map_wnd->IsFleetExploring(fleet_id);

        const auto dest_sys = context.ContextObjects().get<System>(fleet->FinalDestinationID());
        const auto cur_sys = context.ContextObjects().get<System>(fleet->SystemID());
        bool returning_to_current_system = (dest_sys == cur_sys) && !fleet->TravelRoute().empty();
        if (dest_sys && (dest_sys != cur_sys || returning_to_current_system)) {
            const auto [eta_final, eta_next] = fleet->ETA(context);

            // name of final destination
            std::string dest_name = dest_sys->ApparentName(client_empire_id, context.ContextUniverse());
            if (GetOptionsDB().Get<bool>("ui.name.id.shown"))
                dest_name += " (" + std::to_string(dest_sys->ID()) + ")";

            // next system on path
            std::string next_eta_text;
            if (eta_next == Fleet::ETA_UNKNOWN)
                next_eta_text = UserString("FW_FLEET_ETA_UNKNOWN");
            else if (eta_next == Fleet::ETA_NEVER)
                next_eta_text = UserString("FW_FLEET_ETA_NEVER");
            else if (eta_next == Fleet::ETA_OUT_OF_RANGE)
                next_eta_text = UserString("FW_FLEET_ETA_OUT_OF_RANGE");
            else
                next_eta_text = std::to_string(eta_next);

            // final destination
            std::string final_eta_text;
            if (eta_final == Fleet::ETA_UNKNOWN)
                final_eta_text = UserString("FW_FLEET_ETA_UNKNOWN");
            else if (eta_final == Fleet::ETA_NEVER)
                final_eta_text = UserString("FW_FLEET_ETA_NEVER");
            else if (eta_final == Fleet::ETA_OUT_OF_RANGE)
                final_eta_text = UserString("FW_FLEET_ETA_OUT_OF_RANGE");
            else
                final_eta_text = std::to_string(eta_final);

            if (fleet_is_exploring)
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
            std::string cur_system_name = cur_sys->ApparentName(client_empire_id, context.ContextUniverse());
            if (GetOptionsDB().Get<bool>("ui.name.id.shown")) {
                cur_system_name = cur_system_name + " (" + std::to_string(cur_sys->ID()) + ")";
            }

            if (fleet_is_exploring) {
                if (fleet->Fuel(context.ContextObjects()) == fleet->MaxFuel(context.ContextObjects()))
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
    { return GGHumanClientApp::GetApp()->GetClientType() == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR; }

    bool CanDamageShips(const std::vector<int>& ship_ids, const ScriptingContext& context) {
        const auto ships = context.ContextObjects().findRaw<Ship>(ship_ids);
        return std::any_of(ships.begin(), ships.end(), [&context](const auto* ship)
                           { return ship && ship->CanDamageShips(context); });
    }

    FleetAggression AggressionForFleet(FleetAggression aggression_mode, const std::vector<int>& ship_ids,
                                       const ScriptingContext& context)
    {
        if (aggression_mode < FleetAggression::NUM_FLEET_AGGRESSIONS &&
            aggression_mode > FleetAggression::INVALID_FLEET_AGGRESSION)
        { return aggression_mode; }
        // auto aggression; examine ships to see if any are armed...
        if (CanDamageShips(ship_ids, context))
            return FleetDefaults::FLEET_DEFAULT_ARMED;
        return FleetDefaults::FLEET_DEFAULT_UNARMED;
    }

    void CreateNewFleetFromShips(const std::vector<int>& ship_ids,
                                 FleetAggression aggression, ScriptingContext& context,
                                 int empire_id)
    {
        if (ClientPlayerIsModerator())
            return; // TODO: handle moderator actions for this...
        if (empire_id == ALL_EMPIRES)
            return;

        // TODO: Should probably have the sound effect occur exactly once instead
        //       of not at all.
        Sound::TempUISoundDisabler sound_disabler;

        ObjectMap& objects{context.ContextObjects()};

        // validate ships in each group, and generate fleet names for those ships
        const auto ships = objects.findRaw<const Ship>(ship_ids);
        if (ships.empty())
            return;

        const auto first_ship = ships.front();
        const auto system = objects.getRaw<System>(first_ship->SystemID());
        if (!system)
            return;

        // validate that ships are in the same system and all owned by this
        // client's empire.
        // also record the fleets from which ships are taken
        for (const auto* ship : ships) {
             if (ship->SystemID() != system->ID()) {
                 ErrorLogger() << "CreateNewFleetFromShips passed ships with inconsistent system ids";
                 continue;
             }
             if (!ship->OwnedBy(empire_id)) {
                 ErrorLogger() << "CreateNewFleetFromShips passed ships not owned by this client's empire";
                 return;
             }
        }

        // create new fleet with ships
        const auto aggr = AggressionForFleet(aggression, ship_ids, context);
        GGHumanClientApp::GetApp()->Orders().IssueOrder<NewFleetOrder>(
            context, empire_id, "", ship_ids, aggr);
    }

    void CreateNewFleetFromShipsWithDesign(const auto& ship_ids,
                                           int design_id, FleetAggression aggression,
                                           ScriptingContext& context, int empire_id)
    {
        DebugLogger() << "CreateNewFleetFromShipsWithDesign with " << ship_ids.size()
                               << " ship ids and design id: " << design_id;
        if (ship_ids.empty() || design_id == INVALID_DESIGN_ID)
            return;
        if (empire_id == ALL_EMPIRES && !ClientPlayerIsModerator())
            return;

        // select ships with the requested design id
        std::vector<int> ships_of_design_ids;
        ships_of_design_ids.reserve(ship_ids.size());
        for (auto& ship : context.ContextObjects().find<Ship>(ship_ids)) {
            if (ship->DesignID() == design_id)
                ships_of_design_ids.push_back(ship->ID());
        }

        CreateNewFleetFromShips(ships_of_design_ids, aggression, context, empire_id);
    }

    void CreateNewFleetsFromShipsForEachDesign(const auto& ship_ids,
                                               FleetAggression aggression,
                                               ScriptingContext& context,
                                               int empire_id)
    {
        DebugLogger() << "CreateNewFleetsFromShipsForEachDesign with " << ship_ids.size() << " ship ids";
        if (ship_ids.empty())
            return;
        if (empire_id == ALL_EMPIRES && !ClientPlayerIsModerator())
            return;

        // sort ships by ID into container, indexed by design id
        std::map<int, std::vector<int>> designs_ship_ids;
        for (const auto& ship : context.ContextObjects().find<Ship>(ship_ids))
            designs_ship_ids[ship->DesignID()].push_back(ship->ID());

        // note that this will cause a UI update for each call to CreateNewFleetFromShips
        // we can re-evaluate this code if it presents a noticable performance problem
        for (const auto& entry : designs_ship_ids)
            CreateNewFleetFromShips(entry.second, aggression, context, empire_id);
    }

    void MergeFleetsIntoFleet(int fleet_id, ScriptingContext& context) {
        if (ClientPlayerIsModerator())
            return; // TODO: handle moderator actions for this...

        auto* app = GGHumanClientApp::GetApp();
        int client_empire_id = app->EmpireID();
        if (client_empire_id == ALL_EMPIRES)
            return;

        ObjectMap& objects{context.ContextObjects()};

        const auto* target_fleet = objects.getRaw<const Fleet>(fleet_id);
        if (!target_fleet) {
            ErrorLogger() << "MergeFleetsIntoFleet couldn't get a fleet with id " << fleet_id;
            return;
        }

        const auto* system = objects.getRaw<const System>(target_fleet->SystemID());
        if (!system) {
            ErrorLogger() << "MergeFleetsIntoFleet couldn't get system for the target fleet";
            return;
        }

        Sound::TempUISoundDisabler sound_disabler;

        // filter fleets in system to select just those owned by this client's
        // empire, and collect their ship ids
        const auto all_system_fleets = objects.findRaw<const Fleet>(system->FleetIDs());
        std::vector<int> empire_system_fleet_ids;
        empire_system_fleet_ids.reserve(all_system_fleets.size());
        std::vector<int> empire_system_ship_ids;
        empire_system_ship_ids.reserve(all_system_fleets.size());   // probably an underestimate

        for (auto* fleet : all_system_fleets) {
            if (!fleet->OwnedBy(client_empire_id))
                continue;
            if (fleet->ID() == target_fleet->ID() || fleet->ID() == INVALID_OBJECT_ID)
                continue;   // no need to do things to target fleet's contents

            const auto& fleet_ships = fleet->ShipIDs();
            empire_system_ship_ids.insert(empire_system_ship_ids.end(),
                                          fleet_ships.begin(), fleet_ships.end());
            empire_system_fleet_ids.push_back(fleet->ID());
        }


        // order ships moved into target fleet
        app->Orders().IssueOrder<FleetTransferOrder>(
            context, client_empire_id, target_fleet->ID(), std::move(empire_system_ship_ids));
    }

   /** Returns map from object ID to issued colonize orders affecting it. */
    auto PendingScrapOrders() { // TODO: return vector<pair> ?
        std::map<int, int> retval;
        const auto* app = GGHumanClientApp::GetApp();
        if (!app)
            return retval;
        for (const auto& [order_id, order] : app->Orders()) {
            if (auto scrap_order = std::dynamic_pointer_cast<ScrapOrder>(order))
                retval.emplace(scrap_order->ObjectID(), order_id);
        }
        return retval;
    }

    void AddOptions(OptionsDB& db) {
        db.Add("ui.fleet.aggression", UserStringNop("OPTIONS_DB_FLEET_WND_AGGRESSION"), FleetAggression::INVALID_FLEET_AGGRESSION);
        db.Add("ui.fleet.scanline.color", UserStringNop("OPTIONS_DB_UI_FLEET_WND_SCANLINE_CLR"), GG::Clr(24, 24, 24, 192));
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    FleetAggression NewFleetsAggressiveOptionSetting()
    { return GetOptionsDB().Get<FleetAggression>("ui.fleet.aggression"); }

    void SetNewFleetAggressiveOptionSetting(FleetAggression aggression)
    { GetOptionsDB().Set("ui.fleet.aggression", aggression); }

    std::shared_ptr<GG::Texture> FleetAggressiveIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "fleet_aggressive.png"); }
    std::shared_ptr<GG::Texture> FleetAggressiveMouseoverIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "fleet_aggressive_mouseover.png"); }
    std::shared_ptr<GG::Texture> FleetObstructiveIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "fleet_obstructive.png"); }
    std::shared_ptr<GG::Texture> FleetObstructiveMouseoverIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "fleet_obstructive_mouseover.png"); }
    std::shared_ptr<GG::Texture> FleetDefensiveIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "fleet_defensive.png"); }
    std::shared_ptr<GG::Texture> FleetDefensiveMouseoverIcon()
    { return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "buttons" / "fleet_defensive_mouseover.png"); }
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
    std::shared_ptr<FleetWnd> retval;
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
    std::unordered_set<int> fleet_ids{fleet_ids_.begin(), fleet_ids_.end()};
    std::shared_ptr<FleetWnd> retval;
    GG::ProcessThenRemoveExpiredPtrs(
        m_fleet_wnds,
        [&retval, fleet_ids](std::shared_ptr<FleetWnd>& wnd) {
            if (!retval && wnd->ContainsFleets(fleet_ids))
                retval = wnd;
        });
    return retval;
}

int FleetUIManager::SelectedShipID() const {
    const auto active_wnd = GG::LockAndResetIfExpired(m_active_fleet_wnd);
    if (!active_wnd)
        return INVALID_OBJECT_ID;

    const auto selected_ship_ids = active_wnd->SelectedShipIDs();
    return selected_ship_ids.size() == 1 ? *selected_ship_ids.begin() : INVALID_OBJECT_ID;
}

std::set<int> FleetUIManager::SelectedShipIDs() const {
    const auto active_wnd = GG::LockAndResetIfExpired(m_active_fleet_wnd);
    return active_wnd ? active_wnd->SelectedShipIDs() : std::set<int>{};
}

std::shared_ptr<FleetWnd> FleetUIManager::NewFleetWnd(
    const std::vector<int>& fleet_ids,
    double allowed_bounding_box_leeway,
    int selected_fleet_id,
    GG::Flags<GG::WndFlag> flags)
{
    std::string config_name;
    if (!GetOptionsDB().Get<bool>("ui.fleet.multiple.enabled")) {
        CloseAll();
        // Only write to OptionsDB if in single fleet window mode.
        config_name = FLEET_WND_NAME;
    }
    auto retval = GG::Wnd::Create<FleetWnd>(fleet_ids, m_order_issuing_enabled,
                                            allowed_bounding_box_leeway,
                                            selected_fleet_id, flags, config_name);

    using boost::placeholders::_1;

    m_fleet_wnds.insert(std::weak_ptr<FleetWnd>(retval));
    retval->ClosingSignal.connect(boost::bind(&FleetUIManager::FleetWndClosing, this, _1));
    retval->ClickedSignal.connect(boost::bind(&FleetUIManager::FleetWndClicked, this, _1));
    retval->FleetRightClickedSignal.connect(FleetRightClickedSignal);
    retval->ShipRightClickedSignal.connect(ShipRightClickedSignal);

    GG::GUI::GetGUI()->Register(retval);

    return retval;
}

void FleetUIManager::CullEmptyWnds() {
    // scan through FleetWnds, deleting those that have no fleets
    GG::ProcessThenRemoveExpiredPtrs(m_fleet_wnds,
                                     [](const std::shared_ptr<FleetWnd>& wnd) {
                                        if (wnd->FleetIDs().empty())
                                            wnd->CloseClicked();
                                     });
}

void FleetUIManager::SetActiveFleetWnd(std::shared_ptr<FleetWnd> fleet_wnd) {
    const auto active_wnd = GG::LockAndResetIfExpired(m_active_fleet_wnd);
    if (fleet_wnd == active_wnd)
        return;

    // disconnect old active FleetWnd signals
    m_active_fleet_wnd_signals.clear();

    // set new active FleetWnd
    m_active_fleet_wnd = fleet_wnd;

    // connect new active FleetWnd selection changed signal
    m_active_fleet_wnd_signals.emplace_back(fleet_wnd->SelectedFleetsChangedSignal.connect(
        ActiveFleetWndSelectedFleetsChangedSignal));
    m_active_fleet_wnd_signals.emplace_back(fleet_wnd->SelectedShipsChangedSignal.connect(
        ActiveFleetWndSelectedShipsChangedSignal));

    ActiveFleetWndChangedSignal();
}

bool FleetUIManager::CloseAll() {
    bool retval = false;

    // closing a fleet window removes it from m_fleet_wnds
    GG::ProcessThenRemoveExpiredPtrs(m_fleet_wnds,
                                 [&retval](const std::shared_ptr<FleetWnd>& wnd) {
                                     retval = true;
                                     wnd->CloseClicked();
                                 });

    m_active_fleet_wnd.reset();
    ActiveFleetWndChangedSignal();

    // send order changes could be made on fleets
    GGHumanClientApp::GetApp()->SendPartialOrders();

    return retval;
}

void FleetUIManager::RefreshAll(int this_client_empire_id, const ScriptingContext& context) {
    auto refresh_fleetwnd = [this_client_empire_id, &context](auto& wnd)
    { wnd->Refresh(this_client_empire_id, context); };

    GG::ProcessThenRemoveExpiredPtrs(m_fleet_wnds, refresh_fleetwnd);
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
    auto app = GGHumanClientApp::GetApp();
    if (app)
        app->SendPartialOrders();
}

void FleetUIManager::FleetWndClicked(std::shared_ptr<FleetWnd> fleet_wnd) {
    if (fleet_wnd == GG::LockAndResetIfExpired(m_active_fleet_wnd))
        return;
    SetActiveFleetWnd(std::move(fleet_wnd));
}

void FleetUIManager::EnableOrderIssuing(bool enable) {
    m_order_issuing_enabled = enable;
    GG::ProcessThenRemoveExpiredPtrs(m_fleet_wnds,
                                 [&enable](std::shared_ptr<FleetWnd>& wnd)
                                 { wnd->EnableOrderIssuing(enable); });
}

namespace {
    bool ValidShipTransfer(const auto& ship, const auto& new_fleet, const ObjectMap& objects) {
        if (!ship || !new_fleet)
            return false;   // can't transfer no ship or to no fleet

        const auto current_fleet = objects.get<const Fleet>(ship->FleetID());
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

    bool ValidFleetMerge(const auto& fleet, const auto& target_fleet) {
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
        ShipDataPanel(GG::X w, GG::Y h, int ship_id);

        /** Excludes border from the client area. */
        GG::Pt ClientUpperLeft() const noexcept override;
        /** Excludes border from the client area. */
        GG::Pt ClientLowerRight() const noexcept override;

        /** Renders black panel background, border with color depending on the
         *current state and a background for the ship's name text. */
        void Render() override;
        void PreRender() override;

        void SizeMove(GG::Pt ul, GG::Pt lr) override;

        void Select(bool b);

        /** Indicate ship data has changed and needs refresh. */
        void RequireRefresh();

    private:
        double StatValue(MeterType stat_name) const;

        void SetShipIcon();
        void Refresh();
        void DoLayout();
        void Init();

        bool                                m_initialized = false;
        bool                                m_needs_refresh = true;
        int                                 m_ship_id = INVALID_OBJECT_ID;
        std::shared_ptr<GG::StaticGraphic>  m_ship_icon;
        std::vector<std::shared_ptr<GG::StaticGraphic>>
                                            m_ship_icon_overlays;   /// An overlays for orders like scrap, colonize, invade, bombard destroy etc.
        std::shared_ptr<ScanlineControl>    m_scanline_control;
        std::shared_ptr<GG::Label>          m_ship_name_text;
        std::shared_ptr<GG::Label>          m_design_name_text;
        std::vector<std::pair<MeterType, std::shared_ptr<StatisticIcon>>>
                                            m_stat_icons;           /// statistic icons and associated meter types
        bool                                m_selected = false;
        boost::signals2::scoped_connection  m_ship_connection;
        boost::signals2::scoped_connection  m_fleet_connection;
    };

    ShipDataPanel::ShipDataPanel(GG::X w, GG::Y h, int ship_id) :
        Control(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS),
        m_ship_id(ship_id)
    {
        SetChildClippingMode(ChildClippingMode::ClipToClient);
        RequireRefresh();
    }

    GG::Pt ShipDataPanel::ClientUpperLeft() const noexcept
    { return UpperLeft() + GG::Pt(GG::X(DATA_PANEL_BORDER), GG::Y(DATA_PANEL_BORDER)); }

    GG::Pt ShipDataPanel::ClientLowerRight() const noexcept
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
        const GG::Clr background_colour = ClientUI::WndColor();
        const GG::Pt ul = UpperLeft(), lr = LowerRight(), cul = ClientUpperLeft();

        // title background colour and position
        const GG::Clr unselected_colour = ClientUI::WndOuterBorderColor();
        const GG::Clr selected_colour = ClientUI::WndInnerBorderColor();
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

        auto unselected_text_color = ClientUI::TextColor();
        static constexpr GG::Clr selected_text_color = GG::CLR_BLACK;

        GG::Clr text_color_to_use = m_selected ? selected_text_color : unselected_text_color;

        if (Disabled())
            text_color_to_use = DisabledColor(text_color_to_use);

        if (m_ship_name_text)
            m_ship_name_text->SetTextColor(text_color_to_use);
        if (m_design_name_text)
            m_design_name_text->SetTextColor(text_color_to_use);
    }

    void ShipDataPanel::SizeMove(GG::Pt ul, GG::Pt lr) {
        const auto old_size = Size();
        GG::Control::SizeMove(ul, lr);
        if (old_size != Size())
            RequirePreRender();
    }

    void ShipDataPanel::SetShipIcon() {
        DetachChildAndReset(m_ship_icon);
        for (auto& overlay : m_ship_icon_overlays)
            DetachChildAndReset(overlay);
        m_ship_icon_overlays.clear();
        DetachChildAndReset(m_scanline_control);

        const auto* app = GGHumanClientApp::GetApp();
        const ScriptingContext& context = app->GetContext();
        const Universe& universe = context.ContextUniverse();
        const ObjectMap& objects = context.ContextObjects();

        auto ship = objects.get<Ship>(m_ship_id);
        if (!ship)
            return;

        const ShipDesign* design = universe.GetShipDesign(ship->DesignID());
        auto icon{ClientUI::ShipDesignIcon(design ? design->ID() : INVALID_OBJECT_ID)};

        m_ship_icon = GG::Wnd::Create<GG::StaticGraphic>(std::move(icon), DATA_PANEL_ICON_STYLE);
        m_ship_icon->Resize(GG::Pt(DataPanelIconSpace().x, ClientHeight()));
        AttachChild(m_ship_icon);

        // Add the overlay
        auto add_overlay = [this](const std::string& file) {
            if (auto overlay_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / file, true)) {
                auto overlay = GG::Wnd::Create<GG::StaticGraphic>(std::move(overlay_texture),
                                                                  DATA_PANEL_ICON_STYLE);
                overlay->Resize(GG::Pt(DataPanelIconSpace().x, ClientHeight()));
                AttachChild(overlay);
                m_ship_icon_overlays.emplace_back(std::move(overlay));
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

        if ((ship->GetVisibility(app->EmpireID(), universe) < Visibility::VIS_BASIC_VISIBILITY)
            && GetOptionsDB().Get<bool>("ui.map.scanlines.shown"))
        {
            m_scanline_control = GG::Wnd::Create<ScanlineControl>(
                GG::X0, GG::Y0, m_ship_icon->Width(), m_ship_icon->Height(), true,
                GetOptionsDB().Get<GG::Clr>("ui.fleet.scanline.color"));
            AttachChild(m_scanline_control);
        }
    }

    void ShipDataPanel::Refresh() {
        m_needs_refresh = false;

        SetShipIcon();

        const auto* app = GGHumanClientApp::GetApp();
        const ScriptingContext& context = app->GetContext();
        const Universe& universe = context.ContextUniverse();
        const ObjectMap& objects = context.ContextObjects();

        auto ship = objects.get<Ship>(m_ship_id);
        if (!ship) {
            // blank text and delete icons
            m_ship_name_text->SetText("");
            DetachChildAndReset(m_design_name_text);
            for (auto& type_and_icon : m_stat_icons)
                DetachChild(type_and_icon.second);
            m_stat_icons.clear();
            return;
        }

        int empire_id = app->EmpireID();

        // name and design name update
        const std::string& ship_name = ship->PublicName(empire_id, universe);
        std::string id_name_part{GetOptionsDB().Get<bool>("ui.name.id.shown")
            ? " (" + std::to_string(m_ship_id) + ")"
            : ""};
        if (!ship->Unowned() && ship_name == UserString("FW_FOREIGN_SHIP")) {
            const auto ship_owner_empire = context.GetEmpire(ship->Owner());
            const std::string& owner_name = (ship_owner_empire ? ship_owner_empire->Name() : UserString("FW_FOREIGN"));
            m_ship_name_text->SetText(boost::io::str(FlexibleFormat(UserString("FW_EMPIRE_SHIP")) % owner_name) + id_name_part);
        } else {
            m_ship_name_text->SetText(ship_name + id_name_part);
        }

        if (m_design_name_text) {
            const ShipDesign* design = universe.GetShipDesign(ship->DesignID());
            const auto& design_name = design ? design->Name() : UserString("FW_UNKNOWN_DESIGN_NAME");
            if (!ship->SpeciesName().empty()) {
                m_design_name_text->SetText(boost::io::str(FlexibleFormat(UserString("FW_SPECIES_SHIP_DESIGN_LABEL")) %
                                                           design_name %
                                                           UserString(ship->SpeciesName())));
            } else {
                m_design_name_text->SetText(design_name);
            }
        }

        // update stat icon values and browse wnds
        for (auto& [meter_type, icon] : m_stat_icons) {
            icon->SetValue(StatValue(meter_type));

            icon->ClearBrowseInfoWnd();
            if (meter_type == MeterType::METER_CAPACITY             // refers to damage
                || meter_type == MeterType::METER_MAX_CAPACITY ) {  // refers to fighters destroyed
                icon->SetBrowseInfoWnd(GG::Wnd::Create<ShipDamageBrowseWnd>(m_ship_id, meter_type));

            } else if (meter_type == MeterType::METER_TROOPS) {
                icon->SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
                    TroopIcon(), UserString("SHIP_TROOPS_TITLE"),
                    UserString("SHIP_TROOPS_STAT")));

            } else if (meter_type == MeterType::METER_SECONDARY_STAT) {
                icon->SetBrowseInfoWnd(GG::Wnd::Create<ShipFightersBrowseWnd>(m_ship_id, meter_type));
                icon->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.extended.delay"), 1);
                icon->SetBrowseInfoWnd(GG::Wnd::Create<ShipFightersBrowseWnd>(m_ship_id, meter_type, true), 1);

            } else if (meter_type == MeterType::METER_POPULATION) {
                icon->SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
                    ColonyIcon(), UserString("SHIP_COLONY_TITLE"),
                    UserString("SHIP_COLONY_STAT")));

            } else {
                icon->SetBrowseInfoWnd(GG::Wnd::Create<MeterBrowseWnd>(
                    m_ship_id, meter_type, AssociatedMeterType(meter_type)));
            }
        }
    }

    double ShipDataPanel::StatValue(MeterType stat_name) const {
        const auto* app = GGHumanClientApp::GetApp();
        const ScriptingContext& context = app->GetContext();
        const Universe& u = context.ContextUniverse();
        const ObjectMap& o = context.ContextObjects();

        if (auto ship = o.get<Ship>(m_ship_id)) {
            if (stat_name == MeterType::METER_CAPACITY)
                return ship->TotalWeaponsShipDamage(context, 0.0f, true);
            else if (stat_name == MeterType::METER_MAX_CAPACITY) // number of fighters shot down
                return ship->TotalWeaponsFighterDamage(context, true);
            else if (stat_name == MeterType::METER_TROOPS)
                return ship->TroopCapacity(u);
            else if (stat_name == MeterType::METER_SECONDARY_STAT)
                return ship->FighterCount();
            else if (stat_name == MeterType::METER_POPULATION)
                return ship->ColonyCapacity(u);
            else if (ship->UniverseObject::GetMeter(stat_name))
                return ship->GetMeter(stat_name)->Initial();

            ErrorLogger() << "ShipDataPanel::StatValue couldn't get stat of name: " << stat_name;
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
        const GG::Pt name_lr = GG::Pt(ClientWidth() - DATA_PANEL_TEXT_PAD, LabelHeight());
        if (m_ship_name_text)
            m_ship_name_text->SizeMove(name_ul, name_lr);
        if (m_design_name_text)
            m_design_name_text->SizeMove(name_ul, name_lr);

        if (Value(ClientWidth()) < 250)
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

        const auto* app = GGHumanClientApp::GetApp();
        const ScriptingContext& context = app->GetContext();
        const Universe& universe = context.ContextUniverse();
        const ObjectMap& objects = context.ContextObjects();

        // ship name text.  blank if no ship.
        auto ship = objects.get<const Ship>(m_ship_id);
        std::string ship_name{
            (ship ? ship->Name() : "") +
            (GetOptionsDB().Get<bool>("ui.name.id.shown") ? " (" + std::to_string(m_ship_id) + ")" : "")
        };
        m_ship_name_text = GG::Wnd::Create<CUILabel>(std::move(ship_name), GG::FORMAT_LEFT);
        AttachChild(m_ship_name_text);


        // design name and statistic icons
        if (!ship)
            return;

        if (const ShipDesign* design = universe.GetShipDesign(ship->DesignID())) {
            m_design_name_text = GG::Wnd::Create<CUILabel>(design->Name(), GG::FORMAT_RIGHT);
            AttachChild(m_design_name_text);
        }


        //int tooltip_delay = GetOptionsDB().Get<int>("ui.tooltip.delay");

        std::vector<std::pair<MeterType, std::shared_ptr<GG::Texture>>> meters_icons;
        meters_icons.reserve(13);
        meters_icons.emplace_back(MeterType::METER_STRUCTURE,          ClientUI::MeterIcon(MeterType::METER_STRUCTURE));
        if (ship->IsArmed(context)) {
            meters_icons.emplace_back(MeterType::METER_CAPACITY,       DamageIcon());
            meters_icons.emplace_back(MeterType::METER_MAX_CAPACITY,   DestroyIcon()); // number of fighters shot down
        }
        if (ship->HasFighters(universe))
            meters_icons.emplace_back(MeterType::METER_SECONDARY_STAT, FightersIcon());
        if (ship->HasTroops(universe))
            meters_icons.emplace_back(MeterType::METER_TROOPS,         TroopIcon());
        if (ship->CanColonize(universe, GetSpeciesManager()))
            meters_icons.emplace_back(MeterType::METER_POPULATION,     ColonyIcon());
        if (ship->GetMeter(MeterType::METER_INDUSTRY)->Initial() != 0.0f)
            meters_icons.emplace_back(MeterType::METER_INDUSTRY,       IndustryIcon());
        if (ship->GetMeter(MeterType::METER_RESEARCH)->Initial() != 0.0f)
            meters_icons.emplace_back(MeterType::METER_RESEARCH,       ResearchIcon());
        if (ship->GetMeter(MeterType::METER_INFLUENCE)->Initial() != 0.0f)
            meters_icons.emplace_back(MeterType::METER_INFLUENCE,      InfluenceIcon());

        for (auto meter : {MeterType::METER_SHIELD,     MeterType::METER_FUEL,
                           MeterType::METER_DETECTION,  MeterType::METER_STEALTH,
                           MeterType::METER_SPEED})
        { meters_icons.emplace_back(meter, ClientUI::MeterIcon(meter)); }

        m_stat_icons.reserve(meters_icons.size());
        for (auto& [meter_type, icon_texture] : meters_icons) {
            auto icon = GG::Wnd::Create<StatisticIcon>(icon_texture, 0, 0, false,
                                                       StatIconSize().x, StatIconSize().y);
            m_stat_icons.emplace_back(meter_type, icon);

            icon->RightClickedSignal.connect([meter_type{meter_type}](GG::Pt pt) {
                auto zoom_article_action = [meter_type]() { ClientUI::GetClientUI()->ZoomToMeterTypeArticle(meter_type); };
                std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) %
                                                                        UserString(to_string(meter_type)));

                auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);
                popup->AddMenuItem(GG::MenuItem(std::move(popup_label), false, false,
                                                std::move(zoom_article_action)));

                popup->Run();
            });

            AttachChild(std::move(icon));
        }

        // bookkeeping
        m_ship_connection = ship->StateChangedSignal.connect(
            boost::bind(&ShipDataPanel::RequireRefresh, this));

        if (auto fleet = objects.get<const Fleet>(ship->FleetID()))
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
            GG::ListBox::Row(w, h),
            m_ship_id(ship_id)
        {
            SetName("ShipRow");
            SetChildClippingMode(ChildClippingMode::ClipToClient);
            if (GGHumanClientApp::GetApp()->GetContext().ContextObjects().get<const Ship>(m_ship_id))
                SetDragDropDataType(SHIP_DROP_TYPE_STRING);
        }

        void CompleteConstruction() override {
            GG::ListBox::Row::CompleteConstruction();
            m_panel = GG::Wnd::Create<ShipDataPanel>(Width(), Height(), m_ship_id);
            push_back(m_panel);
        }

        void SizeMove(GG::Pt ul, GG::Pt lr) override {
            const auto old_size = Size();
            GG::ListBox::Row::SizeMove(ul, lr);
            if (!empty() && old_size != Size() && m_panel)
                m_panel->Resize(Size());
        }

        [[nodiscard]] int ShipID() const noexcept { return m_ship_id; }

    private:
        int                             m_ship_id = INVALID_OBJECT_ID;
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
    ~FleetDataPanel() = default;

    /** Upper left plus border insets. */
    GG::Pt ClientUpperLeft() const noexcept override;

    /** Lower right minus border insets. */
    GG::Pt ClientLowerRight() const noexcept  override;

    void PreRender() override;
    void Render() override;

    void DragDropHere(GG::Pt pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
                      GG::Flags<GG::ModKey> mod_keys) override;

    void CheckDrops(GG::Pt pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
                    GG::Flags<GG::ModKey> mod_keys) override;

    void DragDropLeave() override;
    void AcceptDrops(GG::Pt pt, std::vector<std::shared_ptr<GG::Wnd>> wnds, GG::Flags<GG::ModKey> mod_keys) override;
    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    bool                Selected() const noexcept { return m_selected; }
    FleetAggression     GetFleetAggression() const noexcept { return m_new_fleet_aggression; }
    void                Select(bool b);
    void                SetSystemID(int id);

    /** Indicate fleet data has changed and needs refresh. */
    void RequireRefresh() noexcept;

    mutable boost::signals2::signal<void (const std::vector<int>&)> NewFleetFromShipsSignal;

protected:
    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) const override;

private:
    void ToggleAggression();
    void Refresh();
    void RefreshStateChangedSignals();
    void SetStatIconValues();
    void UpdateAggressionToggle();
    void DoLayout();
    void Init();
    void ColorTextForSelect();

    const int           m_fleet_id = INVALID_OBJECT_ID;
    int                 m_system_id = INVALID_OBJECT_ID;
    const bool          m_is_new_fleet_drop_target = false;
    FleetAggression     m_new_fleet_aggression = FleetAggression::FLEET_DEFENSIVE;
    bool                m_needs_refresh = true;

    boost::signals2::scoped_connection              m_fleet_connection;
    std::vector<boost::signals2::scoped_connection> m_ship_connections;

    std::shared_ptr<GG::Control>                    m_fleet_icon;
    std::shared_ptr<GG::Label>                      m_fleet_name_text;
    std::shared_ptr<GG::Label>                      m_fleet_destination_text;
    std::shared_ptr<GG::Button>                     m_aggression_toggle;
    std::vector<std::shared_ptr<GG::StaticGraphic>> m_fleet_icon_overlays;
    std::shared_ptr<ScanlineControl>                m_scanline_control;

    std::vector<std::pair<MeterType, std::shared_ptr<StatisticIcon>>> m_stat_icons; // statistic icons and associated meter types

    bool m_selected = false;
    bool m_initialized = false;
};

FleetDataPanel::FleetDataPanel(GG::X w, GG::Y h, int fleet_id) :
    Control(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS),
    m_fleet_id(fleet_id),
    m_new_fleet_aggression(NewFleetsAggressiveOptionSetting())
{
    RequireRefresh();
    SetChildClippingMode(ChildClippingMode::ClipToClient);
}

FleetDataPanel::FleetDataPanel(GG::X w, GG::Y h, int system_id, bool new_fleet_drop_target) :
    Control(GG::X0, GG::Y0, w, h, GG::INTERACTIVE),
    m_system_id(system_id),
    m_is_new_fleet_drop_target(new_fleet_drop_target),
    m_new_fleet_aggression(NewFleetsAggressiveOptionSetting())
{
    RequirePreRender();
    SetChildClippingMode(ChildClippingMode::ClipToClient);
}

GG::Pt FleetDataPanel::ClientUpperLeft() const noexcept
{ return UpperLeft() + GG::Pt(GG::X(DATA_PANEL_BORDER), GG::Y(DATA_PANEL_BORDER)); }

GG::Pt FleetDataPanel::ClientLowerRight() const noexcept
{ return LowerRight() - GG::Pt(GG::X(DATA_PANEL_BORDER), GG::Y(DATA_PANEL_BORDER));  }

void FleetDataPanel::RequireRefresh() noexcept {
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
    const GG::Clr background_colour = ClientUI::WndColor();
    const GG::Pt ul = UpperLeft(), lr = LowerRight(), cul = ClientUpperLeft();

    // title background colour and position
    const GG::Clr unselected_colour = ClientUI::WndOuterBorderColor();
    const GG::Clr selected_colour = ClientUI::WndInnerBorderColor();
    GG::Clr border_colour = m_selected ? selected_colour : unselected_colour;
    if (Disabled())
        border_colour = DisabledColor(border_colour);
    const GG::Pt text_ul = cul + GG::Pt(DataPanelIconSpace().x, GG::Y0);
    const GG::Pt text_lr = cul + GG::Pt(ClientWidth(),           LabelHeight());

    // render
    GG::FlatRectangle(ul,       lr,         background_colour,  border_colour, DATA_PANEL_BORDER);  // background and border
    GG::FlatRectangle(text_ul,  text_lr,    border_colour,      GG::CLR_ZERO,  0);                  // title background box
}

void FleetDataPanel::DragDropHere(GG::Pt pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
                                  GG::Flags<GG::ModKey> mod_keys)
{
    if (!m_is_new_fleet_drop_target) {
        // normally the containing row (or the listbox that contains that) will
        // handle drag-drop related things
        ForwardEventToParent();
    }

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

void FleetDataPanel::CheckDrops(GG::Pt pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
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
{ Select(false); }

void FleetDataPanel::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                                     GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) const
{
    if (!m_is_new_fleet_drop_target) {
        // reject all
        Wnd::DropsAcceptable(first, last, pt, mod_keys);
        return;
    }

    // only used when FleetDataPanel sets independently in the FleetWnd, not
    // in a FleetListBox

    const auto* app = GGHumanClientApp::GetApp();
    const auto& objects = app->GetContext().ContextObjects();
    int this_client_empire_id = GGHumanClientApp::GetApp()->EmpireID();

    auto this_panel_fleet = objects.get<const Fleet>(m_fleet_id);

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
        const ShipRow* ship_row = dynamic_cast<const ShipRow*>(it->first);
        if (!ship_row)
            continue;
        auto ship = objects.get<const Ship>(ship_row->ShipID());
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

void FleetDataPanel::AcceptDrops(GG::Pt pt, std::vector<std::shared_ptr<GG::Wnd>> wnds, GG::Flags<GG::ModKey> mod_keys) {
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
        if (const ShipRow* ship_row = dynamic_cast<const ShipRow*>(wnd.get()))
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

void FleetDataPanel::SizeMove(GG::Pt ul, GG::Pt lr) {
    const auto old_size = Size();
    GG::Control::SizeMove(ul, lr);
    if (old_size != Size())
        DoLayout();
}

void FleetDataPanel::ToggleAggression() {
    if (!m_aggression_toggle)
        return;

    ScriptingContext& context = IApp::GetApp()->GetContext();
    if (auto fleet = context.ContextObjects().get<const Fleet>(m_fleet_id)) {
        if (ClientPlayerIsModerator())
            return; // TODO: handle moderator actions for this...

        auto* app = GGHumanClientApp::GetApp();
        int client_empire_id = app->EmpireID();
        if (client_empire_id == ALL_EMPIRES)
            return;

        FleetAggression new_aggression_state = [old_aggression{fleet->Aggression()}]() {
            switch (old_aggression) {
            case FleetAggression::FLEET_PASSIVE:        return FleetAggression::FLEET_DEFENSIVE;    break;
            case FleetAggression::FLEET_DEFENSIVE:      return FleetAggression::FLEET_OBSTRUCTIVE;  break;
            case FleetAggression::FLEET_OBSTRUCTIVE:    return FleetAggression::FLEET_AGGRESSIVE;   break;
            case FleetAggression::FLEET_AGGRESSIVE:     return FleetAggression::FLEET_PASSIVE;      break;
            default:                                    return FleetAggression::FLEET_DEFENSIVE;    break;
            }
        }();

        // toggle fleet aggression status
        context.ContextUniverse().InhibitUniverseObjectSignals(true);
        app->Orders().IssueOrder<AggressiveOrder>(context, client_empire_id, m_fleet_id, new_aggression_state);
        context.ContextUniverse().InhibitUniverseObjectSignals(false);
        UpdateAggressionToggle();

    } else if (m_is_new_fleet_drop_target) {
        // cycle new fleet aggression
        m_new_fleet_aggression = [old_aggression{m_new_fleet_aggression}]() {
            switch (old_aggression) {
            case FleetAggression::FLEET_AGGRESSIVE:         return FleetAggression::FLEET_OBSTRUCTIVE;          break;
            case FleetAggression::FLEET_OBSTRUCTIVE:        return FleetAggression::FLEET_DEFENSIVE;            break;
            case FleetAggression::FLEET_DEFENSIVE:          return FleetAggression::FLEET_PASSIVE;              break;
            case FleetAggression::FLEET_PASSIVE:            return FleetAggression::INVALID_FLEET_AGGRESSION;   break;
            case FleetAggression::INVALID_FLEET_AGGRESSION:
            default:                                        return FleetAggression::FLEET_AGGRESSIVE;           break;
            }
        }();

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

    const ScriptingContext& context = IApp::GetApp()->GetContext();
    const Universe& u = context.ContextUniverse();
    const ObjectMap& o = context.ContextObjects();
    const EmpireManager& e = context.Empires();

    if (m_is_new_fleet_drop_target) {
        m_fleet_name_text->SetText(UserString("FW_NEW_FLEET_LABEL"));
        m_fleet_destination_text->Clear();

        auto new_fleet_texture = ClientUI::GetTexture(
            ClientUI::ArtDir() / "icons" / "buttons" / "new_fleet.png", true);
        m_fleet_icon = GG::Wnd::Create<GG::StaticGraphic>(
            std::move(new_fleet_texture), DATA_PANEL_ICON_STYLE);
        AttachChild(m_fleet_icon);

    } else if (auto fleet = o.get<Fleet>(m_fleet_id)) {
        int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();
        // set fleet name and destination text
        std::string public_fleet_name = fleet->PublicName(client_empire_id, u);
        if (!fleet->Unowned() && public_fleet_name == UserString("FW_FOREIGN_FLEET")) {
            auto ship_owner_empire = e.GetEmpire(fleet->Owner());
            const std::string& owner_name = (ship_owner_empire ? ship_owner_empire->Name() : UserString("FW_FOREIGN"));
            std::string fleet_name = boost::io::str(FlexibleFormat(UserString("FW_EMPIRE_FLEET")) % owner_name);
            if (GetOptionsDB().Get<bool>("ui.name.id.shown"))
                fleet_name = fleet_name + " (" + std::to_string(m_fleet_id) + ")";
            m_fleet_name_text->SetText(std::move(fleet_name));
        } else {
            if (GetOptionsDB().Get<bool>("ui.name.id.shown"))
                public_fleet_name = public_fleet_name + " (" + std::to_string(m_fleet_id) + ")";
            m_fleet_name_text->SetText(std::move(public_fleet_name));
        }
        m_fleet_destination_text->SetText(FleetDestinationText(m_fleet_id, context));

        // set icons
        std::vector<std::shared_ptr<GG::Texture>> icons{
            FleetHeadIcons(fleet.get(), FleetButton::SizeType::LARGE)};
        icons.emplace_back(FleetSizeIcon(fleet.get(), FleetButton::SizeType::LARGE));
        std::vector<GG::Flags<GG::GraphicStyle>> styles(icons.size(), DATA_PANEL_ICON_STYLE);

        m_fleet_icon = GG::Wnd::Create<MultiTextureStaticGraphic>(std::move(icons), std::move(styles));
        AttachChild(m_fleet_icon);

        if (auto fleet_owner_empire = e.GetEmpire(fleet->Owner()))
            m_fleet_icon->SetColor(fleet_owner_empire->Color());
        else if (fleet->Unowned() && fleet->HasMonsters(u))
            m_fleet_icon->SetColor(GG::CLR_RED);

        auto all_ships = [&fleet, &o](const std::function<bool(const std::shared_ptr<const Ship>&)>& pred) {
            // Searching for each Ship one at a time is probably faster than
            // FindObjects(ship_ids), because an early exit avoids searching the
            // remaining ids.
            const auto& ship_ids{fleet->ShipIDs()};
            return std::all_of(
                ship_ids.begin(), ship_ids.end(),
                [&pred, &o](const int ship_id) {
                    auto ship = o.get<Ship>(ship_id);
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
                auto overlay = GG::Wnd::Create<GG::StaticGraphic>(std::move(overlay_texture),
                                                                  DATA_PANEL_ICON_STYLE);
                overlay->Resize(GG::Pt(DataPanelIconSpace().x, ClientHeight()));
                AttachChild(overlay);
                m_fleet_icon_overlays.emplace_back(std::move(overlay));
            }
        };

        // Add overlays for all ships colonizing, invading etc.
        if (all_ships([](const std::shared_ptr<const Ship>& ship) { return ship->OrderedScrapped(); }))
            add_overlay("scrapped.png");

        if (all_ships([](const std::shared_ptr<const Ship>& ship)
                      { return ship->OrderedColonizePlanet() != INVALID_OBJECT_ID; }))
        { add_overlay("colonizing.png"); }

        if (all_ships([](const std::shared_ptr<const Ship>& ship)
                      { return ship->OrderedInvadePlanet() != INVALID_OBJECT_ID; }))
        { add_overlay("invading.png"); }

        if (all_ships([](const std::shared_ptr<const Ship>& ship)
                      { return ship->OrderedBombardPlanet() != INVALID_OBJECT_ID; }))
        { add_overlay("bombarding.png"); }

        // Moving fleets can't be gifted.  The order will be automatically
        // cancelled on the server.  This make the UI appear to cancel the
        // order when the ship is moved without requiring the player to
        // re-order the gifting if the ship is stopped.
        if (fleet->OrderedGivenToEmpire() != ALL_EMPIRES && fleet->TravelRoute().empty())
            add_overlay("gifting.png");

        if ((fleet->GetVisibility(client_empire_id, u) < Visibility::VIS_BASIC_VISIBILITY)
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
    m_ship_connections.clear();

    const auto& objects = GGHumanClientApp::GetApp()->GetContext().ContextObjects();

    auto fleet = objects.get<Fleet>(m_fleet_id);
    if (!fleet)
        return;

    m_fleet_connection = fleet->StateChangedSignal.connect(
        boost::bind(&FleetDataPanel::RequireRefresh, this));

    m_ship_connections.reserve(fleet->NumShips());
    for (auto& ship : objects.find<const Ship>(fleet->ShipIDs()))
        m_ship_connections.emplace_back(
            ship->StateChangedSignal.connect(
                boost::bind(&FleetDataPanel::RequireRefresh, this)));
}

void FleetDataPanel::SetStatIconValues() {
    const auto* app = GGHumanClientApp::GetApp();
    int client_empire_id = app->EmpireID();
    const ScriptingContext& context = app->GetContext();
    const Universe& universe = context.ContextUniverse();
    const ObjectMap& objects = context.ContextObjects();

    const auto& this_client_known_destroyed_objects = universe.EmpireKnownDestroyedObjectIDs(client_empire_id);
    const auto& this_client_stale_object_info = universe.EmpireStaleKnowledgeObjectIDs(client_empire_id);
    int ship_count =        0;
    float damage_tally  =   0.0f;
    float destroy_tally =   0.0f;
    float fighters_tally  = 0.0f;
    float structure_tally = 0.0f;
    float shield_tally =    0.0f;
    float min_fuel =        0.0f;
    float min_speed =       0.0f;
    float troops_tally =    0.0f;
    float colony_tally =    0.0f;
    std::vector<float> fuels;
    std::vector<float> speeds;

    auto fleet = objects.get<Fleet>(m_fleet_id);

    fuels.reserve(fleet->NumShips());
    speeds.reserve(fleet->NumShips());
    for (const auto& ship : objects.find<const Ship>(fleet->ShipIDs())) {
        int ship_id = ship->ID();
        // skip known destroyed and stale info objects
        if (this_client_known_destroyed_objects.contains(ship_id))
            continue;
        if (this_client_stale_object_info.contains(ship_id))
            continue;

        if (universe.GetShipDesign(ship->DesignID())) {
            ship_count++;
            damage_tally += ship->TotalWeaponsShipDamage(context, 0.0f, true);
            destroy_tally += ship->TotalWeaponsFighterDamage(context, true);
            fighters_tally += ship->FighterCount();
            troops_tally += ship->TroopCapacity(universe);
            colony_tally += ship->ColonyCapacity(universe);
            structure_tally += ship->GetMeter(MeterType::METER_STRUCTURE)->Initial();
            shield_tally += ship->GetMeter(MeterType::METER_SHIELD)->Initial();
            fuels.push_back(ship->GetMeter(MeterType::METER_FUEL)->Initial());
            speeds.push_back(ship->GetMeter(MeterType::METER_SPEED)->Initial());
        }
    }
    if (!fuels.empty())
        min_fuel = *std::min_element(fuels.cbegin(), fuels.cend());
    if (!speeds.empty())
        min_speed = *std::min_element(speeds.cbegin(), speeds.cend());

    for (auto& [stat_name, icon] : m_stat_icons) {
        DetachChild(icon);
        switch (stat_name) {
        case MeterType::METER_SIZE:
            icon->SetValue(ship_count);
            AttachChild(icon);
            break;
        case MeterType::METER_CAPACITY:
            icon->SetValue(damage_tally);
            if (fleet->CanDamageShips(context))
                AttachChild(icon);
            break;
        case MeterType::METER_MAX_CAPACITY:
            icon->SetValue(destroy_tally);
            if (fleet->CanDestroyFighters(context))
                AttachChild(icon);
            break;
        case MeterType::METER_SECONDARY_STAT:
            icon->SetValue(fighters_tally);
            if (fleet->HasFighterShips(universe))
                AttachChild(icon);
            break;
        case MeterType::METER_TROOPS:
            icon->SetValue(troops_tally);
            if (fleet->HasTroopShips(universe))
                AttachChild(icon);
            break;
        case MeterType::METER_POPULATION:
            icon->SetValue(colony_tally);
            if (fleet->HasColonyShips(universe))
                AttachChild(icon);
            break;
        case MeterType::METER_INDUSTRY: {
            const auto resource_output = fleet->ResourceOutput(ResourceType::RE_INDUSTRY, objects);
            icon->SetValue(resource_output);
            if (resource_output != 0.0f)
                AttachChild(icon);
        }
            break;
        case MeterType::METER_RESEARCH: {
            const auto resource_output = fleet->ResourceOutput(ResourceType::RE_RESEARCH, objects);
            icon->SetValue(resource_output);
            if (resource_output != 0.0f)
                AttachChild(icon);
        }
            break;
        case MeterType::METER_INFLUENCE: {
            const auto resource_output = fleet->ResourceOutput(ResourceType::RE_INFLUENCE, objects);
            icon->SetValue(resource_output);
            if (resource_output != 0.0f)
                AttachChild(icon);
        }
            break;
        case MeterType::METER_STRUCTURE:
            icon->SetValue(structure_tally);
            AttachChild(icon);
            break;
        case MeterType::METER_SHIELD:
            icon->SetValue(shield_tally/ship_count);
            AttachChild(icon);
            break;
        case MeterType::METER_FUEL:
            icon->SetValue(min_fuel);
            AttachChild(icon);
            break;
        case MeterType::METER_SPEED:
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

    FleetAggression aggression = FleetAggression::FLEET_AGGRESSIVE;

    if (m_is_new_fleet_drop_target) {
        aggression = m_new_fleet_aggression;
    } else if (auto fleet = GGHumanClientApp::GetApp()->GetContext().ContextObjects().get<Fleet>(m_fleet_id)) {
        aggression = fleet->Aggression();
    } else {
        DetachChild(m_aggression_toggle);
        return;
    }

    if (aggression == FleetAggression::FLEET_AGGRESSIVE) {
        m_aggression_toggle->SetUnpressedGraphic(GG::SubTexture(FleetAggressiveIcon()));
        m_aggression_toggle->SetPressedGraphic(GG::SubTexture(FleetAggressiveIcon()));
        m_aggression_toggle->SetRolloverGraphic(GG::SubTexture(FleetAggressiveMouseoverIcon()));
        m_aggression_toggle->SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
            FleetAggressiveIcon(), UserString("FW_AGGRESSIVE"), UserString("FW_AGGRESSIVE_DESC")));

    } else if (aggression == FleetAggression::FLEET_OBSTRUCTIVE) {
        m_aggression_toggle->SetUnpressedGraphic(GG::SubTexture(FleetObstructiveIcon()));
        m_aggression_toggle->SetPressedGraphic(GG::SubTexture(FleetObstructiveIcon()));
        m_aggression_toggle->SetRolloverGraphic(GG::SubTexture(FleetObstructiveMouseoverIcon()));
        m_aggression_toggle->SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
            FleetObstructiveIcon(), UserString("FW_OBSTRUCTIVE"), UserString("FW_OBSTRUCTIVE_DESC")));

    } else if (aggression == FleetAggression::FLEET_DEFENSIVE) {
        m_aggression_toggle->SetUnpressedGraphic(GG::SubTexture(FleetDefensiveIcon()));
        m_aggression_toggle->SetPressedGraphic(GG::SubTexture(FleetDefensiveIcon()));
        m_aggression_toggle->SetRolloverGraphic(GG::SubTexture(FleetDefensiveMouseoverIcon()));
        m_aggression_toggle->SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
            FleetDefensiveIcon(), UserString("FW_DEFENSIVE"), UserString("FW_DEFENSIVE_DESC")));

    } else if (aggression == FleetAggression::FLEET_PASSIVE) {
        m_aggression_toggle->SetUnpressedGraphic(GG::SubTexture(FleetPassiveIcon()));
        m_aggression_toggle->SetPressedGraphic(GG::SubTexture(FleetPassiveIcon()));
        m_aggression_toggle->SetRolloverGraphic(GG::SubTexture(FleetPassiveMouseoverIcon()));
        m_aggression_toggle->SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
            FleetPassiveIcon(), UserString("FW_PASSIVE"), UserString("FW_PASSIVE_DESC")));

    } else {    // aggression == FleetAggression::INVALID_FLEET_AGGRESSION
        m_aggression_toggle->SetUnpressedGraphic(GG::SubTexture(FleetAutoIcon()));
        m_aggression_toggle->SetPressedGraphic(GG::SubTexture(FleetAutoIcon()));
        m_aggression_toggle->SetRolloverGraphic(GG::SubTexture(FleetAutoMouseoverIcon()));
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

    if (Value(ClientWidth()) < 250)
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

    const auto* app = GGHumanClientApp::GetApp();
    int client_empire_id = app->EmpireID();
    const auto& context = app->GetContext();
    const auto& u = context.ContextUniverse();
    const auto& o = context.ContextObjects();

    if (m_fleet_id == INVALID_OBJECT_ID) {
        m_aggression_toggle = Wnd::Create<CUIButton>(
            GG::SubTexture(FleetAggressiveIcon()),
            GG::SubTexture(FleetPassiveIcon()),
            GG::SubTexture(FleetAggressiveMouseoverIcon()));
        AttachChild(m_aggression_toggle);
        m_aggression_toggle->LeftClickedSignal.connect(
            boost::bind(&FleetDataPanel::ToggleAggression, this));

    } else if (auto fleet = o.get<const Fleet>(m_fleet_id)) {
        std::vector<std::tuple<MeterType, std::shared_ptr<GG::Texture>, const char*>> meters_icons_browsetext{
            {MeterType::METER_SIZE,           FleetCountIcon(),                                "FW_FLEET_COUNT_SUMMARY"},
            {MeterType::METER_CAPACITY,       DamageIcon(),                                    "FW_FLEET_DAMAGE_SUMMARY"},
            {MeterType::METER_MAX_CAPACITY,   DestroyIcon(), /* number of destroyed fighters */"FW_FLEET_DESTROY_SUMMARY"},
            {MeterType::METER_SECONDARY_STAT, FightersIcon(),                                  "FW_FLEET_FIGHTER_SUMMARY"},
            {MeterType::METER_TROOPS,         TroopIcon(),                                     "FW_FLEET_TROOP_SUMMARY"},
            {MeterType::METER_POPULATION,     ColonyIcon(),                                    "FW_FLEET_COLONY_SUMMARY"},
            {MeterType::METER_INDUSTRY,       IndustryIcon(),                                  "FW_FLEET_INDUSTRY_SUMMARY"},
            {MeterType::METER_RESEARCH,       ResearchIcon(),                                  "FW_FLEET_RESEARCH_SUMMARY"},
            {MeterType::METER_INFLUENCE,      InfluenceIcon(),                                 "FW_FLEET_INFLUENCE_SUMMARY"},
            {MeterType::METER_STRUCTURE,      ClientUI::MeterIcon(MeterType::METER_STRUCTURE), "FW_FLEET_STRUCTURE_SUMMARY"},
            {MeterType::METER_SHIELD,         ClientUI::MeterIcon(MeterType::METER_SHIELD),    "FW_FLEET_SHIELD_SUMMARY"},
            {MeterType::METER_FUEL,           ClientUI::MeterIcon(MeterType::METER_FUEL),      "FW_FLEET_FUEL_SUMMARY"},
            {MeterType::METER_SPEED,          ClientUI::MeterIcon(MeterType::METER_SPEED),     "FW_FLEET_SPEED_SUMMARY"}};

        m_stat_icons.reserve(meters_icons_browsetext.size());
        for (auto& [meter_type, icon, text] : meters_icons_browsetext) {
            auto stat_icon = GG::Wnd::Create<StatisticIcon>(icon, 0, 0, false,
                                                            StatIconSize().x, StatIconSize().y);

            m_stat_icons.emplace_back(meter_type, stat_icon);
            stat_icon->SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
                std::move(icon), UserString(to_string(meter_type)), UserString(text)));

            stat_icon->RightClickedSignal.connect([meter_type{meter_type}](GG::Pt pt){
                auto zoom_article_action = [meter_type]() { ClientUI::GetClientUI()->ZoomToMeterTypeArticle(meter_type); };

                auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);

                std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) %
                                                                        UserString(to_string(meter_type)));
                popup->AddMenuItem(GG::MenuItem(std::move(popup_label), false, false,
                                                zoom_article_action));
                popup->Run();
            });
            AttachChild(std::move(stat_icon));
        }


        if (fleet->OwnedBy(client_empire_id) || fleet->GetVisibility(client_empire_id, u) >= Visibility::VIS_FULL_VISIBILITY) {
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
    const GG::Clr unselected_text_color = ClientUI::TextColor();
    static constexpr GG::Clr selected_text_color = GG::CLR_BLACK;

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
            GG::ListBox::Row(w, h),
            m_fleet_id(fleet_id)
        {
            if (GGHumanClientApp::GetApp()->GetContext().ContextObjects().getRaw<Fleet>(fleet_id))
                SetDragDropDataType(FLEET_DROP_TYPE_STRING);
            SetName("FleetRow");
            SetChildClippingMode(ChildClippingMode::ClipToClient);
        }

        void CompleteConstruction() override {
            GG::ListBox::Row::CompleteConstruction();
            m_panel = GG::Wnd::Create<FleetDataPanel>(Width(), Height(), m_fleet_id);
            push_back(m_panel);
        }

        void SizeMove(GG::Pt ul, GG::Pt lr) override {
            const auto old_size = Size();
            GG::ListBox::Row::SizeMove(ul, lr);
            if (!empty() && old_size != Size() && m_panel)
                m_panel->Resize(Size());
        }

        int  FleetID() const { return m_fleet_id; }

    private:
        int                             m_fleet_id = INVALID_OBJECT_ID;
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

    void EnableOrderIssuing(bool enable) {
        m_order_issuing_enabled = enable;
    }

    void AcceptDrops(GG::Pt pt, std::vector<std::shared_ptr<GG::Wnd>> wnds,
                     GG::Flags<GG::ModKey> mod_keys) override
    {
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
        const FleetRow* drop_target_fleet_row = dynamic_cast<const FleetRow*>(drop_target_row->get());
        if (!drop_target_fleet_row) {
            ErrorLogger() << "FleetsListBox::AcceptDrops  drop target not a fleet row. aborting";
            return;
        }

        auto* app = GGHumanClientApp::GetApp();
        ScriptingContext& context = app->GetContext();

        int target_fleet_id = drop_target_fleet_row->FleetID();
        if (!context.ContextObjects().getRaw<const Fleet>(target_fleet_id)) {
            ErrorLogger() << "FleetsListBox::AcceptDrops  unable to get target fleet with id: " << target_fleet_id;
            return;
        }


        // sort dropped Wnds to extract fleets or ships dropped.  (should only be one or the other in a given drop)
        std::vector<std::shared_ptr<const Fleet>> dropped_fleets;
        dropped_fleets.reserve(wnds.size());
        std::vector<std::shared_ptr<const Ship>> dropped_ships;
        dropped_ships.reserve(wnds.size());

        //DebugLogger() << "... getting/sorting dropped fleets or ships...";
        for (const auto& wnd : wnds) {
            if (drop_target_fleet_row == wnd.get()) {
                ErrorLogger() << "FleetsListBox::AcceptDrops  dropped wnd is same as drop target?! skipping";
                continue;
            }

            if (wnd->DragDropDataType() == FLEET_DROP_TYPE_STRING) {
                const FleetRow* fleet_row = dynamic_cast<const FleetRow*>(wnd.get());
                if (!fleet_row) {
                    ErrorLogger() << "FleetsListBox::AcceptDrops  unable to get fleet row from dropped wnd";
                    continue;
                }
                dropped_fleets.push_back(context.ContextObjects().get<const Fleet>(fleet_row->FleetID()));

            } else if (wnd->DragDropDataType() == SHIP_DROP_TYPE_STRING) {
                const ShipRow* ship_row = dynamic_cast<const ShipRow*>(wnd.get());
                if (!ship_row) {
                    ErrorLogger() << "FleetsListBox::AcceptDrops  unable to get ship row from dropped wnd";
                    continue;
                }
                dropped_ships.push_back(context.ContextObjects().get<const Ship>(ship_row->ShipID()));
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
            app->Orders().IssueOrder<FleetTransferOrder>(context, app->EmpireID(), target_fleet_id, ship_ids);
    }

    void DragDropHere(GG::Pt pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
                      GG::Flags<GG::ModKey> mod_keys) override
    {
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

        FleetDataPanel* drop_target_data_panel = dynamic_cast<FleetDataPanel*>(control);
        if (!drop_target_data_panel)
            return;

        if (drop_target_data_panel->Selected())
            return;

        FleetRow* drop_target_fleet_row = dynamic_cast<FleetRow*>(drop_target_row.get());
        if (!drop_target_fleet_row)
            return;

        const auto* app = GGHumanClientApp::GetApp();
        const ScriptingContext& context = app->GetContext();
        const auto& objects = context.ContextObjects();

        const auto drop_target_fleet = objects.get<Fleet>(drop_target_fleet_row->FleetID());
        if (!drop_target_fleet)
            return;


        // get whether each Wnd is dropable
        DropsAcceptable(drop_wnds_acceptable.begin(), drop_wnds_acceptable.end(), pt, mod_keys);


        // scan through results in drops_acceptable_map and decide whether overall
        // drop is acceptable.  to be acceptable, all wnds must individually be
        // acceptable for dropping, and there must not be a mix of ships and fleets
        // being dropped.
        bool fleets_seen = false;
        bool ships_seen = false;

        for (const auto& [dropped_wnd, is_acceptable] : drop_wnds_acceptable) {
            if (!is_acceptable)
                return; // a row was an invalid drop. abort without highlighting drop target.

            if (dropped_wnd->DragDropDataType() == FLEET_DROP_TYPE_STRING) {
                fleets_seen = true;
                if (ships_seen)
                    return; // can't drop both at once

                const FleetRow* fleet_row = dynamic_cast<const FleetRow*>(dropped_wnd);
                if (!fleet_row)
                    return;
                const auto fleet = objects.get<Fleet>(fleet_row->FleetID());

                if (!ValidFleetMerge(fleet, drop_target_fleet))
                    return; // not a valid drop

            } else if (dropped_wnd->DragDropDataType() == SHIP_DROP_TYPE_STRING) {
                ships_seen = true;
                if (fleets_seen)
                    return; // can't drop both at once

                const ShipRow* ship_row = dynamic_cast<const ShipRow*>(dropped_wnd);
                if (!ship_row)
                    return;
                const auto ship = objects.get<Ship>(ship_row->ShipID());

                if (!ValidShipTransfer(ship, drop_target_fleet, objects))
                    return; // not a valid drop
            }
        }

        // passed all checks.  drop is valid!
        HighlightRow(row_it);
    }

    void DragDropLeave() override {
        CUIListBox::DragDropLeave();
        ClearHighlighting();
    }

    void SizeMove(GG::Pt ul, GG::Pt lr) override {
        const auto old_size = Size();
        CUIListBox::SizeMove(ul, lr);
        if (old_size != Size()) {
            const GG::Pt row_size = ListRowSize();
            for (auto& row : *this)
                row->Resize(row_size);
        }
    }

    GG::Pt ListRowSize() const
    { return GG::Pt(Width() - RightMargin() - 5, ListRowHeight()); }

protected:
    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) const override
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

        const auto* app = GGHumanClientApp::GetApp();
        const auto& objects = app->GetContext().ContextObjects();

        // extract drop target fleet from row under drop point
        const FleetRow* target_fleet_row = dynamic_cast<const FleetRow*>(row->get());
        std::shared_ptr<const Fleet> target_fleet;
        if (target_fleet_row)
            target_fleet = objects.get<Fleet>(target_fleet_row->FleetID());

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
                if (const FleetRow* fleet_row = dynamic_cast<const FleetRow*>(it->first))
                    if (auto fleet = objects.get<Fleet>(fleet_row->FleetID()))
                        it->second = ValidFleetMerge(fleet, target_fleet);

            } else if (it->first->DragDropDataType() == SHIP_DROP_TYPE_STRING) {
                if (const ShipRow* ship_row = dynamic_cast<const ShipRow*>(it->first))
                    if (auto ship = objects.get<Ship>(ship_row->ShipID()))
                        it->second = ValidShipTransfer(ship, target_fleet, objects);
            } else {
                // no valid drop type string
                ErrorLogger() << "FleetsListBox unrecognized drop type: " << it->first->DragDropDataType();
            }
        }
    }

private:
    void HighlightRow(iterator row_it) {
        if (row_it == end())
            return;

        if (row_it == m_highlighted_row_it)
            return;

        // get FleetDataPanel of row pointed to by row_it
        auto& selected_row = *row_it;
        assert(selected_row);
        assert(!selected_row->empty());
        GG::Control* control = !selected_row->empty() ? selected_row->at(0) : nullptr;
        FleetDataPanel* data_panel = dynamic_cast<FleetDataPanel*>(control);
        if (!data_panel)
            return;

        // don't need to select and shouldn't store as highlighted if row is actually already selected in ListBox itself
        if (data_panel->Selected())
            return;

        // mark data panel selected, which indicates highlighting
        data_panel->Select(true);
        m_highlighted_row_it = row_it;
    }

    void ClearHighlighting() {
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

        FleetDataPanel* data_panel = dynamic_cast<FleetDataPanel*>(control);
        if (!data_panel) {
            ErrorLogger() << "FleetsListBox::ClearHighlighting : no data panel!";
            return;
        }

        data_panel->Select(false);
        m_highlighted_row_it = end();
    }

    void InitRowSizes() {
        SetNumCols(1);
        ManuallyManageColProps();
    }

    iterator m_highlighted_row_it;
    bool     m_order_issuing_enabled = false;
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

    void Refresh() {
        ScopedTimer timer("ShipsListBox::Refresh");

        const auto* app = GGHumanClientApp::GetApp();
        const ScriptingContext& context = app->GetContext();
        const auto& objects = context.ContextObjects();
        const auto& universe = context.ContextUniverse();

        auto fleet = objects.get<Fleet>(m_fleet_id);
        if (!fleet) {
            Clear();
            return;
        }

        const GG::Pt row_size = ListRowSize();
        Clear();

        // repopulate list with ships in current fleet

        SetNumCols(1);
        ManuallyManageColProps();

        int this_client_empire_id = app->EmpireID();
        const auto& this_client_known_destroyed_objects = universe.EmpireKnownDestroyedObjectIDs(this_client_empire_id);
        const auto& this_client_stale_object_info = universe.EmpireStaleKnowledgeObjectIDs(this_client_empire_id);

        const auto& ship_ids = fleet->ShipIDs();
        std::vector<std::shared_ptr<GG::ListBox::Row>> rows;
        rows.reserve(ship_ids.size());
        for (int ship_id : ship_ids) {
            // skip known destroyed and stale info objects
            if (this_client_known_destroyed_objects.contains(ship_id))
                continue;
            if (this_client_stale_object_info.contains(ship_id))
                continue;

            rows.push_back(GG::Wnd::Create<ShipRow>(GG::X1, row_size.y, ship_id));
        }
        Insert(rows);
        for (auto& row : rows)
            row->Resize(row_size);

        SelRowsChangedSignal(this->Selections());
    }

    void SetFleet(int fleet_id) {
        if (m_fleet_id == fleet_id)
            return;

        m_fleet_id = fleet_id;
        Refresh();
    }

    void EnableOrderIssuing(bool enable) {
        m_order_issuing_enabled = enable;
    }

    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) const override
    {
        const auto* app = GGHumanClientApp::GetApp();
        const auto& objects = app->GetContext().ContextObjects();

        for (DropsAcceptableIter it = first; it != last; ++it) {
            it->second = false; // default

            if (!m_order_issuing_enabled)
                continue;

            const auto& ship_row = dynamic_cast<const ShipRow*>(it->first);
            if (!ship_row)
                continue;

            const auto ship = objects.get<Ship>(ship_row->ShipID());
            if (!ship) {
                ErrorLogger() << "ShipsListBox::DropsAcceptable couldn't get ship for ship row";
                continue;
            }

            const auto fleet = objects.get<Fleet>(ship->FleetID());
            if (!fleet) {
                ErrorLogger() << "ShipsListBox::DropsAcceptable couldn't get fleet with id " << ship->FleetID();
                continue;
            }

            if (ship && ValidShipTransfer(ship, fleet, objects))
                continue;   // leave false: ship transfer not valid

            // all tests passed; can drop
            it->second = true;
        }
    }

    void AcceptDrops(GG::Pt pt, std::vector<std::shared_ptr<GG::Wnd>> wnds,
                     GG::Flags<GG::ModKey> mod_keys) override
    {
        if (wnds.empty())
            return;

        std::vector<int> ship_ids;
        ship_ids.reserve(wnds.size());
        for (const auto& wnd : wnds) {
            if (wnd->DragDropDataType() == SHIP_DROP_TYPE_STRING) {
                if (const ShipRow* ship_row = dynamic_cast<const ShipRow*>(wnd.get()))
                    ship_ids.push_back(ship_row->ShipID());
            }
        }

        auto* app = GGHumanClientApp::GetApp();
        ScriptingContext& context = app->GetContext();
        if (!context.ContextObjects().check_if_any<const Ship>(ship_ids))
            return;
        if (ClientPlayerIsModerator())
            return; // TODO: handle moderator actions for this...

        int empire_id = app->EmpireID();
        app->Orders().IssueOrder<FleetTransferOrder>(context, empire_id, m_fleet_id, ship_ids);
    }

    void SizeMove(GG::Pt ul, GG::Pt lr) override {
        const auto old_size = Size();
        CUIListBox::SizeMove(ul, lr);
        if (old_size != Size()) {
            const GG::Pt row_size = ListRowSize();
            for (auto& row : *this)
                row->Resize(row_size);
        }
    }

    GG::Pt ListRowSize() const
    { return GG::Pt(Width() - 5 - RightMargin(), ListRowHeight()); }

private:
    int  m_fleet_id = INVALID_OBJECT_ID;
    bool m_order_issuing_enabled = false;
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
    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    int             FleetID() const;
    std::set<int>   SelectedShipIDs() const;    ///< returns ids of ships selected in the detail panel's ShipsListBox
    void            SetFleet(int fleet_id);                         ///< sets the currently-displayed Fleet.  setting to INVALID_OBJECT_ID shows no fleet
    void            SelectShips(const std::set<int>& ship_ids);///< sets the currently-selected ships in the ships list

    void Refresh();
    void EnableOrderIssuing(bool enabled = true);

    /** emitted when the set of selected ships changes */
    mutable boost::signals2::signal<void (const ShipsListBox::SelectionSet&)> SelectedShipsChangedSignal;
    mutable boost::signals2::signal<void (int)>                               ShipRightClickedSignal;

private:
    int  GetShipIDOfListRow(GG::ListBox::iterator it) const; ///< returns the ID number of the ship in row \a row_idx of the ships listbox
    void DoLayout();
    void UniverseObjectDeleted(const std::shared_ptr<const UniverseObject>& obj);
    void ShipSelectionChanged(const GG::ListBox::SelectionSet& rows);
    void ShipRightClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys);
    int  ShipInRow(GG::ListBox::iterator it) const;

    int                                m_fleet_id = INVALID_OBJECT_ID;
    bool                               m_order_issuing_enabled = false;
    boost::signals2::scoped_connection m_fleet_connection;
    std::shared_ptr<ShipsListBox>      m_ships_lb;
};

FleetDetailPanel::FleetDetailPanel(GG::X w, GG::Y h, int fleet_id, bool order_issuing_enabled,
                                   GG::Flags<GG::WndFlag> flags) :
    GG::Wnd(GG::X0, GG::Y0, w, h, flags),
    m_order_issuing_enabled(order_issuing_enabled)
{
    SetName("FleetDetailPanel");
    SetChildClippingMode(ChildClippingMode::ClipToClient);

    m_ships_lb = GG::Wnd::Create<ShipsListBox>(m_fleet_id, order_issuing_enabled);
    m_ships_lb->SetHiliteColor(GG::CLR_ZERO);

    SetFleet(fleet_id);

    if (!m_order_issuing_enabled) {
        m_ships_lb->SetStyle(GG::LIST_NOSEL | GG::LIST_BROWSEUPDATES);
    } else {
        m_ships_lb->SetStyle(GG::LIST_QUICKSEL | GG::LIST_BROWSEUPDATES);
        m_ships_lb->AllowDropType(SHIP_DROP_TYPE_STRING);
    }

    namespace ph = boost::placeholders;

    m_ships_lb->SelRowsChangedSignal.connect(
        boost::bind(&FleetDetailPanel::ShipSelectionChanged, this, ph::_1));
    m_ships_lb->RightClickedRowSignal.connect(
        boost::bind(&FleetDetailPanel::ShipRightClicked, this, ph::_1, ph::_2, ph::_3));
    GGHumanClientApp::GetApp()->GetContext().ContextUniverse().UniverseObjectDeleteSignal.connect(
        boost::bind(&FleetDetailPanel::UniverseObjectDeleted, this, ph::_1));
}

void FleetDetailPanel::CompleteConstruction() {
    GG::Wnd::CompleteConstruction();

    AttachChild(m_ships_lb);
    DoLayout();
}

int FleetDetailPanel::GetShipIDOfListRow(GG::ListBox::iterator it) const {
    const auto* sr = dynamic_cast<const ShipRow*>(it->get());
    return sr ? sr->ShipID() : INVALID_OBJECT_ID;
}

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
        auto fleet = GGHumanClientApp::GetApp()->GetContext().ContextObjects().get<const Fleet>(m_fleet_id);
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
        if (ship_ids.contains(row->ShipID())) {
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

void FleetDetailPanel::SizeMove(GG::Pt ul, GG::Pt lr) {
    const auto old_size = Size();
    GG::Wnd::SizeMove(ul, lr);
    if (old_size != Size())
        DoLayout();
}

void FleetDetailPanel::EnableOrderIssuing(bool enabled) {
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

void FleetDetailPanel::UniverseObjectDeleted(const std::shared_ptr<const UniverseObject>& obj) {
    if (obj && obj->ID() == m_fleet_id)
        SetFleet(INVALID_OBJECT_ID);
}

void FleetDetailPanel::ShipSelectionChanged(const GG::ListBox::SelectionSet& rows) {
    for (auto it = m_ships_lb->begin(); it != m_ships_lb->end(); ++it) {
        try {
            if (auto* ship_row = it->get()) {
                if (!ship_row->empty())
                    if (auto* ship_panel = dynamic_cast<ShipDataPanel*>(ship_row->at(0)))
                        ship_panel->Select(rows.contains(it));
            }
        } catch (const std::exception& e) {
            ErrorLogger() << "FleetDetailPanel::ShipSelectionChanged caught exception: " << e.what();
            continue;
        }
    }

    SelectedShipsChangedSignal(rows);
}

void FleetDetailPanel::ShipRightClicked(GG::ListBox::iterator it, GG::Pt pt,
                                        GG::Flags<GG::ModKey> modkeys)
{
    // get ship that was clicked, aborting if problems arise doing so
    ShipRow* ship_row = dynamic_cast<ShipRow*>(it->get());
    if (!ship_row)
        return;

    const ScriptingContext& context = GGHumanClientApp::GetApp()->GetContext();
    const ObjectMap& objects{context.ContextObjects()};
    const Universe& universe{context.ContextUniverse()};

    auto ship = objects.get<Ship>(ship_row->ShipID());
    if (!ship)
        return;
    auto fleet = objects.get<Fleet>(m_fleet_id);

    const auto map_wnd = ClientUI::GetClientUI()->GetMapWnd(false);

    if (ClientPlayerIsModerator() &&
        map_wnd && map_wnd->GetModeratorActionSetting() != ModeratorActionSetting::MAS_NoAction)
    {
        ShipRightClickedSignal(ship->ID());  // response handled in MapWnd
        return;
    }

    const ShipDesign* design = universe.GetShipDesign(ship->DesignID()); // may be null
    int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();

    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);

    // Zoom to design
    if (design) {
        std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % design->Name(true));
        auto zoom_to_design_action = [design]() { ClientUI::GetClientUI()->ZoomToShipDesign(design->ID()); };
        popup->AddMenuItem(GG::MenuItem(std::move(popup_label), false, false,
                                        zoom_to_design_action));
    }

    // Rename ship context item
    if (ship->OwnedBy(client_empire_id) || ClientPlayerIsModerator()) {
        auto rename_action = [ship, client_empire_id]() {
            auto edit_wnd = GG::Wnd::Create<CUIEditWnd>(GG::X(350), UserString("ENTER_NEW_NAME"), ship->Name());
            edit_wnd->Run();

            if (ClientPlayerIsModerator())
                // TODO: Moderator action for renaming ships
                return;

            auto* app = GGHumanClientApp::GetApp();
            ScriptingContext& context = app->GetContext();
            if (!RenameOrder::Check(client_empire_id, ship->ID(), edit_wnd->Result(), context))
                return;

            app->Orders().IssueOrder<RenameOrder>(context, client_empire_id, ship->ID(), edit_wnd->Result());
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
            auto* app = GGHumanClientApp::GetApp();
            ScriptingContext& context = app->GetContext();
            app->Orders().IssueOrder<ScrapOrder>(context, client_empire_id, ship->ID());
        };
        popup->AddMenuItem(GG::MenuItem(UserString("ORDER_SHIP_SCRAP"), false, false, scrap_action));

    } else if (!ClientPlayerIsModerator()
               && ship->OwnedBy(client_empire_id))
    {
        auto unscrap_action = [ship]() {
            // find order to scrap this ship, and recind it
            auto pending_scrap_orders = PendingScrapOrders();
            auto pending_order_it = pending_scrap_orders.find(ship->ID());
            if (pending_order_it != pending_scrap_orders.end()) {
                auto* app = GGHumanClientApp::GetApp();
                app->Orders().RescindOrder(pending_order_it->second, app->GetContext());
            }
        };
        // create popup menu with "Cancel Scrap" option
        popup->AddMenuItem(GG::MenuItem(UserString("ORDER_CANCEL_SHIP_SCRAP"), false, false, unscrap_action));
    }

    // Split fleets
    if (ship->OwnedBy(client_empire_id)
        && !ClientPlayerIsModerator()
        && fleet)
    {
        auto split_one_design_action = [this, design, fleet, client_empire_id]() {
            // split ships with same design as clicked ship into separate fleet
            if (!design || !fleet) return;
            const auto parent = this->Parent();
            if (!parent) return;
            const FleetWnd* parent_fleet_wnd = dynamic_cast<const FleetWnd*>(parent.get());
            if (!parent_fleet_wnd)
                return;
            CreateNewFleetFromShipsWithDesign(fleet->ShipIDs(), design->ID(),
                                              parent_fleet_wnd->GetNewFleetAggression(),
                                              IApp::GetApp()->GetContext(), client_empire_id);
        };

        auto split_all_designs_action = [this, fleet, client_empire_id]() {
            // split all ships into new fleets by ship design
            if (!fleet) return;
            const auto parent = this->Parent();
            if (!parent)
                return;
            const FleetWnd* parent_fleet_wnd = dynamic_cast<const FleetWnd*>(parent.get());
            if (!parent_fleet_wnd)
                return;
            CreateNewFleetsFromShipsForEachDesign(fleet->ShipIDs(), parent_fleet_wnd->GetNewFleetAggression(),
                                                  IApp::GetApp()->GetContext(), client_empire_id);
        };

        if (design)
            popup->AddMenuItem( GG::MenuItem(UserString("FW_SPLIT_SHIPS_THIS_DESIGN"), false, false, split_one_design_action));
        popup->AddMenuItem(     GG::MenuItem(UserString("FW_SPLIT_SHIPS_ALL_DESIGNS"), false, false, split_all_designs_action));
    }

    // Allow dismissal of stale visibility information
    if (map_wnd && !ship->OwnedBy(client_empire_id) && fleet) {
        const auto ship_id = ship->ID();
        auto forget_ship_action = [ship_id, map_wnd]() { map_wnd->ForgetObject(ship_id); };

        const auto& visibility_turn_map{context.ContextUniverse().GetObjectVisibilityTurnMapByEmpire(
            ship->ID(), client_empire_id)};

        auto last_turn_visible_it = visibility_turn_map.find(Visibility::VIS_BASIC_VISIBILITY);
        if (last_turn_visible_it != visibility_turn_map.end()
            && last_turn_visible_it->second < context.current_turn)
        {
            popup->AddMenuItem(GG::MenuItem(UserString("FW_ORDER_DISMISS_SENSOR_GHOST"),
                                            false, false, forget_ship_action));
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
    [[nodiscard]] constexpr GG::Rect CreateOrGrowBox(bool create, const GG::Rect box, const GG::Pt pt)
        noexcept(noexcept(std::min(GG::Y1, GG::Y0)) && noexcept(GG::Rect{GG::Pt{GG::X0, GG::Y1}, GG::Pt{GG::X1, GG::Y0}}))
    {
        if (create) {
            return GG::Rect{pt, pt};
        } else {
            return GG::Rect{
                std::min(box.Left(),    pt.x),
                std::min(box.Top(),     pt.y),
                std::max(box.Right(),   pt.x),
                std::max(box.Bottom(),  pt.y)
            };
        }
    }

    /** Is \p ll smaller or equal to the size of \p rr? */
    [[nodiscard]] constexpr bool SmallerOrEqual(const GG::Rect ll, const GG::Rect rr) noexcept
    { return (ll.Width() <= rr.Width() && ll.Height() <= rr.Height()); }
}

FleetWnd::FleetWnd(const std::vector<int>& fleet_ids, bool order_issuing_enabled,
                   double allowed_bounding_box_leeway,
                   int selected_fleet_id,
                   GG::Flags<GG::WndFlag> flags,
                   std::string_view config_name) :
    MapWndPopup("", flags | GG::RESIZABLE, config_name),
    m_fleet_ids(fleet_ids.begin(), fleet_ids.end()),
    m_order_issuing_enabled(order_issuing_enabled)
{
    const auto& objects = GGHumanClientApp::GetApp()->GetContext().ContextObjects();

    if (!m_fleet_ids.empty()) {
        if (auto fleet = objects.get<Fleet>(*m_fleet_ids.begin()))
            m_empire_id = fleet->Owner();
    }

    // verify that the selected fleet id is valid.
    if (selected_fleet_id != INVALID_OBJECT_ID &&
        !m_fleet_ids.contains(selected_fleet_id))
    {
        ErrorLogger() << "FleetWnd::FleetWnd couldn't find requested selected fleet with id " << selected_fleet_id;
        selected_fleet_id = INVALID_OBJECT_ID;
    }

    // Determine the size of the bounding box containing the fleets, plus the leeway
    bool is_first_fleet = true;
    for (const auto& fleet : objects.find<Fleet>(m_fleet_ids)) {
        if (!fleet)
            continue;

        const auto fleet_loc = GG::Pt(GG::X(fleet->X()), GG::Y(fleet->Y()));
        // Grow the fleets bounding box
        m_bounding_box = CreateOrGrowBox(is_first_fleet, m_bounding_box, fleet_loc);
        is_first_fleet = false;
    }
    m_bounding_box = GG::Rect(m_bounding_box.UpperLeft(),
                              m_bounding_box.LowerRight() +
                              GG::Pt(GG::X(allowed_bounding_box_leeway), GG::Y(allowed_bounding_box_leeway)));

    m_fleet_detail_panel = GG::Wnd::Create<FleetDetailPanel>(GG::X1, GG::Y1, selected_fleet_id,
                                                             m_order_issuing_enabled);
}

void FleetWnd::CompleteConstruction() {
    Sound::TempUISoundDisabler sound_disabler;

    // add fleet aggregate stat icons
    const int tooltip_delay = GetOptionsDB().Get<int>("ui.tooltip.delay");
    const auto si_sz = StatIconSize();

    m_stat_icons.reserve(7);
    for (auto [meter_type, icon, text] : {
            std::make_tuple(MeterType::METER_SIZE,          FleetCountIcon(),   UserStringNop("FW_FLEET_COUNT_SUMMARY")),
            std::make_tuple(MeterType::METER_CAPACITY,      DamageIcon(),       UserStringNop("FW_FLEET_DAMAGE_SUMMARY")),
            std::make_tuple(MeterType::METER_MAX_CAPACITY,  DestroyIcon(),      UserStringNop("FW_FLEET_DESTROY_SUMMARY")),
            std::make_tuple(MeterType::METER_SECONDARY_STAT,FightersIcon(),     UserStringNop("FW_FLEET_FIGHTER_SUMMARY")),
            std::make_tuple(MeterType::METER_STRUCTURE,     ClientUI::MeterIcon(MeterType::METER_STRUCTURE),    UserStringNop("FW_FLEET_STRUCTURE_SUMMARY")),
            std::make_tuple(MeterType::METER_SHIELD,        ClientUI::MeterIcon(MeterType::METER_SHIELD),       UserStringNop("FW_FLEET_SHIELD_SUMMARY")),
            std::make_tuple(MeterType::METER_TROOPS,        TroopIcon(),        UserStringNop("FW_FLEET_TROOP_SUMMARY")),
            std::make_tuple(MeterType::METER_POPULATION,    ColonyIcon(),       UserStringNop("FW_FLEET_COLONY_SUMMARY")),
        })
    {
        auto stat_icon = GG::Wnd::Create<StatisticIcon>(std::move(icon), 0, 0, false, si_sz.x, si_sz.y);
        m_stat_icons.emplace_back(meter_type, stat_icon);
        stat_icon->SetBrowseModeTime(tooltip_delay);
        stat_icon->SetBrowseText(UserString(text));
        AttachChild(std::move(stat_icon));
    }

    namespace ph = boost::placeholders;

    // create fleet list box
    m_fleets_lb = GG::Wnd::Create<FleetsListBox>(m_order_issuing_enabled);
    m_fleets_lb->SetHiliteColor(GG::CLR_ZERO);
    m_fleets_lb->SelRowsChangedSignal.connect(
        boost::bind(&FleetWnd::FleetSelectionChanged, this, ph::_1));
    m_fleets_lb->LeftClickedRowSignal.connect(
        boost::bind(&FleetWnd::FleetLeftClicked, this, ph::_1, ph::_2, ph::_3));
    m_fleets_lb->RightClickedRowSignal.connect(
        boost::bind(&FleetWnd::FleetRightClicked, this, ph::_1, ph::_2, ph::_3));
    m_fleets_lb->DoubleClickedRowSignal.connect(
        boost::bind(&FleetWnd::FleetDoubleClicked, this, ph::_1, ph::_2, ph::_3));
    AttachChild(m_fleets_lb);
    m_fleets_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_BROWSEUPDATES);
    m_fleets_lb->AllowDropType(SHIP_DROP_TYPE_STRING);
    m_fleets_lb->AllowDropType(FLEET_DROP_TYPE_STRING);

    // create fleet detail panel
    m_fleet_detail_panel->SelectedShipsChangedSignal.connect(
        boost::bind(&FleetWnd::ShipSelectionChanged, this, ph::_1));
    m_fleet_detail_panel->ShipRightClickedSignal.connect(
        ShipRightClickedSignal);
    AttachChild(m_fleet_detail_panel);

    const auto* app = GGHumanClientApp::GetApp();
    const auto& context = app->GetContext();

    // determine fleets to show and populate list
    Refresh(app->EmpireID(), context);

    // create drop target
    m_new_fleet_drop_target = GG::Wnd::Create<FleetDataPanel>(GG::X1, ListRowHeight(), m_system_id, true);
    AttachChild(m_new_fleet_drop_target);
    m_new_fleet_drop_target->NewFleetFromShipsSignal.connect(
        [this](const auto& ship_ids) {
            auto* app = GGHumanClientApp::GetApp();
            CreateNewFleetFromDrops(ship_ids, app->GetContext(), app->EmpireID());
        });

    context.ContextUniverse().UniverseObjectDeleteSignal.connect(
        boost::bind(&FleetWnd::UniverseObjectDeleted, this, ph::_1));

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
        if (auto mapwnd = client->GetMapWnd(false))
            mapwnd->ClearProjectedFleetMovementLines();
    ClosingSignal(this);
}

void FleetWnd::PreRender() {
    MapWndPopup::PreRender();

    if (m_needs_refresh) {
        const auto* app = GGHumanClientApp::GetApp();
        Refresh(app->EmpireID(), app->GetContext());
    }
}

GG::Rect FleetWnd::CalculatePosition() const {
    GG::Pt ul(GG::X(5), GG::GUI::GetGUI()->AppHeight() - FLEET_WND_HEIGHT - 5);
    GG::Pt wh(FLEET_WND_WIDTH, FLEET_WND_HEIGHT);
    return GG::Rect(ul, ul + wh);
}

void FleetWnd::SetStatIconValues() {
    const auto* app = GGHumanClientApp::GetApp();
    int client_empire_id = app->EmpireID();
    int ship_count =        0;
    float damage_tally =    0.0f;
    float destroy_tally   = 0.0f; // number of destroyed fighters
    float fighters_tally  = 0.0f;
    float structure_tally = 0.0f;
    float shield_tally =    0.0f;
    float troop_tally =     0.0f;
    float colony_tally =    0.0f;

    const ScriptingContext& context = app->GetContext();
    const Universe& universe = context.ContextUniverse();
    const ObjectMap& objects = context.ContextObjects();

    const auto& this_client_known_destroyed_objects = universe.EmpireKnownDestroyedObjectIDs(client_empire_id);
    const auto& this_client_stale_object_info = universe.EmpireStaleKnowledgeObjectIDs(client_empire_id);


    for (const auto& fleet : objects.find<const Fleet>(m_fleet_ids)) {
        if ( !(((m_empire_id == ALL_EMPIRES) && (fleet->Unowned())) || fleet->OwnedBy(m_empire_id)) )
            continue;

        for (auto& ship : objects.find<const Ship>(fleet->ShipIDs())) {
            int ship_id = ship->ID();

            // skip known destroyed and stale info objects
            if (this_client_known_destroyed_objects.contains(ship_id))
                continue;
            if (this_client_stale_object_info.contains(ship_id))
                continue;

            if (universe.GetShipDesign(ship->DesignID())) {
                ship_count++;
                damage_tally += ship->TotalWeaponsShipDamage(context, 0.0f, true);
                destroy_tally += ship->TotalWeaponsFighterDamage(context, true); // TODO: Is it inconsistent to count fighters killing fighters here?
                fighters_tally += ship->FighterCount();
                structure_tally += ship->GetMeter(MeterType::METER_STRUCTURE)->Initial();
                shield_tally += ship->GetMeter(MeterType::METER_SHIELD)->Initial();
                troop_tally += ship->TroopCapacity(universe);
                colony_tally += ship->ColonyCapacity(universe);
            }
        }
    }

    for (auto& [stat_name, icon] : m_stat_icons) {
        if (stat_name == MeterType::METER_SHIELD)
            icon->SetValue(shield_tally/ship_count);
        else if (stat_name == MeterType::METER_STRUCTURE)
            icon->SetValue(structure_tally);
        else if (stat_name == MeterType::METER_CAPACITY)
            icon->SetValue(damage_tally); // FIXME show fighters_shot_tally somewhere
        else if (stat_name == MeterType::METER_MAX_CAPACITY)
            icon->SetValue(destroy_tally);
        else if (stat_name == MeterType::METER_SECONDARY_STAT)
            icon->SetValue(fighters_tally);
        else if (stat_name == MeterType::METER_POPULATION)
            icon->SetValue(colony_tally);
        else if (stat_name == MeterType::METER_SIZE)
            icon->SetValue(ship_count);
        else if (stat_name == MeterType::METER_TROOPS)
            icon->SetValue(troop_tally);
    }
}

void FleetWnd::RefreshStateChangedSignals() {
    const auto& objects = GGHumanClientApp::GetApp()->GetContext().ContextObjects();

    m_system_connection.disconnect();
    if (auto system = objects.get<System>(m_system_id))
        m_system_connection = system->StateChangedSignal.connect(
            boost::bind(&FleetWnd::RequireRefresh, this), boost::signals2::at_front);

    m_fleet_connections.clear(); // should disconnect scoped connections

    for (const auto& fleet : objects.find<Fleet>(m_fleet_ids)) {
        if (fleet)
            m_fleet_connections.push_back(fleet->StateChangedSignal.connect(
                boost::bind(&FleetWnd::RequireRefresh, this)));
    }
}

void FleetWnd::RequireRefresh() {
    m_needs_refresh = true;
    RequirePreRender();
}

void FleetWnd::Refresh(int this_client_empire_id, const ScriptingContext& context) {
    m_needs_refresh = false;

    const auto& universe = context.ContextUniverse();
    const auto& objects = context.ContextObjects();

    const auto& this_client_known_destroyed_objects = universe.EmpireKnownDestroyedObjectIDs(this_client_empire_id);
    const auto& this_client_stale_object_info = universe.EmpireStaleKnowledgeObjectIDs(this_client_empire_id);

    // save selected fleet(s) and ships(s)
    const auto initially_selected_fleets{this->SelectedFleetIDs()};
    const auto initially_selected_ships{this->SelectedShipIDs()};

    // remove existing fleet rows
    const auto initial_fleet_ids{m_fleet_ids};
    m_fleet_ids.clear();

    std::multimap<std::pair<int, GG::Pt>, int> fleet_locations_ids;
    std::multimap<std::pair<int, GG::Pt>, int> selected_fleet_locations_ids;

    // Check all fleets in initial_fleet_ids and keep those that exist.
    std::unordered_set<int> fleets_that_exist;
    GG::Rect fleets_bounding_box;
    for (const auto* fleet : objects.findRaw<const Fleet>(initial_fleet_ids)) {
        if (!fleet)
            continue;

        // skip known destroyed and stale info objects
        if (this_client_known_destroyed_objects.contains(fleet->ID()))
            continue;
        if (this_client_stale_object_info.contains(fleet->ID()))
            continue;

        auto fleet_loc = GG::Pt(GG::X(fleet->X()), GG::Y(fleet->Y()));
        // Grow the fleets bounding box
        fleets_bounding_box = CreateOrGrowBox(fleets_that_exist.empty(), fleets_bounding_box, fleet_loc);

        fleets_that_exist.insert(fleet->ID());
        fleet_locations_ids.emplace(std::pair(fleet->SystemID(), fleet_loc), fleet->ID());
    }

    const auto bounding_box_center = GG::Pt(fleets_bounding_box.MidX(), fleets_bounding_box.MidY());


    // Filter initially selected fleets according to existing fleets
    GG::Rect selected_fleets_bounding_box;
    for (const auto* fleet : objects.findRaw<const Fleet>(initially_selected_fleets)) {
        if (!fleet)
            continue;

        if (!fleets_that_exist.contains(fleet->ID()))
            continue;

        auto fleet_loc = GG::Pt(GG::X(fleet->X()), GG::Y(fleet->Y()));

        // Grow the selected fleets bounding box
        selected_fleets_bounding_box = CreateOrGrowBox(selected_fleet_locations_ids.empty(),
                                                       selected_fleets_bounding_box, fleet_loc);
        selected_fleet_locations_ids.emplace(std::pair(fleet->SystemID(), fleet_loc), fleet->ID());
    }
    const GG::Pt selected_bounding_box_center{selected_fleets_bounding_box.MidX(),
                                              selected_fleets_bounding_box.MidY()};


    // Determine FleetWnd location

    // Are all fleets in one system?  Use that location.
    // Otherwise, are all selected fleets in one system?  Use that location.
    // Otherwise, are all the moving fleets clustered within m_bounding_box of each other?
    // Otherwise, are all the selected fleets clustered within m_bounding_box of each other?
    // Otherwise, is the current location a system?  Use that location.
    // Otherwise remove all fleets as all fleets have gone in separate directions.

    std::pair<int, GG::Pt> location{INVALID_OBJECT_ID, GG::Pt0};
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
            fleets_near_enough.emplace(location, loc_and_id.second);
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
                fleets_near_enough.emplace(location, loc_and_id.second);
        }
        fleet_locations_ids.swap(fleets_near_enough);

    } else if (const auto system = objects.get<System>(m_system_id)) {
        location = {m_system_id, GG::Pt(GG::X(system->X()), GG::Y(system->Y()))};

    } else {
        fleet_locations_ids.clear();
        selected_fleet_locations_ids.clear();
    }


    // Use fleets that are at the determined location
    const auto flt_at_loc = fleet_locations_ids.equal_range(location);
    for (auto it = flt_at_loc.first; it != flt_at_loc.second; ++it)
        m_fleet_ids.insert(it->second);

    m_system_id = location.first;

    if (m_new_fleet_drop_target)
        m_new_fleet_drop_target->SetSystemID(m_system_id);

    // If the location is a system add in any ships from m_empire_id that are in the system.
    if (const auto system = objects.get<System>(m_system_id).get()) {
        m_fleet_ids.clear();
        // get fleets to show from system, based on required ownership
        for (const auto& fleet : objects.find<Fleet>(system->FleetIDs())) {
            int fleet_id = fleet->ID();

            // skip known destroyed and stale info objects
            if (this_client_known_destroyed_objects.contains(fleet_id) ||
                this_client_stale_object_info.contains(fleet_id))
            { continue; }

            if ( ((m_empire_id == ALL_EMPIRES) && (fleet->Unowned())) || fleet->OwnedBy(m_empire_id) )
                m_fleet_ids.emplace(fleet_id);
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

void FleetWnd::LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    MapWndPopup::LClick(pt, mod_keys);
    ClickedSignal(std::static_pointer_cast<FleetWnd>(shared_from_this()));
}

void FleetWnd::DoLayout() {
    const GG::X TOTAL_WIDTH(ClientWidth());
    static constexpr GG::X LEFT{0};
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
    const auto* app = GGHumanClientApp::GetApp();
    int this_client_empire_id = app->EmpireID();
    const auto& objects = app->GetContext().ContextObjects();
    for (const auto& fleet : objects.find<const Fleet>(m_fleet_ids)) {
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
        fleets_list_height /= 2;
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
    auto fleet = GGHumanClientApp::GetApp()->GetContext().ContextObjects().getRaw<const Fleet>(fleet_id);
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
    if (fleet_id == INVALID_OBJECT_ID || !(GGHumanClientApp::GetApp()->GetContext().ContextObjects().getRaw<Fleet>(fleet_id))) {
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
        if (fleet_ids.contains(row->FleetID())) {
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

void FleetWnd::SizeMove(GG::Pt ul, GG::Pt lr) {
    const auto old_size = Size();
    MapWndPopup::SizeMove(ul, lr);
    if (Size() != old_size)
        DoLayout();
}

int FleetWnd::SystemID() const
{ return m_system_id; }

int FleetWnd::EmpireID() const
{ return m_empire_id; }

bool FleetWnd::ContainsFleet(int fleet_id) const {
    const auto& objects = GGHumanClientApp::GetApp()->GetContext().ContextObjects();
    for (auto it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        auto fleet = objects.get<Fleet>(FleetInRow(it));
        if (fleet && fleet->ID() == fleet_id)
            return true;
    }
    return false;
}

template <typename Set>
bool FleetWnd::ContainsFleets(Set fleet_ids) const {
    if (fleet_ids.empty())
        return false;

    const auto& objects = GGHumanClientApp::GetApp()->GetContext().ContextObjects();

    // Remove found ids from fleet_ids.  If fleet_ids is empty, all have been found.
    for (auto it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        if (auto fleet = objects.get<Fleet>(FleetInRow(it)))
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

FleetAggression FleetWnd::GetNewFleetAggression() const {
    if (m_new_fleet_drop_target)
        return m_new_fleet_drop_target->GetFleetAggression();
    return FleetAggression::INVALID_FLEET_AGGRESSION;
}

void FleetWnd::FleetSelectionChanged(const GG::ListBox::SelectionSet& rows) {
    // show appropriate fleet in detail panel.  if one fleet is selected, show
    // its ships.  if more than one fleet is selected or no fleets are selected
    // then show no ships.
    if (rows.size() == 1) {
        // find selected row and fleet
        bool found_row = false;
        for (auto it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
            if (rows.contains(it)) {
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
            if (auto* fleet_row = it->get()) {
                if (!fleet_row->empty())
                    if (auto* fleet_panel = dynamic_cast<FleetDataPanel*>(fleet_row->at(0)))
                        fleet_panel->Select(rows.contains(it));
            }
        } catch (const std::exception& e) {
            ErrorLogger() << "FleetWnd::FleetSelectionChanged caught exception: " << e.what();
            continue;
        }
    }

    m_fleet_detail_panel->Refresh();
    SelectedFleetsChangedSignal();
}

void FleetWnd::FleetRightClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys) {
    const auto* app = GGHumanClientApp::GetApp();
    const int client_empire_id = app->EmpireID();
    const ScriptingContext& context = app->GetContext();
    const Universe& u = context.ContextUniverse();
    const ObjectMap& o = context.ContextObjects();

    const auto fleet = o.get<Fleet>(FleetInRow(it));
    if (!fleet)
        return;

    const auto system = o.get<System>(fleet->SystemID());
    const auto ship_ids_set{fleet->ShipIDs()};

    std::vector<int> damaged_ship_ids;
    std::vector<int> unfueled_ship_ids;
    std::vector<int> not_full_fighters_ship_ids;

    damaged_ship_ids.reserve(ship_ids_set.size());
    unfueled_ship_ids.reserve(ship_ids_set.size());
    not_full_fighters_ship_ids.reserve(ship_ids_set.size());

    for (const auto& ship : o.find<Ship>(ship_ids_set)) {
        if (!ship)
            continue;

        // find damaged ships
        if (ship->GetMeter(MeterType::METER_STRUCTURE)->Initial() < ship->GetMeter(MeterType::METER_MAX_STRUCTURE)->Initial())
            damaged_ship_ids.push_back(ship->ID());
        // find ships with no remaining fuel
        if (ship->GetMeter(MeterType::METER_FUEL)->Initial() < 1.0f)
            unfueled_ship_ids.push_back(ship->ID());
        // find ships that can carry fighters but dont have a full complement of them
        if (ship->HasFighters(u) && ship->FighterCount() < ship->FighterMax())
            not_full_fighters_ship_ids.push_back(ship->ID());
    }

    // determine which other empires are at peace with client empire and have
    // an owned object in this fleet's system
    std::set<int> peaceful_empires_in_system;
    if (system) {
        for (auto& obj : o.find<const UniverseObject>(system->ObjectIDs())) {
            if (obj->GetVisibility(client_empire_id, u) < Visibility::VIS_PARTIAL_VISIBILITY)
                continue;
            if (obj->Owner() == client_empire_id || obj->Unowned())
                continue;
            if (peaceful_empires_in_system.contains(obj->Owner()))
                continue;
            if (context.ContextDiploStatus(client_empire_id, obj->Owner()) < DiplomaticStatus::DIPLO_PEACE)
                continue;
            peaceful_empires_in_system.insert(obj->Owner());
        }
    }


    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);

    const auto mapwnd = ClientUI::GetClientUI()->GetMapWnd(false);

    // add a fleet popup command to send the fleet exploring, and stop it from exploring
    if (system
        && mapwnd && !mapwnd->IsFleetExploring(fleet->ID())
        && !ClientPlayerIsModerator()
        && fleet->OwnedBy(client_empire_id))
    {
        auto explore_action = [fleet, mapwnd]() { mapwnd->SetFleetExploring(fleet->ID()); };

        popup->AddMenuItem(GG::MenuItem(UserString("ORDER_FLEET_EXPLORE"),
                                        false, false, std::move(explore_action)));
        popup->AddMenuItem(GG::MenuItem(true));
    }
    else if (system
             && mapwnd
             && !ClientPlayerIsModerator()
             && fleet->OwnedBy(client_empire_id))
    {
        auto stop_explore_action = [fleet_id{fleet->ID()}, mapwnd]()
        { mapwnd->StopFleetExploring(fleet_id, IApp::GetApp()->GetContext().ContextObjects()); };
        popup->AddMenuItem(GG::MenuItem(UserString("ORDER_CANCEL_FLEET_EXPLORE"), false, false,
                                        std::move(stop_explore_action)));
        popup->AddMenuItem(GG::MenuItem(true));
    }

    // Merge fleets
    if (system
        && fleet->OwnedBy(client_empire_id)
        && !ClientPlayerIsModerator()
       )
    {
        auto merge_action = [id{fleet->ID()}]() { MergeFleetsIntoFleet(id, IApp::GetApp()->GetContext()); };
        popup->AddMenuItem(GG::MenuItem(UserString("FW_MERGE_SYSTEM_FLEETS"), false, false,
                                        std::move(merge_action)));
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
        auto split_damage_action = [nfa{fleet->Aggression()}, &damaged_ship_ids, client_empire_id]()
        { CreateNewFleetFromShips(damaged_ship_ids, nfa, IApp::GetApp()->GetContext(), client_empire_id); };
        popup->AddMenuItem(GG::MenuItem(UserString("FW_SPLIT_DAMAGED_FLEET"),
                                        false, false, std::move(split_damage_action)));
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
        auto split_unfueled_action = [nfa{fleet->Aggression()}, unfueled_ship_ids, client_empire_id]()
        { CreateNewFleetFromShips(unfueled_ship_ids, nfa, IApp::GetApp()->GetContext(), client_empire_id); };

        popup->AddMenuItem(GG::MenuItem(UserString("FW_SPLIT_UNFUELED_FLEET"), false, false,
                                        std::move(split_unfueled_action)));
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
        auto split_not_full_fighters_action =
            [nfa{fleet->Aggression()}, not_full_fighters_ship_ids, client_empire_id]()
            { CreateNewFleetFromShips(not_full_fighters_ship_ids, nfa, IApp::GetApp()->GetContext(), client_empire_id); };
        popup->AddMenuItem(GG::MenuItem(UserString("FW_SPLIT_NOT_FULL_FIGHTERS_FLEET"),
                                        false, false, std::move(split_not_full_fighters_action)));
    }

    // Split fleet - can't split fleets without more than one ship, or which are not in a system
    if (system
        && ship_ids_set.size() > 1
        && (fleet->OwnedBy(client_empire_id))
        && !ClientPlayerIsModerator()
       )
    {
        auto split_action = [this, &ship_ids_set, client_empire_id]() {
            ScopedTimer split_fleet_timer("FleetWnd::SplitFleet");

            FleetAggression new_aggression_setting = FleetAggression::INVALID_FLEET_AGGRESSION;
            if (m_new_fleet_drop_target)
                new_aggression_setting = m_new_fleet_drop_target->GetFleetAggression();

            // assemble container of containers of ids of fleets to create.
            // one ship id per vector
            for (int ship_id : ship_ids_set)
                CreateNewFleetFromShips(std::vector<int>{ship_id}, new_aggression_setting,
                                        IApp::GetApp()->GetContext(), client_empire_id);
        };

        auto split_per_design_action = [this, fleet, client_empire_id]() {
            FleetAggression new_aggression_setting = FleetAggression::INVALID_FLEET_AGGRESSION;
            if (m_new_fleet_drop_target)
                new_aggression_setting = m_new_fleet_drop_target->GetFleetAggression();

            CreateNewFleetsFromShipsForEachDesign(fleet->ShipIDs(), new_aggression_setting,
                                                  IApp::GetApp()->GetContext(), client_empire_id);
        };

        popup->AddMenuItem(GG::MenuItem(UserString("FW_SPLIT_FLEET"),
                                        false, false, std::move(split_action)));
        popup->AddMenuItem(GG::MenuItem(UserString("FW_SPLIT_SHIPS_ALL_DESIGNS"),
                                        false, false, std::move(split_per_design_action)));
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

            auto* app = GGHumanClientApp::GetApp();
            ScriptingContext& context = app->GetContext();

            if (!RenameOrder::Check(client_empire_id, fleet->ID(), edit_wnd->Result(), context))
                return;

            app->Orders().IssueOrder<RenameOrder>(context, client_empire_id, fleet->ID(), edit_wnd->Result());
        };
        popup->AddMenuItem(GG::MenuItem(UserString("RENAME"), false, false,
                                        std::move(rename_action)));
        popup->AddMenuItem(GG::MenuItem(true));
    }

    bool post_scrap_bar = false;

    // add a fleet popup command to order all ships in the fleet scrapped
    if (system
        && fleet->HasShipsWithoutScrapOrders(u)
        && !ClientPlayerIsModerator()
        && fleet->OwnedBy(client_empire_id))
    {
        auto scrap_action = [fleet, client_empire_id]() {
            const auto ship_ids{fleet->ShipIDs()};
            auto* app = GGHumanClientApp::GetApp();
            OrderSet& orders = app->Orders();
            ScriptingContext& context = app->GetContext();
            for (const auto ship_id : ship_ids)
                orders.IssueOrder<ScrapOrder>(context, client_empire_id, ship_id);
        };

        popup->AddMenuItem(GG::MenuItem(UserString("ORDER_FLEET_SCRAP"),
                                        false, false, std::move(scrap_action)));
        post_scrap_bar = true;
    }

    // add a fleet popup command to cancel all scrap orders on ships in this fleet
    if (system
        && fleet->HasShipsOrderedScrapped(u)
        && !ClientPlayerIsModerator())
    {
        auto unscrap_action = [fleet]() {
            auto* app = GGHumanClientApp::GetApp();
            OrderSet& orders = app->Orders();
            ScriptingContext& context = app->GetContext();

            std::vector<int> order_ids_to_rescind;

            for (int ship_id : fleet->ShipIDs()) {
                for (const auto& [order_id, order] : orders) {
                    if (auto scrap_order = std::dynamic_pointer_cast<ScrapOrder>(order)) {
                        if (scrap_order->ObjectID() == ship_id) {
                            order_ids_to_rescind.push_back(order_id);
                            // could break here, but won't to ensure there are no problems with doubled orders
                        }
                    }
                }
            }

            for (const auto& order_id : order_ids_to_rescind)
                orders.RescindOrder(order_id, context);
        };

        popup->AddMenuItem(GG::MenuItem(UserString("ORDER_CANCEL_FLEET_SCRAP"),
                                        false, false, std::move(unscrap_action)));
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
        for (auto& [recipient_empire_id, recipient_empire] : context.Empires()) {
            if (!peaceful_empires_in_system.contains(recipient_empire_id))
                continue;
            auto gift_action = [rei{recipient_empire_id}, fid{fleet->ID()}, client_empire_id]() {
                auto* app = GGHumanClientApp::GetApp();
                OrderSet& orders = app->Orders();
                ScriptingContext& context = app->GetContext();
                orders.IssueOrder<GiveObjectToEmpireOrder>(context, client_empire_id, fid, rei);
            };
            give_away_menu.next_level.emplace_back(recipient_empire->Name(),
                                                   false, false, std::move(gift_action));
        }
        popup->AddMenuItem(std::move(give_away_menu));

        if (fleet->OrderedGivenToEmpire() != ALL_EMPIRES) {
            auto ungift_action = [fleet]() {
                auto* app = GGHumanClientApp::GetApp();
                OrderSet& orders = app->Orders();
                ScriptingContext& context = app->GetContext();
                std::vector<int> order_ids_to_rescind;

                for (const auto& [order_id, order] : orders) {
                    if (auto give_order = std::dynamic_pointer_cast<GiveObjectToEmpireOrder>(order)) {
                        if (give_order->ObjectID() == fleet->ID()) {
                            order_ids_to_rescind.push_back(order_id);
                            // could break here, but won't to ensure there are no problems with doubled orders
                        }
                    }
                }

                for (const auto& order_id : order_ids_to_rescind)
                    orders.RescindOrder(order_id, context);
            };
            GG::MenuItem cancel_give_away_menu{UserString("ORDER_CANCEL_GIVE_FLEET"),
                                               false, false, ungift_action};
            popup->AddMenuItem(std::move(cancel_give_away_menu));
        }
    }


    // Allow dismissal of stale visibility information
    if (mapwnd && !fleet->OwnedBy(client_empire_id)) {
        const auto fleet_id = fleet->ID();
        auto forget_fleet_action = [fleet_id, mapwnd]() { mapwnd->ForgetObject(fleet_id); };
        const auto& visibility_turn_map = u.GetObjectVisibilityTurnMapByEmpire(fleet_id, client_empire_id);
        auto last_turn_visible_it = visibility_turn_map.find(Visibility::VIS_BASIC_VISIBILITY);
        if (last_turn_visible_it != visibility_turn_map.end()
            && last_turn_visible_it->second < IApp::GetApp()->CurrentTurn())
        {
            popup->AddMenuItem(GG::MenuItem(UserString("FW_ORDER_DISMISS_SENSOR_GHOST"),
                                            false, false, std::move(forget_fleet_action)));
        }
    }

    popup->Run();
}

void FleetWnd::FleetLeftClicked(GG::ListBox::iterator it, GG::Pt pt,
                                GG::Flags<GG::ModKey> modkeys)
{ ClickedSignal(std::static_pointer_cast<FleetWnd>(shared_from_this())); }

void FleetWnd::FleetDoubleClicked(GG::ListBox::iterator it, GG::Pt pt,
                                  GG::Flags<GG::ModKey> modkeys)
{ ClickedSignal(std::static_pointer_cast<FleetWnd>(shared_from_this())); }

int FleetWnd::FleetInRow(GG::ListBox::iterator it) const {
    if (it == m_fleets_lb->end())
        return INVALID_OBJECT_ID;

    try {
        //DebugLogger() << "FleetWnd::FleetInRow casting iterator to fleet row";
        if (FleetRow* fleet_row = dynamic_cast<FleetRow*>(it->get()))
            return fleet_row->FleetID();
    } catch (const std::exception& e) {
        ErrorLogger() << "FleetInRow caught exception: " << e.what();
    }

    return INVALID_OBJECT_ID;
}

namespace {
    std::string SystemNameNearestToFleet(int client_empire_id, int fleet_id, const Universe& u) {
        const ObjectMap& objects{u.Objects()};
        auto fleet = objects.get<Fleet>(fleet_id);
        if (!fleet)
            return {};

        int nearest_system_id(u.GetPathfinder().NearestSystemTo(fleet->X(), fleet->Y(), objects));
        if (auto system = objects.get<System>(nearest_system_id))
            return system->ApparentName(client_empire_id, u);
        return {};
    }
}

std::string FleetWnd::TitleText() const {
    // if no fleets available, default to indicating no fleets
    if (m_fleet_ids.empty())
        return UserString("FW_NO_FLEET");
    const ScriptingContext& context = IApp::GetApp()->GetContext();
    const Universe& u{context.ContextUniverse()};
    const ObjectMap& objects{u.Objects()};
    const EmpireManager& empires{context.Empires()};

    int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();

    // at least one fleet is available, so show appropriate title this
    // FleetWnd's empire and system
    auto empire = empires.GetEmpire(m_empire_id);

    if (auto system = objects.get<System>(m_system_id)) {
        std::string sys_name = system->ApparentName(client_empire_id, u);
        return (empire
                ? boost::io::str(FlexibleFormat(UserString("FW_EMPIRE_FLEETS_AT_SYSTEM")) %
                                 empire->Name() % sys_name)
                : boost::io::str(FlexibleFormat(UserString("FW_GENERIC_FLEETS_AT_SYSTEM")) %
                                 sys_name));
    }

    std::string sys_name = SystemNameNearestToFleet(client_empire_id, *m_fleet_ids.begin(), u);
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

void FleetWnd::CreateNewFleetFromDrops(const std::vector<int>& ship_ids,
                                       ScriptingContext& context, int empire_id)
{
    DebugLogger() << "FleetWnd::CreateNewFleetFromDrops with " << ship_ids.size() << " ship ids";

    if (ship_ids.empty())
        return;

    const FleetAggression aggression = m_new_fleet_drop_target ?
        m_new_fleet_drop_target->GetFleetAggression(): FleetAggression::INVALID_FLEET_AGGRESSION;

    // deselect all ships so that response to fleet rearrangement doesn't attempt
    // to get the selected ships that are no longer in their old fleet.
    m_fleet_detail_panel->SelectShips(std::set<int>());

    CreateNewFleetFromShips(ship_ids, aggression, context, empire_id);
}

void FleetWnd::ShipSelectionChanged(const GG::ListBox::SelectionSet& rows)
{ SelectedShipsChangedSignal(); }

void FleetWnd::UniverseObjectDeleted(const std::shared_ptr<const UniverseObject>& obj) {
    // check if deleted object was a fleet.  The universe signals for all
    // object types, not just fleets.
    std::shared_ptr<const Fleet> deleted_fleet = std::dynamic_pointer_cast<const Fleet>(obj);
    if (!deleted_fleet)
        return;

    const ScriptingContext& context = IApp::GetApp()->GetContext();
    const ObjectMap& objects = context.ContextObjects();

    // if detail panel is showing the deleted fleet, reset to show nothing
    if (objects.get<Fleet>(m_fleet_detail_panel->FleetID()) == deleted_fleet)
        m_fleet_detail_panel->SetFleet(INVALID_OBJECT_ID);

    // remove deleted fleet's row
    for (auto it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        int row_fleet_id = FleetInRow(it);
        if (objects.get<Fleet>(row_fleet_id) == deleted_fleet) {
            m_fleets_lb->Erase(it);
            break;
        }
    }
}

void FleetWnd::EnableOrderIssuing(bool enable) {
    m_order_issuing_enabled = enable;
    if (m_new_fleet_drop_target)
        m_new_fleet_drop_target->Disable(!m_order_issuing_enabled);
    if (m_fleets_lb)
        m_fleets_lb->EnableOrderIssuing(m_order_issuing_enabled);
    if (m_fleet_detail_panel)
        m_fleet_detail_panel->EnableOrderIssuing(m_order_issuing_enabled);
}
