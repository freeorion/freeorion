#include "FleetWindow.h"
#include "../util/AppInterface.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "../universe/Fleet.h"
#include "GGMenu.h"
#include "GGTextControl.h"
#include "../client/human/HumanClientApp.h"
#include "../universe/Planet.h"
#include "../universe/Predicates.h"
#include "../universe/Ship.h"
#include "../universe/System.h"
#include "../network/Message.h"
#include "SidePanel.h"


namespace {
    const int CONTROL_MARGIN = 5; // gap to leave between controls in these window
    const std::string g_new_fleet_row_text = "[New fleet]";

    struct FleetRow : public GG::ListBox::Row
    {
    public:
        FleetRow(Fleet* fleet) : m_fleet(fleet) 
        {
            if (!fleet)
                throw std::invalid_argument("Attempted to contruct a FleetRow using a null fleet pointer.");

            push_back(fleet->Name(), ClientUI::FONT,  ClientUI::PTS,  ClientUI::TEXT_COLOR);
            data_type = "Fleet";
        }

        int FleetID() const {return m_fleet->ID();}

        Fleet* const m_fleet;
    };

    struct ShipRow : public GG::ListBox::Row
    {
        ShipRow(Ship* ship) : m_ship(ship) 
        {
            if (!ship)
                throw std::invalid_argument("Attempted to contruct a ShipRow using a null ship pointer.");

            push_back(ship->Name().empty() ? ship->Design().name : ship->Name(), ClientUI::FONT,  ClientUI::PTS,  ClientUI::TEXT_COLOR);
            data_type = "Ship";
        }

        int ShipID() const {return m_ship->ID();}

        Ship* const m_ship;
    };

    bool CanJoin(const Ship* ship, const Fleet* fleet)
    {
        const Fleet* home_fleet = ship->GetFleet();
        return home_fleet->ID() != fleet->ID() && (home_fleet->X() == fleet->X() && home_fleet->Y() == fleet->Y());
    }
}

////////////////////////////////////////////////
// FleetDetailPanel
////////////////////////////////////////////////
namespace {
    const int NEW_FLEET_BUTTON_WIDTH = 75;
    const int FLEET_LISTBOX_WIDTH =  250;
    const int FLEET_LISTBOX_HEIGHT = 150;
}

FleetDetailPanel::FleetDetailPanel(int x, int y, Fleet* fleet, bool read_only, Uint32 flags/* = 0*/) : 
    Wnd(x, y, 1, 1, flags),
    m_fleet(0),
    m_read_only(read_only),
    m_destination_text(0),
    m_ships_lb(0),
    m_ship_status_text(0)
{
    m_destination_text = new GG::TextControl(0, 0, FLEET_LISTBOX_WIDTH, ClientUI::PTS + 4, "", ClientUI::FONT, ClientUI::PTS, GG::TF_LEFT, ClientUI::TEXT_COLOR);
    m_ships_lb = new CUIListBox(0, m_destination_text->LowerRight().y + CONTROL_MARGIN, FLEET_LISTBOX_WIDTH, FLEET_LISTBOX_HEIGHT);
    m_ship_status_text = new GG::TextControl(0, m_ships_lb->LowerRight().y + CONTROL_MARGIN, m_ships_lb->Width(), ClientUI::PTS + 4, 
                                             "", ClientUI::FONT, ClientUI::PTS, GG::TF_LEFT, ClientUI::TEXT_COLOR);
    Resize(m_ship_status_text->LowerRight());

    SetFleet(fleet);
    Init();

    m_universe_object_delete_connection = GG::Connect(GetUniverse().UniverseObjectDeleteSignal(), &FleetDetailPanel::UniverseObjectDelete, this);
}

