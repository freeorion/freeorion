#include "FleetWindow.h"
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

#include <GG/DrawUtil.h>
#include <GG/Menu.h>
#include <GG/StaticGraphic.h>
#include <GG/TextControl.h>

#include <boost/format.hpp>
#include <iostream>

namespace {
    const int NEW_FLEET_BUTTON_WIDTH = 75;
    const int FLEET_LISTBOX_WIDTH =  250;
    const int FLEET_LISTBOX_HEIGHT = 150;
    const int CONTROL_MARGIN = 5; // gap to leave between controls in these window

    class FleetDataPanel : public GG::Control
    {
    public:
        enum {FLEET_ICON_SZ = 38, 
              FLEET_NAME_HT = 20, 
              STAT_ICON_WD = 40};

        FleetDataPanel(int w, int h, const Fleet* fleet) :
            Control(0, 0, w, h, 0),
            m_fleet(fleet),
            m_fleet_icon(0),
            m_fleet_name_text(new GG::TextControl(h, 0, w - h - 5, FLEET_NAME_HT, m_fleet ? m_fleet->Name() : "<i>" + UserString("FW_NEW_FLEET_LABEL") + "</i>", 
                                                  GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS), m_fleet ? ClientUI::TEXT_COLOR : GG::CLR_BLACK, GG::TF_RIGHT | GG::TF_VCENTER)),
            m_num_ships_stat(0),
            m_fleet_strength_stat(0),
            m_damage_icon(0),
            m_colonizer_icon(0),
            m_selected(false)
        {
            EnableChildClipping();

            AttachChild(m_fleet_name_text);

            if (m_fleet) {
                m_num_ships_stat = new StatisticIcon(h, FLEET_NAME_HT, STAT_ICON_WD, h - FLEET_NAME_HT - 1, ClientUI::ART_DIR + "icons/3shipfleet.png", 
                                                     ClientUI::TEXT_COLOR, 0);
                m_fleet_strength_stat = new StatisticIcon(h + STAT_ICON_WD, FLEET_NAME_HT, STAT_ICON_WD, h - FLEET_NAME_HT - 1, ClientUI::ART_DIR + "icons/combatstrength.png", 
                                                          ClientUI::TEXT_COLOR, 0);
                AttachChild(m_num_ships_stat);
                AttachChild(m_fleet_strength_stat);
                GG::Connect(m_fleet->StateChangedSignal, &FleetDataPanel::Refresh, this);
            }

            Refresh();
        }

        virtual void Render()
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

        void Select(bool b)
        {
            if (m_selected != b) {
                m_selected = b;

                const GG::Clr& unselected_text_color = ClientUI::TEXT_COLOR;
                const GG::Clr& selected_text_color = GG::CLR_BLACK;

                GG::Clr text_color_to_use = m_selected ? selected_text_color : unselected_text_color;

                if (!m_fleet) // use different colors for the "new fleet" panel
                    text_color_to_use = GG::CLR_BLACK;

                if (Disabled())
                    text_color_to_use = DisabledColor(text_color_to_use);

                m_fleet_name_text->SetTextColor(text_color_to_use);
            }
        }

