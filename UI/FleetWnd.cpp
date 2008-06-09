#include "FleetWnd.h"
#include "../util/AppInterface.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "../universe/Fleet.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../universe/Planet.h"
#include "../universe/Predicates.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../network/Message.h"
#include "SidePanel.h"
#include "../Empire/Empire.h"

#include <GG/DrawUtil.h>
#include <GG/Menu.h>
#include <GG/Layout.h>
#include <GG/StaticGraphic.h>
#include <GG/TextControl.h>

#include <boost/cast.hpp>
#include <boost/format.hpp>


/** A FleetDetailPanel wrapped in a CUIWnd. */
class FleetDetailWnd : public CUIWnd
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void (FleetDetailWnd*)> ClosingSignalType;    ///< emitted when the detail wnd is closing
    //@}

    /** \name Structors */ //@{
    ~FleetDetailWnd(); ///< virtual dtor
    //@}

    Fleet* GetFleet() const;

    virtual void CloseClicked();

    mutable ClosingSignalType    ClosingSignal;

private:
    /** Basic ctor. */
    FleetDetailWnd(Fleet* fleet, bool read_only,
                   GG::Flags<GG::WndFlag> flags =
                   GG::CLICKABLE | GG::DRAGABLE | GG::RESIZABLE | GG::ONTOP | CLOSABLE);
    std::string TitleText() const;

    FleetDetailPanel* m_fleet_panel;

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

FleetWnd* FleetUIManager::WndForFleet(Fleet* fleet) const
{
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

std::size_t FleetUIManager::OpenDetailWnds(FleetWnd* fleet_wnd) const
{
    std::size_t retval = 0;
    FleetWndMap::const_iterator it = m_fleet_and_detail_wnds.find(fleet_wnd);
    if (it != m_fleet_and_detail_wnds.end())
        retval = it->second.size();
    return retval;
}

FleetWnd* FleetUIManager::NewFleetWnd(std::vector<Fleet*> fleets, int selected_fleet, bool read_only,
                                       GG::Flags<GG::WndFlag> flags/* = GG::CLICKABLE | GG::DRAGABLE | GG::ONTOP | CLOSABLE*/)
{
    if (!GetOptionsDB().Get<bool>("UI.multiple-fleet-windows"))
        CloseAll();
    FleetWnd* retval = new FleetWnd(fleets, selected_fleet, read_only, flags);
    m_fleet_wnds.insert(retval);
    GG::Connect(retval->ClosingSignal, &FleetUIManager::FleetWndClosing, this);
    GG::GUI::GetGUI()->Register(retval);
    return retval;
}

FleetDetailWnd* FleetUIManager::NewFleetDetailWnd(FleetWnd* fleet_wnd, Fleet* fleet, bool read_only,
                                                   GG::Flags<GG::WndFlag> flags/* = GG::CLICKABLE | GG::DRAGABLE | GG::RESIZABLE | GG::ONTOP | CLOSABLE*/)
{
    assert(fleet_wnd);
    assert(m_fleet_wnds.find(fleet_wnd) != m_fleet_wnds.end());
    FleetDetailWnd* retval = 0;
    bool found_fleet = false;
    std::set<FleetDetailWnd*>::const_iterator end_it = m_fleet_and_detail_wnds[fleet_wnd].end();
    for (std::set<FleetDetailWnd*>::const_iterator it = m_fleet_and_detail_wnds[fleet_wnd].begin(); it != end_it; ++it) {
        if (fleet == (*it)->GetFleet()) {
            found_fleet = true;
            break;
        }
    }
    if (!found_fleet) {
        retval = new FleetDetailWnd(fleet, read_only, flags);
        m_fleet_and_detail_wnds[fleet_wnd].insert(retval);
        GG::Connect(retval->ClosingSignal, boost::bind(&FleetUIManager::FleetDetailWndClosing, this, fleet_wnd, _1));
        GG::GUI::GetGUI()->Register(retval);
    }
    return retval;
}

void FleetUIManager::CullEmptyWnds()
{
    for (std::set<FleetWnd*>::iterator it = m_fleet_wnds.begin(); it != m_fleet_wnds.end(); ) {
        std::set<FleetWnd*>::iterator temp_it = it++;
        if ((*temp_it)->Fleets().empty()) {
            delete *temp_it;
        } else {
            for (std::set<FleetDetailWnd*>::iterator it2 = m_fleet_and_detail_wnds[*temp_it].begin(); it2 != m_fleet_and_detail_wnds[*temp_it].end(); ) {
                std::set<FleetDetailWnd*>::iterator temp_it2 = it2++;
                Fleet* fleet = 0;
                if (!(fleet = (*temp_it2)->GetFleet()) || !fleet->NumShips()) {
                    delete *temp_it2;
                }
            }
        }
    }
}

void FleetUIManager::SetActiveFleetWnd(FleetWnd* fleet_wnd)
{ m_active_fleet_wnd = fleet_wnd; }

bool FleetUIManager::CloseAll()
{
    bool retval = !m_fleet_wnds.empty();
    std::vector<FleetWnd*> vec(m_fleet_wnds.begin(), m_fleet_wnds.end());
    for (std::size_t i = 0; i < vec.size(); ++i) {
        delete vec[i];
    }
    m_active_fleet_wnd = 0;
    return retval;
}

FleetUIManager& FleetUIManager::GetFleetUIManager()
{
    static FleetUIManager retval;
    return retval;
}

void FleetUIManager::FleetWndClosing(FleetWnd* fleet_wnd)
{
    if (fleet_wnd == m_active_fleet_wnd)
        m_active_fleet_wnd = 0;
    std::vector<FleetDetailWnd*> vec(m_fleet_and_detail_wnds[fleet_wnd].begin(), m_fleet_and_detail_wnds[fleet_wnd].end());
    for (std::size_t i = 0; i < vec.size(); ++i) {
        delete vec[i];
    }
    m_fleet_wnds.erase(fleet_wnd);
    m_fleet_and_detail_wnds.erase(fleet_wnd);
}

void FleetUIManager::FleetDetailWndClosing(FleetWnd* fleet_wnd, FleetDetailWnd* fleet_detail_wnd)
{ m_fleet_and_detail_wnds[fleet_wnd].erase(fleet_detail_wnd); }


/** Represents a single fleet.  This class is used as the drop-target in
    FleetWnd (if the ctor parameter \a fleet is zero), and also as the sole
    Control in each FleetRow (if the ctor parameter \a fleet is nonzero). */
class FleetDataPanel : public GG::Control
{
public:
    enum {
        FLEET_ICON_SZ = 38, 
        FLEET_NAME_HT = 20, 
        STAT_ICON_WD = 40
    };

    FleetDataPanel(int w, int h, const Fleet* fleet, int empire = -1, int system_id = -1, double x = 0.0, double y = 0.0);
    
    virtual void DropsAcceptable(DropsAcceptableIter first,
                                 DropsAcceptableIter last,
                                 const GG::Pt& pt) const;

    bool Selected() const;

    virtual void Render();
    virtual void DragDropEnter(const GG::Pt& pt, const std::map<Wnd*, GG::Pt>& drag_drop_wnds, GG::Flags<GG::ModKey> mod_keys);
    virtual void DragDropLeave();
    virtual void AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt);
    void Select(bool b);

    mutable boost::signal<void (Ship*, const std::vector<int>&)> NewFleetFromShipsSignal;

private:
    void SetFleetIcon();
    void Refresh();

    const Fleet* const m_fleet;
    const int m_empire;
    const int m_system_id;
    const double m_x;
    const double m_y;

    boost::signals::connection m_fleet_connection;

    GG::StaticGraphic* m_fleet_icon;
    GG::TextControl* m_fleet_name_text;
    StatisticIcon* m_num_ships_stat;
    StatisticIcon* m_fleet_strength_stat;
    GG::StaticGraphic* m_damage_icon;
    GG::StaticGraphic* m_colonizer_icon;
    bool m_selected;
};