FleetDetailPanel::FleetDetailPanel(const GG::XMLElement& elem) : 
    Wnd(elem.Child("GG::ListBox")),
    m_read_only(boost::lexical_cast<bool>(elem.Child("m_read_only").Attribute("value")))
{
    if (elem.Tag() != "FleetDetailPanel")
        throw std::invalid_argument("Attempted to construct a FleetDetailPanel from an XMLElement that had a tag other than \"FleetDetailPanel\"");

    const GG::XMLElement* curr_elem = &elem.Child("m_fleet");
    SetFleet(dynamic_cast<Fleet*>(GetUniverse().Object(boost::lexical_cast<int>(curr_elem->Attribute("value")))));

    curr_elem = &elem.Child("m_destination_text");
    m_destination_text = new GG::TextControl(*curr_elem);

    curr_elem = &elem.Child("m_ships_lb");
    m_ships_lb = new CUIListBox(*curr_elem);

    curr_elem = &elem.Child("m_ship_status_text");
    m_ship_status_text = new GG::TextControl(*curr_elem);

    Init();

    m_universe_object_delete_connection = GG::Connect(GetUniverse().UniverseObjectDeleteSignal(), &FleetDetailPanel::UniverseObjectDelete, this);
}

FleetDetailPanel::~FleetDetailPanel()
{
  m_universe_object_delete_connection.disconnect();
}

int FleetDetailPanel::GetShipIDOfListRow(int row_idx) const
{
    return dynamic_cast<ShipRow&>(m_ships_lb->GetRow(row_idx)).ShipID();
}

GG::XMLElement FleetDetailPanel::XMLEncode() const
{
    GG::XMLElement retval("FleetDetailPanel");
    const_cast<FleetDetailPanel*>(this)->DetachSignalChildren();
    retval.AppendChild(Wnd::XMLEncode());
    const_cast<FleetDetailPanel*>(this)->AttachSignalChildren();

    GG::XMLElement temp("m_fleet");
    temp.SetAttribute("value", boost::lexical_cast<std::string>(m_fleet ? m_fleet->ID() : UniverseObject::INVALID_OBJECT_ID));
    retval.AppendChild(temp);

    temp = GG::XMLElement("m_read_only");
    temp.SetAttribute("value", boost::lexical_cast<std::string>(m_read_only));
    retval.AppendChild(temp);

    temp = GG::XMLElement("m_destination_text");
    temp.AppendChild(m_destination_text->XMLEncode());

    temp = GG::XMLElement("m_ships_lb");
    temp.AppendChild(m_ships_lb->XMLEncode());

    temp = GG::XMLElement("m_ship_status_text");
    temp.AppendChild(m_ship_status_text->XMLEncode());

    return retval;
}

