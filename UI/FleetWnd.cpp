#include "FleetWnd.h"

#include "CUIControls.h"
#include "SidePanel.h"
#include "InfoPanels.h"
#include "ClientUI.h"
#include "Sound.h"
#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../universe/Fleet.h"
#include "../client/human/HumanClientApp.h"
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
    const GG::Flags<GG::GraphicStyle>   DataPanelIconStyle() {
        return GG::GRAPHIC_CENTER | GG::GRAPHIC_VCENTER | GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE;
    }

    const GG::X         DATA_PANEL_TEXT_PAD = GG::X(4); // padding on the left and right of fleet/ship description
    const int           DATA_PANEL_BORDER = 1;          // how thick should the border around ship or fleet panel be


    const int           PAD = 4;
    const std::string   SHIP_DROP_TYPE_STRING = "FleetWnd ShipRow";
    const std::string   FLEET_DROP_TYPE_STRING = "FleetWnd FleetRow";

    const std::string   SPEED_STAT_STRING = "Speed Stat";
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

    GG::Y LabelHeight() {
        return GG::Y(ClientUI::Pts()*3/2);
    }

    /** How big fleet and ship statistics icons should be relative to the
      * current font size.  Icons shouldn't scale below what they are for the
      * default, 12 pt, font size. */
    GG::Pt StatIconSize() {
        const int font_size = std::max(ClientUI::Pts(), 12);
        return GG::Pt(GG::X(font_size*10/3), GG::Y(font_size*4/3));
    }

    GG::Y ListRowHeight() {
        return std::max(DATA_PANEL_ICON_SPACE.y, LabelHeight() + StatIconSize().y) + 2*DATA_PANEL_BORDER + PAD;
    }

    boost::shared_ptr<GG::Texture> SpeedIcon() {
        return ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "meter" / "speed.png", true);
    }

    std::string FleetDestinationText(int fleet_id) {
        std::string retval = "";
        const Fleet* fleet = GetObject<Fleet>(fleet_id);
        if (!fleet)
            return retval;

        const ObjectMap& objects = GetMainObjectMap();

        const System* dest = objects.Object<System>(fleet->FinalDestinationID());
        const System* cur_sys = objects.Object<System>(fleet->SystemID());
        if (dest && dest != cur_sys) {
            std::pair<int, int> eta = fleet->ETA();       // .first is turns to final destination.  .second is turns to next system on route

            // name of final destination
            std::string dest_name = dest->Name();
            if (dest_name.empty())
                dest_name = UserString("UNKNOWN_SYSTEM");

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


            retval = boost::io::str(FlexibleFormat(UserString("FW_FLEET_MOVING_TO")) %
                                                dest_name % final_eta_text % next_eta_text);

        } else if (cur_sys) {
            // name of current system
            std::string cur_name = cur_sys->Name();
            if (cur_name.empty())
                cur_name = UserString("UNKNOWN_SYSTEM");

            retval = boost::io::str(FlexibleFormat(UserString("FW_FLEET_HOLDING_AT")) % cur_name);
        }
        return retval;
    }

    std::string FleetNameText(int fleet_id) {
        std::string retval = UserString("ERROR");
        const Fleet* fleet = GetObject<Fleet>(fleet_id);
        if (!fleet)
            return retval;

        // name of fleet
        std::string name = fleet->Name();
        if (!name.empty())
            return name;

        int empire_id = HumanClientApp::GetApp()->EmpireID();

        if (fleet->Unowned() || fleet->OwnedBy(empire_id))
            return UserString("FLEET");
        else
            return UserString("FW_FOREIGN_FLEET");
    }

    void CreateNewFleetFromShips(const std::vector<int>& ship_ids) {
        if (ship_ids.empty())
            return;

        // get system where new fleet is to be created.
        int first_ship_id = ship_ids.front();
        const Ship* first_ship = GetObject<Ship>(first_ship_id);
        if (!first_ship) {
            Logger().errorStream() << "CreateNewFleetFromShips couldn't get ship with id " << first_ship_id;
            return;
        }
        int system_id = first_ship->SystemID();
        const System* system = GetObject<System>(system_id); // may be null
        if (!system) {
            Logger().errorStream() << "CreateNewFleetFromShips couldn't get a valid system with system id " << system_id;
            return;
        }

        double X = first_ship->X(), Y = first_ship->Y();


        int empire_id = HumanClientApp::GetApp()->EmpireID();
        const ObjectMap& objects = GetUniverse().Objects();

        // verify that all fleets are at the same system and position and owned by the same empire
        for (std::vector<int>::const_iterator it = ship_ids.begin(); it != ship_ids.end(); ++it) {
            const Ship* ship = objects.Object<Ship>(*it);
            if (!ship) {
                Logger().errorStream() << "CreateNewFleetFromShips couldn't get ship with id " << first_ship_id;
                return;
            }
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
        }


        // get new fleet id
        int new_fleet_id = GetNewObjectID();
        if (new_fleet_id == UniverseObject::INVALID_OBJECT_ID) {
            ClientUI::MessageBox(UserString("SERVER_TIMEOUT"), true);
            return;
        }


        // generate new fleet name
        std::string fleet_name = Fleet::GenerateFleetName(ship_ids, new_fleet_id);


        // fleet receiving drops is in a system
        HumanClientApp::GetApp()->Orders().IssueOrder(
            OrderPtr(new NewFleetOrder(empire_id, fleet_name, new_fleet_id, system_id, ship_ids)));
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
}


/** A FleetDetailPanel wrapped in a CUIWnd. */
class FleetDetailWnd : public CUIWnd {
public:
    /** \name Structors */ //@{
    ~FleetDetailWnd(); ///< virtual dtor
    //@}

    /** \name Mutators */ //@{
    Fleet*                  GetFleet() const;
    int                     FleetID() const;
    virtual void            CloseClicked();
    virtual void            SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    void                    Refresh();              ///< update title to show the current fleet name
    //@}

    static const GG::Pt&    LastPosition();         ///< returns the last position of the last FleetWnd that was closed
    static const GG::Pt&    LastSize();             ///< returns the last size ... ''

    mutable boost::signal<void (FleetDetailWnd*)> ClosingSignal;

private:
    /** \name Structors */ //@{
    /** Basic ctor. */
    FleetDetailWnd(int fleet_id, bool read_only, GG::Flags<GG::WndFlag> flags =
                   GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | GG::ONTOP | CLOSABLE);
    //@}

    void                    DoLayout();
    std::string             TitleText() const;

    FleetDetailPanel* m_fleet_panel;

    static GG::Pt     s_last_position;              ///< the latest position to which any FleetDetailWnd has been moved.
    static GG::Pt     s_last_size;                  ///< the latest size to which any FleetDetailWnd has been resized.

    boost::signals::connection  m_fleet_connection; ///< needed to keep track of the fleet name

    friend class FleetUIManager;
};


////////////////////////////////////////////////
// FleetUIManager
////////////////////////////////////////////////
FleetUIManager::FleetUIManager() :
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

