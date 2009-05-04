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
#include "Sound.h"
#include "../Empire/Empire.h"

#include <GG/DrawUtil.h>
#include <GG/Menu.h>
#include <GG/Layout.h>
#include <GG/StaticGraphic.h>
#include <GG/TextControl.h>

#include <boost/cast.hpp>
#include <boost/format.hpp>

namespace {
    const int           ICON_SIZE = 38;
    const int           PAD = 4;
    const std::string   SHIP_DROP_TYPE_STRING = "FleetWnd ShipRow";
    const std::string   FLEET_DROP_TYPE_STRING = "FleetWnd FleetRow";

    GG::Y LabelHeight() {
        return GG::Y(ClientUI::Pts() * 3/2);
    }

    GG::Y StatIconHeight() {
        return GG::Y(ClientUI::Pts()*4/3);
    }

    GG::X StatIconWidth() {
        return GG::X(Value(StatIconHeight()) * 5/2);
    }

    GG::X StatIconSpacingWidth() {
        return StatIconWidth();
    }

    GG::Y ListRowHeight() {
        return GG::Y(std::max(ICON_SIZE, Value(LabelHeight() + StatIconHeight()) + PAD));
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
    virtual void            CloseClicked();
    virtual void            SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    //@}

    static const GG::Pt&    LastPosition();         ///< returns the last position of the last FleetWnd that was closed
    static const GG::Pt&    LastSize();             ///< returns the last size ... ''

    mutable boost::signal<void (FleetDetailWnd*)> ClosingSignal;

private:
    /** \name Structors */ //@{
    /** Basic ctor. */
    FleetDetailWnd(Fleet* fleet, bool read_only, GG::Flags<GG::WndFlag> flags =
                   GG::CLICKABLE | GG::DRAGABLE | GG::RESIZABLE | GG::ONTOP | CLOSABLE);
    //@}

    void                    DoLayout();
    std::string             TitleText() const;

    FleetDetailPanel* m_fleet_panel;

    static GG::Pt     s_last_position;    ///< the latest position to which any FleetDetailWnd has been moved.
    static GG::Pt     s_last_size;        ///< the latest size to which any FleetDetailWnd has been resized.

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

FleetWnd* FleetUIManager::NewFleetWnd(std::vector<Fleet*> fleets, int selected_fleet, bool read_only,
                                      GG::Flags<GG::WndFlag> flags/* = GG::CLICKABLE | GG::DRAGABLE | GG::ONTOP | CLOSABLE*/)
{
    if (!GetOptionsDB().Get<bool>("UI.multiple-fleet-windows"))
        CloseAll();
    FleetWnd* retval = new FleetWnd(fleets, selected_fleet, read_only, flags);
    m_fleet_wnds.insert(retval);
    GG::Connect(retval->ClosingSignal,  &FleetUIManager::FleetWndClosing,   this);
    GG::Connect(retval->ClickedSignal,  &FleetUIManager::FleetWndClicked,   this);
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

void FleetUIManager::CullEmptyWnds() {
    for (std::set<FleetWnd*>::iterator it = m_fleet_wnds.begin(); it != m_fleet_wnds.end(); ) {
        std::set<FleetWnd*>::iterator temp_it = it++;
        if ((*temp_it)->Fleets().empty()) {
            delete *temp_it;
        } else {
            for (std::set<FleetDetailWnd*>::iterator it2 = m_fleet_and_detail_wnds[*temp_it].begin(); it2 != m_fleet_and_detail_wnds[*temp_it].end(); ) {
                std::set<FleetDetailWnd*>::iterator temp_it2 = it2++;
                Fleet* fleet = 0;
                if (!(fleet = (*temp_it2)->GetFleet()) || !fleet->NumShips())
                    delete *temp_it2;
            }
        }
    }
}

void FleetUIManager::SetActiveFleetWnd(FleetWnd* fleet_wnd) {
    if (fleet_wnd != m_active_fleet_wnd) {
        // disconnect old active FleetWnd signal
        if (m_active_fleet_wnd)
            m_ative_fleet_wnd_signal.disconnect();

        // set new active FleetWnd
        m_active_fleet_wnd = fleet_wnd;

        // connect new active FleetWnd selection changed signal
        m_ative_fleet_wnd_signal = GG::Connect(m_active_fleet_wnd->SelectedFleetsChangedSignal, ActiveFleetWndSelectedFleetsChangedSignal);

        ActiveFleetWndChangedSignal();
    }
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
    bool    ValidShip(const Ship* ship, const Fleet* new_fleet) {
        const Fleet* current_fleet = ship->GetFleet();
        return (!current_fleet || current_fleet->ID() != new_fleet->ID()) &&
            (ship->X() == new_fleet->X() && ship->Y() == new_fleet->Y()) &&
            ship->SystemID() == new_fleet->SystemID() &&
            *ship->Owners().begin() == *new_fleet->Owners().begin();
    }

    bool    ValidShip(const Ship* ship, double x, double y, int system_id, int empire) {
        return (ship->X() == x && ship->Y() == y) &&
            ship->SystemID() == system_id &&
            *ship->Owners().begin() == empire;
    }

    bool    ValidFleet(const Fleet* fleet, const Fleet* target_fleet) {
        return fleet->ID() != target_fleet->ID() &&
            fleet->SystemID() == target_fleet->SystemID() &&
            (fleet->X() == target_fleet->X() && fleet->Y() == target_fleet->Y()) &&
            *fleet->Owners().begin() == *target_fleet->Owners().begin();
    }

    ////////////////////////////////////////////////
    // ShipDataPanel
    ////////////////////////////////////////////////
    /** Represents a single ship.  This class is used as the sole Control in
        each ShipRow. */
    class ShipDataPanel : public GG::Control {
    public:
        ShipDataPanel(GG::X w, GG::Y h, const Ship* ship) :
            Control(GG::X0, GG::Y0, w, h, GG::Flags<GG::WndFlag>()),
            m_ship(ship),
            m_ship_icon(0),
            m_ship_name_text(0),
            m_design_name_text(0),
            m_stat_icons(),
            m_selected(false)
        {
            if (!m_ship)
                throw std::invalid_argument("ShipDataPanel::ShipDataPanel() : Attempted to construct a ShipDataPanel from a null ship pointer.");

            EnableChildClipping();

            m_ship_name_text = new GG::TextControl(GG::X(Value(h)), GG::Y0, GG::X1, LabelHeight(),
                                                   m_ship->Name(), ClientUI::GetFont(),
                                                   ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
            AttachChild(m_ship_name_text);

            if (const ShipDesign* design = m_ship->Design()) {
                m_design_name_text = new GG::TextControl(GG::X(Value(h)), GG::Y0, GG::X1, LabelHeight(),
                                                         design->Name(), ClientUI::GetFont(),
                                                         ClientUI::TextColor(), GG::FORMAT_RIGHT | GG::FORMAT_VCENTER);
                AttachChild(m_design_name_text);
            }

            std::vector<MeterType> meters;
            meters.push_back(METER_HEALTH);     meters.push_back(METER_FUEL);   meters.push_back(METER_DETECTION);
            meters.push_back(METER_STEALTH);    meters.push_back(METER_SHIELD);

            for (std::vector<MeterType>::const_iterator it = meters.begin(); it != meters.end(); ++it) {
                StatisticIcon* icon = new StatisticIcon(GG::X0, GG::Y0, StatIconWidth(), StatIconHeight(),
                                                        ClientUI::MeterIcon(*it), 0, 0, true, false,
                                                        GG::Flags<GG::WndFlag>());
                m_stat_icons.push_back(std::make_pair(*it, icon));
                AttachChild(icon);
            }

            m_ship_connection = GG::Connect(m_ship->StateChangedSignal, &ShipDataPanel::Refresh, this);
            Refresh();
        }

        ~ShipDataPanel() {
            delete m_ship_icon;
            m_ship_connection.disconnect();
        }

        virtual void    Render() {
            const GG::Clr& unselected_color = GG::CLR_GRAY;
            const GG::Clr& selected_color = GG::CLR_WHITE;

            GG::Clr color_to_use = m_selected ? selected_color : unselected_color;

            if (Disabled())
                color_to_use = DisabledColor(color_to_use);

            GG::Pt ul = UpperLeft(), lr = LowerRight();
            GG::Pt text_ul = ul + GG::Pt(GG::X(ICON_SIZE),   GG::Y0);
            GG::Pt text_lr = ul + GG::Pt(Width(),            LabelHeight());

            GG::FlatRectangle(ul,       lr,         GG::CLR_ZERO, color_to_use, 1);
            GG::FlatRectangle(text_ul,  text_lr,    color_to_use, GG::CLR_ZERO, 0);
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
            m_ship_icon = NULL;
            const int ICON_OFFSET = Value((Size().y - ICON_SIZE) / 2);
            boost::shared_ptr<GG::Texture> icon;
            const ShipDesign* design = m_ship->Design();
            if (design)
                icon = ClientUI::ShipIcon(design->ID());
            else
                icon = ClientUI::ShipIcon(-1);  // default icon
            m_ship_icon = new GG::StaticGraphic(GG::X(ICON_OFFSET), GG::Y(ICON_OFFSET), GG::X(ICON_SIZE), GG::Y(ICON_SIZE),
                                                icon, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
            AttachChild(m_ship_icon);
        }

        void            Refresh() {
            SetShipIcon();

            m_ship_name_text->SetText(m_ship->Name());

            if (const ShipDesign* design = m_ship->Design()) {
                if (m_design_name_text)
                    m_design_name_text->SetText(design->Name());
            }

            // set stat icon values and positions
            GG::Pt icon_ul(GG::X(ICON_SIZE) + GG::X(PAD), LabelHeight());
            for (std::vector<std::pair<MeterType, StatisticIcon*> >::const_iterator it = m_stat_icons.begin(); it != m_stat_icons.end(); ++it) {
                it->second->SetValue(m_ship->MeterPoints(it->first));
                GG::Pt icon_lr = icon_ul + GG::Pt(StatIconWidth(), StatIconHeight());
                it->second->SizeMove(icon_ul, icon_lr);
                icon_ul += GG::Pt(StatIconSpacingWidth(), GG::Y0);
            }
        }

        void            DoLayout() {
            if (m_ship_icon) {
                // position icon in centre of available space
                int ICON_OFFSET = std::max(0, (Value(Height()) - ICON_SIZE) / 2);
                GG::Pt icon_ul = GG::Pt(GG::X(ICON_OFFSET), GG::Y(ICON_OFFSET));
                GG::Pt icon_lr = icon_ul + GG::Pt(GG::X(ICON_SIZE), GG::Y(ICON_SIZE));

                m_ship_icon->SizeMove(icon_ul, icon_lr);
            }


            GG::Pt name_ul = GG::Pt(GG::X(ICON_SIZE + PAD), GG::Y0);
            GG::Pt name_lr = GG::Pt(Width() - GG::X(PAD),   LabelHeight());
            m_ship_name_text->SizeMove(name_ul, name_lr);

            if (m_design_name_text)
                m_design_name_text->SizeMove(name_ul, name_lr);


            // set stat icon positions
            GG::Pt icon_ul(GG::X(ICON_SIZE) + GG::X(PAD), LabelHeight());
            for (std::vector<std::pair<MeterType, StatisticIcon*> >::const_iterator it = m_stat_icons.begin(); it != m_stat_icons.end(); ++it) {
                GG::Pt icon_lr = icon_ul + GG::Pt(StatIconWidth(), StatIconHeight());
                it->second->SizeMove(icon_ul, icon_lr);
                icon_ul += GG::Pt(StatIconSpacingWidth(), GG::Y0);
            }
        }

        const Ship* const                                   m_ship;
        GG::StaticGraphic*                                  m_ship_icon;
        GG::TextControl*                                    m_ship_name_text;
        GG::TextControl*                                    m_design_name_text;
        std::vector<std::pair<MeterType, StatisticIcon*> >  m_stat_icons;   // statistic icons and associated meter types
        bool                                                m_selected;
        boost::signals::connection                          m_ship_connection;
    };


    ////////////////////////////////////////////////
    // ShipRow
    ////////////////////////////////////////////////
    /** A ListBox::Row subclass used to represent ships in ShipListBoxes. */
    class ShipRow : public GG::ListBox::Row {
    public:
        ShipRow(Ship* ship, GG::X w, GG::Y h) :
            GG::ListBox::Row(w, h, ship ? SHIP_DROP_TYPE_STRING : ""),
            m_ship(ship)
        {
            SetName("ShipRow");
            EnableChildClipping();
            push_back(new ShipDataPanel(w, h, m_ship));
        }

        void            SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            const GG::Pt old_size = Size();
            GG::ListBox::Row::SizeMove(ul, lr);
            //std::cout << "ShipRow::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
            if (!empty() && old_size != Size())
                at(0)->Resize(Size());
        }

        int             ShipID() const {return m_ship->ID();}

        Ship* const     m_ship;
    };
}

////////////////////////////////////////////////
// FleetDataPanel
////////////////////////////////////////////////
/** Represents a single fleet.  This class is used as the drop-target in
    FleetWnd (if the ctor parameter \a fleet is zero), and also as the sole
    Control in each FleetRow (if the ctor parameter \a fleet is nonzero). */
class FleetDataPanel : public GG::Control {
public:
    FleetDataPanel(GG::X w, GG::Y h, const Fleet* fleet, int empire = ALL_EMPIRES,
                   int system_id = UniverseObject::INVALID_OBJECT_ID,
                   double x = UniverseObject::INVALID_POSITION, double y = UniverseObject::INVALID_POSITION);

    virtual void        DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last, const GG::Pt& pt) const;

    bool                Selected() const;

    virtual void        Render();
    virtual void        DragDropEnter(const GG::Pt& pt, const std::map<Wnd*, GG::Pt>& drag_drop_wnds, GG::Flags<GG::ModKey> mod_keys);
    virtual void        DragDropLeave();
    virtual void        AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt);
    void                Select(bool b);
    virtual void        SizeMove(const GG::Pt& ul, const GG::Pt& lr);

    mutable boost::signal<void (Ship*, const std::vector<int>&)> NewFleetFromShipsSignal;

private:
    void                Refresh();
    double              StatValue(MeterType meter_type) const;

    void                DoLayout();

    const Fleet* const  m_fleet;
    const int           m_empire;
    const int           m_system_id;
    const double        m_x;
    const double        m_y;

    boost::signals::connection  m_fleet_connection;

    GG::Control*        m_fleet_icon;
    GG::TextControl*    m_fleet_name_text;

    std::vector<std::pair<MeterType, StatisticIcon*> >
                        m_stat_icons;   // statistic icons and associated meter types

    bool                m_selected;
};


FleetDataPanel::FleetDataPanel(GG::X w, GG::Y h, const Fleet* fleet,
                               int empire, int system_id, double x, double y) :
    Control(GG::X0, GG::Y0, w, h, fleet ? GG::Flags<GG::WndFlag>() : GG::CLICKABLE),
    m_fleet(fleet),
    m_empire(empire),
    m_system_id(system_id),
    m_x(x),
    m_y(y),
    m_fleet_icon(0),
    m_fleet_name_text(0),
    m_stat_icons(),
    m_selected(false)
{
    EnableChildClipping();

    m_fleet_name_text = new GG::TextControl(GG::X0, GG::Y0, GG::X1, LabelHeight(), "", ClientUI::GetFont(),
                                            m_fleet ? ClientUI::TextColor() : GG::CLR_BLACK,
                                            GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    AttachChild(m_fleet_name_text);


    if (m_fleet) {
        std::vector<MeterType> meters;
        meters.push_back(METER_FUEL);

        for (std::vector<MeterType>::const_iterator it = meters.begin(); it != meters.end(); ++it) {
            StatisticIcon* icon = new StatisticIcon(GG::X0, GG::Y0, StatIconWidth(), StatIconHeight(),
                                                    ClientUI::MeterIcon(*it), 0, 0, true, false,
                                                    GG::Flags<GG::WndFlag>());
            m_stat_icons.push_back(std::make_pair(*it, icon));
            AttachChild(icon);
        }

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
    GG::Pt text_ul = ul + GG::Pt(GG::X(ICON_SIZE),   GG::Y0);
    GG::Pt text_lr = ul + GG::Pt(Width(),            LabelHeight());

    GG::FlatRectangle(ul,       lr,         GG::CLR_ZERO, color_to_use, 1);
    GG::FlatRectangle(text_ul,  text_lr,    color_to_use, GG::CLR_ZERO, 0);
}

void FleetDataPanel::DragDropEnter(const GG::Pt& pt, const std::map<Wnd*, GG::Pt>& drag_drop_wnds, GG::Flags<GG::ModKey> mod_keys)
{
    Select(true);
    for (std::map<Wnd*, GG::Pt>::const_iterator it = drag_drop_wnds.begin(); it != drag_drop_wnds.end(); ++it) {
        if (it->first->DragDropDataType() != SHIP_DROP_TYPE_STRING ||
            !ValidShip(boost::polymorphic_downcast<ShipRow*>(it->first)->m_ship, m_x, m_y, m_system_id, m_empire))
        {
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

    if (m_fleet) {
        // set fleet name
        m_fleet_name_text->SetText(m_fleet->Name());

        // set icons
        boost::shared_ptr<GG::Texture> head_icon = FleetHeadIcon(m_fleet, FleetButton::FLEET_BUTTON_LARGE);
        boost::shared_ptr<GG::Texture> size_icon = FleetSizeIcon(m_fleet, FleetButton::FLEET_BUTTON_LARGE);
        std::vector<boost::shared_ptr<GG::Texture> > icons; icons.push_back(head_icon); icons.push_back(size_icon);

        m_fleet_icon = new MultiTextureStaticGraphic(GG::X0, GG::Y0, GG::X(ICON_SIZE), GG::Y(ICON_SIZE),
                                                     icons);
        AttachChild(m_fleet_icon);

        // set stat icon values
        for (std::vector<std::pair<MeterType, StatisticIcon*> >::const_iterator it = m_stat_icons.begin(); it != m_stat_icons.end(); ++it) {
            it->second->SetValue(StatValue(it->first));
        }
    } else {
        m_fleet_name_text->SetText(UserString("FW_NEW_FLEET_LABEL"));
    }

    DoLayout();
}

double FleetDataPanel::StatValue(MeterType meter_type) const
{
    if (m_fleet) {
        if (meter_type == METER_FUEL)
            return m_fleet->Fuel();
        //else if (meter_type == METER_FUEL)
            // find least of fleet's ships' stealth...
        else
            return 0.0;
    } else {
        return 0.0;
    }
}

void FleetDataPanel::DoLayout()
{
    if (m_fleet_icon) {
        // position icon in centre of available space
        int ICON_OFFSET = std::max(0, (Value(Height()) - ICON_SIZE) / 2);
        GG::Pt icon_ul = GG::Pt(GG::X(ICON_OFFSET), GG::Y(ICON_OFFSET));
        GG::Pt icon_lr = icon_ul + GG::Pt(GG::X(ICON_SIZE), GG::Y(ICON_SIZE));

        m_fleet_icon->SizeMove(icon_ul, icon_lr);
    }

    GG::Pt name_ul = GG::Pt(GG::X(ICON_SIZE + PAD), GG::Y0);
    GG::Pt name_lr = GG::Pt(Width() - GG::X(PAD),   LabelHeight());
    m_fleet_name_text->SizeMove(name_ul, name_lr);

    // set stat icon positions
    GG::Pt icon_ul(GG::X(ICON_SIZE + PAD), LabelHeight());
    for (std::vector<std::pair<MeterType, StatisticIcon*> >::const_iterator it = m_stat_icons.begin(); it != m_stat_icons.end(); ++it) {
        GG::Pt icon_lr = icon_ul + GG::Pt(StatIconWidth(), StatIconHeight());
        it->second->SizeMove(icon_ul, icon_lr);
        icon_ul += GG::Pt(StatIconSpacingWidth(), GG::Y0);
    }
}


namespace {
    ////////////////////////////////////////////////
    // FleetRow
    ////////////////////////////////////////////////
    /** A ListBox::Row subclass used to represent fleets in FleetsListBoxes. */
    class FleetRow : public GG::ListBox::Row {
    public:
        FleetRow(Fleet* fleet, GG::X w, GG::Y h) :
            GG::ListBox::Row(w, h, fleet ? FLEET_DROP_TYPE_STRING : ""),
            m_fleet(fleet)
        {
            SetName("FleetRow");
            EnableChildClipping();
            push_back(new FleetDataPanel(w, h, m_fleet));
        }

        void            SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            const GG::Pt old_size = Size();
            GG::ListBox::Row::SizeMove(ul, lr);
            //std::cout << "FleetRow::SizeMove size: (" << Value(Width()) << ", " << Value(Height()) << ")" << std::endl;
            if (!empty() && old_size != Size())
                at(0)->Resize(Size());
        }

        int             FleetID() const {return m_fleet ? m_fleet->ID() : UniverseObject::INVALID_OBJECT_ID;}

        Fleet* const    m_fleet;
    };
}

////////////////////////////////////////////////
// FleetsListBox
////////////////////////////////////////////////
/** A CUIListBox subclass used to list all the fleets, and handle drag-and-drop
    operations on them, in FleetWnd. */
class FleetsListBox : public CUIListBox {
public:
    FleetsListBox(GG::X x, GG::Y y, GG::X w, GG::Y h, bool read_only) :
        CUIListBox(x, y, w, h),
        m_selected_fleet(end()),
        m_read_only(read_only)
    {}

    FleetsListBox(bool read_only) :
        CUIListBox(GG::X0, GG::Y0, GG::X1, GG::Y1),
        m_selected_fleet(end()),
        m_read_only(read_only)
    {}

    mutable boost::signal<void (Ship*, const std::vector<int>&)> NewFleetFromShipsSignal;

    virtual void    DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last, const GG::Pt& pt) const {
        iterator row = RowUnderPt(pt);
        bool row_in_range = row != end();
        const Fleet* target_fleet = row_in_range ?
            boost::polymorphic_downcast<const FleetRow*>(*row)->m_fleet :
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

    virtual void    AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt) {
        iterator row = RowUnderPt(pt);
        assert(!m_read_only && row != end());

        Fleet* target_fleet = boost::polymorphic_downcast<FleetRow*>(*row)->m_fleet;
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

    virtual void    DragDropEnter(const GG::Pt& pt, const std::map<Wnd*, GG::Pt>& drag_drop_wnds, GG::Flags<GG::ModKey> mod_keys) {
        DragDropHere(pt, drag_drop_wnds, mod_keys);
    }

    virtual void    DragDropHere(const GG::Pt& pt, const std::map<Wnd*, GG::Pt>& drag_drop_wnds, GG::Flags<GG::ModKey> mod_keys) {
        CUIListBox::DragDropHere(pt, drag_drop_wnds, mod_keys);

        iterator row = RowUnderPt(pt);

        if (m_selected_fleet != row)
            ClearSelection();

        if (m_read_only || row == end())
            return;

        FleetRow* fleet_row = boost::polymorphic_downcast<FleetRow*>(*row);
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
                m_selected_fleet = row;
            }
        }
    }

    virtual void    DragDropLeave() {
        ClearSelection();
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
    void            ClearSelection() {
        if (m_selected_fleet != end())
            boost::polymorphic_downcast<FleetDataPanel*>((*boost::polymorphic_downcast<FleetRow*>(*m_selected_fleet))[0])->Select(false);
        m_selected_fleet = end();
    }

    iterator    m_selected_fleet;
    const bool  m_read_only;
};

////////////////////////////////////////////////
// ShipsListBox
////////////////////////////////////////////////
/** A CUIListBox subclass used to list all the ships, and handle drag-and-drop
    operations on them, in FleetDetailPanel. */
class ShipsListBox : public CUIListBox {
public:
    ShipsListBox(GG::X x, GG::Y y, GG::X w, GG::Y h, Fleet* fleet, bool read_only) :
        CUIListBox(x, y, w, h),
        m_fleet(fleet),
        m_read_only(read_only)
    {}

    ShipsListBox(Fleet* fleet, bool read_only) :
        CUIListBox(GG::X0, GG::Y0, GG::X1, GG::Y1),
        m_fleet(fleet),
        m_read_only(read_only)
    {}

    void            SetFleet(Fleet* fleet) {
        if (m_fleet == fleet)
            return;

        m_fleet = fleet;
        Clear();

        if (!m_fleet)
            return;

        Universe& universe = GetUniverse();
        const GG::Pt row_size = ListRowSize();

        for (Fleet::const_iterator it = m_fleet->begin(); it != m_fleet->end(); ++it) {
            ShipRow* row = new ShipRow(universe.Object<Ship>(*it), GG::X1, row_size.y);
            Insert(row);
            row->Resize(row_size);
        }
    }

    virtual void    DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last, const GG::Pt& pt) const
    {
        for (DropsAcceptableIter it = first; it != last; ++it) {
            const ShipRow* ship_row = 0;
            it->second = !m_read_only &&
                (ship_row = dynamic_cast<const ShipRow*>(it->first)) &&
                ValidShip(ship_row->m_ship, m_fleet);
        }
    }

    virtual void    AcceptDrops(const std::vector<GG::Wnd*>& wnds, const GG::Pt& pt)
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
    Fleet*      m_fleet;
    const bool  m_read_only;
};


////////////////////////////////////////////////
// FleetDetailPanel
////////////////////////////////////////////////
/** Used in FleetDetailWnd's and in the lower half of FleetWnd's to show the
    ships in a fleet, and some basic info about the fleet. */
class FleetDetailPanel : public GG::Wnd {
public:
    FleetDetailPanel(GG::X w, GG::Y h, Fleet* fleet, bool read_only, GG::Flags<GG::WndFlag> flags = GG::Flags<GG::WndFlag>()); ///< ctor

    Fleet*          GetFleet() const {return m_fleet;}  ///< returns the currently-displayed fleet (may be 0)
    void            SetFleet(Fleet* fleet);             ///< sets the currently-displayed Fleet (may be null)
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);

private:
    int             GetShipIDOfListRow(GG::ListBox::iterator it) const; ///< returns the ID number of the ship in row \a row_idx of the ships listbox
    void            Refresh();
    void            DoLayout();
    void            UniverseObjectDeleted(const UniverseObject* obj);
    void            ShipSelectionChanged(const GG::ListBox::SelectionSet& rows);
    void            ShipBrowsed(GG::ListBox::iterator it);
    void            ShipRightClicked(GG::ListBox::iterator it, const GG::Pt& pt);
    std::string     DestinationText() const;
    std::string     ShipStatusText(int ship_id) const;

    Fleet*                      m_fleet;
    const bool                  m_read_only;
    boost::signals::connection  m_fleet_connection;

    GG::TextControl*            m_destination_text;
    ShipsListBox*               m_ships_lb;
    GG::TextControl*            m_ship_status_text;
};


FleetDetailPanel::FleetDetailPanel(GG::X w, GG::Y h, Fleet* fleet, bool read_only, GG::Flags<GG::WndFlag> flags/* = GG::Flags<GG::WndFlag>()*/) : 
    GG::Wnd(GG::X0, GG::Y0, w, h, flags),
    m_fleet(0),
    m_read_only(read_only),
    m_destination_text(0),
    m_ships_lb(0),
    m_ship_status_text(0)
{
    SetName("FleetDetailPanel");
    EnableChildClipping(true);

    m_destination_text = new GG::TextControl(GG::X0, GG::Y0, w, LabelHeight(), "", ClientUI::GetFont(),
                                             ClientUI::TextColor(), GG::FORMAT_LEFT);
    AttachChild(m_destination_text);

    m_ships_lb = new ShipsListBox(0, read_only);
    AttachChild(m_ships_lb);

    m_ship_status_text = new GG::TextControl(GG::X0, GG::Y0, w, LabelHeight(), "", ClientUI::GetFont(),
                                             ClientUI::TextColor(), GG::FORMAT_LEFT);
    m_ships_lb->SetHiliteColor(GG::CLR_ZERO);
    AttachChild(m_ship_status_text);

    if (fleet)
        SetFleet(fleet);

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

void FleetDetailPanel::SetFleet(Fleet* fleet)
{
    Fleet* old_fleet = m_fleet;
    m_fleet = fleet;

    if (m_fleet != old_fleet || !m_fleet)
        m_fleet_connection.disconnect();

    *m_destination_text << "";
    *m_ship_status_text << "";

    m_ships_lb->SetFleet(fleet);


    if (m_fleet) {
        if (m_fleet->NumShips()) {
            *m_destination_text << DestinationText();
            if (old_fleet != fleet)
                m_fleet_connection = GG::Connect(m_fleet->StateChangedSignal, &FleetDetailPanel::Refresh, this);
        } else {
            HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new DeleteFleetOrder(HumanClientApp::GetApp()->EmpireID(), m_fleet->ID())));
        }
    }
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
    SetFleet(m_fleet);
}

void FleetDetailPanel::DoLayout()
{
    GG::X   LEFT = GG::X0;
    GG::X   RIGHT = ClientWidth();
    GG::Y   top = GG::Y0;
    GG::Y   bottom = ClientHeight();

    GG::Pt ul = GG::Pt(LEFT, top);
    GG::Pt lr = GG::Pt(RIGHT, top + LabelHeight());
    m_destination_text->SizeMove(ul, lr);
    top += (LabelHeight() + GG::Y(PAD));

    lr = GG::Pt(RIGHT, bottom);
    ul = GG::Pt(LEFT, bottom - LabelHeight());
    m_ship_status_text->SizeMove(ul, lr);
    bottom -= (LabelHeight() + GG::Y(PAD));

    ul = GG::Pt(LEFT, top);
    lr = GG::Pt(RIGHT, bottom);
    m_ships_lb->SizeMove(ul, lr);
}

void FleetDetailPanel::UniverseObjectDeleted(const UniverseObject* obj)
{
    if (obj == m_fleet)
        SetFleet(0);
}

void FleetDetailPanel::ShipSelectionChanged(const GG::ListBox::SelectionSet& rows)
{
    for (GG::ListBox::iterator it = m_ships_lb->begin(); it != m_ships_lb->end(); ++it) {
        ShipDataPanel* ship_panel = boost::polymorphic_downcast<ShipDataPanel*>((**it)[0]);
        ship_panel->Select(rows.find(it) != rows.end());
    }
}

void FleetDetailPanel::ShipBrowsed(GG::ListBox::iterator it)
{
    if (it != m_ships_lb->end())
        *m_ship_status_text << ShipStatusText(GetShipIDOfListRow(it));
    else
        *m_ship_status_text << "";
}

void FleetDetailPanel::ShipRightClicked(GG::ListBox::iterator it, const GG::Pt& pt)
{
    ShipRow* ship_row = dynamic_cast<ShipRow*>(*it);

    if (!ship_row)
        return;

    Ship* ship = ship_row->m_ship;

    if (!ship ||
        ship->Owners().size() != 1 ||
        HumanClientApp::GetApp()->EmpireID() != *ship->Owners().begin())
    {
        return;
    }

    GG::MenuItem menu_contents;
    menu_contents.next_level.push_back(GG::MenuItem(UserString("RENAME"), 1, false, false));

    GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(),
                        menu_contents, ClientUI::TextColor());