namespace {
    const int NEW_FLEET_BUTTON_WIDTH = 75;
    const int FLEET_LISTBOX_WIDTH =  250;
    const int FLEET_LISTBOX_HEIGHT = 150;
    const int CONTROL_MARGIN = 5; // gap to leave between controls in these windows
    const std::string SHIP_DROP_TYPE_STRING = "FleetWnd ShipRow";
    const std::string FLEET_DROP_TYPE_STRING = "FleetWnd FleetRow";

    /** Represents a single ship.  This class is used as the sole Control in
        each ShipRow. */
    class ShipDataPanel : public GG::Control
    {
    public:
        enum {
            SHIP_ICON_SZ = FleetDataPanel::FLEET_ICON_SZ, 
            SHIP_NAME_HT = FleetDataPanel::FLEET_NAME_HT, 
            STAT_ICON_WD = FleetDataPanel::STAT_ICON_WD
        };

        ShipDataPanel(int w, int h, const Ship* ship) :
            Control(0, 0, w, h, GG::Flags<GG::WndFlag>()),
            m_ship(ship),
            m_ship_icon(0),
            m_ship_name_text(new GG::TextControl(h, 0, w - h - 5, SHIP_NAME_HT, m_ship->Name(), GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()),
                                                 ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER)),
            m_ship_strength_stat(0),
            m_damage_stat(0),
            m_colonizer_icon(0),
            m_selected(false)
        {
            if (!m_ship)
                throw std::invalid_argument("ShipDataPanel::ShipDataPanel() : Attempted to construct a ShipDataPanel from a null ship pointer.");

            EnableChildClipping();

            AttachChild(m_ship_name_text);

            m_ship_strength_stat = new StatisticIcon(h, SHIP_NAME_HT, STAT_ICON_WD, h - SHIP_NAME_HT - 1, ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "combatstrength.png"),
                                                     0, 0, true, false, GG::Flags<GG::WndFlag>());
            AttachChild(m_ship_strength_stat);
            m_ship_connection = GG::Connect(m_ship->StateChangedSignal, &ShipDataPanel::Refresh, this);

            Refresh();
        }

        ~ShipDataPanel()
        {
            m_ship_connection.disconnect();
        }

        virtual void Render()
        {
            const GG::Clr& unselected_color = GG::CLR_GRAY;
            const GG::Clr& selected_color = GG::CLR_WHITE;

            GG::Clr color_to_use = m_selected ? selected_color : unselected_color;

            if (Disabled())
                color_to_use = DisabledColor(color_to_use);

            GG::Pt ul = UpperLeft(), lr = LowerRight();
            GG::Pt text_ul = m_ship_name_text->UpperLeft(), text_lr = m_ship_name_text->LowerRight();

            GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, GG::CLR_ZERO, color_to_use, 1);
            GG::FlatRectangle(text_ul.x, text_ul.y, text_lr.x + 5, text_lr.y, color_to_use, GG::CLR_ZERO, 0);
        }

        void Select(bool b)
        {
            if (m_selected != b) {
                m_selected = b;

                const GG::Clr& unselected_text_color = ClientUI::TextColor();
                const GG::Clr& selected_text_color = GG::CLR_BLACK;

                GG::Clr text_color_to_use = m_selected ? selected_text_color : unselected_text_color;

                if (Disabled())
                    text_color_to_use = DisabledColor(text_color_to_use);

                m_ship_name_text->SetTextColor(text_color_to_use);
            }
        }

    private:
        void SetShipIcon()
        {
            const int ICON_OFFSET = (Size().y - SHIP_ICON_SZ) / 2;
            std::string design_name;
            const ShipDesign* design = m_ship->Design();
            if (design) {
                design_name = design->Name();
                std::string::size_type space = design_name.find(' ');
                if (space != std::string::npos) {
                    design_name[space] = '_';
                }
            } else {
                design_name = "Scout";
            }
            m_ship_icon = new GG::StaticGraphic(ICON_OFFSET, ICON_OFFSET, SHIP_ICON_SZ, SHIP_ICON_SZ, 
                                                ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / (design_name + ".png")), GG::GRAPHIC_FITGRAPHIC);
            AttachChild(m_ship_icon);
        }

        void Refresh()
        {
            const ShipDesign* design = m_ship->Design();

            SetShipIcon();
            m_ship_name_text->SetText(m_ship->Name());
            m_ship_strength_stat->SetValue(design ? design->Attack() : UNKNOWN_UI_DISPLAY_VALUE);

            const int ICON_SPACING = 5;
            const int ICON_SZ = Height() - SHIP_NAME_HT - 1;
            int x_position = m_ship_strength_stat->LowerRight().x - ClientUpperLeft().x + ICON_SPACING;
            int damage_pts = 0; // TODO: acount for damaged ships once damage system is in place
            if (damage_pts) {
                if (!m_damage_stat) {
                    m_damage_stat = new StatisticIcon(x_position, SHIP_NAME_HT, STAT_ICON_WD, ICON_SZ, ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "damagemarker.png"),
                                                      damage_pts, 0, true, false, GG::Flags<GG::WndFlag>());
                    AttachChild(m_damage_stat);
                }
                x_position += m_damage_stat->Width(); // no icon spacing is needed after stat icons
            } else if (m_damage_stat) {
                DeleteChild(m_damage_stat);
                m_damage_stat = 0;
            }
            if (m_ship->Design() && design->CanColonize()) {
                if (!m_colonizer_icon) {
                    m_colonizer_icon = new GG::StaticGraphic(x_position, SHIP_NAME_HT, ICON_SZ, ICON_SZ, 
                                                             ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "colonymarker.png"),
                                                             GG::GRAPHIC_FITGRAPHIC);
                    AttachChild(m_colonizer_icon);
                }
                x_position += m_colonizer_icon->Width() + ICON_SPACING;
            } else if (m_colonizer_icon) {
                DeleteChild(m_colonizer_icon);
                m_colonizer_icon = 0;
            }
        }

        const Ship* const           m_ship;
        GG::StaticGraphic*          m_ship_icon;
        GG::TextControl*            m_ship_name_text;
        StatisticIcon*              m_ship_strength_stat;
        StatisticIcon*              m_damage_stat;
        StatisticIcon*              m_fuel_stat;
        GG::StaticGraphic*          m_colonizer_icon;
        bool                        m_selected;
        boost::signals::connection  m_ship_connection;
    };

    /** A ListBox::Row subclass used to represent fleets in FleetsListBox's. */
    struct FleetRow : public GG::ListBox::Row
    {
        enum {PANEL_WD = FLEET_LISTBOX_WIDTH - 8, PANEL_HT = 38};

        FleetRow(Fleet* fleet) :
            GG::ListBox::Row(PANEL_WD - ClientUI::ScrollWidth(), PANEL_HT + 4, fleet ? FLEET_DROP_TYPE_STRING : ""),
            m_fleet(fleet)
        {
            SetText("FleetRow");
            EnableChildClipping();
            push_back(new FleetDataPanel(Width(), PANEL_HT, m_fleet));
        }

        int FleetID() const {return m_fleet ? m_fleet->ID() : UniverseObject::INVALID_OBJECT_ID;}

        Fleet* const m_fleet;
    };

    /** A ListBox::Row subclass used to represent ships in ShipListBox's. */
    struct ShipRow : public GG::ListBox::Row
    {
        enum {PANEL_WD = FleetRow::PANEL_WD, PANEL_HT = FleetRow::PANEL_HT};

        ShipRow(Ship* ship) :
            GG::ListBox::Row(PANEL_WD - ClientUI::ScrollWidth(), PANEL_HT + 4, SHIP_DROP_TYPE_STRING),
            m_ship(ship)
        {
            SetText("ShipRow");
            if (!ship)
                throw std::invalid_argument("ShipRow::ShipRow() : Attempted to contruct a ShipRow using a null ship pointer.");

            EnableChildClipping();
            push_back(new ShipDataPanel(Width(), PANEL_HT, m_ship));
       }

        int ShipID() const {return m_ship->ID();}

        Ship* const m_ship;
    };

    bool ValidShip(const Ship* ship, const Fleet* new_fleet)
    {
        const Fleet* current_fleet = ship->GetFleet();
        return (!current_fleet || current_fleet->ID() != new_fleet->ID()) &&
            (ship->X() == new_fleet->X() && ship->Y() == new_fleet->Y()) &&
            ship->SystemID() == new_fleet->SystemID() &&
            *ship->Owners().begin() == *new_fleet->Owners().begin();
    }

    bool ValidShip(const Ship* ship, double x, double y, int system_id, int empire)
    {
        return (ship->X() == x && ship->Y() == y) &&
            ship->SystemID() == system_id &&
            *ship->Owners().begin() == empire;
    }

    bool ValidFleet(const Fleet* fleet, const Fleet* target_fleet)
    {
        return fleet->ID() != target_fleet->ID() &&
            fleet->SystemID() == target_fleet->SystemID() &&
            (fleet->X() == target_fleet->X() && fleet->Y() == target_fleet->Y()) &&
            *fleet->Owners().begin() == *target_fleet->Owners().begin();
    }

}