FleetWnd* FleetUIManager::WndForFleet(const Fleet* fleet) const {
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

std::size_t FleetUIManager::OpenDetailWnds(FleetWnd* fleet_wnd) const {
    std::size_t retval = 0;
    FleetWndMap::const_iterator it = m_fleet_and_detail_wnds.find(fleet_wnd);
    if (it != m_fleet_and_detail_wnds.end())
        retval = it->second.size();
    return retval;
}

FleetWnd* FleetUIManager::NewFleetWnd(const std::vector<int>& fleet_ids, bool read_only,
                                      int selected_fleet_id/* = UniverseObject::INVALID_OBJECT_ID*/,
                                      GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | CLOSABLE*/)
{
    if (!GetOptionsDB().Get<bool>("UI.multiple-fleet-windows"))
        CloseAll();
    FleetWnd* retval = new FleetWnd(fleet_ids, read_only, selected_fleet_id, flags);

    m_fleet_wnds.insert(retval);
    GG::Connect(retval->ClosingSignal,  &FleetUIManager::FleetWndClosing,   this);
    GG::Connect(retval->ClickedSignal,  &FleetUIManager::FleetWndClicked,   this);
    GG::GUI::GetGUI()->Register(retval);
    return retval;
}

FleetWnd* FleetUIManager::NewFleetWnd(int system_id, int empire_id, bool read_only,
                                      int selected_fleet_id/* = UniverseObject::INVALID_OBJECT_ID*/,
                                      GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | CLOSABLE*/)
{
    if (!GetOptionsDB().Get<bool>("UI.multiple-fleet-windows"))
        CloseAll();
    FleetWnd* retval = new FleetWnd(system_id, empire_id, read_only, selected_fleet_id, flags);

    m_fleet_wnds.insert(retval);
    GG::Connect(retval->ClosingSignal,  &FleetUIManager::FleetWndClosing,   this);
    GG::Connect(retval->ClickedSignal,  &FleetUIManager::FleetWndClicked,   this);
    GG::GUI::GetGUI()->Register(retval);
    return retval;
}

FleetDetailWnd* FleetUIManager::NewFleetDetailWnd(FleetWnd* fleet_wnd, int fleet_id, bool read_only,
                                                  GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | GG::ONTOP | CLOSABLE*/)
{
    assert(fleet_wnd);
    assert(m_fleet_wnds.find(fleet_wnd) != m_fleet_wnds.end());


    // attempt to locate an existing FleetDetailWnd for passed fleet
    const std::set<FleetDetailWnd*>& detail_wnds = m_fleet_and_detail_wnds[fleet_wnd];
    for (std::set<FleetDetailWnd*>::const_iterator it = detail_wnds.begin(); it != detail_wnds.end(); ++it) {
        const FleetDetailWnd* detail_wnd = *it;
        if (detail_wnd->FleetID() == fleet_id)
            return 0;   // no new wnd needed
    }

    // couldn't find existing FleetDetailWnd for passed fleet.  make a new one.
    FleetDetailWnd* retval = new FleetDetailWnd(fleet_id, read_only, flags);
    m_fleet_and_detail_wnds[fleet_wnd].insert(retval);
    GG::Connect(retval->ClosingSignal, boost::bind(&FleetUIManager::FleetDetailWndClosing, this, fleet_wnd, _1));
    GG::GUI::GetGUI()->Register(retval);
    return retval;
}

void FleetUIManager::CullEmptyWnds() {
    // scan through FleetWnds, deleting those that have no fleets
    for (std::set<FleetWnd*>::iterator it = m_fleet_wnds.begin(); it != m_fleet_wnds.end(); ) {
        std::set<FleetWnd*>::iterator cur_wnd_it = it++;
        FleetWnd* cur_wnd = *cur_wnd_it;
        if (cur_wnd->FleetIDs().empty()) {
            delete cur_wnd;

        } else {
            // don't need to delete this FleetWnd, but need to check all its FleetDetailWnds
            std::set<FleetDetailWnd*>& detail_wnds = m_fleet_and_detail_wnds[cur_wnd];
            for (std::set<FleetDetailWnd*>::iterator it2 = detail_wnds.begin(); it2 != detail_wnds.end(); ) {
                std::set<FleetDetailWnd*>::iterator cur_detail_wnd_it = it2++;
                FleetDetailWnd* cur_detail_wnd = *cur_detail_wnd_it;
                Fleet* fleet = cur_detail_wnd->GetFleet();
                if (!fleet || !fleet->NumShips())
                    delete cur_detail_wnd;
            }
        }
    }
}

void FleetUIManager::SetActiveFleetWnd(FleetWnd* fleet_wnd) {
    if (fleet_wnd == m_active_fleet_wnd)
        return;

    // disconnect old active FleetWnd signal
    if (m_active_fleet_wnd)
        m_active_fleet_wnd_signal.disconnect();

    // set new active FleetWnd
    m_active_fleet_wnd = fleet_wnd;

    // connect new active FleetWnd selection changed signal
    m_active_fleet_wnd_signal = GG::Connect(m_active_fleet_wnd->SelectedFleetsChangedSignal, ActiveFleetWndSelectedFleetsChangedSignal);

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
    std::vector<FleetDetailWnd*> vec(m_fleet_and_detail_wnds[fleet_wnd].begin(), m_fleet_and_detail_wnds[fleet_wnd].end());
    for (std::size_t i = 0; i < vec.size(); ++i) {
        delete vec[i];
    }
    m_fleet_wnds.erase(fleet_wnd);
    m_fleet_and_detail_wnds.erase(fleet_wnd);
    if (active_wnd_affected)
        ActiveFleetWndChangedSignal();  // let anything that cares know the active fleetwnd just closed
}

void FleetUIManager::FleetDetailWndClosing(FleetWnd* fleet_wnd, FleetDetailWnd* fleet_detail_wnd)
{ m_fleet_and_detail_wnds[fleet_wnd].erase(fleet_detail_wnd); }

void FleetUIManager::FleetWndClicked(FleetWnd* fleet_wnd) {
    if (fleet_wnd == m_active_fleet_wnd)
        return;
    SetActiveFleetWnd(fleet_wnd);
}


namespace {
    ////////////////////////////////////////////////
    // Free Functions
    ////////////////////////////////////////////////
    bool    ValidShipTransfer(const Ship* ship, const Fleet* new_fleet) {
        if (!ship || !new_fleet)
            return false;   // can't transfer no ship or to no fleet

        const Fleet* current_fleet = GetObject<Fleet>(ship->FleetID());
        if (current_fleet && current_fleet->ID() == new_fleet->ID())
            return false;   // can't transfer a fleet to a fleet it already is in

        if (ship->X() != new_fleet->X() || ship->Y() != new_fleet->Y())
            return false;   // can't move fleets during a transfer.  can only transfer fleet at same location as ship

        if (ship->SystemID() != new_fleet->SystemID())
            return false;   // fleets need to be in same system.  probably redundant with checking position

        const std::set<int>& ship_owners = ship->Owners();
        const std::set<int>& fleet_owners = new_fleet->Owners();

        if (ship_owners.empty() || fleet_owners.empty())
            return false;   // need to own a ship to move it...

        if (ship_owners != fleet_owners)
            return false;   // need to have same owner(s)

        // all tests passed.  can transfer
        return true;
    }

    bool    ValidShipOwnerAndLocation(const Ship* ship, double x, double y, int system_id, int empire) {
        if (!ship)
            return false;   // not a ship?

        if (ship->X() != x || ship->Y() != y)
            return false;   // ship at wrong location

        if (ship->SystemID() != system_id)
            return false;   // ship at wrong system

        const std::set<int>& owners = ship->Owners();
        if (owners.find(empire) == owners.end())
            return false;   // ship not owned by empire

        // all tests passed; valid ship
        return true;
    }

    bool    ValidFleetMerge(const Fleet* fleet, const Fleet* target_fleet) {
        if (!fleet || !target_fleet)
            return false;   // missing objects

        if (fleet->SystemID() != target_fleet->SystemID())
            return false;   // at different systems

        if (fleet->X() != target_fleet->X() || fleet->Y() != target_fleet->Y())
            return false;   // at different locations.

        const std::set<int>& fleet_owners = fleet->Owners();
        const std::set<int>& target_fleet_owners = target_fleet->Owners();

        if (fleet_owners != target_fleet_owners)
            return false;   // different owners

        // all tests passed; can merge fleets
        return true;
    }

    ////////////////////////////////////////////////
    // ShipDataPanel
    ////////////////////////////////////////////////
    /** Represents a single ship.  This class is used as the sole Control in
      * each ShipRow. */
    class ShipDataPanel : public GG::Control {
    public:
        ShipDataPanel(GG::X w, GG::Y h, int ship_id) :
            Control(GG::X0, GG::Y0, w, h, GG::Flags<GG::WndFlag>()),
            m_ship_id(ship_id),
            m_ship_icon(0),
            m_scrap_indicator(0),
            m_ship_name_text(0),
            m_design_name_text(0),
            m_stat_icons(),
            m_selected(false)
        {
            const Ship* ship = GetObject<Ship>(m_ship_id);

            SetChildClippingMode(ClipToClient);

            // ship name text.  blank if no ship.  TODO: if no ship show "No Ship" or somesuch?
            std::string ship_name = "";
            if (ship)
                ship_name = ship->Name();

            m_ship_name_text = new GG::TextControl(GG::X(Value(h)), GG::Y0, GG::X1, LabelHeight(),
                                                   ship_name, ClientUI::GetFont(),
                                                   ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
            AttachChild(m_ship_name_text);


            // design name and statistic icons
            if (ship) {
                if (const ShipDesign* design = ship->Design()) {
                    m_design_name_text = new GG::TextControl(GG::X(Value(h)), GG::Y0, GG::X1, LabelHeight(),
                                                             design->Name(), ClientUI::GetFont(),
                                                             ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
                    AttachChild(m_design_name_text);
                }


                int tooltip_delay = GetOptionsDB().Get<int>("UI.tooltip-delay");


                // meter stat icons
                std::vector<MeterType> meters;
                meters.push_back(METER_HEALTH);     meters.push_back(METER_FUEL);   meters.push_back(METER_DETECTION);
                meters.push_back(METER_STEALTH);    meters.push_back(METER_SHIELD);

                for (std::vector<MeterType>::const_iterator it = meters.begin(); it != meters.end(); ++it) {
                    StatisticIcon* icon = new StatisticIcon(GG::X0, GG::Y0, StatIconSize().x, StatIconSize().y,
                                                            ClientUI::MeterIcon(*it), 0, 0, false);
                    m_stat_icons.push_back(std::make_pair(MeterStatString(*it), icon));
                    AttachChild(icon);
                    icon->SetBrowseModeTime(tooltip_delay);
                }


                // speed stat icon
                StatisticIcon* icon = new StatisticIcon(GG::X0, GG::Y0, StatIconSize().x, StatIconSize().y,
                                                        SpeedIcon(), 0, 0, false);
                m_stat_icons.push_back(std::make_pair(SPEED_STAT_STRING, icon));
                AttachChild(icon);
                icon->SetBrowseModeTime(tooltip_delay);


                // bookkeeping
                m_ship_connection = GG::Connect(ship->StateChangedSignal, &ShipDataPanel::Refresh, this);

                if (Fleet* fleet = GetObject<Fleet>(ship->FleetID()))
                    m_fleet_connection = GG::Connect(fleet->StateChangedSignal, &ShipDataPanel::Refresh, this);

                Refresh();
            }
        }

        ~ShipDataPanel() {
            m_ship_connection.disconnect();
            m_fleet_connection.disconnect();
        }

        /** Excludes border from the client area. */
        virtual GG::Pt ClientUpperLeft() const
        {
            return UpperLeft() + GG::Pt(GG::X(DATA_PANEL_BORDER), GG::Y(DATA_PANEL_BORDER));
        }

        /** Excludes border from the client area. */
        virtual GG::Pt ClientLowerRight() const
        {
            return LowerRight() - GG::Pt(GG::X(DATA_PANEL_BORDER), GG::Y(DATA_PANEL_BORDER)); 
        }

        /** Renders black panel background, border with color depending on the current state and a background for the ship's name text. */
        virtual void    Render() {
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

        void            Select(bool b) {
            if (m_selected != b) {
                m_selected = b;

                const GG::Clr& unselected_text_color = ClientUI::TextColor();
                const GG::Clr& selected_text_color = GG::CLR_BLACK;

                GG::Clr text_color_to_use = m_selected ? selected_text_color : unselected_text_color;

                if (Disabled())
                    text_color_to_use = DisabledColor(text_color_to_use);

                m_ship_name_text->SetTextColor(text_color_to_use);
                if (m_design_name_text)
                    m_design_name_text->SetTextColor(text_color_to_use);
            }
        }

        virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            const GG::Pt old_size = Size();
            GG::Control::SizeMove(ul, lr);
            //std::cout << "ShipDataPanel::SizeMove new size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
            if (old_size != Size())
                DoLayout();
        }

    private:
        void            SetShipIcon() {
            delete m_ship_icon;
            m_ship_icon = 0;

            delete m_scrap_indicator;
            m_scrap_indicator = 0;

            const Ship* ship = GetObject<Ship>(m_ship_id);
            if (!ship)
                return;

            boost::shared_ptr<GG::Texture> icon;

            if (const ShipDesign* design = ship->Design())
                icon = ClientUI::ShipIcon(design->ID());
            else
                icon = ClientUI::ShipIcon(UniverseObject::INVALID_OBJECT_ID);  // default icon

            m_ship_icon = new GG::StaticGraphic(GG::X0, GG::Y0, DATA_PANEL_ICON_SPACE.x, ClientHeight(), icon, DataPanelIconStyle());
            AttachChild(m_ship_icon);

            if (ship->OrderedScrapped()) {
                boost::shared_ptr<GG::Texture> scrap_texture = ClientUI::GetTexture(ClientUI::ArtDir() / "misc" / "scrapped.png", true);
                m_scrap_indicator = new GG::StaticGraphic(GG::X0, GG::Y0, DATA_PANEL_ICON_SPACE.x, ClientHeight(), scrap_texture, DataPanelIconStyle());
                AttachChild(m_scrap_indicator);
            }
        }

        void            Refresh() {
            SetShipIcon();

            const Ship* ship = GetObject<Ship>(m_ship_id);
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
            std::string ship_name = ship->Name();
            if (ship_name.empty()) {
                if (ship->Unowned() || ship->OwnedBy(empire_id))
                    ship_name = UserString("SHIP");
                else
                    ship_name = UserString("FOREIGN_SHIP");
            }
            m_ship_name_text->SetText(ship_name);

            if (m_design_name_text) {
                std::string design_name = UserString("FW_UNKNOWN_DESIGN_NAME");
                if (const ShipDesign* design = ship->Design())
                    design_name = design->Name();
                m_design_name_text->SetText(design_name);
            }


            // update stat icon values and browse wnds
            for (std::vector<std::pair<std::string, StatisticIcon*> >::const_iterator it = m_stat_icons.begin(); it != m_stat_icons.end(); ++it) {
                //std::cout << "setting ship stat " << it->first << " to value: " << StatValue(it->first) << std::endl;
                it->second->SetValue(StatValue(it->first));

                it->second->ClearBrowseInfoWnd();
                if (it->first == SPEED_STAT_STRING) {
                    const std::string speed_stat_title = UserString("SHIP_SPEED_STAT_TITLE");
                    const std::string speed_stat_main = UserString("SHIP_SPEED_STAT_MAIN");
                    boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd(new IconTextBrowseWnd(SpeedIcon(), speed_stat_title, speed_stat_main));
                    it->second->SetBrowseInfoWnd(browse_wnd);
                } else {
                    MeterType meter_type = MeterTypeFromStatString(it->first);
                    MeterType associated_meter_type = AssociatedMeterType(meter_type);
                    boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd(new MeterBrowseWnd(m_ship_id, meter_type, associated_meter_type));
                    it->second->SetBrowseInfoWnd(browse_wnd);
                }
            }


            DoLayout();
        }

        double StatValue(const std::string& stat_name) const {
            if (const Ship* ship = GetObject<Ship>(m_ship_id)) {
                if (stat_name == SPEED_STAT_STRING) {
                    return ship->Speed();
                } else {
                    MeterType meter_type = MeterTypeFromStatString(stat_name);
                    //std::cout << "got meter type " << boost::lexical_cast<std::string>(meter_type) << " from stat_name " << stat_name << std::endl;
                    if (ship->UniverseObject::GetMeter(meter_type)) {
                        //std::cout << " ... ship has meter! returning meter points value " << ship->CurrentMeterValue(meter_type) << std::endl;
                        return ship->CurrentMeterValue(meter_type);
                    }
                }
            }
            return 0.0;
        }

        void            DoLayout() {
            // resize ship and scrap indicator icons, they can fit and position themselves in the space provided
            // client height should never be less than the height of the space resereved for the icon
            if (m_ship_icon)
                m_ship_icon->Resize(GG::Pt(DATA_PANEL_ICON_SPACE.x, ClientHeight()));
            if (m_scrap_indicator)
                m_scrap_indicator->Resize(GG::Pt(DATA_PANEL_ICON_SPACE.x, ClientHeight()));

            // position ship name text at the top to the right of icons
            const GG::Pt name_ul = GG::Pt(DATA_PANEL_ICON_SPACE.x + DATA_PANEL_TEXT_PAD, GG::Y0);
            const GG::Pt name_lr = GG::Pt(ClientWidth() - DATA_PANEL_TEXT_PAD,           LabelHeight());
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

        int                         m_ship_id;
        GG::StaticGraphic*          m_ship_icon;
        GG::StaticGraphic*          m_scrap_indicator;
        GG::TextControl*            m_ship_name_text;
        GG::TextControl*            m_design_name_text;

        std::vector<std::pair<std::string, StatisticIcon*> >    m_stat_icons;   // statistic icons and associated meter types

        bool                        m_selected;
        boost::signals::connection  m_ship_connection;
        boost::signals::connection  m_fleet_connection;
    };


    ////////////////////////////////////////////////
    // ShipRow
    ////////////////////////////////////////////////
    /** A ListBox::Row subclass used to represent ships in ShipListBoxes. */
    class ShipRow : public GG::ListBox::Row {
    public:
        ShipRow(GG::X w, GG::Y h, int ship_id) :
            GG::ListBox::Row(w, h, ""),
            m_ship_id(ship_id)
        {
            SetName("ShipRow");
            SetChildClippingMode(ClipToClient);
            if (GetObject<Ship>(m_ship_id))
                SetDragDropDataType(SHIP_DROP_TYPE_STRING);
            push_back(new ShipDataPanel(w, h, m_ship_id));
        }

        void            SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            const GG::Pt old_size = Size();
            GG::ListBox::Row::SizeMove(ul, lr);
            //std::cout << "ShipRow::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
            if (!empty() && old_size != Size())
                at(0)->Resize(Size());
        }

        int             ShipID() const {return m_ship_id;}
        Ship*           GetShip() const {return GetObject<Ship>(m_ship_id);}

    private:
        int m_ship_id;
    };
}

////////////////////////////////////////////////
// FleetDataPanel
////////////////////////////////////////////////
/** Represents a single fleet.  This class is used as the drop-target in
  * FleetWnd (if the ctor parameter \a fleet is zero), and also as the sole
  * Control in each FleetRow (if the ctor parameter \a fleet is nonzero). */
class FleetDataPanel : public GG::Control {
public:
    FleetDataPanel(GG::X w, GG::Y h, int fleet_id, int empire = ALL_EMPIRES,
                   int system_id = UniverseObject::INVALID_OBJECT_ID,
                   double x = UniverseObject::INVALID_POSITION, double y = UniverseObject::INVALID_POSITION);

    virtual GG::Pt      ClientUpperLeft() const;  ///< upper left plus border insets
    virtual GG::Pt      ClientLowerRight() const; ///< lower right minus border insets

    virtual void        DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last, const GG::Pt& pt) const;

    bool                Selected() const;

    virtual void        Render();
    virtual void        DragDropEnter(const GG::Pt& pt, const std::map<GG::Wnd*, GG::Pt>& drag_drop_wnds, GG::Flags<GG::ModKey> mod_keys);
    virtual void        DragDropLeave();
    virtual void        AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt);
    void                Select(bool b);
    virtual void        SizeMove(const GG::Pt& ul, const GG::Pt& lr);

    mutable boost::signal<void (const std::vector<int>&)> NewFleetFromShipsSignal;

private:
    void                Refresh();
    double              StatValue(const std::string& stat_name) const;
    std::string         StatTooltip(const std::string& stat_name) const;
    void                DoLayout();

    const int           m_fleet_id;
    const int           m_empire;
    const int           m_system_id;
    const double        m_x;
    const double        m_y;

    boost::signals::connection  m_fleet_connection;

    GG::Control*        m_fleet_icon;
    GG::TextControl*    m_fleet_name_text;
    GG::TextControl*    m_fleet_destination_text;

    std::vector<std::pair<std::string, StatisticIcon*> >    m_stat_icons;   // statistic icons and associated meter types

    bool                m_selected;
};


FleetDataPanel::FleetDataPanel(GG::X w, GG::Y h, int fleet_id,
                               int empire, int system_id, double x, double y) :
    Control(GG::X0, GG::Y0, w, h, GetObject<Fleet>(fleet_id) ? GG::Flags<GG::WndFlag>() : GG::INTERACTIVE),
    m_fleet_id(fleet_id),
    m_empire(empire),
    m_system_id(system_id),
    m_x(x),
    m_y(y),
    m_fleet_icon(0),
    m_fleet_name_text(0),
    m_fleet_destination_text(0),
    m_stat_icons(),
    m_selected(false)
{
    const Fleet* fleet = GetObject<Fleet>(m_fleet_id);

    SetChildClippingMode(ClipToClient);

    m_fleet_name_text = new GG::TextControl(GG::X0, GG::Y0, GG::X1, LabelHeight(), "", ClientUI::GetFont(),
                                            ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    AttachChild(m_fleet_name_text);

    m_fleet_destination_text = new GG::TextControl(GG::X0, GG::Y0, GG::X1, LabelHeight(), "", ClientUI::GetFont(),
                                                   ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
    AttachChild(m_fleet_destination_text);

    if (fleet) {
        int tooltip_delay = GetOptionsDB().Get<int>("UI.tooltip-delay");

        // stat icon for fleet fuel
        StatisticIcon* icon = new StatisticIcon(GG::X0, GG::Y0, StatIconSize().x, StatIconSize().y,
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
    }

    Refresh();
}

GG::Pt FleetDataPanel::ClientUpperLeft() const
{
    return UpperLeft() + GG::Pt(GG::X(DATA_PANEL_BORDER), GG::Y(DATA_PANEL_BORDER));
}

GG::Pt FleetDataPanel::ClientLowerRight() const
{
    return LowerRight() - GG::Pt(GG::X(DATA_PANEL_BORDER), GG::Y(DATA_PANEL_BORDER)); 
}

bool FleetDataPanel::Selected() const
{
    return m_selected;
}

void FleetDataPanel::Render()
{
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

void FleetDataPanel::DragDropEnter(const GG::Pt& pt, const std::map<GG::Wnd*, GG::Pt>& drag_drop_wnds, GG::Flags<GG::ModKey> mod_keys)
{
    Select(true);   // default

    // make map from Wnd to bool indicating whether it is acceptable to drop here
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
{
    Select(false);
}

void FleetDataPanel::DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last, const GG::Pt& pt) const
{
    // for every Wnd being dropped...
    for (DropsAcceptableIter it = first; it != last; ++it) {
        it->second = false; // default

        // reject drops if a dropped Wnd isn't a valid ShipRow
        if (it->first->DragDropDataType() != SHIP_DROP_TYPE_STRING)
            continue;

        // reject drops if a ship doesn't exist or isn't owned by the empire that owns the fleet in this panel
        const ShipRow* ship_row = boost::polymorphic_downcast<const ShipRow*>(it->first);
        if (!ship_row)
            continue;

        // reject nonexistant or invalid ships
        const Ship* ship = ship_row->GetShip();
        if (!ship || !ValidShipOwnerAndLocation(ship, m_x, m_y, m_system_id, m_empire))
            continue;

        // all tests passed; can drop
        it->second = true;
    }
}

void FleetDataPanel::AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt)
{
    std::vector<int> ship_ids;
    for (std::vector<Wnd*>::const_iterator it = wnds.begin(); it != wnds.end(); ++it) {
        const GG::Wnd* wnd = *it;

        if (wnd->DragDropDataType() != SHIP_DROP_TYPE_STRING)
            continue;

        const ShipRow* ship_row = boost::polymorphic_downcast<const ShipRow*>(wnd);
        if (!ship_row)
            continue;

        ship_ids.push_back(ship_row->ShipID());
    }
    NewFleetFromShipsSignal(ship_ids);
}

void FleetDataPanel::Select(bool b)
{
    if (m_selected != b) {
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
}

void FleetDataPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    const GG::Pt old_size = Size();
    GG::Control::SizeMove(ul, lr);
    //std::cout << "FleetDataPanel::SizeMove new size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
    if (old_size != Size())
        DoLayout();
}

void FleetDataPanel::Refresh()
{
    DeleteChild(m_fleet_icon);
    m_fleet_icon = 0;

    if (const Fleet* fleet = GetObject<Fleet>(m_fleet_id)) {
        // set fleet name and destination text
        m_fleet_name_text->SetText(FleetNameText(m_fleet_id));
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

        // set stat icon values
        for (std::vector<std::pair<std::string, StatisticIcon*> >::const_iterator it = m_stat_icons.begin(); it != m_stat_icons.end(); ++it) {
            it->second->SetValue(StatValue(it->first));
        }
    } else {
        // this class has two roles.  if there is a fleet object attached to it
        // it displays that fleet's info.  if no fleet object is attached, then
        // the class becomes a UI drop target for creating new fleets.
        m_fleet_name_text->SetText(UserString("FW_NEW_FLEET_LABEL"));
        m_fleet_destination_text->Clear();
    }

    DoLayout();
}

double FleetDataPanel::StatValue(const std::string& stat_name) const
{
    if (const Fleet* fleet = GetObject<Fleet>(m_fleet_id)) {
        if (stat_name == SPEED_STAT_STRING)
            return fleet->Speed();
        else if (stat_name == MeterStatString(METER_FUEL))
            return fleet->Fuel();
    }
    return 0.0;
}

std::string FleetDataPanel::StatTooltip(const std::string& stat_name) const
{
    if (stat_name == SPEED_STAT_STRING)
        return UserString("FW_FLEET_SPEED_SUMMARY");
    else if (stat_name == MeterStatString(METER_FUEL))
        return UserString("FW_FLEET_FUEL_SUMMARY");
    else
        return "";
}

void FleetDataPanel::DoLayout()
{
    if (m_fleet_icon) {
        // fleet icon will scale and position itself in the provided space
        m_fleet_icon->Resize(GG::Pt(DATA_PANEL_ICON_SPACE.x, ClientHeight()));
    }

    // position fleet name and destination texts
    const GG::Pt name_ul = GG::Pt(DATA_PANEL_ICON_SPACE.x + DATA_PANEL_TEXT_PAD, GG::Y0);
    const GG::Pt name_lr = GG::Pt(ClientWidth() - DATA_PANEL_TEXT_PAD,           LabelHeight());
    m_fleet_name_text->SizeMove(name_ul, name_lr);
    m_fleet_destination_text->SizeMove(name_ul, name_lr);

    // position stat icons, centering them vertically if there's more space than required
    GG::Pt icon_ul = GG::Pt(name_ul.x, LabelHeight() + std::max(GG::Y0, (ClientHeight() - LabelHeight() - StatIconSize().y) / 2));
    for (std::vector<std::pair<std::string, StatisticIcon*> >::const_iterator it = m_stat_icons.begin(); it != m_stat_icons.end(); ++it) {
        it->second->SizeMove(icon_ul, icon_ul + StatIconSize());
        icon_ul.x += StatIconSize().x;
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
            GG::ListBox::Row(w, h, GetObject<Fleet>(fleet_id) ? FLEET_DROP_TYPE_STRING : ""),
            m_fleet_id(fleet_id)
        {
            SetName("FleetRow");
            SetChildClippingMode(ClipToClient);
            push_back(new FleetDataPanel(w, h, m_fleet_id));
        }

        virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            const GG::Pt old_size = Size();
            GG::ListBox::Row::SizeMove(ul, lr);
            //std::cout << "FleetRow::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
            if (!empty() && old_size != Size())
                at(0)->Resize(Size());
        }

        int             FleetID() const {return m_fleet_id;}
        Fleet*          GetFleet() const {return GetObject<Fleet>(m_fleet_id);}
    private:
        int m_fleet_id;
    };
}

////////////////////////////////////////////////
// FleetsListBox
////////////////////////////////////////////////
/** A CUIListBox subclass used to list all the fleets, and handle drag-and-drop
  * operations on them, in FleetWnd. */
class FleetsListBox : public CUIListBox {
public:
    FleetsListBox(GG::X x, GG::Y y, GG::X w, GG::Y h, bool read_only) :
        CUIListBox(x, y, w, h),
        m_highlighted_row_it(end()),
        m_read_only(read_only)
    {
        InitRowSizes();
    }

    FleetsListBox(bool read_only) :
        CUIListBox(GG::X0, GG::Y0, GG::X1, GG::Y1),
        m_highlighted_row_it(end()),
        m_read_only(read_only)
    {
        InitRowSizes();
    }

    virtual void    DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last, const GG::Pt& pt) const {
        // default result, possibly to be updated later: reject all drops
        for (DropsAcceptableIter it = first; it != last; ++it)
            it->second = false;


        // early termination check: if this FleetsListBox is read only, all drops are unacceptable
        if (m_read_only)
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
        const Fleet* target_fleet = 0;
        if (const FleetRow* fleet_row = boost::polymorphic_downcast<const FleetRow*>(*row))
            target_fleet = fleet_row->GetFleet();


        // loop through dropped Wnds, checking if each is a valid ship or fleet.  this doesn't
        // consider whether there is a mixture of fleets and ships, as each row is considered
        // independently.  actual drops will probably only accept one or the other, not a mixture
        // of fleets and ships being dropped simultaneously.

        for (DropsAcceptableIter it = first; it != last; ++it) {

            // for either of fleet or ship being dropped, check if merge or transfer is valid.
            // if any of the nested if's fail, the default rejection of the drop will remain set

            if (it->first->DragDropDataType() == FLEET_DROP_TYPE_STRING) {
                if (const FleetRow* fleet_row = boost::polymorphic_downcast<const FleetRow*>(it->first))
                    if (const Fleet* fleet = fleet_row->GetFleet())
                        it->second = ValidFleetMerge(fleet, target_fleet);

            } else if (it->first->DragDropDataType() == SHIP_DROP_TYPE_STRING) {
                if (const ShipRow* ship_row = boost::polymorphic_downcast<const ShipRow*>(it->first))
                    if (const Ship* ship = ship_row->GetShip())
                        it->second = ValidShipTransfer(ship, target_fleet);
            } else {
                // no valid drop type string
                std::cerr << "row drop " << std::endl;
            }
        }
    }

    virtual void    AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt) {
        //std::cout << "FleetsListBox::AcceptDrops" << std::endl;
        assert(!wnds.empty());

        iterator drop_target_row = RowUnderPt(pt);
        assert(!m_read_only && drop_target_row != end());


        // get drop target fleet
        const FleetRow* drop_target_fleet_row = boost::polymorphic_downcast<FleetRow*>(*drop_target_row);
        assert(drop_target_fleet_row);

        Fleet* target_fleet = drop_target_fleet_row->GetFleet();
        assert(target_fleet);

        int target_fleet_id = target_fleet->ID();


        // sort dropped Wnds to extract fleets or ships dropped.  (should only be one or the other in a given drop)
        std::vector<Fleet*> dropped_fleets;
        std::vector<Ship*> dropped_ships;

        for (std::vector<Wnd*>::const_iterator it = wnds.begin(); it != wnds.end(); ++it) {
            const GG::Wnd* wnd = *it;

            if (wnd->DragDropDataType() == FLEET_DROP_TYPE_STRING) {
                const FleetRow* fleet_row = boost::polymorphic_downcast<const FleetRow*>(wnd);
                assert(fleet_row);
                dropped_fleets.push_back(fleet_row->GetFleet());

            } else if (wnd->DragDropDataType() == SHIP_DROP_TYPE_STRING) {
                const ShipRow* ship_row = boost::polymorphic_downcast<const ShipRow*>(wnd);
                assert(ship_row);
                dropped_ships.push_back(ship_row->GetShip());
            }
        }

        assert(dropped_ships.empty() != dropped_fleets.empty());    // should only be dropping fleets or ships, not a mix of both


        int empire_id = HumanClientApp::GetApp()->EmpireID();
        const ObjectMap& objects = GetUniverse().Objects();


        if (!dropped_fleets.empty()) {
            // dropping fleets.  get each ships of all source fleets and transfer to the target fleet
            //std::cout << ".... dropped " << dropped_fleets.size() << " fleets" << std::endl;

            for (std::vector<Fleet*>::const_iterator it = dropped_fleets.begin(); it != dropped_fleets.end(); ++it) {
                const Fleet* dropped_fleet = *it;
                assert(dropped_fleet);
                int dropped_fleet_id = dropped_fleet->ID();

                // get fleet's ships in a vector, as this is what FleetTransferOrder takes
                const std::set<int>& ship_ids_set = dropped_fleet->ShipIDs();
                const std::vector<int> ship_ids_vec(ship_ids_set.begin(), ship_ids_set.end());

                // order the transfer
                HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(
                    new FleetTransferOrder(empire_id, dropped_fleet_id, target_fleet_id, ship_ids_vec)));

                // delete empty fleets
                if (dropped_fleet->Empty())
                    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(
                        new DeleteFleetOrder(empire_id, dropped_fleet_id)));
            }

        } else if (!dropped_ships.empty()) {
            // dropping ships.  transfer to target fleet.
            //std::cout << ".... dropped " << dropped_ships.size() << " ships" << std::endl;

            // get source fleet of ship(s).  assumes all ships are from the same source fleet.
            const Ship* first_ship = dropped_ships[0];
            assert(first_ship);
            int fleet_id = first_ship->FleetID();

            // compile ship IDs into a vector, while also recording original fleets from which ships are being taken
            std::vector<int> ship_ids_vec;
            std::set<int> dropped_ships_fleets;
            for (std::vector<Ship*>::const_iterator it = dropped_ships.begin(); it != dropped_ships.end(); ++it) {
                const Ship* ship = *it;
                assert(ship);
                ship_ids_vec.push_back(ship->ID());
                dropped_ships_fleets.insert(ship->FleetID());
            }

            // order the transfer
            HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(
                new FleetTransferOrder(empire_id, fleet_id, target_fleet_id, ship_ids_vec)));

            // delete empty fleets
            for (std::set<int>::const_iterator it = dropped_ships_fleets.begin(); it != dropped_ships_fleets.end(); ++it) {
                const Fleet* fleet = objects.Object<Fleet>(*it);
                if (fleet && fleet->Empty())
                    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(
                        new DeleteFleetOrder(empire_id, fleet->ID())));
            }
        }
    }

    virtual void    DragDropEnter(const GG::Pt& pt, const std::map<Wnd*, GG::Pt>& drag_drop_wnds, GG::Flags<GG::ModKey> mod_keys) {
        DragDropHere(pt, drag_drop_wnds, mod_keys);
    }

    virtual void    DragDropHere(const GG::Pt& pt, const std::map<Wnd*, GG::Pt>& drag_drop_wnds, GG::Flags<GG::ModKey> mod_keys) {
        CUIListBox::DragDropHere(pt, drag_drop_wnds, mod_keys);


        // get FleetRow under drop point
        iterator row_it = RowUnderPt(pt);


        // default to removing highlighting of any row that has it.
        // used to check: if (m_highlighted_row_it != row_it) before doing this...
        ClearHighlighting();


        // abort if this FleetsListBox can't be manipulated or if 
        if (m_read_only || row_it == end())
            return;


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

        Fleet* drop_target_fleet = drop_target_fleet_row->GetFleet();
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
                Fleet* fleet = fleet_row->GetFleet();

                if (!ValidFleetMerge(fleet, drop_target_fleet))
                    return; // not a valid drop

            } else if (dropped_wnd->DragDropDataType() == SHIP_DROP_TYPE_STRING) {
                ships_seen = true;
                if (fleets_seen)
                    return; // can't drop both at once

                const ShipRow* ship_row = boost::polymorphic_downcast<const ShipRow*>(dropped_wnd);
                assert(ship_row);
                Ship* ship = ship_row->GetShip();

                if (!ValidShipTransfer(ship, drop_target_fleet))
                    return; // not a valid drop
            }
        }

        // passed all checks.  drop is valid!
        HighlightRow(row_it);
    }

    virtual void    DragDropLeave() {
        ClearHighlighting();
    }

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

    GG::Pt          ListRowSize() const {
        return GG::Pt(Width() - ClientUI::ScrollWidth() - 5, ListRowHeight());
    }

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
    const bool  m_read_only;
};