    private:
        void SetFleetIcon()
        {
            DeleteChild(m_fleet_icon);

            const int ICON_OFFSET = (Size().y - FLEET_ICON_SZ) / 2;
            boost::shared_ptr<GG::Texture> icon;

            if (m_fleet) { // a regular fleet data panel
                if (2 <= m_fleet->NumShips() && m_fleet->NumShips() <= 5) {
                    icon = GG::GUI::GetGUI()->GetTexture(ClientUI::ART_DIR + "icons/" + 
                                                         boost::lexical_cast<std::string>(m_fleet->NumShips()) + "shipfleet.png");
                } else if (5 < m_fleet->NumShips()) {
                    icon = GG::GUI::GetGUI()->GetTexture(ClientUI::ART_DIR + "icons/5shipfleet.png");
                } else if (m_fleet->NumShips() == 1) {
                    Ship* ship = GetUniverse().Object<Ship>(*m_fleet->begin());
                    std::string design_name;
                    if (ship->Design()) {
                        design_name = ship->Design()->name;
                        unsigned int space;
                        if ((space = design_name.find(' ')) != std::string::npos) {
                            design_name[space] = '_';
                        }
                    } else {
                        design_name = "Scout";
                    }
                    icon = GG::GUI::GetGUI()->GetTexture(ClientUI::ART_DIR + "icons/" + design_name + ".png");
                }
            } else { // the "new fleet" data panel
                icon = GG::GUI::GetGUI()->GetTexture(ClientUI::ART_DIR + "icons/newfleet.png");
            }

            if (icon) {
                m_fleet_icon = new GG::StaticGraphic(ICON_OFFSET, ICON_OFFSET, FLEET_ICON_SZ, FLEET_ICON_SZ, icon, GG::GR_FITGRAPHIC);
                AttachChild(m_fleet_icon);
            }
        }
        void Refresh()
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
                        if (design->colonize)
                            contains_colony_ship = true;
                        attack_strength += design->attack;
                        attack_factor_unknown = false;
                    }
                    // TODO: account for damaged ships once damage system is in place
                }
                m_fleet_strength_stat->SetValue(attack_factor_unknown ? StatisticIcon::UNKNOWN_VALUE : attack_strength);

                const int ICON_SPACING = 5;
                const int ICON_SZ = Height() - FLEET_NAME_HT - 1;
                int x_position = m_fleet_strength_stat->LowerRight().x - ClientUpperLeft().x + ICON_SPACING;
                if (damaged_ships) {
                    if (!m_damage_icon) {
                        m_damage_icon = new GG::StaticGraphic(x_position, FLEET_NAME_HT, ICON_SZ, ICON_SZ, 
                                                              GG::GUI::GetGUI()->GetTexture(ClientUI::ART_DIR + "icons/damagemarker.png"), GG::GR_FITGRAPHIC);
                        AttachChild(m_damage_icon);
                    }
                    x_position += m_damage_icon->Width() + ICON_SPACING;
                } else if (m_damage_icon) {
                    DeleteChild(m_damage_icon);
                }
                if (contains_colony_ship) {
                    if (!m_colonizer_icon) {
                        m_colonizer_icon = new GG::StaticGraphic(x_position, FLEET_NAME_HT, ICON_SZ, ICON_SZ, 
                                                                 GG::GUI::GetGUI()->GetTexture(ClientUI::ART_DIR + "icons/colonymarker.png"), GG::GR_FITGRAPHIC);
                        AttachChild(m_colonizer_icon);
                    }
                    x_position += m_colonizer_icon->Width() + ICON_SPACING;
                } else if (m_colonizer_icon) {
                    DeleteChild(m_colonizer_icon);
                }
            }
        }

        const Fleet* const m_fleet;
        GG::StaticGraphic* m_fleet_icon;
        GG::TextControl* m_fleet_name_text;
        StatisticIcon* m_num_ships_stat;
        StatisticIcon* m_fleet_strength_stat;
        GG::StaticGraphic* m_damage_icon;
        GG::StaticGraphic* m_colonizer_icon;
        bool m_selected;
    };

    class ShipDataPanel : public GG::Control
    {
    public:
        enum {SHIP_ICON_SZ = FleetDataPanel::FLEET_ICON_SZ, 
              SHIP_NAME_HT = FleetDataPanel::FLEET_NAME_HT, 
              STAT_ICON_WD = FleetDataPanel::STAT_ICON_WD};

        ShipDataPanel(int w, int h, const Ship* ship) :
            Control(0, 0, w, h, 0),
            m_ship(ship),
            m_ship_icon(0),
            m_ship_name_text(new GG::TextControl(h, 0, w - h - 5, SHIP_NAME_HT, m_ship->Name(), GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS), ClientUI::TEXT_COLOR, 
                                                 GG::TF_RIGHT | GG::TF_VCENTER)),
            m_ship_strength_stat(0),
            m_damage_stat(0),
            m_colonizer_icon(0),
            m_selected(false)
        {
            if (!m_ship)
                throw std::invalid_argument("ShipDataPanel::ShipDataPanel() : Attempted to construct a ShipDataPanel from a null ship pointer.");

            EnableChildClipping();

            AttachChild(m_ship_name_text);

            m_ship_strength_stat = new StatisticIcon(h, SHIP_NAME_HT, STAT_ICON_WD, h - SHIP_NAME_HT - 1, ClientUI::ART_DIR + "icons/combatstrength.png", 
                                                     ClientUI::TEXT_COLOR, 0);
            AttachChild(m_ship_strength_stat);
            GG::Connect(m_ship->StateChangedSignal, &ShipDataPanel::Refresh, this);

            Refresh();
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

                const GG::Clr& unselected_text_color = ClientUI::TEXT_COLOR;
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
            if (m_ship->Design()) {
                design_name = m_ship->Design()->name;
                unsigned int space;
                if ((space = design_name.find(' ')) != std::string::npos) {
                    design_name[space] = '_';
                }
            } else {
                design_name = "Scout";
            }
            m_ship_icon = new GG::StaticGraphic(ICON_OFFSET, ICON_OFFSET, SHIP_ICON_SZ, SHIP_ICON_SZ, 
                                                GG::GUI::GetGUI()->GetTexture(ClientUI::ART_DIR + "icons/" + design_name + ".png"), GG::GR_FITGRAPHIC);
            AttachChild(m_ship_icon);
        }
        void Refresh()
        {
            SetShipIcon();
            m_ship_name_text->SetText(m_ship->Name());
            m_ship_strength_stat->SetValue(m_ship->Design() ? m_ship->Design()->attack : StatisticIcon::UNKNOWN_VALUE);

            const int ICON_SPACING = 5;
            const int ICON_SZ = Height() - SHIP_NAME_HT - 1;
            int x_position = m_ship_strength_stat->LowerRight().x - ClientUpperLeft().x + ICON_SPACING;
            int damage_pts = 0; // TODO: acount for damaged ships once damage system is in place
            if (damage_pts) {
                if (!m_damage_stat) {
                    m_damage_stat = new StatisticIcon(x_position, SHIP_NAME_HT, STAT_ICON_WD, ICON_SZ, ClientUI::ART_DIR + "icons/damagemarker.png", 
                                                      ClientUI::TEXT_COLOR, damage_pts);
                    AttachChild(m_damage_stat);
                }
                x_position += m_damage_stat->Width(); // no icon spacing is needed after stat icons
            } else if (m_damage_stat) {
                DeleteChild(m_damage_stat);
            }
            if (m_ship->Design() && m_ship->Design()->colonize) {
                if (!m_colonizer_icon) {
                    m_colonizer_icon = new GG::StaticGraphic(x_position, SHIP_NAME_HT, ICON_SZ, ICON_SZ, 
                                                             GG::GUI::GetGUI()->GetTexture(ClientUI::ART_DIR + "icons/colonymarker.png"), GG::GR_FITGRAPHIC);
                    AttachChild(m_colonizer_icon);
                }
                x_position += m_colonizer_icon->Width() + ICON_SPACING;
            } else if (m_colonizer_icon) {
                DeleteChild(m_colonizer_icon);
            }
        }

        const Ship* const m_ship;
        GG::StaticGraphic* m_ship_icon;
        GG::TextControl* m_ship_name_text;
        StatisticIcon* m_ship_strength_stat;
        StatisticIcon* m_damage_stat;
        GG::StaticGraphic* m_colonizer_icon;
        bool m_selected;
    };

    struct FleetRow : public GG::ListBox::Row
    {
        enum {PANEL_WD = FLEET_LISTBOX_WIDTH - 8, PANEL_HT = 38};

        FleetRow(Fleet* fleet) :
            GG::ListBox::Row(PANEL_WD - ClientUI::SCROLL_WIDTH, PANEL_HT + 4, "Fleet"),
            m_fleet(fleet)
        {
            EnableChildClipping();
            push_back(new FleetDataPanel(Width(), PANEL_HT, m_fleet));
        }

        int FleetID() const {return m_fleet ? m_fleet->ID() : UniverseObject::INVALID_OBJECT_ID;}

        Fleet* const m_fleet;
    };

    struct ShipRow : public GG::ListBox::Row
    {
        enum {PANEL_WD = FleetRow::PANEL_WD, PANEL_HT = FleetRow::PANEL_HT};

        ShipRow(Ship* ship) :
            GG::ListBox::Row(PANEL_WD - ClientUI::SCROLL_WIDTH, PANEL_HT + 4, "Ship"),
            m_ship(ship)
        {
            if (!ship)
                throw std::invalid_argument("ShipRow::ShipRow() : Attempted to contruct a ShipRow using a null ship pointer.");

            EnableChildClipping();
            push_back(new ShipDataPanel(Width(), PANEL_HT, m_ship));
       }

        int ShipID() const {return m_ship->ID();}

        Ship* const m_ship;
    };

    bool CanJoin(const Ship* ship, const Fleet* fleet)
    {
        const Fleet* home_fleet = ship->GetFleet();
        return home_fleet->ID() != fleet->ID() && (home_fleet->X() == fleet->X() && home_fleet->Y() == fleet->Y());
    }

    bool temp_header_bool = RecordHeaderFile(FleetWindowRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}