////////////////////////////////////////////////
// FleetDataPanel
////////////////////////////////////////////////
FleetDataPanel::FleetDataPanel(int w, int h, const Fleet* fleet,
                               int empire/* = -1*/, int system_id/* = -1*/, double x/* = 0.0*/, double y/* = 0.0*/) :
    Control(0, 0, w, h, fleet ? GG::Flags<GG::WndFlag>() : GG::CLICKABLE),
    m_fleet(fleet),
    m_empire(empire),
    m_system_id(system_id),
    m_x(x),
    m_y(y),
    m_fleet_icon(0),
    m_fleet_name_text(new GG::TextControl(h, 0, w - h - 5, FLEET_NAME_HT, m_fleet ? m_fleet->Name() : "<i>" + UserString("FW_NEW_FLEET_LABEL") + "</i>",
                                          GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), m_fleet ? ClientUI::TextColor() : GG::CLR_BLACK,
                                          GG::FORMAT_RIGHT | GG::FORMAT_VCENTER)),
    m_num_ships_stat(0),
    m_fleet_strength_stat(0),
    m_damage_icon(0),
    m_colonizer_icon(0),
    m_selected(false)
{
    EnableChildClipping();

    AttachChild(m_fleet_name_text);

    if (m_fleet) {
        m_num_ships_stat = new StatisticIcon(h, FLEET_NAME_HT, STAT_ICON_WD, h - FLEET_NAME_HT - 1,
                                             ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "3shipfleet.png"),
                                             0, 0, true, false, GG::Flags<GG::WndFlag>());
        m_fleet_strength_stat = new StatisticIcon(h + STAT_ICON_WD, FLEET_NAME_HT, STAT_ICON_WD, h - FLEET_NAME_HT - 1,
                                                  ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "combatstrength.png"),
                                                  0, 0, true, false, GG::Flags<GG::WndFlag>());
        AttachChild(m_num_ships_stat);
        AttachChild(m_fleet_strength_stat);
        m_fleet_connection = GG::Connect(m_fleet->StateChangedSignal, &FleetDataPanel::Refresh, this);
    }

    Refresh();
}

bool FleetDataPanel::Selected() const
{
    return m_selected;
}

void FleetDataPanel::Render()
{
    const GG::Clr& unselected_color = GG::CLR_GRAY;
    const GG::Clr& selected_color = GG::CLR_WHITE;

    GG::Clr color_to_use = m_selected ? selected_color : unselected_color;

    if (Disabled())
        color_to_use = DisabledColor(color_to_use);

    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Pt text_ul = m_fleet_name_text->UpperLeft(), text_lr = m_fleet_name_text->LowerRight();

    GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, GG::CLR_ZERO, color_to_use, 1);
    GG::FlatRectangle(text_ul.x, text_ul.y, text_lr.x + 5, text_lr.y, color_to_use, GG::CLR_ZERO, 0);
}

void FleetDataPanel::DragDropEnter(const GG::Pt& pt, const std::map<Wnd*, GG::Pt>& drag_drop_wnds, GG::Flags<GG::ModKey> mod_keys)
{
    Select(true);
    for (std::map<Wnd*, GG::Pt>::const_iterator it = drag_drop_wnds.begin(); it != drag_drop_wnds.end(); ++it) {
        if (it->first->DragDropDataType() != SHIP_DROP_TYPE_STRING ||
            !ValidShip(boost::polymorphic_downcast<ShipRow*>(it->first)->m_ship, m_x, m_y, m_system_id, m_empire)) {
            Select(false);
            break;
        }
    }
}

void FleetDataPanel::DragDropLeave()
{
    Select(false);
}

void FleetDataPanel::DropsAcceptable(DropsAcceptableIter first,
                                     DropsAcceptableIter last,
                                     const GG::Pt& pt) const
{
    for (DropsAcceptableIter it = first; it != last; ++it) {
        if (it->first->DragDropDataType() == SHIP_DROP_TYPE_STRING) {
            const Ship* ship = boost::polymorphic_downcast<const ShipRow*>(it->first)->m_ship;
            it->second = ValidShip(ship, m_x, m_y, m_system_id, m_empire);
        }
    }
}

void FleetDataPanel::AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt)
{
    std::vector<Ship*> ships;
    std::vector<int> ship_ids;
    for (std::vector<Wnd*>::const_iterator it = wnds.begin(); it != wnds.end(); ++it) {
        assert((*it)->DragDropDataType() == SHIP_DROP_TYPE_STRING);
        ships.push_back(boost::polymorphic_downcast<ShipRow*>(*it)->m_ship);
        ship_ids.push_back(ships.back()->ID());
    }
    NewFleetFromShipsSignal(ships[0], ship_ids);
}

void FleetDataPanel::Select(bool b)
{
    if (m_selected != b) {
        m_selected = b;

        const GG::Clr& unselected_text_color = ClientUI::TextColor();
        const GG::Clr& selected_text_color = GG::CLR_BLACK;

        GG::Clr text_color_to_use = m_selected ? selected_text_color : unselected_text_color;

        if (!m_fleet) // use different colors for the "new fleet" panel
            text_color_to_use = GG::CLR_BLACK;

        if (Disabled())
            text_color_to_use = DisabledColor(text_color_to_use);

        m_fleet_name_text->SetTextColor(text_color_to_use);
    }
}