////////////////////////////////////////////////
// ShipsListBox
////////////////////////////////////////////////
/** A CUIListBox subclass used to list all the ships, and handle drag-and-drop
  * operations on them, in FleetDetailPanel. */
class ShipsListBox : public CUIListBox {
public:
    ShipsListBox(GG::X x, GG::Y y, GG::X w, GG::Y h, int fleet_id, bool read_only) :
        CUIListBox(x, y, w, h),
        m_fleet_id(fleet_id),
        m_read_only(read_only)
    {}

    ShipsListBox(int fleet_id, bool read_only) :
        CUIListBox(GG::X0, GG::Y0, GG::X1, GG::Y1),
        m_fleet_id(fleet_id),
        m_read_only(read_only)
    {}

    void            Refresh() {
        Clear();

        const Fleet* fleet = GetObject<Fleet>(m_fleet_id);
        if (!fleet)
            return;

        const GG::Pt row_size = ListRowSize();

        // preinitialize listbox/row column widths, because what
        // ListBox::Insert does on default is not suitable for this case
        SetNumCols(1);
        SetColWidth(0, GG::X0);
        LockColWidths();

        for (Fleet::const_iterator it = fleet->begin(); it != fleet->end(); ++it) {
            int ship_id = *it;
            ShipRow* row = new ShipRow(GG::X1, row_size.y, ship_id);
            Insert(row);
            row->Resize(row_size);
        }
    }