////////////////////////////////////////////////
// FleetDetailPanel
////////////////////////////////////////////////
FleetDetailPanel::FleetDetailPanel(int x, int y, Fleet* fleet, bool read_only, Uint32 flags/* = 0*/) : 
    Wnd(x, y, 1, 1, flags),
    m_fleet(0),
    m_read_only(read_only),
    m_destination_text(0),
    m_ships_lb(0),
    m_ship_status_text(0)
{
    m_destination_text = new GG::TextControl(0, 0, FLEET_LISTBOX_WIDTH, ClientUI::PTS + 4, "", GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS), ClientUI::TEXT_COLOR, GG::TF_LEFT);
    m_ships_lb = new CUIListBox(0, m_destination_text->LowerRight().y + CONTROL_MARGIN, FLEET_LISTBOX_WIDTH, FLEET_LISTBOX_HEIGHT);
    m_ship_status_text = new GG::TextControl(0, m_ships_lb->LowerRight().y + CONTROL_MARGIN, m_ships_lb->Width(), ClientUI::PTS + 4, 
                                             "", GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS), ClientUI::TEXT_COLOR, GG::TF_LEFT);
    m_ships_lb->SetHiliteColor(GG::CLR_ZERO);
    Resize(m_ship_status_text->LowerRight());

    SetFleet(fleet);
    Init();

    m_universe_object_delete_connection = GG::Connect(GetUniverse().UniverseObjectDeleteSignal, &FleetDetailPanel::UniverseObjectDelete, this);
}

FleetDetailPanel::~FleetDetailPanel()
{
  m_universe_object_delete_connection.disconnect();
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
    if ((m_fleet = fleet)) {
        Universe& universe = GetUniverse();
        if (m_fleet->NumShips()) {
            for (Fleet::const_iterator it = m_fleet->begin(); it != m_fleet->end(); ++it) {
                m_ships_lb->Insert(new ShipRow(universe.Object<Ship>(*it)));
            }
        } else {
            PanelEmptySignal(m_fleet);
            return; // return immediately, since the signal above may invalidate this
        }
        *m_destination_text << DestinationText();
        if (fleet != old_fleet)
            m_fleet_connection = GG::Connect(m_fleet->StateChangedSignal, &FleetDetailPanel::Refresh, this);
    }
}

void FleetDetailPanel::CloseClicked()
{
}

void FleetDetailPanel::Init()
{
    if (m_read_only) {
        m_ships_lb->SetStyle(GG::LB_NOSEL | GG::LB_BROWSEUPDATES);
    } else {
        m_ships_lb->SetStyle(GG::LB_QUICKSEL | GG::LB_BROWSEUPDATES);
        m_ships_lb->AllowDropType("Ship");
    }

    AttachSignalChildren();

    GG::Connect(m_ships_lb->SelChangedSignal, &FleetDetailPanel::ShipSelectionChanged, this);
    GG::Connect(m_ships_lb->BrowsedSignal, &FleetDetailPanel::ShipBrowsed, this);
    GG::Connect(m_ships_lb->DroppedSignal, &FleetDetailPanel::ShipDroppedIntoList, this);
    GG::Connect(m_ships_lb->RightClickedSignal, &FleetDetailPanel::ShipRightClicked, this);
}

void FleetDetailPanel::AttachSignalChildren()
{
    AttachChild(m_destination_text);
    AttachChild(m_ships_lb);
    AttachChild(m_ship_status_text);
}

void FleetDetailPanel::DetachSignalChildren()
{
    DetachChild(m_destination_text);
    DetachChild(m_ships_lb);
    DetachChild(m_ship_status_text);
}