void FleetDataPanel::SetFleetIcon()
{
    DeleteChild(m_fleet_icon);
    m_fleet_icon = 0;

    const int ICON_OFFSET = (Size().y - FLEET_ICON_SZ) / 2;
    boost::shared_ptr<GG::Texture> icon;

    if (m_fleet) { // a regular fleet data panel
        if (2 <= m_fleet->NumShips() && m_fleet->NumShips() <= 5) {
            icon = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" /
                                        (boost::lexical_cast<std::string>(m_fleet->NumShips()) + "shipfleet.png"));
        } else if (5 < m_fleet->NumShips()) {
            icon = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "5shipfleet.png");
        } else if (m_fleet->NumShips() == 1) {
            Ship* ship = GetUniverse().Object<Ship>(*m_fleet->begin());
            const ShipDesign* design = ship->Design();
            std::string design_name;
            if (design) {
                design_name = design->Name();
                std::string::size_type space = design_name.find(' ');
                if (space != std::string::npos) {
                    design_name[space] = '_';
                }
            } else {
                design_name = "Scout";
            }
            icon = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / (design_name + ".png"));
        }
    } else { // the "new fleet" data panel
        icon = ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "newfleet.png");
    }

    if (icon) {
        m_fleet_icon = new GG::StaticGraphic(ICON_OFFSET, ICON_OFFSET, FLEET_ICON_SZ, FLEET_ICON_SZ, icon, GG::GRAPHIC_FITGRAPHIC);
        AttachChild(m_fleet_icon);
    }
}

void FleetDataPanel::Refresh()
{
    SetFleetIcon();
    if (m_fleet) {
        m_fleet_name_text->SetText(m_fleet->Name());
        m_num_ships_stat->SetValue(m_fleet->NumShips());
        int attack_strength = 0;
        bool damaged_ships = false;
        bool contains_colony_ship = false;
        bool attack_factor_unknown = true;
        for (Fleet::const_iterator it = m_fleet->begin(); it != m_fleet->end(); ++it) {
            Ship* ship = GetUniverse().Object<Ship>(*it);
            const ShipDesign* design = ship->Design();
            if (design) {
                if (design->CanColonize())
                    contains_colony_ship = true;
                attack_strength += static_cast<int>(design->Attack());
                attack_factor_unknown = false;
            }
            // TODO: account for damaged ships once damage system is in place
        }
        m_fleet_strength_stat->SetValue(attack_factor_unknown ? UNKNOWN_UI_DISPLAY_VALUE : attack_strength);

        const int ICON_SPACING = 5;
        const int ICON_SZ = Height() - FLEET_NAME_HT - 1;
        int x_position = m_fleet_strength_stat->LowerRight().x - ClientUpperLeft().x + ICON_SPACING;
        if (damaged_ships) {
            if (!m_damage_icon) {
                m_damage_icon = new GG::StaticGraphic(x_position, FLEET_NAME_HT, ICON_SZ, ICON_SZ, 
                                                      ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "damagemarker.png"),
                                                      GG::GRAPHIC_FITGRAPHIC);
                AttachChild(m_damage_icon);
            }
            x_position += m_damage_icon->Width() + ICON_SPACING;
        } else if (m_damage_icon) {
            DeleteChild(m_damage_icon);
            m_damage_icon = 0;
        }
        if (contains_colony_ship) {
            if (!m_colonizer_icon) {
                m_colonizer_icon = new GG::StaticGraphic(x_position, FLEET_NAME_HT, ICON_SZ, ICON_SZ, 
                                                         ClientUI::GetTexture(ClientUI::ArtDir() / "icons" / "colonymarker.png"),
                                                         GG::GRAPHIC_FITGRAPHIC);
                AttachChild(m_colonizer_icon);
            }
            x_position += m_colonizer_icon->Width() + ICON_SPACING;
        } else if (m_colonizer_icon) {
            DeleteChild(m_colonizer_icon);
            m_colonizer_icon = 0;
        }
    }
}


////////////////////////////////////////////////
// FleetDetailPanel
////////////////////////////////////////////////
class ShipsListBox;
/** Used in FleetDetailWnd's and in the lower half of FleetWnd's to show the
    ships in a fleet, and some basic info about the fleet. */
class FleetDetailPanel : public GG::Wnd
{
public:
    FleetDetailPanel(Fleet* fleet, bool read_only, GG::Flags<GG::WndFlag> flags = GG::Flags<GG::WndFlag>()); ///< ctor

    Fleet* GetFleet() const {return m_fleet;} ///< returns the currently-displayed fleet (may be 0)

    void SetFleet(Fleet* fleet); ///< sets the currently-displayed Fleet (may be null)

private:
    int         GetShipIDOfListRow(int row_idx) const; ///< returns the ID number of the ship in row \a row_idx of the ships listbox
    void        Init();
    void        Refresh();
    void        UniverseObjectDeleted(const UniverseObject *);
    void        ShipSelectionChanged(const std::set<int>& rows);
    void        ShipBrowsed(int row_idx);
    void        ShipRightClicked(int row_idx, GG::ListBox::Row* row, const GG::Pt& pt);
    std::string DestinationText() const;
    std::string ShipStatusText(int ship_id) const;

    Fleet*                      m_fleet;
    const bool                  m_read_only;
    boost::signals::connection  m_fleet_connection;

    GG::TextControl*            m_destination_text;
    ShipsListBox*               m_ships_lb;
    GG::TextControl*            m_ship_status_text;
};

////////////////////////////////////////////////
// FleetsListBox
////////////////////////////////////////////////
/** A CUIListBox subclass used to list all the fleets, and handle drag-and-drop
    operations on them, in FleetWnd. */
class FleetsListBox : public CUIListBox
{
public:
    FleetsListBox(int x, int y, int w, int h, bool read_only) :
        CUIListBox(x, y, w, h),
        m_selected_fleet_row(-1),
        m_read_only(read_only)
    {}

    mutable boost::signal<void (Ship*, const std::vector<int>&)> NewFleetFromShipsSignal;

    virtual void DropsAcceptable(DropsAcceptableIter first,
                                 DropsAcceptableIter last,
                                 const GG::Pt& pt) const
    {
        int row_index = RowUnderPt(pt);
        bool row_in_range = 0 <= row_index && row_index < NumRows();
        const Fleet* target_fleet = row_in_range ?
            boost::polymorphic_downcast<const FleetRow*>(&GetRow(row_index))->m_fleet :
            0;
        for (DropsAcceptableIter it = first; it != last; ++it) {
            if (m_read_only || !row_in_range) {
                it->second = false;
            } else {
                if (it->first->DragDropDataType() == FLEET_DROP_TYPE_STRING) {
                    const Fleet* fleet = boost::polymorphic_downcast<const FleetRow*>(it->first)->m_fleet;
                    it->second = ValidFleet(fleet, target_fleet);
                } else if (it->first->DragDropDataType() == SHIP_DROP_TYPE_STRING) {
                    const Ship* ship = boost::polymorphic_downcast<const ShipRow*>(it->first)->m_ship;
                    it->second = ValidShip(ship, target_fleet);
                }
            }
        }
    }