    void            SetFleet(int fleet_id) {
        if (m_fleet_id == fleet_id)
            return;

        m_fleet_id = fleet_id;
        Refresh();
    }

    virtual void    DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last, const GG::Pt& pt) const {
        for (DropsAcceptableIter it = first; it != last; ++it) {
            it->second = false; // default


            if (m_read_only)
                continue;   // leave false: not modifyable ShipsListBox


            const ShipRow* ship_row = dynamic_cast<const ShipRow*>(it->first);
            if (!ship_row)
                continue;   // leave false: dropped Wnd not a ShipRow


            const Ship* ship = ship_row->GetShip();
            if (!ship) {
                Logger().errorStream() << "ShipsListBox::DropsAcceptable couldn't get ship for ship row";
                continue;
            }

            const Fleet* fleet = GetObject<Fleet>(ship->FleetID());
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

        Ship* ship_from_dropped_wnd = 0;
        std::vector<int> ship_ids;
        for (std::vector<GG::Wnd*>::const_iterator it = wnds.begin(); it != wnds.end(); ++it) {
            const GG::Wnd* wnd = *it;
            if (wnd->DragDropDataType() == SHIP_DROP_TYPE_STRING) {
                const ShipRow* row = boost::polymorphic_downcast<const ShipRow*>(wnd);
                assert(row);
                ship_ids.push_back(row->ShipID());
                ship_from_dropped_wnd = row->GetShip();
            }
        }


        if (!ship_from_dropped_wnd)
            return;

        int dropped_ship_fleet_id = ship_from_dropped_wnd->FleetID();
        int empire_id = HumanClientApp::GetApp()->EmpireID();

        HumanClientApp::GetApp()->Orders().IssueOrder(
            OrderPtr(new FleetTransferOrder(empire_id, dropped_ship_fleet_id, m_fleet_id, ship_ids)));
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

    Fleet*          GetFleet() const {return GetObject<Fleet>(m_fleet_id);}

private:
    int         m_fleet_id;
    const bool  m_read_only;
};


////////////////////////////////////////////////
// FleetDetailPanel
////////////////////////////////////////////////
/** Used in FleetDetailWnd and in the lower half of FleetWnd to show the
  * ships in a fleet, and some basic info about the fleet. */
class FleetDetailPanel : public GG::Wnd {
public:
    FleetDetailPanel(GG::X w, GG::Y h, int fleet_id, bool read_only, GG::Flags<GG::WndFlag> flags = GG::Flags<GG::WndFlag>()); ///< ctor

    Fleet*          GetFleet() const;           ///< returns the currently-displayed fleet  may be 0
    int             FleetID() const;
    void            SetFleet(int fleet_id);     ///< sets the currently-displayed Fleet.  setting to UniverseObject::INVALID_OBJECT_ID shows no fleet

    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);

private:
    int             GetShipIDOfListRow(GG::ListBox::iterator it) const; ///< returns the ID number of the ship in row \a row_idx of the ships listbox
    void            Refresh();
    void            DoLayout();
    void            UniverseObjectDeleted(const UniverseObject* obj);
    void            ShipSelectionChanged(const GG::ListBox::SelectionSet& rows);
    void            ShipBrowsed(GG::ListBox::iterator it);
    void            ShipRightClicked(GG::ListBox::iterator it, const GG::Pt& pt);

    int                         m_fleet_id;
    const bool                  m_read_only;
    boost::signals::connection  m_fleet_connection;

    ShipsListBox*               m_ships_lb;
};


