#include "FleetWindow.h"

#include "../util/AppInterface.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "../universe/Fleet.h"
#include "GGMenu.h"
#include "GGTextControl.h"
#include "../client/human/HumanClientApp.h"
#include "../universe/Predicates.h"
#include "../universe/Ship.h"
#include "../universe/System.h"
#include "../network/Message.h"


namespace {
const int CONTROL_MARGIN = 5; // gap to leave between controls in these windows

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

        push_back(ship->Design().name, ClientUI::FONT,  ClientUI::PTS,  ClientUI::TEXT_COLOR);
        data_type = "Ship";
    }

    int ShipID() const {return m_ship->ID();}

    Ship* const m_ship;
};
}


////////////////////////////////////////////////
// FleetDetailPanel
////////////////////////////////////////////////
namespace {
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
}

FleetDetailPanel::~FleetDetailPanel()
{
    if (m_fleet && !m_fleet->NumShips())
        m_closing_empty_fleet_sig(m_fleet);
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
    if (m_fleet && !m_fleet->NumShips())
        m_closing_empty_fleet_sig(m_fleet);
    m_fleet_connection.disconnect();
    *m_destination_text << "";
    m_ships_lb->Clear();
    if (m_fleet = fleet) {
        Universe& universe = GetUniverse();
        for (Fleet::const_iterator it = m_fleet->begin(); it != m_fleet->end(); ++it) {
            m_ships_lb->Insert(new ShipRow(dynamic_cast<Ship*>(universe.Object(*it))));
        }
        *m_destination_text << DestinationText();
        if (m_read_only) 
            m_fleet_connection = GG::Connect(m_fleet->StateChangedSignal(), &FleetDetailPanel::Refresh, this);
    }
}