    virtual void AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt)
    {
        int row_index = RowUnderPt(pt);
        assert(!m_read_only && 0 <= row_index && row_index < NumRows());

        Fleet* target_fleet = boost::polymorphic_downcast<FleetRow*>(&GetRow(row_index))->m_fleet;
        std::vector<Fleet*> fleets;
        std::vector<Ship*> ships;
        std::vector<int> ship_ids;
        for (std::vector<Wnd*>::const_iterator it = wnds.begin(); it != wnds.end(); ++it) {
            if ((*it)->DragDropDataType() == FLEET_DROP_TYPE_STRING) {
                fleets.push_back(boost::polymorphic_downcast<FleetRow*>(*it)->m_fleet);
            } else if ((*it)->DragDropDataType() == SHIP_DROP_TYPE_STRING) {
                ships.push_back(boost::polymorphic_downcast<ShipRow*>(*it)->m_ship);
                ship_ids.push_back(ships.back()->ID());
            }
        }
        assert(ships.empty() != fleets.empty());

        if (!fleets.empty()) {
            for (unsigned int i = 0; i < fleets.size(); ++i)
            {
                int fleet_id = fleets[i]->ID();
                HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(
                    new FleetTransferOrder(HumanClientApp::GetApp()->EmpireID(), fleets[i]->ID(), target_fleet->ID(),
                                           std::vector<int>(fleets[i]->begin(), fleets[i]->end()))));
                // if the transfer did not result in the indirect deletion of the fleet, delete it if it is empty
                Fleet* fleet = GetUniverse().Object<Fleet>(fleet_id);
                if (fleet && fleet->NumShips() == 0) {
                    HumanClientApp::GetApp()->Orders().IssueOrder(
                        OrderPtr(new DeleteFleetOrder(HumanClientApp::GetApp()->EmpireID(), fleets[i]->ID())));
                }
            }
        } else if (!ships.empty()) {
            HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(
                new FleetTransferOrder(HumanClientApp::GetApp()->EmpireID(), ships[0]->FleetID(), target_fleet->ID(),
                                       ship_ids)));
        }
    }

    virtual void DragDropEnter(const GG::Pt& pt, const std::map<Wnd*, GG::Pt>& drag_drop_wnds, GG::Flags<GG::ModKey> mod_keys)
    {
        DragDropHere(pt, drag_drop_wnds, mod_keys);
    }

    virtual void DragDropHere(const GG::Pt& pt, const std::map<Wnd*, GG::Pt>& drag_drop_wnds, GG::Flags<GG::ModKey> mod_keys)
    {
        int row_index = RowUnderPt(pt);

        if (m_selected_fleet_row != row_index)
            ClearSelection();

        if (m_read_only || row_index < 0 || NumRows() <= row_index)
            return;

        FleetRow* fleet_row = boost::polymorphic_downcast<FleetRow*>(&GetRow(row_index));
        FleetDataPanel* fleet_data_panel = boost::polymorphic_downcast<FleetDataPanel*>((*fleet_row)[0]);
        if (!fleet_data_panel->Selected()) {
            bool valid_drop = true;
            bool fleets_seen = false;
            bool ships_seen = false;
            for (std::map<Wnd*, GG::Pt>::const_iterator it = drag_drop_wnds.begin(); it != drag_drop_wnds.end(); ++it) {
                if (it->first->DragDropDataType() == FLEET_DROP_TYPE_STRING) {
                    fleets_seen = true;
                    if (!ValidFleet(boost::polymorphic_downcast<FleetRow*>(it->first)->m_fleet, fleet_row->m_fleet)) {
                        valid_drop = false;
                        break;
                    }
                } else if (it->first->DragDropDataType() == SHIP_DROP_TYPE_STRING) {
                    if (!ValidShip(boost::polymorphic_downcast<ShipRow*>(it->first)->m_ship, fleet_row->m_fleet)) {
                        valid_drop = false;
                        break;
                    }
                    ships_seen = true;
                } else {
                    valid_drop = false;
                    break;
                }
            }
            if (fleets_seen && ships_seen)
                valid_drop = false;
            if (valid_drop) {
                fleet_data_panel->Select(true);
                m_selected_fleet_row = row_index;
            }
        }
    }

    virtual void DragDropLeave()
    {
        ClearSelection();
    }

private:
    void ClearSelection()
    {
        if (m_selected_fleet_row != -1)
            boost::polymorphic_downcast<FleetDataPanel*>((*boost::polymorphic_downcast<FleetRow*>(&GetRow(m_selected_fleet_row)))[0])->Select(false);
        m_selected_fleet_row = -1;
    }

    int m_selected_fleet_row;
    const bool m_read_only;
};

////////////////////////////////////////////////
// ShipsListBox
////////////////////////////////////////////////
/** A CUIListBox subclass used to list all the ships, and handle drag-and-drop
    operations on them, in FleetDetailPanel. */
class ShipsListBox : public CUIListBox
{
public:
    ShipsListBox(int x, int y, int w, int h, Fleet* fleet, bool read_only) :
        CUIListBox(x, y, w, h),
        m_fleet(fleet),
        m_read_only(read_only)
    {}

    void SetFleet(Fleet* fleet) {m_fleet = fleet;}

    virtual void DropsAcceptable(DropsAcceptableIter first,
                                 DropsAcceptableIter last,
                                 const GG::Pt& pt) const
    {
        for (DropsAcceptableIter it = first; it != last; ++it) {
            const ShipRow* ship_row = 0;
            it->second = !m_read_only &&
                (ship_row = dynamic_cast<const ShipRow*>(it->first)) &&
                ValidShip(ship_row->m_ship, m_fleet);
        }
    }

    virtual void AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt)
    {
        assert(wnds.front()->DragDropDataType() == SHIP_DROP_TYPE_STRING);
        std::vector<int> ship_ids;
        for (std::vector<Wnd*>::const_iterator it = wnds.begin(); it != wnds.end(); ++it) {
            if ((*it)->DragDropDataType() == SHIP_DROP_TYPE_STRING) {
                ship_ids.push_back(boost::polymorphic_downcast<ShipRow*>(*it)->m_ship->ID());
            }
        }
        ShipRow* ship_row = boost::polymorphic_downcast<ShipRow*>(wnds.front());
        HumanClientApp::GetApp()->Orders().IssueOrder(
            OrderPtr(new FleetTransferOrder(HumanClientApp::GetApp()->EmpireID(),
                                            ship_row->m_ship->FleetID(),
                                            m_fleet->ID(),
                                            ship_ids)));
    }

private:
    Fleet* m_fleet;
    const bool m_read_only;
};

////////////////////////////////////////////////
// FleetDetailPanel implementation
////////////////////////////////////////////////
FleetDetailPanel::FleetDetailPanel(Fleet* fleet, bool read_only, GG::Flags<GG::WndFlag> flags/* = GG::Flags<GG::WndFlag>()*/) : 
    Wnd(0, 0, 1, 1, flags),
    m_fleet(0),
    m_read_only(read_only),
    m_destination_text(0),
    m_ships_lb(0),
    m_ship_status_text(0)
{
    SetText("FleetDetailPanel");
    EnableChildClipping(true);

    m_destination_text = new GG::TextControl(0, 0, FLEET_LISTBOX_WIDTH, ClientUI::Pts() + 4, "temp", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::TextColor(), GG::FORMAT_LEFT);
    m_ships_lb = new ShipsListBox(0, m_destination_text->LowerRight().y + CONTROL_MARGIN, FLEET_LISTBOX_WIDTH, FLEET_LISTBOX_HEIGHT, 0, read_only);
    m_ship_status_text = new GG::TextControl(0, m_ships_lb->LowerRight().y + CONTROL_MARGIN, m_ships_lb->Width(), ClientUI::Pts() + 4, 
                                             "temp", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::TextColor(), GG::FORMAT_LEFT);
    m_destination_text->SetMinSize(true);
    m_ships_lb->SetMinSize(m_ships_lb->Size());
    m_ships_lb->SetHiliteColor(GG::CLR_ZERO);
    m_ship_status_text->SetMinSize(true);
    Resize(m_ship_status_text->LowerRight());

    m_destination_text->SetText("");
    m_ship_status_text->SetText("");
    m_destination_text->SetMinSize(false);
    m_ship_status_text->SetMinSize(false);

    if (fleet) SetFleet(fleet);
    Init();

    GG::Connect(GetUniverse().UniverseObjectDeleteSignal, &FleetDetailPanel::UniverseObjectDeleted, this);
}