void FleetDetailPanel::SetFleet(Fleet* fleet)
{
    Fleet* old_fleet = m_fleet;
    if (fleet != old_fleet)
        m_fleet_connection.disconnect();
    *m_destination_text << "";
    m_ships_lb->Clear();
    if (m_fleet = fleet) {
        Universe& universe = GetUniverse();
        if (m_fleet->NumShips()) {
            for (Fleet::const_iterator it = m_fleet->begin(); it != m_fleet->end(); ++it) {
                m_ships_lb->Insert(new ShipRow(dynamic_cast<Ship*>(universe.Object(*it))));
            }
        } else {
            m_panel_empty_sig(m_fleet);
        }
        *m_destination_text << DestinationText();
        if (fleet != old_fleet)
            m_fleet_connection = GG::Connect(m_fleet->StateChangedSignal(), &FleetDetailPanel::Refresh, this);
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
        m_ships_lb->SetStyle(GG::LB_QUICKSEL | GG::LB_DRAGDROP | GG::LB_BROWSEUPDATES);
        m_ships_lb->AllowDropType("Ship");
    }

    AttachSignalChildren();

    GG::Connect(m_ships_lb->BrowsedSignal(), &FleetDetailPanel::ShipBrowsed, this);
    GG::Connect(m_ships_lb->DroppedSignal(), &FleetDetailPanel::ShipDroppedIntoList, this);
    GG::Connect(m_ships_lb->RightClickedSignal(), &FleetDetailPanel::ShipRightClicked, this);
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

void FleetDetailPanel::ShipBrowsed(int row_idx)
{
    if (0 <= row_idx && row_idx < m_ships_lb->NumRows()) {
        *m_ship_status_text << ShipStatusText(GetShipIDOfListRow(row_idx));
    } else {
        *m_ship_status_text << "";
    }
}

void FleetDetailPanel::ShipDroppedIntoList(int row_idx, const GG::ListBox::Row* row)
{
    const ShipRow* ship_row = dynamic_cast<const ShipRow*>(row);
    int ship_id = ship_row->ShipID();
    if (!m_fleet) {
        // creating a new fleet can fail but will be handled by listbox exception
        m_fleet = m_need_new_fleet_sig(ship_id);

        m_fleet_connection.disconnect();
        m_fleet_connection = GG::Connect(m_fleet->StateChangedSignal(), &FleetDetailPanel::Refresh, this);
        
        *m_destination_text << DestinationText();
        if (Parent())
            Parent()->SetText(m_fleet->Name());
    }
    Ship* ship = dynamic_cast<Ship*>(GetUniverse().Object(ship_id));
    if (CanJoin(ship, m_fleet)) {
        HumanClientApp::Orders().IssueOrder(new FleetTransferOrder(HumanClientApp::GetApp()->PlayerID(), ship->FleetID(), m_fleet->ID(), std::vector<int>(1, ship_id)));
    } else {
        throw GG::ListBox::DontAcceptDropException();
    }
}

void FleetDetailPanel::ShipRightClicked(int row_idx, const GG::ListBox::Row* row, const GG::Pt& pt)
{
    const ShipRow* ship_row = dynamic_cast<const ShipRow*>(row);

    if (ship_row->m_ship->Owners().size() != 1 || HumanClientApp::GetApp()->PlayerID() != *ship_row->m_ship->Owners().begin())
        return;

    Ship* ship = dynamic_cast<Ship*>(GetUniverse().Object(ship_row->ShipID()));

    GG::MenuItem menu_contents;
    menu_contents.next_level.push_back(GG::MenuItem("Rename", 1, false, false));

    if (ship->Design().colonize && m_fleet->SystemID() != UniverseObject::INVALID_OBJECT_ID)
        menu_contents.next_level.push_back(GG::MenuItem("Colonize Planet", 2, false, false));

    GG::PopupMenu popup(pt.x, pt.y, GG::App::GetApp()->GetFont(ClientUI::FONT, ClientUI::PTS), menu_contents, ClientUI::TEXT_COLOR);

    if (popup.Run()) {
        switch (popup.MenuID()) {
        case 1: { // rename ship
            std::string ship_name = m_ships_lb->GetRow(row_idx)[row_idx]->WindowText();
            CUIEditWnd edit_wnd(350, "Enter new name", ship_name);
            edit_wnd.Run();
            if (edit_wnd.Result() != "") {
                HumanClientApp::Orders().IssueOrder(new RenameOrder(HumanClientApp::GetApp()->PlayerID(), ship->ID(), edit_wnd.Result()));
                m_ships_lb->GetRow(row_idx)[row_idx]->SetText(edit_wnd.Result());
            }
            break;}
        case 2: { // colonize planet
            int planet_id = HumanClientApp::GetUI()->SelectPlanet(m_fleet->SystemID());
            if ( planet_id != UniverseObject::INVALID_OBJECT_ID )
            {
                // check some conditions. If this is ever the final UI for colonization, once should display a cursor to reflect
                // the fact colonization cannot happen. For now, clicking on an invalid system will NOT end the UI
                const Planet *planet = dynamic_cast<const Planet*>(GetUniverse().Object( planet_id ));
                if ( planet->Owners().size() != 0 )
                    return;

                HumanClientApp::Orders().IssueOrder(new FleetColonizeOrder( HumanClientApp::GetApp()->PlayerID(), ship->GetFleet()->ID(), planet_id ));

                HumanClientApp::GetUI()->GetMapWnd()->GetSidePanel()->SetSystem(planet->SystemID());
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
        retval = "Moving to " + dest->Name() + ", ETA " + boost::lexical_cast<std::string>(eta.first);
        if (eta.first != eta.second)
            retval += "(" + boost::lexical_cast<std::string>(m_fleet->ETA().second) + ")";
    } else if (current) {
        retval = "At " + current->Name() + " System";
    }
    return retval;
}

std::string FleetDetailPanel::ShipStatusText(int ship_id) const
{
    Ship* ship = dynamic_cast<Ship*>(GetUniverse().Object(ship_id));
    std::string retval;

    // if we do not own the fleet or it's not a colony ship
    if ( m_read_only || !ship->Design().colonize )
    {
        retval = "Ship Class \"";
        retval += ship->Design().name;
        retval += "\"";
    }
    else
    {
        // the colonization UI is probably temporary, do not put string in translation table until UI is solidified
        retval = "Right-Click to Colonize";
    }
    return retval;
}



////////////////////////////////////////////////
// FleetDetailWnd
////////////////////////////////////////////////
FleetDetailWnd::FleetDetailWnd(int x, int y, Fleet* fleet, bool read_only, Uint32 flags/* = CLICKABLE | DRAGABLE | ONTOP | CLOSABLE | MINIMIZABLE*/) : 
    CUI_Wnd("", x, y, 1, 1, flags),
    m_fleet_panel(0)
{
    m_fleet_panel = new FleetDetailPanel(LeftBorder() + 3, TopBorder() + 3, fleet, read_only);
    Resize(m_fleet_panel->Size() + GG::Pt(LeftBorder() + RightBorder() + 6, TopBorder() + BottomBorder() + 6));
    AttachSignalChildren();
    SetText(TitleText());
    GG::Connect(m_fleet_panel->NeedNewFleetSignal(), &FleetDetailWnd::PanelNeedsNewFleet, this);
}

FleetDetailWnd::FleetDetailWnd(const GG::XMLElement& elem) : 
    CUI_Wnd(elem.Child("CUI_Wnd"))
{
    // TODO : implement as needed
}

FleetDetailWnd::~FleetDetailWnd()
{
}

void FleetDetailWnd::CloseClicked()
{
    CUI_Wnd::CloseClicked();
    m_closing_sig(this);
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
    std::string retval = "New fleet";
    if (const Fleet* fleet = m_fleet_panel->GetFleet()) {
        retval = fleet->Name();
    }
    return retval;
}



////////////////////////////////////////////////
// FleetWnd
////////////////////////////////////////////////
FleetWnd::FleetWnd(int x, int y, std::vector<Fleet*> fleets, bool read_only, Uint32 flags/* = CLICKABLE | DRAGABLE | ONTOP | CLOSABLE | MINIMIZABLE*/) : 
    MapWndPopup("", x, y, 1, 1, flags),
    m_empire_id(-1),
    m_read_only(read_only),
    m_moving_fleets(true),
    m_current_fleet(-1),
    m_fleets_lb(0),
    m_fleet_detail_panel(0),
    m_new_fleet_button(0)
{
    m_fleets_lb = new CUIListBox(LeftBorder() + 3, TopBorder() + 3, FLEET_LISTBOX_WIDTH, FLEET_LISTBOX_HEIGHT);
    m_fleet_detail_panel = new FleetDetailPanel(LeftBorder() + 3, m_fleets_lb->LowerRight().y + CONTROL_MARGIN, 0, read_only );
    m_new_fleet_button = new CUIButton(FLEET_LISTBOX_WIDTH + 6 + LeftBorder() - 5 - NEW_FLEET_BUTTON_WIDTH, 
                                       m_fleet_detail_panel->LowerRight().y + CONTROL_MARGIN, NEW_FLEET_BUTTON_WIDTH, "New fleet");

    Resize(m_new_fleet_button->LowerRight() + GG::Pt(RightBorder() + 5, BottomBorder() + 5));
    GG::Pt window_posn = UpperLeft();
    if (GG::App::GetApp()->AppWidth() < LowerRight().x)
        window_posn.x = GG::App::GetApp()->AppWidth() - Width();
    if (GG::App::GetApp()->AppHeight() < LowerRight().y)
        window_posn.y = GG::App::GetApp()->AppHeight() - Height();
    MoveTo(window_posn);

    if (read_only)
        m_new_fleet_button->Disable();

    Init(fleets);
    m_universe_object_delete_connection = GG::Connect(GetUniverse().UniverseObjectDeleteSignal(), &FleetWnd::UniverseObjectDelete, this);
}

FleetWnd::FleetWnd( const GG::XMLElement& elem) : 
    MapWndPopup(elem.Child("CUI_Wnd")),
    m_empire_id(-1),
    m_read_only(true)
{
    // TODO : implement as needed (note that the initializations above must be changed as well)
    m_universe_object_delete_connection = GG::Connect(GetUniverse().UniverseObjectDeleteSignal(), &FleetWnd::UniverseObjectDelete, this);
}

FleetWnd::~FleetWnd()
{
    RemoveEmptyFleets();
    m_universe_object_delete_connection.disconnect();
}

void FleetWnd::CloseClicked()
{
    CUI_Wnd::CloseClicked();
    for (std::map<Fleet*, FleetDetailWnd*>::iterator it = m_open_fleet_windows.begin(); it != m_open_fleet_windows.end(); ++it) {
        delete it->second;
    }
    for (std::set<FleetDetailWnd*>::iterator it = m_new_fleet_windows.begin(); it != m_new_fleet_windows.end(); ++it) {
        delete *it;
    }
    for (int i = 0; i < m_fleets_lb->NumRows(); ++i) {
        if (Fleet* fleet = FleetInRow(i))
            m_not_showing_fleet_sig(fleet);
    }
    delete this;
}

void FleetWnd::Init(const std::vector<Fleet*>& fleets)
{
    if (m_read_only) {
        m_fleets_lb->SetStyle(GG::LB_NOSORT | GG::LB_BROWSEUPDATES | GG::LB_SINGLESEL);
    } else {
        m_fleets_lb->SetStyle(GG::LB_NOSORT | GG::LB_BROWSEUPDATES | GG::LB_DRAGDROP);
        m_fleets_lb->AllowDropType("Ship");
        m_fleets_lb->AllowDropType("Fleet");
    }

    for (unsigned int i = 0; i < fleets.size(); ++i) {
        m_fleets_lb->Insert(new FleetRow(fleets[i]));
        m_showing_fleet_sig(fleets[i]);
    }
    if (!m_read_only) {
        GG::ListBox::Row* row = new GG::ListBox::Row();
        row->push_back(g_new_fleet_row_text, ClientUI::FONT,  ClientUI::PTS,  ClientUI::TEXT_COLOR);
        m_fleets_lb->Insert(row);
    }

    AttachSignalChildren();

    GG::Connect(m_fleets_lb->BrowsedSignal(), &FleetWnd::FleetBrowsed, this);
    GG::Connect(m_fleets_lb->SelChangedSignal(), &FleetWnd::FleetSelectionChanged, this);
    GG::Connect(m_fleets_lb->RightClickedSignal(), &FleetWnd::FleetRightClicked, this);
    GG::Connect(m_fleets_lb->DoubleClickedSignal(), &FleetWnd::FleetDoubleClicked, this);
    GG::Connect(m_fleets_lb->DeletedSignal(), &FleetWnd::FleetDeleted, this);
    GG::Connect(m_fleets_lb->DroppedSignal(), &FleetWnd::ObjectDroppedIntoList, this);
    GG::Connect(m_new_fleet_button->ClickedSignal(), &FleetWnd::NewFleetButtonClicked, this);

    SetText(TitleText());

    if (fleets.size() == 1) {
        m_fleets_lb->SelectRow(0);
        m_current_fleet = 0;
        m_fleet_detail_panel->SetFleet(FleetInRow(0));
    }
}

void FleetWnd::AttachSignalChildren()
{
    AttachChild(m_fleets_lb);
    AttachChild(m_fleet_detail_panel);
    AttachChild(m_new_fleet_button);
}

void FleetWnd::DetachSignalChildren()
{
    DetachChild(m_fleets_lb);
    DetachChild(m_fleet_detail_panel);
    DetachChild(m_new_fleet_button);
}

void FleetWnd::SystemClicked(int system_id)
{
    if (!m_read_only && system_id != -1) {
        int empire_id = HumanClientApp::GetApp()->PlayerID();
        for (std::set<int>::const_iterator it = m_fleets_lb->Selections().begin(); it != m_fleets_lb->Selections().end(); ++it) {
            Fleet* fleet = FleetInRow(*it);
            if (fleet->NumShips()) {
                // TODO: allow technologies or other factors to allow a fleet to turn around in mid-flight, without completing its current leg
                int start_system = fleet->SystemID() == UniverseObject::INVALID_OBJECT_ID ? fleet->NextSystemID() : fleet->SystemID();
                HumanClientApp::Orders().IssueOrder(new FleetMoveOrder(empire_id, fleet->ID(), start_system, system_id));
                if (fleet->SystemID() == UniverseObject::INVALID_OBJECT_ID)
                    ClientUI::GetClientUI()->GetMapWnd()->SetFleetMovement(fleet);
            }
        }
    }
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
        m_fleets_lb->ClearRow(m_fleets_lb->NumRows() - 1);
        return;
    }
    if (!m_read_only && m_fleets_lb->Caret() != m_fleets_lb->NumRows() - 1)
        m_current_fleet = m_fleets_lb->Caret();
    Fleet* fleet = 0 <= m_current_fleet ? FleetInRow(m_current_fleet) : 0;
    m_fleet_detail_panel->SetFleet(fleet);
}

void FleetWnd::FleetRightClicked(int row_idx, const GG::ListBox::Row* row, const GG::Pt& pt)
{
    if (!m_read_only && row_idx == m_fleets_lb->NumRows() - 1)
        return;

    Fleet* fleet = FleetInRow(row_idx);
    if (fleet->Owners().size() != 1 || HumanClientApp::GetApp()->PlayerID() != *fleet->Owners().begin())
        return;

    GG::MenuItem menu_contents;
    menu_contents.next_level.push_back(GG::MenuItem("Rename", 1, false, false));
    GG::PopupMenu popup(pt.x, pt.y, GG::App::GetApp()->GetFont(ClientUI::FONT, ClientUI::PTS), menu_contents, ClientUI::TEXT_COLOR);

    if (popup.Run()) {
      switch (popup.MenuID()) {
      case 1: { // rename fleet
          std::string fleet_name = fleet->Name();
          CUIEditWnd edit_wnd(350, "Enter new name", fleet_name);
          edit_wnd.Run();
          if (edit_wnd.Result() != "") {
              HumanClientApp::Orders().IssueOrder(new RenameOrder(HumanClientApp::GetApp()->PlayerID(), fleet->ID(), edit_wnd.Result()));
              m_fleets_lb->GetRow(row_idx)[0]->SetText(edit_wnd.Result());
          }
          break;
      }
      default:
          break;
      }
    }
}

void FleetWnd::FleetDoubleClicked(int row_idx, const GG::ListBox::Row* row)
{
    if (!m_read_only && row_idx == m_fleets_lb->NumRows() - 1)
        return;

    Fleet* row_fleet = FleetInRow(row_idx);
    int num_open_windows = m_new_fleet_windows.size() + m_open_fleet_windows.size();
    GG::Pt window_posn(std::max(0, 25 + LowerRight().x + num_open_windows * 25), std::max(0, UpperLeft().y + num_open_windows * 25));
    if (!m_open_fleet_windows[row_fleet]) {
        FleetDetailWnd* fleet_wnd = m_open_fleet_windows[row_fleet] = new FleetDetailWnd(window_posn.x, window_posn.y, row_fleet, m_read_only);
        if (GG::App::GetApp()->AppWidth() < fleet_wnd->LowerRight().x)
            window_posn.x = GG::App::GetApp()->AppWidth() - fleet_wnd->Width();
        if (GG::App::GetApp()->AppHeight() < fleet_wnd->LowerRight().y)
            window_posn.y = GG::App::GetApp()->AppHeight() - fleet_wnd->Height();
        fleet_wnd->MoveTo(window_posn);
        GG::App::GetApp()->Register(fleet_wnd);
        GG::Connect(fleet_wnd->ClosingSignal(), &FleetWnd::FleetDetailWndClosing, this);
    }
}

void FleetWnd::FleetDeleted(int row_idx)
{
    if (m_current_fleet == row_idx)
        m_current_fleet = -1;
}

void FleetWnd::ObjectDroppedIntoList(int row_idx, const GG::ListBox::Row* row)
{
    if (m_read_only)
        throw GG::ListBox::DontAcceptDropException();

    // disallow drops that aren't over an item; "- 1" is used, because the list 
    // is now 1 larger, since "row" was just dropped into it
    if (row_idx < 0 || m_fleets_lb->NumRows() - 1 <= row_idx)
        throw GG::ListBox::DontAcceptDropException();

    const FleetRow* fleet_row = dynamic_cast<const FleetRow*>(row);
    const ShipRow* ship_row = fleet_row ? 0 : dynamic_cast<const ShipRow*>(row);

    // disallow drops of unknown Row types
    if (!ship_row && !fleet_row)
        throw GG::ListBox::DontAcceptDropException();

    if (row_idx == m_fleets_lb->NumRows() - 2) { // drop was into the new fleet row; "- 2" is used, because the list is now 1 larger, since the ShipRow was just dropped into it
        if (ship_row) {
            Fleet* target_fleet = CreateNewFleetFromDrop(ship_row->ShipID());
            HumanClientApp::Orders().IssueOrder(new FleetTransferOrder(HumanClientApp::GetApp()->PlayerID(), ship_row->m_ship->FleetID(), 
                                                                       target_fleet->ID(), std::vector<int>(1, ship_row->ShipID())));
            m_fleets_lb->Delete(row_idx); // remove the ship from the list, since it was just placed into a fleet
        } else if (fleet_row) { // disallow drops of fleets onto the new fleet row
            throw GG::ListBox::DontAcceptDropException();
        }
    } else { // drop was onto some existing fleet
        if (ship_row) {
            Fleet* target_fleet = FleetInRow(row_idx + 1);

            if (!CanJoin(ship_row->m_ship, target_fleet))
                throw GG::ListBox::DontAcceptDropException();

            m_fleets_lb->Delete(row_idx); // remove the ship from the list, since it will be placed into a fleet
            HumanClientApp::Orders().IssueOrder(new FleetTransferOrder(HumanClientApp::GetApp()->PlayerID(), ship_row->m_ship->FleetID(), 
                                                                       target_fleet->ID(), std::vector<int>(1, ship_row->ShipID())));
        } else if (fleet_row) {
            Fleet* target_fleet = FleetInRow(row_idx + 1);

            // disallow drops across fleet windows; fleets must be at the same location
            if (target_fleet == fleet_row->m_fleet || target_fleet->X() != fleet_row->m_fleet->X() || target_fleet->Y() != fleet_row->m_fleet->Y())
                throw GG::ListBox::DontAcceptDropException();

            m_fleets_lb->Delete(row_idx); // remove the fleet from the list; we don't want this duplicate lying about
            std::vector<int> ships;
            ships.insert(ships.end(), fleet_row->m_fleet->begin(), fleet_row->m_fleet->end());
            HumanClientApp::Orders().IssueOrder(new FleetTransferOrder(HumanClientApp::GetApp()->PlayerID(), fleet_row->FleetID(), 
                                                                       target_fleet->ID(), ships));
        }
    }

    RemoveEmptyFleets();
}

void FleetWnd::NewFleetButtonClicked()
{
    int num_open_windows = m_new_fleet_windows.size() + m_open_fleet_windows.size();
    GG::Pt window_posn(std::max(0, 25 + LowerRight().x + num_open_windows * 25), std::max(0, UpperLeft().y + num_open_windows * 25));
    FleetDetailWnd* fdw = new FleetDetailWnd(window_posn.x, window_posn.y, 0, m_read_only);
    if (GG::App::GetApp()->AppWidth() < fdw->LowerRight().x)
        window_posn.x = GG::App::GetApp()->AppWidth() - fdw->Width();
    if (GG::App::GetApp()->AppHeight() < fdw->LowerRight().y)
        window_posn.y = GG::App::GetApp()->AppHeight() - fdw->Height();
    fdw->MoveTo(window_posn);
    m_new_fleet_windows.insert(fdw);
    GG::App::GetApp()->Register(fdw);
    GG::Connect(fdw->NeedNewFleetSignal(), &FleetWnd::NewFleetWndReceivedShip, this);
    GG::Connect(fdw->ClosingSignal(), &FleetWnd::FleetDetailWndClosing, this);
}
 
Fleet* FleetWnd::NewFleetWndReceivedShip(FleetDetailWnd* fleet_wnd, int ship_id)
{
    Fleet* new_fleet = CreateNewFleetFromDrop(ship_id);
    m_showing_fleet_sig(new_fleet);
    m_new_fleet_windows.erase(fleet_wnd);
    m_open_fleet_windows[new_fleet] = fleet_wnd;
    return new_fleet;
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
    return "Empire " + boost::lexical_cast<std::string>(*existing_fleet->Owners().begin()) + " Fleets";
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

    HumanClientApp::Orders().IssueOrder(new DeleteFleetOrder(HumanClientApp::GetApp()->PlayerID(), fleet->ID()));
    std::map<Fleet*, FleetDetailWnd*>::iterator it = m_open_fleet_windows.find(fleet);
    if (it != m_open_fleet_windows.end()) {
        delete it->second;
        m_open_fleet_windows.erase(it);
    }
    m_not_showing_fleet_sig(fleet);
    for (int i = 0; i < m_fleets_lb->NumRows(); ++i) {
        if (FleetInRow(i) == fleet) {
            m_fleets_lb->Delete(i);
            break;
        }
    }
}

Fleet* FleetWnd::CreateNewFleetFromDrop(int ship_id)
{
    Fleet* existing_fleet = FleetInRow(0);
    Ship *ship = dynamic_cast<Ship*>(GetUniverse().Object(ship_id));

    if (!existing_fleet || !ship || existing_fleet->SystemID() != ship->GetFleet()->SystemID())
        throw GG::ListBox::DontAcceptDropException();

    int empire_id = HumanClientApp::GetApp()->PlayerID();
    int new_fleet_id = ClientApp::GetNewObjectID();

    if (new_fleet_id == UniverseObject::INVALID_OBJECT_ID)
    {
        ClientUI::MessageBox(ClientUI::String("SERVER_TIMEOUT"));
        throw GG::ListBox::DontAcceptDropException();
    }

    std::string fleet_name = "New fleet " + boost::lexical_cast<std::string>(new_fleet_id);

    Fleet* new_fleet = 0;
    if (existing_fleet->SystemID() != UniverseObject::INVALID_OBJECT_ID) {
        HumanClientApp::Orders().IssueOrder(new NewFleetOrder(empire_id, fleet_name, new_fleet_id, existing_fleet->SystemID()));
        System::ObjectVec fleets = existing_fleet->GetSystem()->FindObjectsInOrbit(-1, IsStationaryFleetFunctor(empire_id));
        for (unsigned int i = 0; i < fleets.size(); ++i) {
            if (fleets[i]->Name() == fleet_name) {
                new_fleet = dynamic_cast<Fleet*>(fleets[i]);
                break;
            }
        }
    } else {
        HumanClientApp::Orders().IssueOrder(new NewFleetOrder(empire_id, fleet_name, new_fleet_id, existing_fleet->X(), existing_fleet->Y()));
        std::vector<Fleet*> fleets = GetUniverse().FindObjects<Fleet>();
        for (unsigned int i = 0; i < fleets.size(); ++i) {
            if (fleets[i]->Name() == fleet_name && fleets[i]->X() == existing_fleet->X() && fleets[i]->Y() == existing_fleet->Y()) {
                new_fleet = fleets[i];
                break;
            }
        }
    }

    m_fleets_lb->Insert(new FleetRow(new_fleet), m_read_only ? m_fleets_lb->NumRows() : m_fleets_lb->NumRows() - 1);

    return new_fleet;
}

void FleetWnd::RemoveEmptyFleets()
{
    // check for leftover empty fleets in fleet list
    for (int i = m_fleets_lb->NumRows() - 1; i >= 0; --i) {
        Fleet* current_fleet = 0;
        if ((current_fleet = FleetInRow(i)) && !current_fleet->NumShips()) {
            DeleteFleet(current_fleet);
        }
    }
}

void FleetWnd::UniverseObjectDelete(const UniverseObject *obj)
{
    const Fleet *fleet;
    // only look for obj if FleetWnd contains fleets and obj is a fleet
    if ((m_open_fleet_windows.empty() && m_fleets_lb->NumRows() == (m_read_only ? 0 : 1))
        || !(fleet = dynamic_cast<const Fleet *>(obj)))
        return;

    if (m_fleet_detail_panel->GetFleet() == fleet)
        m_fleet_detail_panel->SetFleet(0);

    for(std::map<Fleet*, FleetDetailWnd*>::iterator it = m_open_fleet_windows.begin(); it != m_open_fleet_windows.end(); ++it) 
        if (it->first == fleet)
        {
            delete it->second;
            m_open_fleet_windows.erase(it);
            break;
        }

        for (int i = 0; i < m_fleets_lb->NumRows(); ++i) {
            if (FleetInRow(i) == fleet) {
                m_not_showing_fleet_sig(FleetInRow(i));
                m_fleets_lb->Delete(i);
                break;
            }
        }
}