    if (popup.Run()) {
        switch (popup.MenuID()) {
        case 1: { // rename ship
            std::string ship_name = ship->Name();
            CUIEditWnd edit_wnd(GG::X(350), UserString("ENTER_NEW_NAME"), ship_name);
            edit_wnd.Run();

            std::string new_name = edit_wnd.Result();

            if (new_name != "" && new_name != ship_name) {
                HumanClientApp::GetApp()->Orders().IssueOrder(
                    OrderPtr(new RenameOrder(HumanClientApp::GetApp()->EmpireID(), ship->ID(),
                                             new_name)));
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
        retval = boost::io::str(FlexibleFormat(UserString("FW_FLEET_HOLDING_AT")) % current->Name());
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
// static(s)
GG::Pt FleetDetailWnd::s_last_position =    GG::Pt(GG::X(300), GG::Y0);
GG::Pt FleetDetailWnd::s_last_size =        GG::Pt(GG::X(300), GG::Y(200));

FleetDetailWnd::FleetDetailWnd(Fleet* fleet, bool read_only, GG::Flags<GG::WndFlag> flags/* = CLICKABLE | DRAGABLE | RESIZABLE | ONTOP | CLOSABLE*/) : 
    CUIWnd("", s_last_position.x, s_last_position.y, s_last_size.x, s_last_size.y, flags),
    m_fleet_panel(0)
{
    Sound::TempUISoundDisabler sound_disabler;
    m_fleet_panel = new FleetDetailPanel(GG::X1, GG::Y1, fleet, read_only);
    AttachChild(m_fleet_panel);
    SetName(TitleText());
    EnableChildClipping(false);
    DoLayout();
}

FleetDetailWnd::~FleetDetailWnd()
{
    ClosingSignal(this);
}

Fleet* FleetDetailWnd::GetFleet() const
{ return m_fleet_panel->GetFleet(); }

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
    const Fleet* fleet = m_fleet_panel->GetFleet();
    assert(fleet);
    return fleet->Name();
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
GG::Pt FleetWnd::s_last_size =      GG::Pt(GG::X(300), GG::Y(400));

FleetWnd::FleetWnd(std::vector<Fleet*> fleets, int selected_fleet, bool read_only,
                   GG::Flags<GG::WndFlag> flags/* = CLICKABLE | DRAGABLE | ONTOP | CLOSABLE*/) : 
    MapWndPopup("", s_last_position.x, s_last_position.y, s_last_size.x, s_last_size.y, flags | GG::RESIZABLE),
    m_empire_id(ALL_EMPIRES),
    m_system_id(UniverseObject::INVALID_OBJECT_ID),
    m_read_only(read_only),
    m_moving_fleets(true),
    m_current_fleet(),
    m_fleets_lb(0),
    m_new_fleet_drop_target(0),
    m_fleet_detail_panel(0)
{
    Init(fleets, selected_fleet, read_only);
}

FleetWnd::FleetWnd(GG::X w, GG::Y h, std::vector<Fleet*> fleets, int selected_fleet, bool read_only,
                   GG::Flags<GG::WndFlag> flags/* = CLICKABLE | DRAGABLE | ONTOP | CLOSABLE*/) : 
    MapWndPopup("", s_last_position.x, s_last_position.y, w, h, flags | GG::RESIZABLE),
    m_empire_id(ALL_EMPIRES),
    m_system_id(UniverseObject::INVALID_OBJECT_ID),
    m_read_only(read_only),
    m_moving_fleets(true),
    m_current_fleet(),
    m_fleets_lb(0),
    m_new_fleet_drop_target(0),
    m_fleet_detail_panel(0)
{
    Init(fleets, selected_fleet, read_only);
}

FleetWnd::~FleetWnd()
{
    ClientUI::GetClientUI()->GetMapWnd()->ClearProjectedFleetMovementLines();
    ClosingSignal(this);
}

void FleetWnd::Init(const std::vector<Fleet*>& fleets, int selected_fleet, bool read_only)
{
    EnableChildClipping();
    SetMinSize(GG::Pt(CUIWnd::MinimizedWidth(), BORDER_TOP + INNER_BORDER_ANGLE_OFFSET + BORDER_BOTTOM +
                                                ListRowHeight() + 2*GG::Y(PAD)));

    // ensure position is not off screen
    GG::Pt window_pos = UpperLeft();
    if (GG::GUI::GetGUI()->AppWidth() < LowerRight().x)
        window_pos.x = GG::GUI::GetGUI()->AppWidth() - Width();
    if (GG::GUI::GetGUI()->AppHeight() < LowerRight().y)
        window_pos.y = GG::GUI::GetGUI()->AppHeight() - Height();
    MoveTo(window_pos);


    // determine system and empire ownership of fleets in this window
    // also ensure that empty fleetwnds are non-interactive, and that the indicated \a selected_fleet is valid
    if (fleets.empty()) {
        read_only = true;
    } else {
        if (selected_fleet < 0 || selected_fleet >= static_cast<int>(fleets.size())) {
            Logger().errorStream() << "FleetWnd::FleetWnd selected fleet outside valid range";
            selected_fleet = 0;
        }

        // get system id from fleets
        m_system_id = fleets[0]->SystemID();
        for (std::vector<Fleet*>::const_iterator it = fleets.begin(); it != fleets.end(); ++it) {
            if ((*it)->SystemID() != m_system_id) {
                m_system_id = UniverseObject::INVALID_OBJECT_ID;
                break;
            }
        }

        // get owner
        if (fleets[0]->Owners().size() == 1) {
            m_empire_id = *fleets[0]->Owners().begin();
        }
    }

    Sound::TempUISoundDisabler sound_disabler;


    // create fleet list box
    m_fleets_lb = new FleetsListBox(read_only);
    m_fleets_lb->SetHiliteColor(GG::CLR_ZERO);
    GG::Connect(m_fleets_lb->SelChangedSignal,      &FleetWnd::FleetSelectionChanged,   this);
    GG::Connect(m_fleets_lb->LeftClickedSignal,     &FleetWnd::FleetLeftClicked,        this);
    GG::Connect(m_fleets_lb->RightClickedSignal,    &FleetWnd::FleetRightClicked,       this);
    GG::Connect(m_fleets_lb->DoubleClickedSignal,   &FleetWnd::FleetDoubleClicked,      this);
    GG::Connect(m_fleets_lb->ErasedSignal,          &FleetWnd::FleetDeleted,            this);
    AttachChild(m_fleets_lb);

    m_current_fleet = m_fleets_lb->end();   // indicates no selected fleet

    // create fleet detail panel
    m_fleet_detail_panel = new FleetDetailPanel(GG::X1, GG::Y1, 0, read_only);
    AttachChild(m_fleet_detail_panel);

    // create
    if (!m_read_only) {
        if (fleets.empty()) {
            m_new_fleet_drop_target = new FleetDataPanel(GG::X1, ListRowHeight(), 0,
                                                         m_empire_id, m_system_id);
        } else {
            m_new_fleet_drop_target = new FleetDataPanel(GG::X1, ListRowHeight(), 0,
                                                         m_empire_id, m_system_id,
                                                         fleets[0]->X(), fleets[0]->Y());
        }
        AttachChild(m_new_fleet_drop_target);

        m_fleets_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_BROWSEUPDATES);
        m_fleets_lb->AllowDropType(SHIP_DROP_TYPE_STRING);
        m_fleets_lb->AllowDropType(FLEET_DROP_TYPE_STRING);

        GG::Connect(m_new_fleet_drop_target->NewFleetFromShipsSignal,   &FleetWnd::CreateNewFleetFromDrops, this);
    } else {
        m_fleets_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_BROWSEUPDATES | GG::LIST_SINGLESEL);
    }


    // add fleets to list box
    for (unsigned int i = 0; i < fleets.size(); ++i) {
        AddFleet(fleets[i]);
    }


    // random other signals... deletion and changes
    GG::Connect(GetUniverse().UniverseObjectDeleteSignal,               &FleetWnd::UniverseObjectDeleted,   this);

    if (const System* system = GetUniverse().Object<System>(m_system_id))
        GG::Connect(system->StateChangedSignal,                         &FleetWnd::SystemChangedSlot,       this);


    // window title - depends on fleets
    SetName(TitleText());


    // autoselect a fleet, if supposed to
    if (GetOptionsDB().Get<bool>("UI.fleet-autoselect") && !fleets.empty()) {
        GG::ListBox::iterator it = boost::next(m_fleets_lb->begin(), selected_fleet);
        m_fleets_lb->SelectRow(it);
        FleetSelectionChanged(m_fleets_lb->Selections());
        m_current_fleet = it;
        m_fleet_detail_panel->SetFleet(FleetInRow(it));
        m_fleets_lb->BringRowIntoView(it);
    }

    DoLayout();
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
    const GG::Y AVAILABLE_HEIGHT = TOTAL_HEIGHT - 4*GG::Y(PAD); // top and bottom pads, and space between contents pads

    const GG::Y LISTBOX_TOP = GG::Y(PAD);

    // TODO: If size too small for new fleet drop target or ships panel, don't show them.  keep fleet list visible
    //       as long as possible when height is reduced.
    const GG::Y DATA_PANEL_BOTTOM = TOTAL_HEIGHT - GG::Y(PAD);

    const GG::Y ROW_HEIGHT = m_fleets_lb->ListRowSize().y;

    // subtract space for new fleet drop target, and divide remainder between list box and fleet data panel
    const GG::Y DATA_PANEL_HEIGHT = (AVAILABLE_HEIGHT - ROW_HEIGHT) / 3 * 2;
    const GG::Y LIST_BOX_HEIGHT = (AVAILABLE_HEIGHT - ROW_HEIGHT) / 3;

    const GG::Y LISTBOX_BOTTOM = LISTBOX_TOP + LIST_BOX_HEIGHT;
    const GG::Y DATA_PANEL_TOP = DATA_PANEL_BOTTOM - DATA_PANEL_HEIGHT;
    const GG::Y DROP_TARGET_TOP = LISTBOX_BOTTOM + GG::Y(PAD);
    const GG::Y DROP_TARGET_BOTTOM = DROP_TARGET_TOP + ROW_HEIGHT;

    m_fleets_lb->SizeMove(              GG::Pt(LEFT, LISTBOX_TOP),      GG::Pt(RIGHT, LISTBOX_BOTTOM));
    m_new_fleet_drop_target->SizeMove(  GG::Pt(LEFT, DROP_TARGET_TOP),  GG::Pt(RIGHT, DROP_TARGET_BOTTOM));
    m_fleet_detail_panel->SizeMove(     GG::Pt(LEFT, DATA_PANEL_TOP),   GG::Pt(RIGHT, DATA_PANEL_BOTTOM));
}

void FleetWnd::AddFleet(Fleet* fleet)
{
    if (!fleet)
        return;

    const GG::Pt row_size = m_fleets_lb->ListRowSize();
    FleetRow* row = new FleetRow(fleet, GG::X1, row_size.y);
    m_fleets_lb->Insert(row);
    row->Resize(row_size);
}

void FleetWnd::SelectFleet(Fleet* fleet)
{
    if (!fleet) {
        m_fleets_lb->DeselectAll();
        return;
    }

    //std::cout << "FleetWnd::SelectFleet " << fleet->Name() << " (" << fleet->ID() << ")" << std::endl;

    for (GG::ListBox::iterator it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        FleetRow* row = dynamic_cast<FleetRow*>(*it);
        if (row && row->m_fleet == fleet) {
            m_fleets_lb->DeselectAll();
            m_fleets_lb->SelectRow(it);

            FleetSelectionChanged(m_fleets_lb->Selections());

            m_current_fleet = it;
            m_fleet_detail_panel->SetFleet(fleet);
            m_fleets_lb->BringRowIntoView(it);
            SelectedFleetsChangedSignal();
            break;
        }
    }
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

bool FleetWnd::ContainsFleet(int fleet_id) const
{
    for (GG::ListBox::iterator it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        Fleet* fleet = FleetInRow(it);
        if (fleet && fleet->ID() == fleet_id)
            return true;
    }
    return false;
}

std::set<Fleet*> FleetWnd::Fleets() const
{
    std::set<Fleet*> retval;
    for (GG::ListBox::iterator it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        if (Fleet* fleet = FleetInRow(it)) {
            retval.insert(fleet);
        }
    }
    return retval;
}

std::set<Fleet*> FleetWnd::SelectedFleets() const
{
    std::set<Fleet*> retval;
    for (GG::ListBox::iterator it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        if (m_fleets_lb->Selected(it)) {
            Fleet* fleet = FleetInRow(it);
            if (fleet) retval.insert(fleet);
        }
    }
    return retval;
}

void FleetWnd::FleetSelectionChanged(const GG::ListBox::SelectionSet& rows)
{
    m_current_fleet = m_fleets_lb->Caret();

    Fleet* fleet = FleetInRow(m_current_fleet);
    m_fleet_detail_panel->SetFleet(fleet);

    for (GG::ListBox::iterator it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        FleetDataPanel* fleet_panel = boost::polymorphic_downcast<FleetDataPanel*>((**it)[0]);
        fleet_panel->Select(rows.find(it) != rows.end());
    }

    ClickedSignal(this);
    SelectedFleetsChangedSignal();
}

void FleetWnd::FleetRightClicked(GG::ListBox::iterator it, const GG::Pt& pt)
{
    Fleet* fleet = FleetInRow(it);
    if (!fleet ||
        fleet->Owners().size() != 1 ||
        HumanClientApp::GetApp()->EmpireID() != *fleet->Owners().begin())
    {
        return;
    }

    GG::MenuItem menu_contents;
    menu_contents.next_level.push_back(GG::MenuItem(UserString("RENAME"), 1, false, false));

    GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(),
                        menu_contents, ClientUI::TextColor());

    if (popup.Run()) {
        switch (popup.MenuID()) {
        case 1: { // rename fleet
            std::string fleet_name = fleet->Name();
            CUIEditWnd edit_wnd(GG::X(350), UserString("ENTER_NEW_NAME"), fleet_name);
            edit_wnd.Run();

            std::string new_name = edit_wnd.Result();

            if (new_name != "" && new_name != fleet_name) {
                HumanClientApp::GetApp()->Orders().IssueOrder(
                    OrderPtr(new RenameOrder(HumanClientApp::GetApp()->EmpireID(), fleet->ID(),
                                             new_name)));
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
    Fleet* row_fleet = FleetInRow(it);
    if (!row_fleet) {
        Logger().errorStream() << "FleetWnd::FleetDoubleClicked couldn't get fleet in clicked row";
        return;
    }
    int num_open_windows = FleetUIManager::GetFleetUIManager().OpenDetailWnds(this);
    GG::Pt window_posn(std::max(GG::X0, 25 + LowerRight().x + num_open_windows * 25),
                       std::max(GG::Y0, UpperLeft().y + num_open_windows * 25));
    if (FleetDetailWnd* fleet_detail_wnd =
        FleetUIManager::GetFleetUIManager().NewFleetDetailWnd(this, row_fleet, m_read_only)) {
        if (GG::GUI::GetGUI()->AppWidth() < fleet_detail_wnd->LowerRight().x)
            window_posn.x = GG::GUI::GetGUI()->AppWidth() - fleet_detail_wnd->Width();
        if (GG::GUI::GetGUI()->AppHeight() < fleet_detail_wnd->LowerRight().y)
            window_posn.y = GG::GUI::GetGUI()->AppHeight() - fleet_detail_wnd->Height();
        fleet_detail_wnd->MoveTo(window_posn);
    }
}

void FleetWnd::FleetDeleted(GG::ListBox::iterator it)
{
    if (m_current_fleet == it)
        m_current_fleet = m_fleets_lb->end();
    if (m_fleets_lb->Empty())
        CloseClicked();
}

Fleet* FleetWnd::FleetInRow(GG::ListBox::iterator it) const
{
    if (it == m_fleets_lb->end())
        return NULL;

    if (FleetRow* fleet_row = dynamic_cast<FleetRow*>(*it))
        return fleet_row->m_fleet;
    else
        return NULL;
}

std::string FleetWnd::TitleText() const
{
    // no fleets
    if (!m_fleets_lb || m_fleets_lb->Empty())
        return UserString("FW_NO_FLEET");
    Fleet* first_fleet = FleetInRow(m_fleets_lb->begin());
    if (!first_fleet)
        return UserString("FW_NO_FLEET");


    if (const Empire* empire = Empires().Lookup(m_empire_id)) {
        if (const System* system = GetUniverse().Object<System>(m_system_id)) {
            return boost::io::str(FlexibleFormat(UserString("FW_EMPIRE_FLEETS_AT_SYSTEM")) %
                                  empire->Name() % system->Name());
        } else {
            return boost::io::str(FlexibleFormat(UserString("FW_EMPIRE_FLEETS")) %
                                  empire->Name());
        }
    } else {
        if (const System* system = GetUniverse().Object<System>(m_system_id)) {
            return boost::io::str(FlexibleFormat(UserString("FW_GENERIC_FLEETS_AT_SYSTEM")) %
                                  system->Name());
        } else {
            return boost::io::str(FlexibleFormat(UserString("FW_GENERIC_FLEETS")));
        }
    }
}

void FleetWnd::CreateNewFleetFromDrops(Ship* first_ship, const std::vector<int>& ship_ids)
{
    Fleet* some_fleet = FleetInRow(m_fleets_lb->begin());

    if (!some_fleet || !first_ship)
        return;

    System* system = some_fleet->GetSystem();
    double some_fleet_x = some_fleet->X();
    double some_fleet_y = some_fleet->Y();

    int empire_id = HumanClientApp::GetApp()->EmpireID();
    int new_fleet_id = GetNewObjectID();
    if (new_fleet_id == UniverseObject::INVALID_OBJECT_ID) {
        ClientUI::MessageBox(UserString("SERVER_TIMEOUT"), true);
        return;
    }

    std::string fleet_name = Fleet::GenerateFleetName(ship_ids, new_fleet_id);

    Fleet* new_fleet = 0;
    if (system) {
        HumanClientApp::GetApp()->Orders().IssueOrder(
            OrderPtr(new NewFleetOrder(empire_id, fleet_name, new_fleet_id, system->ID(), ship_ids)));
        System::ObjectVec fleets = system->FindObjectsInOrbit(-1, StationaryFleetVisitor(empire_id));
        for (unsigned int i = 0; i < fleets.size(); ++i) {
            if (fleets[i]->ID() == new_fleet_id) {
                new_fleet = universe_object_cast<Fleet*>(fleets[i]);
                break;
            }
        }
    } else {
        HumanClientApp::GetApp()->Orders().IssueOrder(
            OrderPtr(new NewFleetOrder(empire_id, fleet_name, new_fleet_id,
                                       some_fleet_x, some_fleet_y, ship_ids)));
        std::vector<Fleet*> fleets = GetUniverse().FindObjects<Fleet>();
        for (unsigned int i = 0; i < fleets.size(); ++i) {
            if (fleets[i]->Name() == fleet_name &&
                fleets[i]->X() == some_fleet_x &&
                fleets[i]->Y() == some_fleet_y) {
                new_fleet = fleets[i];
                break;
            }
        }
        AddFleet(new_fleet);
    }
}

void FleetWnd::UniverseObjectDeleted(const UniverseObject *obj)
{
    if (Fleet* fleet = FleetInRow(m_current_fleet)) {
        if (fleet == obj)
            m_current_fleet = m_fleets_lb->end();
    }

    const Fleet* fleet;
    if (!(fleet = universe_object_cast<const Fleet*>(obj)))
        return;

    if (m_fleet_detail_panel->GetFleet() == fleet)
        m_fleet_detail_panel->SetFleet(0);

    for (GG::ListBox::iterator it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        if (FleetInRow(it) == fleet) {
            delete m_fleets_lb->Erase(it);
            break;
        }
    }
}

void FleetWnd::SystemChangedSlot()
{
    const System* system = FleetInRow(m_fleets_lb->begin())->GetSystem();
    System::ConstObjectVec system_fleet_vec = system->FindObjects(OwnedVisitor<Fleet>(m_empire_id));
    std::set<const Fleet*> system_fleet_set;
    for (System::ConstObjectVec::iterator it = system_fleet_vec.begin();
         it != system_fleet_vec.end();
         ++it) {
        system_fleet_set.insert(boost::polymorphic_downcast<const Fleet*>(*it));
    }
    for (GG::ListBox::iterator it = m_fleets_lb->begin(); it != m_fleets_lb->end(); ++it) {
        system_fleet_set.erase(FleetInRow(it));
    }
    for (std::set<const Fleet*>::const_iterator it = system_fleet_set.begin();
         it != system_fleet_set.end();
         ++it) {
        AddFleet(const_cast<Fleet*>(*it));
    }
}

const GG::Pt& FleetWnd::LastPosition()
{
    return s_last_position;
}

const GG::Pt& FleetWnd::LastSize()
{
    return s_last_size;
}