int FleetDetailPanel::GetShipIDOfListRow(int row_idx) const
{
    return dynamic_cast<ShipRow&>(m_ships_lb->GetRow(row_idx)).ShipID();
}

void FleetDetailPanel::SetFleet(Fleet* fleet)
{
    Fleet* old_fleet = m_fleet;

    if (fleet != old_fleet)
        m_fleet_connection.disconnect();

    *m_destination_text << "";
    *m_ship_status_text << "";
    m_ships_lb->Clear();
    m_ships_lb->SetFleet(fleet);
    
    m_fleet = fleet;

    if (m_fleet) {
        if (m_fleet->NumShips()) {
            Universe& universe = GetUniverse();
            for (Fleet::const_iterator it = m_fleet->begin(); it != m_fleet->end(); ++it)
                m_ships_lb->Insert(new ShipRow(universe.Object<Ship>(*it)));
            *m_destination_text << DestinationText();
            if (old_fleet != fleet)
                m_fleet_connection = GG::Connect(m_fleet->StateChangedSignal, &FleetDetailPanel::Refresh, this);
        } else {
            m_fleet_connection.disconnect();
            HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new DeleteFleetOrder(HumanClientApp::GetApp()->EmpireID(), m_fleet->ID())));
        }
    }
}

void FleetDetailPanel::Init()
{
    if (m_read_only) {
        m_ships_lb->SetStyle(GG::LIST_NOSEL | GG::LIST_BROWSEUPDATES);
    } else {
        m_ships_lb->SetStyle(GG::LIST_QUICKSEL | GG::LIST_BROWSEUPDATES);
        m_ships_lb->AllowDropType(SHIP_DROP_TYPE_STRING);
    }

    AttachChild(m_destination_text);
    AttachChild(m_ships_lb);
    AttachChild(m_ship_status_text);
    VerticalLayout();
    GetLayout()->SetCellMargin(4);
    GetLayout()->SetMinimumRowHeight(0, ClientUI::Pts() + 4);
    GetLayout()->SetRowStretch(1, 1.0);
    GetLayout()->SetMinimumRowHeight(2, ClientUI::Pts() + 4);

    GG::Connect(m_ships_lb->SelChangedSignal, &FleetDetailPanel::ShipSelectionChanged, this);
    GG::Connect(m_ships_lb->BrowsedSignal, &FleetDetailPanel::ShipBrowsed, this);
    GG::Connect(m_ships_lb->RightClickedSignal, &FleetDetailPanel::ShipRightClicked, this);
}

void FleetDetailPanel::Refresh()
{
    SetFleet(m_fleet);
}

void FleetDetailPanel::UniverseObjectDeleted(const UniverseObject *obj)
{
    if (obj == m_fleet)
        SetFleet(0);
}

void FleetDetailPanel::ShipSelectionChanged(const std::set<int>& rows)
{
    for (int i = 0; i < m_ships_lb->NumRows(); ++i) {
        ShipDataPanel* ship_panel = boost::polymorphic_downcast<ShipDataPanel*>(m_ships_lb->GetRow(i)[0]);
        ship_panel->Select(rows.find(i) != rows.end());
    }
}

void FleetDetailPanel::ShipBrowsed(int row_idx)
{
    if (0 <= row_idx && row_idx < m_ships_lb->NumRows()) {
        *m_ship_status_text << ShipStatusText(GetShipIDOfListRow(row_idx));
    } else {
        *m_ship_status_text << "";
    }
}

void FleetDetailPanel::ShipRightClicked(int row_idx, GG::ListBox::Row* row, const GG::Pt& pt)
{
    ShipRow* ship_row = dynamic_cast<ShipRow*>(row);

    if (!ship_row || ship_row->m_ship->Owners().size() != 1 || HumanClientApp::GetApp()->EmpireID() != *ship_row->m_ship->Owners().begin())
        return;

    Ship* ship = GetUniverse().Object<Ship>(ship_row->ShipID());

    GG::MenuItem menu_contents;
    menu_contents.next_level.push_back(GG::MenuItem(UserString("RENAME"), 1, false, false));

    GG::PopupMenu popup(pt.x, pt.y, GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), menu_contents, ClientUI::TextColor());

    if (popup.Run()) {
        switch (popup.MenuID()) {
        case 1: { // rename ship
            std::string ship_name = m_ships_lb->GetRow(row_idx)[0]->WindowText();
            CUIEditWnd edit_wnd(350, UserString("ENTER_NEW_NAME"), ship_name);
            edit_wnd.Run();
            if (edit_wnd.Result() != "") {
                HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new RenameOrder(HumanClientApp::GetApp()->EmpireID(), ship->ID(), edit_wnd.Result())));
                m_ships_lb->GetRow(row_idx)[0]->SetText(edit_wnd.Result());
            }
            break;}
        default:
            break;
        }
    }
}

std::string FleetDetailPanel::DestinationText() const
{
    std::string retval = "ERROR";
    System* dest = m_fleet->FinalDestination();
    System* current = m_fleet->GetSystem();
    if (dest && dest != current) {
        std::pair<int, int> eta = m_fleet->ETA();       // .first is turns to final destination.  .second is turns to next system on route

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

    } else if (current) {
        retval = boost::io::str(FlexibleFormat(UserString("FW_FLEET_AT")) % current->Name());
    }
    return retval;
}

std::string FleetDetailPanel::ShipStatusText(int ship_id) const
{
    Ship* ship = GetUniverse().Object<Ship>(ship_id);
    const ShipDesign* design = ship->Design();
    return UserString("FW_SHIP_CLASS") + " \"" + (design ? design->Name() : UserString("FW_UNKNOWN_DESIGN_NAME")) + "\"";
}



////////////////////////////////////////////////
// FleetDetailWnd
////////////////////////////////////////////////
FleetDetailWnd::FleetDetailWnd(Fleet* fleet, bool read_only, GG::Flags<GG::WndFlag> flags/* = CLICKABLE | DRAGABLE | RESIZABLE | ONTOP | CLOSABLE*/) : 
    CUIWnd("", 0, 0, 1, 1, flags),
    m_fleet_panel(0)
{
    TempUISoundDisabler sound_disabler;
    m_fleet_panel = new FleetDetailPanel(fleet, read_only);
    Resize(m_fleet_panel->Size() + (Size() - ClientSize()) + GG::Pt(14, 14));
    AttachChild(m_fleet_panel);
    GridLayout();
    GetLayout()->SetBorderMargin(7);
    SetMaxSize(GG::Pt(Width(), MaxSize().y));
    SetText(TitleText());
    EnableChildClipping(false);
}

FleetDetailWnd::~FleetDetailWnd()
{
    ClosingSignal(this);
}

Fleet* FleetDetailWnd::GetFleet() const
{ return m_fleet_panel->GetFleet(); }