FleetDetailPanel::FleetDetailPanel(GG::X w, GG::Y h, int fleet_id, bool read_only, GG::Flags<GG::WndFlag> flags/* = GG::Flags<GG::WndFlag>()*/) :
    GG::Wnd(GG::X0, GG::Y0, w, h, flags),
    m_fleet_id(UniverseObject::INVALID_OBJECT_ID),
    m_read_only(read_only),
    m_ships_lb(0)
{
    SetName("FleetDetailPanel");
    SetChildClippingMode(ClipToClient);

    m_ships_lb = new ShipsListBox(0, read_only);
    AttachChild(m_ships_lb);
    m_ships_lb->SetHiliteColor(GG::CLR_ZERO);

    SetFleet(fleet_id);

    if (m_read_only) {
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
{
    return boost::polymorphic_downcast<ShipRow*>(*it)->ShipID();
}

void FleetDetailPanel::SetFleet(int fleet_id)
{
    // save old fleet id and set to new id
    int old_fleet_id = m_fleet_id;
    m_fleet_id = fleet_id;

    // if set fleet changed, or if current fleet is no fleet, disconnect any 
    // signals related to that fleet
    if (m_fleet_id != old_fleet_id || m_fleet_id == UniverseObject::INVALID_OBJECT_ID)
        m_fleet_connection.disconnect();

    if (m_fleet_id == old_fleet_id)
        m_ships_lb->Refresh();
    else
        m_ships_lb->SetFleet(fleet_id);

    if (const Fleet* fleet = GetObject<Fleet>(fleet_id)) {
        if (fleet && !fleet->Empty()) {
            // update desintation text and change signal connection
            if (old_fleet_id != fleet_id)
                m_fleet_connection = GG::Connect(fleet->StateChangedSignal, &FleetDetailPanel::Refresh, this, boost::signals::at_front);
        } else {
            Logger().debugStream() << "FleetDetailPanel::SetFleet ignoring set to missing or empty fleet id (" << fleet_id << ")";
        }
    }
}

Fleet* FleetDetailPanel::GetFleet() const
{
    return GetObject<Fleet>(m_fleet_id);
}

int FleetDetailPanel::FleetID() const
{
    return m_fleet_id;
}

void FleetDetailPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    const GG::Pt old_size = Size();
    GG::Wnd::SizeMove(ul, lr);
    if (old_size != Size())
        DoLayout();
}

void FleetDetailPanel::Refresh()
{
    SetFleet(m_fleet_id);
}

void FleetDetailPanel::DoLayout()
{
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

void FleetDetailPanel::UniverseObjectDeleted(const UniverseObject* obj)
{
    if (obj && obj->ID() == m_fleet_id)
        SetFleet(UniverseObject::INVALID_OBJECT_ID);
}

void FleetDetailPanel::ShipSelectionChanged(const GG::ListBox::SelectionSet& rows)
{
    for (GG::ListBox::iterator it = m_ships_lb->begin(); it != m_ships_lb->end(); ++it) {
        ShipDataPanel* ship_panel = boost::polymorphic_downcast<ShipDataPanel*>((**it)[0]);
        ship_panel->Select(rows.find(it) != rows.end());
    }
}

void FleetDetailPanel::ShipBrowsed(GG::ListBox::iterator it)
{}

void FleetDetailPanel::ShipRightClicked(GG::ListBox::iterator it, const GG::Pt& pt)
{
    // get ship that was clicked, aborting if problems arise doing so
    ShipRow* ship_row = dynamic_cast<ShipRow*>(*it);
    if (!ship_row)
        return;

    Ship* ship = ship_row->GetShip();
    if (!ship)
        return;


    // verify that this client's player's empire owns this fleet.  if not, abort.
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    const std::set<int>& owners = ship->Owners();

    if (owners.find(empire_id) == owners.end())
        return;


    // create popup menu with a rename ship option in it.
    GG::MenuItem menu_contents;
    menu_contents.next_level.push_back(GG::MenuItem(UserString("RENAME"), 1, false, false));


    if (!ship->OrderedScrapped()) {
        // create popup menu with "Scrap" option
        menu_contents.next_level.push_back(GG::MenuItem(UserString("ORDER_SHIP_SCRAP"), 3, false, false));
    } else {
        // create popup menu with "Cancel Scrap" option
        menu_contents.next_level.push_back(GG::MenuItem(UserString("ORDER_CANCEL_SHIP_SCRAP"), 4, false, false));
    }

    GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents, ClientUI::TextColor());
    if (popup.Run()) {
        switch (popup.MenuID()) {
        case 1: { // rename ship
            std::string ship_name = ship->Name();
            CUIEditWnd edit_wnd(GG::X(350), UserString("ENTER_NEW_NAME"), ship_name);
            edit_wnd.Run();

            std::string new_name = edit_wnd.Result();

            if (new_name != "" && new_name != ship_name) {
                HumanClientApp::GetApp()->Orders().IssueOrder(
                    OrderPtr(new RenameOrder(empire_id, ship->ID(), new_name)));
            }
            break;
        }

        case 3: { // scrap ship
            HumanClientApp::GetApp()->Orders().IssueOrder(
                OrderPtr(new ScrapOrder(empire_id, ship->ID())));
            break;
        }

        case 4: { // un-scrap ship
            // find order to scrap this ship, and recind it
            std::map<int, int> pending_scrap_orders = PendingScrapOrders();
            std::map<int, int>::const_iterator it = pending_scrap_orders.find(ship->ID());
            if (it != pending_scrap_orders.end()) {
                HumanClientApp::GetApp()->Orders().RecindOrder(it->second);
            break;
            }
        }

        default:
            break;
        }
    }
}


////////////////////////////////////////////////
// FleetDetailWnd
////////////////////////////////////////////////
// static(s)
GG::Pt FleetDetailWnd::s_last_position =    GG::Pt(GG::X(300), GG::Y0);
GG::Pt FleetDetailWnd::s_last_size =        GG::Pt(GG::X(300), GG::Y(200));

FleetDetailWnd::FleetDetailWnd(int fleet_id, bool read_only, GG::Flags<GG::WndFlag> flags/* = INTERACTIVE | DRAGABLE | RESIZABLE | ONTOP | CLOSABLE*/) :
    CUIWnd("", s_last_position.x, s_last_position.y, s_last_size.x, s_last_size.y, flags),
    m_fleet_panel(0)
{
    Sound::TempUISoundDisabler sound_disabler;

    m_fleet_panel = new FleetDetailPanel(GG::X1, GG::Y1, fleet_id, read_only);
    AttachChild(m_fleet_panel);
    SetChildClippingMode(DontClip);

    SetName(TitleText());
    m_fleet_connection = GG::Connect(GetObject<Fleet>(fleet_id)->StateChangedSignal, &FleetDetailWnd::Refresh, this);

    DoLayout();
}

FleetDetailWnd::~FleetDetailWnd()
{
    ClosingSignal(this);
}

Fleet* FleetDetailWnd::GetFleet() const
{
    return m_fleet_panel->GetFleet();
}

int FleetDetailWnd::FleetID() const
{
    return m_fleet_panel->FleetID();
}

void FleetDetailWnd::CloseClicked()
{
    s_last_position = UpperLeft();
    CUIWnd::CloseClicked();
    delete this;
}

void FleetDetailWnd::DoLayout()
{
    const GG::X TOTAL_WIDTH = ClientWidth();
    const GG::X LEFT = GG::X(PAD);
    const GG::X RIGHT = TOTAL_WIDTH - GG::X(PAD);

    const GG::Y TOTAL_HEIGHT = ClientHeight();
    const GG::Y TOP = GG::Y(PAD);
    const GG::Y BOTTOM = TOTAL_HEIGHT - GG::Y(PAD);

    m_fleet_panel->SizeMove(GG::Pt(LEFT, TOP), GG::Pt(RIGHT, BOTTOM));
}

void FleetDetailWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    GG::Pt old_size = Size();
    CUIWnd::SizeMove(ul, lr);
    s_last_position = ul;
    s_last_size = Size();
    if (s_last_size != old_size)
        DoLayout();
}