void FleetDetailPanel::Refresh()
{
    SetFleet(m_fleet);
}

void FleetDetailPanel::UniverseObjectDelete(const UniverseObject *obj)
{
  if(obj == m_fleet)
    SetFleet(NULL);
}

void FleetDetailPanel::ShipSelectionChanged(const std::set<int>& rows)
{
    for (int i = 0; i < m_ships_lb->NumRows(); ++i) {
        ShipDataPanel* ship_panel = static_cast<ShipDataPanel*>(m_ships_lb->GetRow(i)[0]);
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

void FleetDetailPanel::ShipDroppedIntoList(int row_idx, GG::ListBox::Row* row)
{
    ShipRow* ship_row = dynamic_cast<ShipRow*>(row);
    int ship_id = ship_row->ShipID();
    if (!m_fleet) {
        // creating a new fleet can fail but will be handled by listbox exception
        m_fleet = NeedNewFleetSignal(ship_id);

        m_fleet_connection.disconnect();
        m_fleet_connection = GG::Connect(m_fleet->StateChangedSignal, &FleetDetailPanel::Refresh, this);
        
        *m_destination_text << DestinationText();
        if (Parent())
            Parent()->SetText(m_fleet->Name());
    }
    Ship* ship = GetUniverse().Object<Ship>(ship_id);
    if (CanJoin(ship, m_fleet)) {
        HumanClientApp::Orders().IssueOrder(new FleetTransferOrder(HumanClientApp::GetApp()->EmpireID(), ship->FleetID(), m_fleet->ID(), std::vector<int>(1, ship_id)));
    } else {
        throw GG::ListBox::DontAcceptDrop();
    }
}

void FleetDetailPanel::ShipRightClicked(int row_idx, GG::ListBox::Row* row, const GG::Pt& pt)
{
    ShipRow* ship_row = dynamic_cast<ShipRow*>(row);

    if (ship_row->m_ship->Owners().size() != 1 || HumanClientApp::GetApp()->EmpireID() != *ship_row->m_ship->Owners().begin())
        return;

    Ship* ship = GetUniverse().Object<Ship>(ship_row->ShipID());

    GG::MenuItem menu_contents;
    menu_contents.next_level.push_back(GG::MenuItem(UserString("RENAME"), 1, false, false));

    GG::PopupMenu popup(pt.x, pt.y, GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS), menu_contents, ClientUI::TEXT_COLOR);

    if (popup.Run()) {
        switch (popup.MenuID()) {
        case 1: { // rename ship
            std::string ship_name = m_ships_lb->GetRow(row_idx)[0]->WindowText();
            CUIEditWnd edit_wnd(350, UserString("ENTER_NEW_NAME"), ship_name);
            edit_wnd.Run();
            if (edit_wnd.Result() != "") {
                HumanClientApp::Orders().IssueOrder(new RenameOrder(HumanClientApp::GetApp()->EmpireID(), ship->ID(), edit_wnd.Result()));
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
        std::pair<int, int> eta = m_fleet->ETA();
        retval = boost::io::str(boost::format(UserString("FW_FLEET_MOVING_TO")) % (dest->Name().empty() ? UserString("UNKNOWN_SYSTEM") : dest->Name()) % eta.first);
        if (eta.first != eta.second)
            retval += "(" + boost::lexical_cast<std::string>(m_fleet->ETA().second) + ")";
    } else if (current) {
        retval = boost::io::str(boost::format(UserString("FW_FLEET_AT")) % current->Name());
    }
    return retval;
}

std::string FleetDetailPanel::ShipStatusText(int ship_id) const
{
    Ship* ship = GetUniverse().Object<Ship>(ship_id);
    return UserString("FW_SHIP_CLASS") + " \"" + (ship->Design() ? ship->Design()->name : UserString("FW_UNKNOWN_DESIGN_NAME")) + "\"";
}



////////////////////////////////////////////////
// FleetDetailWnd
////////////////////////////////////////////////
FleetDetailWnd::FleetDetailWnd(int x, int y, Fleet* fleet, bool read_only, Uint32 flags/* = CLICKABLE | DRAGABLE | ONTOP | CLOSABLE | MINIMIZABLE*/) : 
    CUIWnd("", x, y, 1, 1, flags),
    m_fleet_panel(0)
{
    TempUISoundDisabler sound_disabler;
    m_fleet_panel = new FleetDetailPanel(3, 3, fleet, read_only);
    Resize(m_fleet_panel->Size() + GG::Pt(3, 3) + (Size() - ClientSize()));
    AttachSignalChildren();
    SetText(TitleText());
    GG::Connect(m_fleet_panel->NeedNewFleetSignal, &FleetDetailWnd::PanelNeedsNewFleet, this);
    EnableChildClipping(false);
}

FleetDetailWnd::~FleetDetailWnd()
{
}

void FleetDetailWnd::CloseClicked()
{
    CUIWnd::CloseClicked();
    ClosingSignal(this);
    delete this;
}

void FleetDetailWnd::AttachSignalChildren()
{
    AttachChild(m_fleet_panel);
}

void FleetDetailWnd::DetachSignalChildren()
{
    DetachChild(m_fleet_panel);
}

std::string FleetDetailWnd::TitleText() const
{
    std::string retval = UserString("FW_NEW_FLEET_NAME");
    if (const Fleet* fleet = m_fleet_panel->GetFleet()) {
        retval = fleet->Name();
    }
    return retval;
}



////////////////////////////////////////////////
// FleetWnd
////////////////////////////////////////////////
// static(s)
std::set<FleetWnd*> FleetWnd::s_open_fleet_wnds;
GG::Pt FleetWnd::s_last_position;
FleetWnd::FleetWndItr FleetWnd::FleetWndBegin() {return s_open_fleet_wnds.begin();}
FleetWnd::FleetWndItr FleetWnd::FleetWndEnd  () {return s_open_fleet_wnds.end();}

FleetWnd::FleetWnd(int x, int y, std::vector<Fleet*> fleets, int selected_fleet, bool read_only,
                   Uint32 flags/* = CLICKABLE | DRAGABLE | ONTOP | CLOSABLE | MINIMIZABLE*/) : 
    MapWndPopup("", x, y, 1, 1, flags | GG::RESIZABLE),
    m_empire_id(-1),
    m_system_id(UniverseObject::INVALID_OBJECT_ID),
    m_read_only(read_only),
    m_moving_fleets(true),
    m_current_fleet(-1),
    m_fleets_lb(0),
    m_fleet_detail_panel(0)
{
    assert(0 <= selected_fleet && selected_fleet < static_cast<int>(fleets.size()));

    if (!fleets.empty())
    {
        m_system_id = fleets[0]->SystemID();
        for (unsigned int i = 1; i < fleets.size(); ++i) {
            if (m_system_id != fleets[i]->SystemID())
                m_system_id = UniverseObject::INVALID_OBJECT_ID;
        }
    }

    TempUISoundDisabler sound_disabler;

    m_fleets_lb = new CUIListBox(3, 3, FLEET_LISTBOX_WIDTH, FLEET_LISTBOX_HEIGHT);
    m_fleet_detail_panel = new FleetDetailPanel(3, m_fleets_lb->LowerRight().y + CONTROL_MARGIN, 0, read_only);

    m_fleets_lb->SetHiliteColor(GG::CLR_ZERO);

    Resize(m_fleet_detail_panel->LowerRight() + GG::Pt(3, 3) + (Size() - ClientSize()));
    GG::Pt window_posn = UpperLeft();
    if (GG::GUI::GetGUI()->AppWidth() < LowerRight().x)
        window_posn.x = GG::GUI::GetGUI()->AppWidth() - Width();
    if (GG::GUI::GetGUI()->AppHeight() < LowerRight().y)
        window_posn.y = GG::GUI::GetGUI()->AppHeight() - Height();
    MoveTo(window_posn);

    EnableChildClipping(false);

    Init(fleets, selected_fleet);
    m_universe_object_delete_connection = GG::Connect(GetUniverse().UniverseObjectDeleteSignal, &FleetWnd::UniverseObjectDelete, this);

    if (const System* system = fleets.back()->GetSystem())
        m_system_changed_connection = Connect(system->StateChangedSignal, &FleetWnd::SystemChangedSlot, this);

    s_open_fleet_wnds.insert(this);
}

FleetWnd::~FleetWnd()
{
    RemoveEmptyFleets();
    m_universe_object_delete_connection.disconnect();

    s_open_fleet_wnds.erase(this);
}

void FleetWnd::CloseClicked()
{
    s_last_position = UpperLeft();
    CUIWnd::CloseClicked();
    m_lb_delete_connection.disconnect();
    for (std::map<Fleet*, FleetDetailWnd*>::iterator it = m_open_fleet_windows.begin(); it != m_open_fleet_windows.end(); ++it) {
        delete it->second;
    }
    for (std::set<FleetDetailWnd*>::iterator it = m_new_fleet_windows.begin(); it != m_new_fleet_windows.end(); ++it) {
        delete *it;
    }
    for (int i = 0; i < m_fleets_lb->NumRows(); ++i) {
        if (Fleet* fleet = FleetInRow(i))
            NotShowingFleetSignal(fleet);
    }
    delete this;
}

void FleetWnd::Init(const std::vector<Fleet*>& fleets, int selected_fleet)
{
    if (m_read_only) {
        m_fleets_lb->SetStyle(GG::LB_NOSORT | GG::LB_BROWSEUPDATES | GG::LB_SINGLESEL);
    } else {
        m_fleets_lb->SetStyle(GG::LB_NOSORT | GG::LB_BROWSEUPDATES);
        m_fleets_lb->AllowDropType("Ship");
        m_fleets_lb->AllowDropType("Fleet");
    }

    if (!m_read_only) {
        m_fleets_lb->Insert(new FleetRow(0));
    }
    for (unsigned int i = 0; i < fleets.size(); ++i) {
        AddFleet(fleets[i]);
    }

    AttachSignalChildren();

    // everyone seems to hate the rollover effect, so it's disabled here, but all the other code for it is still in place
    //GG::Connect(m_fleets_lb->BrowsedSignal, &FleetWnd::FleetBrowsed, this);

    GG::Connect(m_fleet_detail_panel->PanelEmptySignal, &FleetWnd::FleetPanelEmpty, this);
    GG::Connect(m_fleets_lb->SelChangedSignal, &FleetWnd::FleetSelectionChanged, this);
    GG::Connect(m_fleets_lb->RightClickedSignal, &FleetWnd::FleetRightClicked, this);
    GG::Connect(m_fleets_lb->DoubleClickedSignal, &FleetWnd::FleetDoubleClicked, this);
    m_lb_delete_connection = GG::Connect(m_fleets_lb->ErasedSignal, &FleetWnd::FleetDeleted, this);
    GG::Connect(m_fleets_lb->DroppedSignal, &FleetWnd::ObjectDroppedIntoList, this);

    SetText(TitleText());

    if (GetOptionsDB().Get<bool>("UI.fleet-autoselect") && !fleets.empty()) {
        m_fleets_lb->SelectRow(selected_fleet);
        m_current_fleet = selected_fleet;
        m_fleet_detail_panel->SetFleet(FleetInRow(selected_fleet));
        m_fleets_lb->BringRowIntoView(selected_fleet);
    }
}

void FleetWnd::AttachSignalChildren()
{
    AttachChild(m_fleets_lb);
    AttachChild(m_fleet_detail_panel);
}

void FleetWnd::DetachSignalChildren()
{
    DetachChild(m_fleets_lb);
    DetachChild(m_fleet_detail_panel);
}

void FleetWnd::SystemClicked(int system_id)
{
    if (system_id != -1) {
        int empire_id = HumanClientApp::GetApp()->EmpireID();
        for (std::set<int>::const_iterator it = m_fleets_lb->Selections().begin(); it != m_fleets_lb->Selections().end(); ++it) {
            Fleet* fleet = FleetInRow(*it);
            if (fleet->OwnedBy(empire_id) && fleet->NumShips()) {
                // TODO: allow technologies or other factors to allow a fleet to turn around in mid-flight, without completing its current leg
                int start_system = fleet->SystemID() == UniverseObject::INVALID_OBJECT_ID ? fleet->NextSystemID() : fleet->SystemID();

                std::list<System*> route = GetUniverse().ShortestPath(start_system, system_id).first;
                // disallow "offroad" (direct non-starlane non-wormhole) travel
                if (route.size() == 2 && *route.begin() != *route.rbegin() &&
                    !(*route.begin())->HasStarlaneTo((*route.rbegin())->ID()) && !(*route.begin())->HasWormholeTo((*route.rbegin())->ID()) &&
                    !(*route.rbegin())->HasStarlaneTo((*route.begin())->ID()) && !(*route.rbegin())->HasWormholeTo((*route.begin())->ID())) {
                    return;
                }

                HumanClientApp::Orders().IssueOrder(new FleetMoveOrder(empire_id, fleet->ID(), start_system, system_id));
                if (fleet->SystemID() == UniverseObject::INVALID_OBJECT_ID)
                    ClientUI::GetClientUI()->GetMapWnd()->SetFleetMovement(fleet);
            }
        }
    } 
}

void FleetWnd::AddFleet(Fleet* fleet)
{
    m_fleets_lb->Insert(new FleetRow(fleet), m_fleets_lb->NumRows() - (m_read_only ? 0 : 1));
    ShowingFleetSignal(fleet, this);
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

bool FleetWnd::ContainsFleet(int fleet_id) const
{
    for (int i = 0; i < m_fleets_lb->NumRows(); i++) {
        Fleet* fleet = FleetInRow(i);
        if (fleet && fleet->ID() == fleet_id)
            return true;
    }
    return false;
}

void FleetWnd::FleetBrowsed(int row_idx)
{
    Fleet* fleet = 0 <= row_idx ? FleetInRow(row_idx) : (0 <= m_current_fleet ? FleetInRow(m_current_fleet) : 0);
    if (m_fleet_detail_panel->GetFleet() != fleet)
        m_fleet_detail_panel->SetFleet(fleet);
}

void FleetWnd::FleetSelectionChanged(const std::set<int>& rows)
{
    // disallow selection of the new fleet slot
    if (!m_read_only && rows.find(m_fleets_lb->NumRows() - 1) != rows.end()) {
        m_fleets_lb->DeselectRow(m_fleets_lb->NumRows() - 1);
    }

    m_current_fleet = m_fleets_lb->Caret();

    Fleet* fleet = 0 <= m_current_fleet ? FleetInRow(m_current_fleet) : 0;
    m_fleet_detail_panel->SetFleet(fleet);

    for (int i = 0; i < m_fleets_lb->NumRows(); ++i) {
        FleetDataPanel* fleet_panel = static_cast<FleetDataPanel*>(m_fleets_lb->GetRow(i)[0]);
        fleet_panel->Select(rows.find(i) != rows.end());
    }
}

void FleetWnd::FleetRightClicked(int row_idx, GG::ListBox::Row* row, const GG::Pt& pt)
{
    if (!m_read_only && row_idx == m_fleets_lb->NumRows() - 1)
        return;

    Fleet* fleet = FleetInRow(row_idx);
    if (fleet->Owners().size() != 1 || HumanClientApp::GetApp()->EmpireID() != *fleet->Owners().begin())
        return;

    GG::MenuItem menu_contents;
    menu_contents.next_level.push_back(GG::MenuItem(UserString("RENAME"), 1, false, false));
    GG::PopupMenu popup(pt.x, pt.y, GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS), menu_contents, ClientUI::TEXT_COLOR);

    if (popup.Run()) {
      switch (popup.MenuID()) {
      case 1: { // rename fleet
          std::string fleet_name = fleet->Name();
          CUIEditWnd edit_wnd(350, UserString("ENTER_NEW_NAME"), fleet_name);
          edit_wnd.Run();
          if (edit_wnd.Result() != "") {
              HumanClientApp::Orders().IssueOrder(new RenameOrder(HumanClientApp::GetApp()->EmpireID(), fleet->ID(), edit_wnd.Result()));
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
    if (!m_read_only && row_idx == m_fleets_lb->NumRows() - 1)
        return;

    Fleet* row_fleet = FleetInRow(row_idx);
    int num_open_windows = m_new_fleet_windows.size() + m_open_fleet_windows.size();
    GG::Pt window_posn(std::max(0, 25 + LowerRight().x + num_open_windows * 25), std::max(0, UpperLeft().y + num_open_windows * 25));
    if (!m_open_fleet_windows[row_fleet]) {
        FleetDetailWnd* fleet_wnd = m_open_fleet_windows[row_fleet] = new FleetDetailWnd(window_posn.x, window_posn.y, row_fleet, m_read_only);
        if (GG::GUI::GetGUI()->AppWidth() < fleet_wnd->LowerRight().x)
            window_posn.x = GG::GUI::GetGUI()->AppWidth() - fleet_wnd->Width();
        if (GG::GUI::GetGUI()->AppHeight() < fleet_wnd->LowerRight().y)
            window_posn.y = GG::GUI::GetGUI()->AppHeight() - fleet_wnd->Height();
        fleet_wnd->MoveTo(window_posn);
        GG::GUI::GetGUI()->Register(fleet_wnd);
        GG::Connect(fleet_wnd->ClosingSignal, &FleetWnd::FleetDetailWndClosing, this);
    }
}

void FleetWnd::FleetDeleted(int row_idx, GG::ListBox::Row* row)
{
    if (m_current_fleet == row_idx)
        m_current_fleet = -1;
    if (m_fleets_lb->Empty() || m_fleets_lb->NumRows() == 1 && !FleetInRow(0))
        CloseClicked();
}

void FleetWnd::ObjectDroppedIntoList(int row_idx, GG::ListBox::Row* row)
{
    if (m_read_only)
        throw GG::ListBox::DontAcceptDrop();

    // disallow drops that aren't over an item; "- 1" is used, because the list 
    // is now 1 larger, since "row" was just dropped into it
    if (row_idx < 0 || m_fleets_lb->NumRows() - 1 <= row_idx)
        throw GG::ListBox::DontAcceptDrop();

    FleetRow* fleet_row = dynamic_cast<FleetRow*>(row);
    ShipRow* ship_row = fleet_row ? 0 : dynamic_cast<ShipRow*>(row);

    // disallow drops of unknown Row types
    if (!ship_row && !fleet_row)
        throw GG::ListBox::DontAcceptDrop();

    if (row_idx == m_fleets_lb->NumRows() - 2) { // drop was into the new fleet row; "- 2" is used, because the list is now 1 larger, since the ShipRow was just dropped into it
        if (ship_row) {
            int ship_id = ship_row->ShipID();
            // special case: disallow drag-dropping a ship into the new fleet row when there is only 1, and only 1 fleet
            if (m_fleets_lb->NumRows() == 3) { // if there is exactly one fleet in the window ("3" is used because there is also the new fleet row and the just-dropped ship
                Ship* ship = GetUniverse().Object<Ship>(ship_id);
                assert(ship->GetFleet());
                if (ship->GetFleet()->NumShips() == 1)
                    throw GG::ListBox::DontAcceptDrop();
            }
            delete m_fleets_lb->Erase(row_idx); // remove the ship from the list, since it will be placed into a fleet
            CreateNewFleetFromDrop(ship_id);
        } else if (fleet_row) { // disallow drops of fleets onto the new fleet row
            throw GG::ListBox::DontAcceptDrop();
        }
    } else { // drop was onto some existing fleet
        if (ship_row) {
            Fleet* target_fleet = FleetInRow(row_idx + 1);

            if (!CanJoin(ship_row->m_ship, target_fleet))
                throw GG::ListBox::DontAcceptDrop();

            delete m_fleets_lb->Erase(row_idx); // remove the ship from the list, since it will be placed into a fleet
            HumanClientApp::Orders().IssueOrder(new FleetTransferOrder(HumanClientApp::GetApp()->EmpireID(), ship_row->m_ship->FleetID(), 
                                                                       target_fleet->ID(), std::vector<int>(1, ship_row->ShipID())));
        } else if (fleet_row) {
            Fleet* target_fleet = FleetInRow(row_idx + 1);

            // disallow drops across fleet windows; fleets must be at the same location
            if (target_fleet == fleet_row->m_fleet || target_fleet->X() != fleet_row->m_fleet->X() || target_fleet->Y() != fleet_row->m_fleet->Y())
                throw GG::ListBox::DontAcceptDrop();

            delete m_fleets_lb->Erase(row_idx); // remove the fleet from the list; we don't want this duplicate lying about
            std::vector<int> ships;
            ships.insert(ships.end(), fleet_row->m_fleet->begin(), fleet_row->m_fleet->end());
            HumanClientApp::Orders().IssueOrder(new FleetTransferOrder(HumanClientApp::GetApp()->EmpireID(), fleet_row->FleetID(), 
                                                                       target_fleet->ID(), ships));
        }
    }

    RemoveEmptyFleets();
}

void FleetWnd::FleetDetailWndClosing(FleetDetailWnd* wnd)
{
    Fleet* fleet = wnd->GetFleetDetailPanel().GetFleet();

    if (m_new_fleet_windows.find(wnd) != m_new_fleet_windows.end()) {
        m_new_fleet_windows.erase(wnd);
    } else {
        m_open_fleet_windows.erase(fleet);
    }

    if (fleet && !fleet->NumShips()) {
        DeleteFleet(fleet);
    }

    RemoveEmptyFleets();
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

void FleetWnd::DeleteFleet(Fleet* fleet)
{
    try {
        FleetInRow(m_current_fleet);
    } catch (const std::out_of_range& re) {
        m_current_fleet = -1; // if the fleet deleted was the currently selected one, we need to fix that situation
    }
    if (m_fleet_detail_panel->GetFleet() == fleet)
        m_fleet_detail_panel->SetFleet(0);

    std::map<Fleet*, FleetDetailWnd*>::iterator it = m_open_fleet_windows.find(fleet);
    if (it != m_open_fleet_windows.end()) {
        delete it->second;
        m_open_fleet_windows.erase(it);
    }
    NotShowingFleetSignal(fleet);
    for (int i = 0; i < m_fleets_lb->NumRows(); ++i) {
        if (FleetInRow(i) == fleet) {
            delete m_fleets_lb->Erase(i);
            break;
        }
    }
    HumanClientApp::Orders().IssueOrder(new DeleteFleetOrder(HumanClientApp::GetApp()->EmpireID(), fleet->ID()));
}

void FleetWnd::FleetPanelEmpty(Fleet* fleet)
{
    DeleteFleet(fleet);
}

Fleet* FleetWnd::CreateNewFleetFromDrop(int ship_id)
{
    Fleet* existing_fleet = FleetInRow(0);
    Ship *ship = GetUniverse().Object<Ship>(ship_id);

    if (!existing_fleet || !ship || existing_fleet->SystemID() != ship->GetFleet()->SystemID())
        throw GG::ListBox::DontAcceptDrop();

    System* system = existing_fleet->GetSystem();
    double existing_fleet_x = existing_fleet->X();
    double existing_fleet_y = existing_fleet->Y();

    int empire_id = HumanClientApp::GetApp()->EmpireID();
    int new_fleet_id = ClientApp::GetNewObjectID();

    if (new_fleet_id == UniverseObject::INVALID_OBJECT_ID) {
        ClientUI::MessageBox(UserString("SERVER_TIMEOUT"), true);
        throw GG::ListBox::DontAcceptDrop();
    }

    std::string fleet_name = UserString("FW_NEW_FLEET_NAME") + boost::lexical_cast<std::string>(new_fleet_id);

    Fleet* new_fleet = 0;
    if (system) {
        m_system_changed_connection.disconnect();
        HumanClientApp::Orders().IssueOrder(new NewFleetOrder(empire_id, fleet_name, new_fleet_id, system->ID(), ship_id));
        System::ObjectVec fleets = system->FindObjectsInOrbit(-1, StationaryFleetVisitor(empire_id));
        for (unsigned int i = 0; i < fleets.size(); ++i) {
            if (fleets[i]->Name() == fleet_name) {
                new_fleet = universe_object_cast<Fleet*>(fleets[i]);
                break;
            }
        }
        m_system_changed_connection = Connect(system->StateChangedSignal, &FleetWnd::SystemChangedSlot, this);
    } else {
        HumanClientApp::Orders().IssueOrder(new NewFleetOrder(empire_id, fleet_name, new_fleet_id, existing_fleet_x, existing_fleet_y, ship_id));
        std::vector<Fleet*> fleets = GetUniverse().FindObjects<Fleet>();
        for (unsigned int i = 0; i < fleets.size(); ++i) {
            if (fleets[i]->Name() == fleet_name && fleets[i]->X() == existing_fleet_x && fleets[i]->Y() == existing_fleet_y) {
                new_fleet = fleets[i];
                break;
            }
        }
    }

    AddFleet(new_fleet);

    return new_fleet;
}

void FleetWnd::RemoveEmptyFleets()
{
    // check for leftover empty fleets in fleet list
    for (int i = m_fleets_lb->NumRows() - 1; i >= 0; --i) {
        Fleet* current_fleet = 0;
        if ((current_fleet = FleetInRow(i)) && !current_fleet->NumShips()) {
            std::cout << "FleetWnd::RemoveEmptyFleets(): removing " << current_fleet->Name() << std::endl;
            DeleteFleet(current_fleet);
        }
    }
}

void FleetWnd::UniverseObjectDelete(const UniverseObject *obj)
{
    const Fleet *fleet;
    // only look for obj if FleetWnd contains fleets and obj is a fleet
    if ((m_open_fleet_windows.empty() && m_fleets_lb->NumRows() == (m_read_only ? 0 : 1))
        || !(fleet = universe_object_cast<const Fleet *>(obj)))
        return;

    if (m_fleet_detail_panel->GetFleet() == fleet)
        m_fleet_detail_panel->SetFleet(0);

    for (std::map<Fleet*, FleetDetailWnd*>::iterator it = m_open_fleet_windows.begin(); it != m_open_fleet_windows.end(); ++it) {
        if (it->first == fleet) {
            delete it->second;
            m_open_fleet_windows.erase(it);
            break;
        }
    }

    for (int i = 0; i < m_fleets_lb->NumRows(); ++i) {
        if (FleetInRow(i) == fleet) {
            NotShowingFleetSignal(FleetInRow(i));
            delete m_fleets_lb->Erase(i);
            break;
        }
    }
}

void FleetWnd::SystemChangedSlot()
{
    const System* system = FleetInRow(0)->GetSystem();
    std::vector<const Fleet*> system_fleet_vec = system->FindObjects<Fleet>();
    std::set<const Fleet*> system_fleet_set(system_fleet_vec.begin(), system_fleet_vec.end());
    for (int i = 0; i < m_fleets_lb->NumRows(); ++i) {
        system_fleet_set.erase(FleetInRow(i));
    }
    for (std::set<const Fleet*>::const_iterator it = system_fleet_set.begin(); it != system_fleet_set.end(); ++it) {
        AddFleet(const_cast<Fleet*>(*it));
    }
}

bool FleetWnd::FleetWndsOpen()
{
    return !s_open_fleet_wnds.empty();
}

bool FleetWnd::CloseAllFleetWnds()
{
    bool retval = 0 < s_open_fleet_wnds.size();

    while (0 < s_open_fleet_wnds.size()) {
        (*s_open_fleet_wnds.begin())->Close();
        s_open_fleet_wnds.erase(*s_open_fleet_wnds.begin());
    }

    return retval;
}

GG::Pt FleetWnd::LastPosition()
{
    return s_last_position;
}