void FleetDetailWnd::CloseClicked()
{
    CUIWnd::CloseClicked();
    delete this;
}

std::string FleetDetailWnd::TitleText() const
{
    const Fleet* fleet = m_fleet_panel->GetFleet();
    assert(fleet);
    return fleet->Name();
}



////////////////////////////////////////////////
// FleetWnd
////////////////////////////////////////////////
// static(s)
GG::Pt FleetWnd::s_last_position;

FleetWnd::FleetWnd(std::vector<Fleet*> fleets, int selected_fleet, bool read_only,
                   GG::Flags<GG::WndFlag> flags/* = CLICKABLE | DRAGABLE | ONTOP | CLOSABLE*/) : 
    MapWndPopup("", 0, 0, 1, 1, flags | GG::RESIZABLE),
    m_empire_id(*fleets[0]->Owners().begin()),
    m_system_id(UniverseObject::INVALID_OBJECT_ID),
    m_read_only(read_only),
    m_moving_fleets(true),
    m_current_fleet(-1),
    m_fleets_lb(0),
    m_new_fleet_drop_target(0),
    m_fleet_detail_panel(0)
{
    assert(0 <= selected_fleet && selected_fleet < static_cast<int>(fleets.size()));
    assert(!fleets.empty());

    m_system_id = fleets[0]->SystemID();
    for (unsigned int i = 1; i < fleets.size(); ++i) {
        if (m_system_id != fleets[i]->SystemID())
            m_system_id = UniverseObject::INVALID_OBJECT_ID;
    }

    TempUISoundDisabler sound_disabler;

    m_fleets_lb = new FleetsListBox(0, 0, FLEET_LISTBOX_WIDTH, FLEET_LISTBOX_HEIGHT, read_only);
    if (!m_read_only) {
        m_new_fleet_drop_target = new FleetDataPanel(FleetRow::PANEL_WD, FleetRow::PANEL_HT, 0, m_empire_id, m_system_id, fleets[0]->X(), fleets[0]->Y());
        m_new_fleet_drop_target->SetMinSize(GG::Pt(1, FleetRow::PANEL_HT));
        m_new_fleet_drop_target->MoveTo(GG::Pt(0, 5));
    }
    m_fleet_detail_panel = new FleetDetailPanel(0, read_only);
    m_fleet_detail_panel->MoveTo(GG::Pt(0, 10));

    m_fleets_lb->SetHiliteColor(GG::CLR_ZERO);
    m_fleets_lb->SetMinSize(m_fleets_lb->Size());

    Resize(m_fleet_detail_panel->LowerRight() + GG::Pt(3, 3) + (Size() - ClientSize()));
    GG::Pt window_posn = UpperLeft();
    if (GG::GUI::GetGUI()->AppWidth() < LowerRight().x)
        window_posn.x = GG::GUI::GetGUI()->AppWidth() - Width();
    if (GG::GUI::GetGUI()->AppHeight() < LowerRight().y)
        window_posn.y = GG::GUI::GetGUI()->AppHeight() - Height();
    MoveTo(window_posn);

    EnableChildClipping(false);

    Init(fleets, selected_fleet);
    GG::Connect(GetUniverse().UniverseObjectDeleteSignal, &FleetWnd::UniverseObjectDeleted, this);

    if (const System* system = fleets.back()->GetSystem())
        Connect(system->StateChangedSignal, &FleetWnd::SystemChangedSlot, this);

    SetMaxSize(GG::Pt(Width(), MaxSize().y));
}

FleetWnd::~FleetWnd()
{
    ClientUI::GetClientUI()->GetMapWnd()->SetProjectedFleetMovement(0, std::list<System*>());
    ClosingSignal(this);
}

void FleetWnd::CloseClicked()
{
    s_last_position = UpperLeft();
    CUIWnd::CloseClicked();
    delete this;
}

void FleetWnd::Init(const std::vector<Fleet*>& fleets, int selected_fleet)
{
    if (m_read_only) {
        m_fleets_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_BROWSEUPDATES | GG::LIST_SINGLESEL);
    } else {
        m_fleets_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_BROWSEUPDATES);
        m_fleets_lb->AllowDropType(SHIP_DROP_TYPE_STRING);
        m_fleets_lb->AllowDropType(FLEET_DROP_TYPE_STRING);
    }

    for (unsigned int i = 0; i < fleets.size(); ++i) {
        AddFleet(fleets[i]);
    }

    AttachChild(m_fleets_lb);
    AttachChild(m_fleet_detail_panel);
    if (!m_read_only)
        AttachChild(m_new_fleet_drop_target);
    VerticalLayout();
    if (!m_read_only) {
        GetLayout()->SetRowStretch(0, 1.0);
        GetLayout()->SetRowStretch(2, 1.0);
    }
    GetLayout()->SetBorderMargin(7);

    GG::Connect(m_fleets_lb->SelChangedSignal, &FleetWnd::FleetSelectionChanged, this);
    GG::Connect(m_fleets_lb->RightClickedSignal, &FleetWnd::FleetRightClicked, this);
    GG::Connect(m_fleets_lb->DoubleClickedSignal, &FleetWnd::FleetDoubleClicked, this);
    GG::Connect(m_fleets_lb->ErasedSignal, &FleetWnd::FleetDeleted, this);
    if (!m_read_only)
        GG::Connect(m_new_fleet_drop_target->NewFleetFromShipsSignal, &FleetWnd::CreateNewFleetFromDrops, this);

    SetText(TitleText());

    if (GetOptionsDB().Get<bool>("UI.fleet-autoselect") && !fleets.empty()) {
        m_fleets_lb->SelectRow(selected_fleet);
        m_current_fleet = selected_fleet;
        m_fleet_detail_panel->SetFleet(FleetInRow(selected_fleet));
        m_fleets_lb->BringRowIntoView(selected_fleet);
    }
}

void FleetWnd::AddFleet(Fleet* fleet)
{
    m_fleets_lb->Insert(new FleetRow(fleet));
}

void FleetWnd::SelectFleet(Fleet* fleet)
{
    for (int i = 0; i < m_fleets_lb->NumRows(); i++) {
        FleetRow* row = dynamic_cast<FleetRow*>(&m_fleets_lb->GetRow(i));
        if (row && row->m_fleet == fleet) {
            m_fleets_lb->DeselectAll();
            m_fleets_lb->SelectRow(i);
            m_current_fleet = i;
            m_fleet_detail_panel->SetFleet(fleet);
            m_fleets_lb->BringRowIntoView(i);
            break;
        }
    }
}

int FleetWnd::SystemID() const
{
    return m_system_id;
}

bool FleetWnd::ContainsFleet(int fleet_id) const
{
    for (int i = 0; i < m_fleets_lb->NumRows(); i++) {
        Fleet* fleet = FleetInRow(i);
        if (fleet && fleet->ID() == fleet_id)
            return true;
    }
    return false;
}

std::set<Fleet*> FleetWnd::Fleets() const
{
    std::set<Fleet*> retval;
    for (int i = 0; i < m_fleets_lb->NumRows(); ++i) {
        if (Fleet* fleet = FleetInRow(i)) {
            retval.insert(fleet);
        }
    }
    return retval;
}

std::set<Fleet*> FleetWnd::SelectedFleets() const
{
    std::set<Fleet*> retval;
    for (int i = 0; i < m_fleets_lb->NumRows(); ++i) {
        if (m_fleets_lb->Selected(i)) {
            Fleet* fleet = FleetInRow(i);
            if (fleet) retval.insert(fleet);
        }
    }
    return retval;
}