std::string FleetDetailWnd::TitleText() const
{
    if (const Fleet* fleet = m_fleet_panel->GetFleet())
        return fleet->Name();   // TODO: Should this return fleet->PublicName() depending on ownership...?
    else
        return "";
}

void FleetDetailWnd::Refresh()
{
    SetName(TitleText());
}

const GG::Pt& FleetDetailWnd::LastPosition()
{
    return s_last_position;
}

const GG::Pt& FleetDetailWnd::LastSize()
{
    return s_last_size;
}


////////////////////////////////////////////////
// FleetWnd
////////////////////////////////////////////////
// static(s)
GG::Pt FleetWnd::s_last_position =  GG::Pt(GG::X0, GG::Y0);
GG::Pt FleetWnd::s_last_size =      GG::Pt(GG::X(360), GG::Y(400));

FleetWnd::FleetWnd(const std::vector<int>& fleet_ids, bool read_only,
         int selected_fleet_id/* = UniverseObject::INVALID_OBJECT_ID*/,
         GG::Flags<GG::WndFlag> flags/* = INTERACTIVE | DRAGABLE | ONTOP | CLOSABLE*/) :
    MapWndPopup("", s_last_position.x, s_last_position.y, s_last_size.x, s_last_size.y, flags | GG::RESIZABLE),
    m_fleet_ids(),
    m_empire_id(ALL_EMPIRES),
    m_system_id(UniverseObject::INVALID_OBJECT_ID),
    m_read_only(read_only),
    m_fleets_lb(0),
    m_new_fleet_drop_target(0),
    m_fleet_detail_panel(0)
{
    for (std::vector<int>::const_iterator it = fleet_ids.begin(); it != fleet_ids.end(); ++it)
        m_fleet_ids.insert(*it);

    Init(selected_fleet_id);
}