void FleetDetailPanel::CloseClicked()
{
    if (m_ships_lb->Empty())
        m_closing_empty_fleet_sig(m_fleet);
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

void FleetDetailPanel::ShipBrowsed(int row_idx)
{
    if (row_idx != -1) {
        *m_ship_status_text << ShipStatusText(GetShipIDOfListRow(row_idx));
    } else {
        *m_ship_status_text << "";
    }
}

void FleetDetailPanel::ShipDroppedIntoList(int row_idx, const GG::ListBox::Row* row)
{
    int ship_id = dynamic_cast<const ShipRow*>(row)->ShipID();
    if (!m_fleet) {
        // creating a new fleet can fail but will be handled by listbox eception
        m_fleet = m_create_new_fleet_sig(ship_id);
        
        *m_destination_text << DestinationText();
        if (Parent())
            Parent()->SetText(m_fleet->Name());
    }
    Ship* ship = dynamic_cast<Ship*>(GetUniverse().Object(ship_id));
    HumanClientApp::Orders().IssueOrder(new FleetTransferOrder(HumanClientApp::GetApp()->PlayerID(), ship->FleetID(), m_fleet->ID(), std::vector<int>(1, ship_id)));
}

std::string FleetDetailPanel::DestinationText() const
{
    std::string retval = "ERROR";
    System* dest = m_fleet->Destination();
    System* current = m_fleet->GetSystem();
    if (dest && dest != current) {
        retval = "En route to " + dest->Name() + " System (ETA " + boost::lexical_cast<std::string>(m_fleet->ETA()) + " turns)";
    } else if (current) {
        retval = "At " + current->Name() + " System";
    }
    return retval;
}

std::string FleetDetailPanel::ShipStatusText(int ship_id) const
{
    std::string retval = "Ship Class \"";
    retval += dynamic_cast<const Ship*>(GetUniverse().Object(ship_id))->Design().name;
    retval += "\"";
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
    Resize(m_fleet_panel->WindowDimensions() + GG::Pt(LeftBorder() + RightBorder() + 6, TopBorder() + BottomBorder() + 6));
    AttachSignalChildren();
    SetText(TitleText());
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
namespace {
const int NEW_FLEET_BUTTON_WIDTH = 75;
class EditWnd : public CUI_Wnd
{
public:
    EditWnd(int w, const std::string& text, Uint32 flags = Wnd::MODAL) : 
        CUI_Wnd("Enter new name", 0, 0, 1, 1, flags)
    {
        m_edit = new CUIEdit(LeftBorder() + 3, TopBorder() + 3, w - 2 * BUTTON_WIDTH - 2 * CONTROL_MARGIN - 6 - LeftBorder() - RightBorder(), ClientUI::PTS + 10, text);
        m_ok_bn = new CUIButton(m_edit->LowerRight().x + CONTROL_MARGIN, TopBorder() + 3, BUTTON_WIDTH, ClientUI::String("OK"));
        m_cancel_bn = new CUIButton(m_ok_bn->LowerRight().x + CONTROL_MARGIN, TopBorder() + 3, BUTTON_WIDTH, ClientUI::String("CANCEL"));

        Resize(w, m_cancel_bn->LowerRight().y + BottomBorder() + 3);
        MoveTo((GG::App::GetApp()->AppWidth() - w) / 2, (GG::App::GetApp()->AppHeight() - Height()) / 2);

        AttachChild(m_edit);
        AttachChild(m_ok_bn);
        AttachChild(m_cancel_bn);

        GG::Connect(m_ok_bn->ClickedSignal(), &EditWnd::OkClicked, this);
        GG::Connect(m_cancel_bn->ClickedSignal(), &CUI_Wnd::CloseClicked, static_cast<CUI_Wnd*>(this));
    }

    const std::string& Result() const {return m_result;}

private:
    void OkClicked() {m_result = m_edit->WindowText(); CloseClicked();}

    std::string m_result;

    CUIEdit*    m_edit;
    CUIButton*  m_ok_bn;
    CUIButton*  m_cancel_bn;

    static const int BUTTON_WIDTH = 75;
    static const int CONTROL_MARGIN = 5;
};
}

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
    m_fleet_detail_panel = new FleetDetailPanel(LeftBorder() + 3, m_fleets_lb->LowerRight().y + CONTROL_MARGIN, 0, true);
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
}

FleetWnd::FleetWnd( const GG::XMLElement& elem) : 
    MapWndPopup(elem.Child("CUI_Wnd")),
    m_empire_id(-1),
    m_read_only(true)
{
    // TODO : implement as needed (note that the initializations above must be changed as well)
}

FleetWnd::~FleetWnd()
{
}

void FleetWnd::CloseClicked()
{
    CUI_Wnd::CloseClicked();
    for (std::map<Fleet*, FleetDetailWnd*>::iterator it = m_open_fleet_windows.begin(); it != m_open_fleet_windows.end(); ++it) {
        GG::App::GetApp()->Remove(it->second);
        delete it->second;
    }
    for (std::set<FleetDetailWnd*>::iterator it = m_new_fleet_windows.begin(); it != m_new_fleet_windows.end(); ++it) {
        GG::App::GetApp()->Remove(*it);
        delete *it;
    }
    for (int i = 0; i < m_fleets_lb->NumRows(); ++i) {
        Fleet* fleet = FleetInRow(i);
        m_not_showing_fleet_sig(fleet);
    }
    delete this;
}

void FleetWnd::Init(const std::vector<Fleet*>& fleets)
{
    m_fleets_lb->SetStyle(GG::LB_NOSORT | GG::LB_BROWSEUPDATES);

    for (unsigned int i = 0; i < fleets.size(); ++i) {
        m_fleets_lb->Insert(new FleetRow(fleets[i]));
        m_showing_fleet_sig(fleets[i]);
    }

    AttachSignalChildren();

    GG::Connect(m_fleets_lb->BrowsedSignal(), &FleetWnd::FleetBrowsed, this);
    GG::Connect(m_fleets_lb->SelChangedSignal(), &FleetWnd::FleetSelectionChanged, this);
    GG::Connect(m_fleets_lb->RightClickedSignal(), &FleetWnd::FleetRightClicked, this);
    GG::Connect(m_fleets_lb->DoubleClickedSignal(), &FleetWnd::FleetDoubleClicked, this);
    GG::Connect(m_fleets_lb->DeletedSignal(), &FleetWnd::FleetDeleted, this);
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
        for (std::set<int>::iterator it = m_fleets_lb->Selections().begin(); it != m_fleets_lb->Selections().end(); ++it) {
            Fleet* fleet = FleetInRow(*it);
            if (fleet->NumShips()) {
                HumanClientApp::Orders().IssueOrder(new FleetMoveOrder(empire_id, fleet->ID(), system_id));
                std::map<Fleet*, FleetDetailWnd*>::iterator it = m_open_fleet_windows.find(fleet);
                if (it != m_open_fleet_windows.end())
                    it->second->GetFleetDetailPanel().SetFleet(it->second->GetFleetDetailPanel().GetFleet());
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
    m_current_fleet = m_fleets_lb->Caret();
    Fleet* fleet = 0 <= m_current_fleet ? FleetInRow(m_current_fleet) : 0;
    m_fleet_detail_panel->SetFleet(fleet);
}

void FleetWnd::FleetRightClicked(int row_idx, const GG::ListBox::Row* row, const GG::Pt& pt)
{
    GG::MenuItem menu_contents;
    menu_contents.next_level.push_back(GG::MenuItem("Rename", 1, false, false));
    GG::PopupMenu popup(pt.x, pt.y, GG::App::GetApp()->GetFont(ClientUI::FONT, ClientUI::PTS), menu_contents, ClientUI::TEXT_COLOR);
    switch (popup.Run()) {
    case 1: { // rename fleet
        Fleet* fleet = FleetInRow(row_idx);
        std::string fleet_name = fleet->Name();
        EditWnd edit_wnd(350, fleet_name);
        edit_wnd.Run();
        if (edit_wnd.Result() != "") {
            fleet->Rename(edit_wnd.Result());
            m_fleets_lb->GetRow(row_idx)[0]->SetText(edit_wnd.Result());
        }
        break;
    }
    default:
        break;
    }
}

void FleetWnd::FleetDoubleClicked(int row_idx, const GG::ListBox::Row* row)
{
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
        GG::Connect(fleet_wnd->ClosingEmptyFleetSignal(), &FleetWnd::EmptyFleetBoxClosing, this);
    }
}

void FleetWnd::FleetDeleted(int row_idx)
{
    if (m_current_fleet == row_idx)
        m_current_fleet = -1;
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
    GG::Connect(fdw->CreateNewFleetSignal(), &FleetWnd::NewFleetBoxReceivedShip, this);
    GG::Connect(fdw->ClosingSignal(), &FleetWnd::FleetDetailWndClosing, this);
    GG::Connect(fdw->ClosingEmptyFleetSignal(), &FleetWnd::EmptyFleetBoxClosing, this);
}
 
Fleet* FleetWnd::NewFleetBoxReceivedShip(int ship_id)
{
    Fleet* existing_fleet = FleetInRow(0);
    int empire_id = HumanClientApp::GetApp()->PlayerID();

    int new_fleet_id = ClientApp::GetNewObjectID();

    if ( new_fleet_id == UniverseObject::INVALID_OBJECT_ID )
    {
      // display message
      ClientUI::MessageBox(ClientUI::String("SERVER_TIMEOUT"));

      // throw exception to UI so drop does not occur
      throw GG::ListBox::DontAcceptDropException( );
    }

    std::string fleet_name = "New fleet " + boost::lexical_cast<std::string>( new_fleet_id );

    if (existing_fleet->SystemID() != UniverseObject::INVALID_OBJECT_ID) {
        HumanClientApp::Orders().IssueOrder(new NewFleetOrder(empire_id, fleet_name, new_fleet_id, existing_fleet->SystemID()));
    } else {
        HumanClientApp::Orders().IssueOrder(new NewFleetOrder(empire_id, fleet_name, new_fleet_id, existing_fleet->X(), existing_fleet->Y()));
    }
    Fleet* fleet = 0;
    if (existing_fleet->SystemID() != UniverseObject::INVALID_OBJECT_ID) {
        System::ObjectVec fleets = existing_fleet->GetSystem()->FindObjectsInOrbit(-1, IsStationaryFleetFunctor(empire_id));
        for (unsigned int i = 0; i < fleets.size(); ++i) {
            if (fleets[i]->Name() == fleet_name) {
                fleet = dynamic_cast<Fleet*>(fleets[i]);
                break;
            }
        }
    } else {
        Universe::ObjectVec fleets = GetUniverse().FindObjects(&IsFleet);
        for (unsigned int i = 0; i < fleets.size(); ++i) {
            if (fleets[i]->Name() == fleet_name && fleets[i]->X() == existing_fleet->X() && fleets[i]->Y() == existing_fleet->Y()) {
                fleet = dynamic_cast<Fleet*>(fleets[i]);
                break;
            }
        }
    }
    m_showing_fleet_sig(fleet);
    m_fleets_lb->Insert(new FleetRow(fleet));
    return fleet;
}

void FleetWnd::EmptyFleetBoxClosing(Fleet* fleet)
{
    Fleet* current_fleet = 0;
    try {
        current_fleet = FleetInRow(m_current_fleet);
    } catch (const std::out_of_range& re) {
        m_current_fleet = -1; // if the fleet deleted was the currently selected one, we need to fix that situation
    }
    if (m_fleet_detail_panel->GetFleet() == fleet)
        m_fleet_detail_panel->SetFleet(0);

    HumanClientApp::Orders().IssueOrder(new DeleteFleetOrder(HumanClientApp::GetApp()->PlayerID(), fleet->ID()));
    m_not_showing_fleet_sig(fleet);
    for (int i = 0; i < m_fleets_lb->NumRows(); ++i) {
        if (FleetInRow(i) == fleet) {
            m_fleets_lb->Delete(i);
            break;
        }
    }
}

void FleetWnd::FleetDetailWndClosing(FleetDetailWnd* wnd)
{
    if (m_new_fleet_windows.find(wnd) != m_new_fleet_windows.end()) {
        m_new_fleet_windows.erase(wnd);
    } else {
        for (std::map<Fleet*, FleetDetailWnd*>::iterator it = m_open_fleet_windows.begin(); it != m_open_fleet_windows.end(); ++it) {
            if (it->second == wnd) {
                m_open_fleet_windows.erase(it);
                break;
            }
        }
    }
}

Fleet* FleetWnd::FleetInRow(int idx) const
{
    return dynamic_cast<FleetRow&>(m_fleets_lb->GetRow(idx)).m_fleet;
}

std::string FleetWnd::TitleText() const
{
    Fleet* existing_fleet = FleetInRow(0);
    return "Empire " + boost::lexical_cast<std::string>(*existing_fleet->Owners().begin()) + " Fleets";
}