void FleetWnd::FleetSelectionChanged(const std::set<int>& rows)
{
    m_current_fleet = m_fleets_lb->Caret();

    Fleet* fleet = 0 <= m_current_fleet ? FleetInRow(m_current_fleet) : 0;
    m_fleet_detail_panel->SetFleet(fleet);

    for (int i = 0; i < m_fleets_lb->NumRows(); ++i) {
        FleetDataPanel* fleet_panel = boost::polymorphic_downcast<FleetDataPanel*>(m_fleets_lb->GetRow(i)[0]);
        fleet_panel->Select(rows.find(i) != rows.end());
    }
}

void FleetWnd::FleetRightClicked(int row_idx, GG::ListBox::Row* row, const GG::Pt& pt)
{
    Fleet* fleet = FleetInRow(row_idx);
    if (fleet->Owners().size() != 1 || HumanClientApp::GetApp()->EmpireID() != *fleet->Owners().begin())
        return;

    GG::MenuItem menu_contents;
    menu_contents.next_level.push_back(GG::MenuItem(UserString("RENAME"), 1, false, false));
    GG::PopupMenu popup(pt.x, pt.y, GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), menu_contents, ClientUI::TextColor());

    if (popup.Run()) {
        switch (popup.MenuID()) {
        case 1: { // rename fleet
            std::string fleet_name = fleet->Name();
            CUIEditWnd edit_wnd(350, UserString("ENTER_NEW_NAME"), fleet_name);
            edit_wnd.Run();
            if (edit_wnd.Result() != "") {
                HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new RenameOrder(HumanClientApp::GetApp()->EmpireID(), fleet->ID(), edit_wnd.Result())));
                m_fleets_lb->GetRow(row_idx)[0]->SetText(edit_wnd.Result());
            }
            break;
        }
        default:
            break;
        }
    }
}

void FleetWnd::FleetDoubleClicked(int row_idx, GG::ListBox::Row* row)
{
    Fleet* row_fleet = FleetInRow(row_idx);
    int num_open_windows = FleetUIManager::GetFleetUIManager().OpenDetailWnds(this);
    GG::Pt window_posn(std::max(0, 25 + LowerRight().x + num_open_windows * 25), std::max(0, UpperLeft().y + num_open_windows * 25));
    if (FleetDetailWnd* fleet_detail_wnd = FleetUIManager::GetFleetUIManager().NewFleetDetailWnd(this, row_fleet, m_read_only)) {
        if (GG::GUI::GetGUI()->AppWidth() < fleet_detail_wnd->LowerRight().x)
            window_posn.x = GG::GUI::GetGUI()->AppWidth() - fleet_detail_wnd->Width();
        if (GG::GUI::GetGUI()->AppHeight() < fleet_detail_wnd->LowerRight().y)
            window_posn.y = GG::GUI::GetGUI()->AppHeight() - fleet_detail_wnd->Height();
        fleet_detail_wnd->MoveTo(window_posn);
    }
}

void FleetWnd::FleetDeleted(int row_idx, GG::ListBox::Row* row)
{
    if (m_current_fleet == row_idx)
        m_current_fleet = -1;
    if (m_fleets_lb->Empty())
        CloseClicked();
}

Fleet* FleetWnd::FleetInRow(int idx) const
{
    FleetRow* fleet_row = dynamic_cast<FleetRow*>(&m_fleets_lb->GetRow(idx));
    return fleet_row ? fleet_row->m_fleet : 0;
}

std::string FleetWnd::TitleText() const
{
    Fleet* existing_fleet = FleetInRow(0);
    return boost::io::str(boost::format(UserString("FW_EMPIRE_FLEETS")) % Empires().Lookup(*existing_fleet->Owners().begin())->Name());
}

void FleetWnd::CreateNewFleetFromDrops(Ship* first_ship, const std::vector<int>& ship_ids)
{
    Fleet* some_fleet = FleetInRow(0);

    if (!some_fleet || !first_ship)
        return;

    System* system = some_fleet->GetSystem();
    double some_fleet_x = some_fleet->X();
    double some_fleet_y = some_fleet->Y();

    int empire_id = HumanClientApp::GetApp()->EmpireID();
    int new_fleet_id = HumanClientApp::GetApp()->GetNewObjectID();
    if (new_fleet_id == UniverseObject::INVALID_OBJECT_ID) {
        ClientUI::MessageBox(UserString("SERVER_TIMEOUT"), true);
        return;
    }

    std::string fleet_name = UserString("FW_NEW_FLEET_NAME") + boost::lexical_cast<std::string>(new_fleet_id);

    Fleet* new_fleet = 0;
    if (system) {
        HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new NewFleetOrder(empire_id, fleet_name, new_fleet_id, system->ID(), ship_ids)));
        System::ObjectVec fleets = system->FindObjectsInOrbit(-1, StationaryFleetVisitor(empire_id));
        for (unsigned int i = 0; i < fleets.size(); ++i) {
            if (fleets[i]->ID() == new_fleet_id) {
                new_fleet = universe_object_cast<Fleet*>(fleets[i]);
                break;
            }
        }
    } else {
        HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new NewFleetOrder(empire_id, fleet_name, new_fleet_id, some_fleet_x, some_fleet_y, ship_ids)));
        std::vector<Fleet*> fleets = GetUniverse().FindObjects<Fleet>();
        for (unsigned int i = 0; i < fleets.size(); ++i) {
            if (fleets[i]->Name() == fleet_name && fleets[i]->X() == some_fleet_x && fleets[i]->Y() == some_fleet_y) {
                new_fleet = fleets[i];
                break;
            }
        }
        AddFleet(new_fleet);
    }
}

void FleetWnd::UniverseObjectDeleted(const UniverseObject *obj)
{
    try {
        FleetInRow(m_current_fleet);
    } catch (const std::out_of_range& re) {
        m_current_fleet = -1; // if the object deleted was the currently selected fleet, we need to fix that situation
    }

    const Fleet* fleet;
    // only look for obj if FleetWnd contains fleets and obj is a fleet
    if (!(fleet = universe_object_cast<const Fleet*>(obj)))
        return;

    if (m_fleet_detail_panel->GetFleet() == fleet)
        m_fleet_detail_panel->SetFleet(0);

    for (int i = 0; i < m_fleets_lb->NumRows(); ++i) {
        if (FleetInRow(i) == fleet) {
            delete m_fleets_lb->Erase(i);
            break;
        }
    }
}

void FleetWnd::SystemChangedSlot()
{
    const System* system = FleetInRow(0)->GetSystem();
    System::ConstObjectVec system_fleet_vec = system->FindObjects(OwnedVisitor<Fleet>(m_empire_id));
    std::set<const Fleet*> system_fleet_set;
    for (System::ConstObjectVec::iterator it = system_fleet_vec.begin(); it != system_fleet_vec.end(); ++it) {
        system_fleet_set.insert(boost::polymorphic_downcast<const Fleet*>(*it));
    }
    for (int i = 0; i < m_fleets_lb->NumRows(); ++i) {
        system_fleet_set.erase(FleetInRow(i));
    }
    for (std::set<const Fleet*>::const_iterator it = system_fleet_set.begin(); it != system_fleet_set.end(); ++it) {
        AddFleet(const_cast<Fleet*>(*it));
    }
}

GG::Pt FleetWnd::LastPosition()
{
    return s_last_position;
}