FleetWnd::FleetWnd(int system_id, int empire_id, bool read_only,
         int selected_fleet_id/* = UniverseObject::INVALID_OBJECT_ID*/,
         GG::Flags<GG::WndFlag> flags/* = INTERACTIVE | DRAGABLE | ONTOP | CLOSABLE*/) :
    MapWndPopup("", s_last_position.x, s_last_position.y, s_last_size.x, s_last_size.y, flags | GG::RESIZABLE),
    m_fleet_ids(),
    m_empire_id(empire_id),
    m_system_id(system_id),
    m_read_only(read_only),
    m_fleets_lb(0),
    m_new_fleet_drop_target(0),
    m_fleet_detail_panel(0)
{
    //// get fleets for specified system and empire
    //// need to know what fleets to put in the Wnd, given the specified system and empire.
    //const ObjectMap& visible_objects = GetUniverse().Objects();

    //if (const System* system = GetEmpireKnownObject<System>(m_system_id, HumanClientApp::GetApp()->EmpireID())) {
    //    // get all fleets in system.
    //    std::vector<const Fleet*> all_fleets = visible_objects.FindObjects<Fleet>();
    //    for (std::vector<const Fleet*>::const_iterator it = all_fleets.begin(); it != all_fleets.end(); ++it) {
    //        const Fleet* fleet = *it;
    //        if (fleet->SystemID() == m_system_id && fleet->OwnedBy(m_empire_id))
    //            m_fleet_ids.insert(fleet->ID());
    //    }
    //}

    Init(selected_fleet_id);
}

FleetWnd::~FleetWnd()
{
    ClientUI::GetClientUI()->GetMapWnd()->ClearProjectedFleetMovementLines();
    ClosingSignal(this);
}

void FleetWnd::Init(int selected_fleet_id)
{
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


    // create fleet list box
    m_fleets_lb = new FleetsListBox(m_read_only);
    m_fleets_lb->SetHiliteColor(GG::CLR_ZERO);
    GG::Connect(m_fleets_lb->SelChangedSignal,      &FleetWnd::FleetSelectionChanged,   this);
    GG::Connect(m_fleets_lb->LeftClickedSignal,     &FleetWnd::FleetLeftClicked,        this);
    GG::Connect(m_fleets_lb->RightClickedSignal,    &FleetWnd::FleetRightClicked,       this);
    GG::Connect(m_fleets_lb->DoubleClickedSignal,   &FleetWnd::FleetDoubleClicked,      this);
    AttachChild(m_fleets_lb);

    // create fleet detail panel
    m_fleet_detail_panel = new FleetDetailPanel(GG::X1, GG::Y1, 0, m_read_only);
    AttachChild(m_fleet_detail_panel);


    // determine fleets to show and populate list
    Refresh();


    // create drop target, if this fleetwnd is manipulable
    if (!m_read_only) {
        if (m_fleet_ids.empty()) {
            // shouldn't be needed, as fleetwnd with no fleets shouldn't be opened, but just in case...
            m_new_fleet_drop_target = new FleetDataPanel(GG::X1, ListRowHeight(), 0,
                                                         m_empire_id, m_system_id);
        } else {
            // get universe position of fleet and use to create drop target for creating new fleets
            int fleet_id = *(m_fleet_ids.begin());
            const Fleet* first_fleet = GetObject<Fleet>(fleet_id);
            assert(first_fleet);
            m_new_fleet_drop_target = new FleetDataPanel(GG::X1, ListRowHeight(), 0,
                                                         m_empire_id, m_system_id,
                                                         first_fleet->X(), first_fleet->Y());
        }
        AttachChild(m_new_fleet_drop_target);

        m_fleets_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_BROWSEUPDATES);
        m_fleets_lb->AllowDropType(SHIP_DROP_TYPE_STRING);
        m_fleets_lb->AllowDropType(FLEET_DROP_TYPE_STRING);

        GG::Connect(m_new_fleet_drop_target->NewFleetFromShipsSignal,   &FleetWnd::CreateNewFleetFromDrops, this);
    } else {
        m_fleets_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_BROWSEUPDATES | GG::LIST_SINGLESEL);
    }


    // random other signals... deletion and state changes
    GG::Connect(GetUniverse().UniverseObjectDeleteSignal,               &FleetWnd::UniverseObjectDeleted,   this);

    if (const System* system = GetObject<System>(m_system_id)) {
        GG::Connect(system->StateChangedSignal,                         &FleetWnd::SystemChangedSlot,       this);
        GG::Connect(system->FleetRemovedSignal,                         &FleetWnd::SystemFleetRemovedSlot,  this);
        GG::Connect(system->FleetInsertedSignal,                        &FleetWnd::SystemFleetInsertedSlot, this);
    }


    // window title
    SetName(TitleText());


    // verify that the selected fleet id is valid.
    if (selected_fleet_id != UniverseObject::INVALID_OBJECT_ID) {
        if (m_fleet_ids.find(selected_fleet_id) == m_fleet_ids.end()) {
            Logger().errorStream() << "FleetWnd::Init couldn't find requested selected fleet with id " << selected_fleet_id;
            selected_fleet_id = UniverseObject::INVALID_OBJECT_ID;
        }
    }


    // autoselect a fleet, if supposed to
    if (selected_fleet_id != UniverseObject::INVALID_OBJECT_ID && GetOptionsDB().Get<bool>("UI.fleet-autoselect")) {
        for (GG::ListBox::iterator it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
            int fleet_in_row = FleetInRow(it);
            if (fleet_in_row == selected_fleet_id) {
                m_fleets_lb->SelectRow(it);
                m_fleets_lb->BringRowIntoView(it);
                m_fleet_detail_panel->SetFleet(selected_fleet_id);
                break;
            }
        }
    }

    DoLayout();
}

void FleetWnd::Refresh()
{
    const ObjectMap& objects = GetUniverse().Objects(); // objects visisble to this client's empire

    // save selected fleet(s)
    std::set<int> initially_selected_fleets = this->SelectedFleetIDs();

    // remove existing fleet rows
    m_fleets_lb->Clear();   // deletes rows when removing; they don't need to be manually deleted

    // repopulate m_fleet_ids according to FleetWnd settings
    if (GetMainObjectMap().Object<System>(m_system_id)) {
        // get fleets to show from system, based on required ownership
        m_fleet_ids.clear();
        std::vector<const Fleet*> all_fleets = objects.FindObjects<Fleet>();
        for (std::vector<const Fleet*>::const_iterator it = all_fleets.begin(); it != all_fleets.end(); ++it) {
            const Fleet* fleet = *it;
            if (fleet->SystemID() != m_system_id)
                continue;
            if (m_empire_id == ALL_EMPIRES || fleet->OwnedBy(m_empire_id)) {
                m_fleet_ids.insert(fleet->ID());
                AddFleet(fleet->ID());
            }
        }
    } else {
        // check all fleets whose IDs are already in m_fleet_ids, removing any that no longer exist.
        std::set<int> validated_fleet_ids;
        for (std::set<int>::const_iterator it = m_fleet_ids.begin(); it != m_fleet_ids.end(); ++it) {
            int fleet_id = *it;
            if (objects.Object<Fleet>(fleet_id)) {
                validated_fleet_ids.insert(fleet_id);
                AddFleet(fleet_id);
            }
        }
        m_fleet_ids = validated_fleet_ids;
    }

    // reselect previously-selected fleets
    this->SetSelectedFleets(initially_selected_fleets);
}

void FleetWnd::CloseClicked()
{
    s_last_position = UpperLeft();
    CUIWnd::CloseClicked();
    delete this;
}

void FleetWnd::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    MapWndPopup::LClick(pt, mod_keys);
    ClickedSignal(this);
}

void FleetWnd::DoLayout()
{
    const GG::X TOTAL_WIDTH = ClientWidth();
    const GG::X LEFT = GG::X(PAD);
    const GG::X RIGHT = TOTAL_WIDTH - GG::X(PAD);

    const GG::Y TOTAL_HEIGHT = ClientHeight();
    const GG::Y AVAILABLE_HEIGHT = TOTAL_HEIGHT - 3*GG::Y(PAD); // top and bottom pads, and space between contents pads

    const GG::Y LISTBOX_TOP = GG::Y(PAD);

    // TODO: If size too small for new fleet drop target or ships panel, don't show them.  keep fleet list visible
    //       as long as possible when height is reduced.
    const GG::Y DATA_PANEL_BOTTOM = TOTAL_HEIGHT - GG::Y(PAD);

    const GG::Y ROW_HEIGHT = m_fleets_lb->ListRowSize().y;


    GG::Y       NEW_FLEET_DROP_TARGET_HEIGHT = ROW_HEIGHT + GG::Y(PAD); // space for row, and one more unit of pad space
    if (!m_new_fleet_drop_target)
        NEW_FLEET_DROP_TARGET_HEIGHT = GG::Y0;


    // subtract space for new fleet drop target, and divide remainder between list box and fleet data panel
    const GG::Y DATA_PANEL_HEIGHT = (AVAILABLE_HEIGHT - NEW_FLEET_DROP_TARGET_HEIGHT) / 3 * 2;
    const GG::Y LIST_BOX_HEIGHT = (AVAILABLE_HEIGHT - NEW_FLEET_DROP_TARGET_HEIGHT) / 3;

    // get other edges of list box and data panel
    const GG::Y LISTBOX_BOTTOM = LISTBOX_TOP + LIST_BOX_HEIGHT;
    const GG::Y DATA_PANEL_TOP = DATA_PANEL_BOTTOM - DATA_PANEL_HEIGHT;

    // if present, use these to set position of drop target.  if not present, list box and data panel will use up the space
    if (m_new_fleet_drop_target) {
        const GG::Y DROP_TARGET_TOP = LISTBOX_BOTTOM + GG::Y(PAD);
        const GG::Y DROP_TARGET_BOTTOM = DROP_TARGET_TOP + ROW_HEIGHT;
        m_new_fleet_drop_target->SizeMove(  GG::Pt(LEFT, DROP_TARGET_TOP),  GG::Pt(RIGHT, DROP_TARGET_BOTTOM));
    }

    m_fleets_lb->SizeMove(                  GG::Pt(LEFT, LISTBOX_TOP),      GG::Pt(RIGHT, LISTBOX_BOTTOM));
    m_fleet_detail_panel->SizeMove(         GG::Pt(LEFT, DATA_PANEL_TOP),   GG::Pt(RIGHT, DATA_PANEL_BOTTOM));
}

