
#include "FleetWindow.h"

#ifndef _ClientApp_h_
#include "../client/ClientApp.h"
#endif

#ifndef _ClientUI_h_
#include "ClientUI.h"
#endif

#include "../universe/Fleet.h"

using std::vector;

namespace 
{
    const int fleet_list_x = 210;
    const int fleet_list_y = 45;
    const int fleet_list_width = 170;
    const int fleet_list_height =60;
    
    const int move_button_x = fleet_list_x;
    const int move_button_y = fleet_list_y + fleet_list_height+1;
    const int move_button_width = fleet_list_width / 3;
    const int move_button_height = 30;
    
    const int merge_button_x = move_button_x + move_button_width;
    const int merge_button_y = move_button_y;
    const int merge_button_width = move_button_width;
    const int merge_button_height = move_button_height;
    
    const int split_button_x = merge_button_x + merge_button_width;
    const int split_button_y = move_button_y;
    const int split_button_width = move_button_width;
    const int split_button_height = move_button_height;
    
    const int ship_list_x = 10;
    const int ship_list_y = 165;
    const int ship_list_width = 100;
    const int ship_list_height = 220;
    
    const int done_button_x = move_button_x;
    const int done_button_y = move_button_y;
    const int done_button_width = move_button_width;
    const int done_button_height = move_button_height;
    
    const int receiving_list_x = fleet_list_x;
    const int receiving_list_y = ship_list_y;
    const int receiving_list_width = fleet_list_width;
    const int receiving_list_height = ship_list_height;
}



FleetWindow::FleetWindow(int x, int y, vector<int> fleets_here) :
    m_fleets(fleets_here),
    CUI_Wnd("Fleet Window", x, y, 400, 400, GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE | CUI_Wnd::CLOSABLE | CUI_Wnd::MINIMIZABLE | GG::Wnd::ONTOP)
{

    m_ship_list_label  = new GG::TextControl(ship_list_x, ship_list_y-20, "Ships in Fleet", ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
    m_fleets_list_label = new GG::TextControl(fleet_list_x, fleet_list_y-20, "Fleets Here", ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);

    m_ship_list = new GG::ListBox(ship_list_x, ship_list_y, ship_list_width, ship_list_height, ClientUI::WND_INNER_BORDER_COLOR, ClientUI::CTRL_COLOR);

    m_fleets_list = new GG::ListBox(fleet_list_x, fleet_list_y, fleet_list_width, fleet_list_height, ClientUI::WND_INNER_BORDER_COLOR, ClientUI::CTRL_COLOR); 
    m_fleets_list->SetStyle(GG::LB_SINGLESEL);

    m_receiving_fleet_list = new GG::ListBox(receiving_list_x, receiving_list_y, receiving_list_width, receiving_list_height, ClientUI::WND_INNER_BORDER_COLOR, ClientUI::CTRL_COLOR); 
    m_receiving_fleet_label = new GG::TextControl(receiving_list_x, receiving_list_y-20, "Ships in Other Fleet", ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);

    m_move_button = new GG::Button(move_button_x, move_button_y, move_button_width, move_button_height, "Move", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR, ClientUI::TEXT_COLOR);
    m_split_button = new GG::Button(split_button_x, split_button_y, split_button_width, split_button_height, "Split", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR, ClientUI::TEXT_COLOR);
    m_merge_button =  new GG::Button(merge_button_x, merge_button_y, merge_button_width, merge_button_height, "Merge", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR, ClientUI::TEXT_COLOR);
    m_done_button = new GG::Button(done_button_x, done_button_y, done_button_width, done_button_height, "Done", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR, ClientUI::TEXT_COLOR);

    AttachChild(m_receiving_fleet_list);
    AttachChild(m_receiving_fleet_label);
    AttachChild(m_ship_list_label);
    AttachChild(m_fleets_list_label);
    AttachChild(m_ship_list);
    AttachChild(m_fleets_list);
    AttachChild(m_done_button);
    AttachChild(m_move_button);
    AttachChild(m_split_button);    
    AttachChild(m_merge_button);

    GG::Connect(m_done_button->ClickedSignal(), &FleetWindow::DoneButtonClicked, this);
    GG::Connect(m_move_button->ClickedSignal(), &FleetWindow::MoveButtonClicked, this);   
    GG::Connect(m_split_button->ClickedSignal(), &FleetWindow::SplitButtonClicked, this);
    GG::Connect(m_merge_button->ClickedSignal(), &FleetWindow::MergeButtonClicked, this);
   
    SetDialogMode(FLEET_VIEW);
    RefreshFleetList();
}


FleetWindow::FleetWindow(const GG::XMLElement& elem) : CUI_Wnd(elem)
{

}

FleetWindow::~FleetWindow()
{
}


void FleetWindow::MergeButtonClicked()
{
    SetDialogMode(FLEET_MERGE);
}

void FleetWindow::MoveButtonClicked()
{
    // TODO: hide this window, tell galaxy map to go into 'movement' mode
}

void FleetWindow::SplitButtonClicked()
{   
    SetDialogMode(FLEET_SPLIT);
}

void FleetWindow::DoneButtonClicked()
{
    SetDialogMode(FLEET_VIEW);
}

void FleetWindow::OnSwitchFleet()
{
    // this should be called when the selected fleet is changed,
    // and should fill the ship list with the ships in that fleet.
}

void FleetWindow::SetDialogMode(FleetWindow::DialogMode mode)
{

    switch(mode)
    {
        case FLEET_VIEW:
            m_move_button->Show();
            m_split_button->Show();
            m_merge_button->Show();
            
            m_done_button->Hide();
            m_done_button->MoveTo(-100,-100);

            m_receiving_fleet_list->Hide();
            m_receiving_fleet_label->Hide();
            break;
            
        case FLEET_SPLIT:
        case FLEET_MERGE:
            m_move_button->Hide();
            m_split_button->Hide();
            m_merge_button->Hide();

            m_done_button->MoveTo(done_button_x, done_button_y);
            m_done_button->Show();
                        
            m_receiving_fleet_list->Show();
            m_receiving_fleet_label->Show();
            break;
    }
}



void FleetWindow::RefreshFleetList()
{
    m_fleets_list->Clear();
    
    for(std::vector<int>::iterator itr = m_fleets.begin(); itr != m_fleets.end(); itr++)
    {
        const Fleet* flt = dynamic_cast<const Fleet*>(ClientApp::GetUniverse().Object(*itr));
        if(!flt)
        {
            throw std::runtime_error("RefreshFleetList(): Bad fleet ID found in m_fleets.");
        }
        GG::ListBox::Row new_row;
        
        new_row.push_back(flt->Name(), ClientUI::FONT, ClientUI::PTS);
        m_fleets_list->Insert(new_row);
    }
}

void FleetWindow::FillShipList(int fleet)
{
    const Fleet* flt = dynamic_cast<const Fleet*>(ClientApp::GetUniverse().Object(fleet));
    if(!flt)
    {
        throw std::runtime_error("FillShipList(): Bad fleet ID passed.");
    }
    
    GG::ListBox::Row new_row;
    
    for(Fleet::const_iterator itr = flt->begin(); itr != flt->end(); itr++)
    {
        const Ship* shp = dynamic_cast<const Ship*>(ClientApp::GetUniverse().Object(*itr));
        if(!shp)
        {
            throw std::runtime_error("FillShipList(): Bad Ship ID discovered in fleet.");
        }
        new_row.push_back(shp->Name(), ClientUI::FONT, ClientUI::PTS);
        m_ship_list->Insert(new_row);
    }
}