void FleetWnd::AddFleet(int fleet_id)
{
    const Fleet* fleet = GetObject<Fleet>(fleet_id);
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

void FleetWnd::SelectFleet(int fleet_id)
{
    if (fleet_id == UniverseObject::INVALID_OBJECT_ID || !(GetObject<Fleet>(fleet_id))) {
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

void FleetWnd::SetSelectedFleets(const std::set<int>& fleet_ids)
{
    m_fleets_lb->DeselectAll();

    // early exit if nothing to select
    if (fleet_ids.empty()) {
        FleetSelectionChanged(m_fleets_lb->Selections());
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

    FleetSelectionChanged(m_fleets_lb->Selections());
}

void FleetWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    GG::Pt old_size = Size();
    MapWndPopup::SizeMove(ul, lr);
    s_last_position = ul;
    s_last_size = Size();
    if (s_last_size != old_size)
        DoLayout();
}

int FleetWnd::SystemID() const
{
    return m_system_id;
}

int FleetWnd::EmpireID() const
{
    return m_empire_id;
}

bool FleetWnd::ContainsFleet(int fleet_id) const
{
    for (GG::ListBox::iterator it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        Fleet* fleet = GetObject<Fleet>(FleetInRow(it));
        if (fleet && fleet->ID() == fleet_id)
            return true;
    }
    return false;
}

const std::set<int>& FleetWnd::FleetIDs() const
{
    return m_fleet_ids;
}

std::set<int> FleetWnd::SelectedFleetIDs() const
{
    std::set<int> retval;
    for (GG::ListBox::iterator it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        if (!m_fleets_lb->Selected(it))
            continue;

        int selected_fleet_id = FleetInRow(it);
        if (selected_fleet_id != UniverseObject::INVALID_OBJECT_ID)
            retval.insert(selected_fleet_id);
    }
    return retval;
}

void FleetWnd::FleetSelectionChanged(const GG::ListBox::SelectionSet& rows)
{
    // show appropriate fleet in detail panel.  if one fleet is selected, show
    // its ships.  if more than one fleet is selected or no fleets are selected
    // then show no ships.
    if (rows.size() == 1) {
        // find selected row and fleet
        for (GG::ListBox::iterator it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
            if (rows.find(it) != rows.end()) {
                m_fleet_detail_panel->SetFleet(FleetInRow(it));
                break;
            }
        }
    } else {
        m_fleet_detail_panel->SetFleet(UniverseObject::INVALID_OBJECT_ID);
    }

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

    ClickedSignal(this);
    SelectedFleetsChangedSignal();
}

void FleetWnd::FleetRightClicked(GG::ListBox::iterator it, const GG::Pt& pt)
{
    int empire_id = HumanClientApp::GetApp()->EmpireID();

    Fleet* fleet = GetObject<Fleet>(FleetInRow(it));
    if (!fleet || fleet->Owners().size() != 1 || empire_id != *(fleet->Owners().begin())) {
        return;
    }

    const System* system = GetObject<System>(fleet->SystemID());    // may be null

    GG::MenuItem menu_contents;
    menu_contents.next_level.push_back(GG::MenuItem(UserString("RENAME"),               1, false, false));
    if (system && fleet->NumShips() > 1)    // can't split fleets without more than one ship, or which are not in a system
        menu_contents.next_level.push_back(GG::MenuItem(UserString("FW_SPLIT_FLEET"),   2, false, false));

    GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents, ClientUI::TextColor());

    if (popup.Run()) {
        switch (popup.MenuID()) {
        case 1: { // rename fleet
            std::string fleet_name = fleet->Name();
            CUIEditWnd edit_wnd(GG::X(350), UserString("ENTER_NEW_NAME"), fleet_name);
            edit_wnd.Run();

            std::string new_name = edit_wnd.Result();

            if (!new_name.empty() && new_name != fleet_name) {
                HumanClientApp::GetApp()->Orders().IssueOrder(
                    OrderPtr(new RenameOrder(empire_id, fleet->ID(), new_name)));
            }
            break;
        }

        case 2: { // split ships in fleet into one-ship fleets
            std::set<int> ship_ids_set = fleet->ShipIDs();

            if (ship_ids_set.size() >= 2) {
                // remove first ship from set
                std::set<int>::iterator it = ship_ids_set.begin();
                ship_ids_set.erase(it);

                // put remaining ships into their own fleets
                for (it = ship_ids_set.begin(); it != ship_ids_set.end(); ++it) {
                    std::vector<int> ship_ids_vec;
                    ship_ids_vec.push_back(*it);
                    CreateNewFleetFromShips(ship_ids_vec);
                }
            }
            break;
        }

        default:
            break;
        }
    }
}

void FleetWnd::FleetLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt)
{
    ClickedSignal(this);
}

void FleetWnd::FleetDoubleClicked(GG::ListBox::iterator it)
{
    ClickedSignal(this);
    int clicked_fleet_id = FleetInRow(it);
    if (clicked_fleet_id == UniverseObject::INVALID_OBJECT_ID) {
        Logger().errorStream() << "FleetWnd::FleetDoubleClicked couldn't get fleet in clicked row";
        return;
    }

    int num_open_windows = FleetUIManager::GetFleetUIManager().OpenDetailWnds(this);

    GG::Pt window_posn(std::max(GG::X0, 25 + LowerRight().x + num_open_windows * 25),
                       std::max(GG::Y0, UpperLeft().y + num_open_windows * 25));

    FleetDetailWnd* fleet_detail_wnd = FleetUIManager::GetFleetUIManager().NewFleetDetailWnd(this, clicked_fleet_id, m_read_only);
    if (fleet_detail_wnd) {
        if (GG::GUI::GetGUI()->AppWidth() < fleet_detail_wnd->LowerRight().x)
            window_posn.x = GG::GUI::GetGUI()->AppWidth() - fleet_detail_wnd->Width();

        if (GG::GUI::GetGUI()->AppHeight() < fleet_detail_wnd->LowerRight().y)
            window_posn.y = GG::GUI::GetGUI()->AppHeight() - fleet_detail_wnd->Height();

        fleet_detail_wnd->MoveTo(window_posn);
    }
}

int FleetWnd::FleetInRow(GG::ListBox::iterator it) const
{
    if (it == m_fleets_lb->end())
        return UniverseObject::INVALID_OBJECT_ID;

    if (FleetRow* fleet_row = dynamic_cast<FleetRow*>(*it))
        return fleet_row->FleetID();

    return UniverseObject::INVALID_OBJECT_ID;
}

std::string FleetWnd::TitleText() const
{
    // if no fleets available, default to indicating no fleets
    if (m_fleet_ids.empty())
        return UserString("FW_NO_FLEET");

    const ObjectMap& objects = GetMainObjectMap();

    // at least one fleet is available, so show appropriate title this
    // FleetWnd's empire and system
    if (const Empire* empire = Empires().Lookup(m_empire_id)) {
        if (const System* system = objects.Object<System>(m_system_id)) {
            std::string sys_name = system->Name();
            if (sys_name.empty())
                sys_name = UserString("SP_UNKNOWN_SYSTEM");
            return boost::io::str(FlexibleFormat(UserString("FW_EMPIRE_FLEETS_AT_SYSTEM")) %
                                  empire->Name() % sys_name);
        } else {
            return boost::io::str(FlexibleFormat(UserString("FW_EMPIRE_FLEETS")) %
                                  empire->Name());
        }
    } else {
        if (const System* system = objects.Object<System>(m_system_id)) {
            std::string sys_name = system->Name();
            if (sys_name.empty())
                sys_name = UserString("SP_UNKNOWN_SYSTEM");
            return boost::io::str(FlexibleFormat(UserString("FW_GENERIC_FLEETS_AT_SYSTEM")) %
                                  sys_name);
        } else {
            return boost::io::str(FlexibleFormat(UserString("FW_GENERIC_FLEETS")));
        }
    }
}

void FleetWnd::CreateNewFleetFromDrops(const std::vector<int>& ship_ids)
{
    CreateNewFleetFromShips(ship_ids);
}

void FleetWnd::UniverseObjectDeleted(const UniverseObject *obj)
{
    // check if deleted object was a fleet.  if not, abort.
    const Fleet* deleted_fleet = universe_object_cast<const Fleet*>(obj);
    if (!deleted_fleet)
        return;

    // if detail panel is showing the deleted fleet, reset to show nothing
    if (m_fleet_detail_panel->GetFleet() == deleted_fleet)
        m_fleet_detail_panel->SetFleet(UniverseObject::INVALID_OBJECT_ID);

    const ObjectMap& objects = GetUniverse().Objects();

    // remove deleted fleet's row
    for (GG::ListBox::iterator it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        int row_fleet_id = FleetInRow(it);
        if (objects.Object<Fleet>(row_fleet_id) == deleted_fleet) {
            delete m_fleets_lb->Erase(it);
            break;
        }
    }
}

void FleetWnd::SystemChangedSlot()
{
    //std::cout << "FleetWnd::SystemChangedSlot" << std::endl;

    const System* system = GetObject<System>(m_system_id);
    if (!system) {
        Logger().errorStream() << "FleetWnd::SystemChangedSlot called but couldn't get System with id " << m_system_id;
        return;
    }
    // nothing to do.  fleet insertions or removals should have emitted
    // FleetRemovedSignal or FleetInsertedSignal which have their own
    // specialized FleetWnd handlers
}

void FleetWnd::SystemFleetInsertedSlot(Fleet& fleet)
{
    Refresh();
}

void FleetWnd::SystemFleetRemovedSlot(Fleet& fleet)
{
    Refresh();
}

const GG::Pt& FleetWnd::LastPosition()
{
    return s_last_position;
}

const GG::Pt& FleetWnd::LastSize()
{
    return s_last_size;
}

