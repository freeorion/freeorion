#ifndef _ServerConnectWnd_h_
#include "ServerConnectWnd.h"
#endif

#ifndef _GGDrawUtil_h_
#include "../GG/GGDrawUtil.h"
#endif

#ifndef _GGControl_h_
#include "../GG/GGControl.h"
#endif

#ifndef _GGMessageDlg_h_
#include "../GG/dialogs/GGMessageDlg.h"
#endif

#ifndef _GGClr_h_
#include "../GG/GGClr.h"
#endif

//tsev{
#ifndef _ClientUI_h_
#include "ClientUI.h"
#endif

#ifndef _CUI_Wnd_h_
#include "CUI_Wnd.h"
#endif
// }tsev

#include <sstream>
#include <iomanip>

using namespace std;
 
// Compile time constants
/*
const GG::Clr ServerConnectWnd::BACKCOLOR(1.0, 1.0, 1.0, 0.75);    
const string ServerConnectWnd::DEF_FONT("arial.ttf");
*/

ServerConnectWnd::ServerConnectWnd(int x, int y, int w, int h) : CUI_ModalWnd("Connect to Server", x, y, w, h, GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE)
{
     
#define POS_LABEL_SERVER_CONNECT    10,5
#define POS_LABEL_SERVER_SERVER     10,80
#define POS_BTN_SEARCH_MORE         10,150,220,30
#define POS_CBO_AVAILABLE_SERVERS   120,80,300,20
#define POS_BTN_OK                  350,300,80,30
#define POS_BTN_CANCEL              230,300,80,30
    
    // Some default values
    m_ended_with_ok = false;
    m_btn_search_more = new GG::Button(POS_BTN_SEARCH_MORE,"Search more...",ClientUI::FONT,ClientUI::PTS,ClientUI::CTRL_COLOR,ClientUI::TEXT_COLOR);
    m_cbo_available_servers = new GG::DropDownList(POS_CBO_AVAILABLE_SERVERS,120,ClientUI::CTRL_COLOR,GG::CLR_WHITE);
    m_btn_ok = new GG::Button(POS_BTN_OK,"OK",ClientUI::FONT,ClientUI::PTS,ClientUI::CTRL_COLOR,ClientUI::TEXT_COLOR);
    m_btn_cancel = new GG::Button(POS_BTN_CANCEL,"Cancel",ClientUI::FONT,ClientUI::PTS,ClientUI::CTRL_COLOR,ClientUI::TEXT_COLOR);
    
    // Attach static labels
//    AttachChild(new GG::StaticText(POS_LABEL_SERVER_CONNECT,"Connect to server",ClientUI::FONT,ClientUI::PTS + 4,ClientUI::TEXT_COLOR));
    AttachChild(new GG::StaticText(POS_LABEL_SERVER_SERVER,"Server:",ClientUI::FONT,ClientUI::PTS,ClientUI::TEXT_COLOR));
    
    // Attach signal connections
    InitControls();
}

ServerConnectWnd::ServerConnectWnd(const GG::XMLElement& elem) : CUI_ModalWnd(elem.Child("CUI_ModalWnd"))
{
 
    m_ended_with_ok = false;
    
    const GG::XMLElement*  curr_elem  = &elem.Child("m_cbo_available_servers");
    m_cbo_available_servers = new GG::DropDownList(curr_elem->Child("GG::DropDownList"));
    
    curr_elem  = &elem.Child("m_btn_search_more");
    m_btn_search_more = new GG::Button(curr_elem->Child("GG::Button"));
    
    curr_elem  = &elem.Child("m_btn_ok");
    m_btn_ok = new GG::Button(curr_elem->Child("GG::Button"));
    
    curr_elem  = &elem.Child("m_btn_cancel");
    m_btn_cancel = new GG::Button(curr_elem->Child("GG::Button"));
    
    // Attach children and signal connections
    InitControls();
}

const string& ServerConnectWnd::GetSelectedServer() const
{
    const GG::DropDownList::Row* r = m_cbo_available_servers->CurrentItem();
    // how to get the content of the Row out of the Row? Only way I found is data type...
    return r->data_type;
}

void ServerConnectWnd::AddServer(const std::string& ipaddress, const std::string& ipname)
{
    m_server_list.push_back(IPValue(ipaddress,ipname));   
}

int ServerConnectWnd::Run()
{
    // Fill the drop down list values in
    PopulateServerList();  
    return ModalWnd::Run(); 
}

/// PROTECTED MEMBERS ////

//int ServerConnectWnd::Render()
//{
/*    HumanClientApp::GetApp()->Enter2DMode();
    GG::FlatRectangle(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y, ClientUI::WND_COLOR, ClientUI::BORDER_COLOR, 1);
    HumanClientApp::GetApp()->Exit2DMode();
*/
//    ClientUI::DrawWindow(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y, "Connect to Server");
//    return 0;
//}


/// PRIVATE MEMBERS ////

void ServerConnectWnd::InitControls()
{    
    m_cbo_available_servers->SetStyle(GG::LB_NOSORT);
    
    // Attach & connect signal controls
    AttachControls();
    GG::Connect(m_btn_search_more->ClickedSignal(), &ServerConnectWnd::SearchMore, this);
    GG::Connect(m_btn_ok->ClickedSignal(), &ServerConnectWnd::OnOk, this);
    GG::Connect(m_btn_cancel->ClickedSignal(), &ServerConnectWnd::OnOk, this);
}

/** Take server list (m_server_list) and populate dropdownlist with the values */
void ServerConnectWnd::PopulateServerList()
{
    int lastindex=-1;
    if (m_server_list.size()>0)
    {  
	    for(RowsIterator i=m_server_list.begin(); i!=m_server_list.end(); i++)
	    {
	         lastindex = AddRow((*i),lastindex);
	    }
	    m_cbo_available_servers->Select(0);  
    }
    else
    {
        AddRow(IPValue("127.0.0.1","localhost"));
    }
}

/** Search for more FreeOrion servers. TO DO */
void ServerConnectWnd::SearchMore()
{

    ShowDialog("Retrieving more servers...");
    
    int n = UpdateServerList(4);  
    string feedback("Found xx new servers");
    if (n>0)
    {
        int x = feedback.find("xx");
        ostringstream cntr;
        cntr << setfill(' ') << setw(2) << n;
        feedback.replace(x,2,cntr.str());
        PopulateServerList();
    }
    else
    {
        feedback = "No more servers found";
    }
    
    ShowDialog(feedback);

}

void ServerConnectWnd::OnOk()
{
    m_ended_with_ok=true;
    m_done=true;
}

void ServerConnectWnd::OnCancel()
{
    m_ended_with_ok=false;
    m_done=true;
}

GG::XMLElement ServerConnectWnd::XMLEncode()
{
      DetachControls();
   
      GG::XMLElement retval("ServerConnectWnd");
      retval.AppendChild(GG::ModalWnd::XMLEncode());
      
      GG::XMLElement temp("m_ended_with_OK");
      temp.SetAttribute("value", boost::lexical_cast<std::string>(m_ended_with_ok));
      retval.AppendChild(temp);
      
      temp = GG::XMLElement("m_cbo_available_servers");
      temp.AppendChild(m_cbo_available_servers->XMLEncode());
      retval.AppendChild(temp);
      
      temp = GG::XMLElement("m_btn_search_more");
      temp.AppendChild(m_btn_search_more->XMLEncode());
      retval.AppendChild(temp);
      
      temp = GG::XMLElement("m_btn_ok");
      temp.AppendChild(m_btn_ok->XMLEncode());
      retval.AppendChild(temp);
      
      temp = GG::XMLElement("m_btn_cancel");
      temp.AppendChild(m_btn_cancel->XMLEncode());
      retval.AppendChild(temp);
      
      AttachControls();
      
      return retval;  
}

int ServerConnectWnd::UpdateServerList(int maxitems)
{

    int foundServers = 0;
    
    IPValue r1("64.239.105.78","www.freeorion.org");
    IPValue r2("195.34.43.9","games.freeorion.org");
    IPValue r3("10.55.8.4");
    IPValue r4("10.29.34.3");
    IPValue r5("10.23.211.12");
      
    // Reset list
    m_server_list.clear();
    
    switch (maxitems)
    {
	    case 2:
	         m_server_list.push_back(r1);
	         m_server_list.push_back(r2);
	         foundServers = 2;
	    break;
        
        case 4:
             m_server_list.push_back(r3);
             m_server_list.push_back(r4);  
             m_server_list.push_back(r5);  
             foundServers = 3;     
	    break;
        
        default:
	    break;
    }
    
    return foundServers;
}

void ServerConnectWnd::AttachControls()
{
    AttachChild(m_btn_search_more);
    AttachChild(m_cbo_available_servers);
    AttachChild(m_btn_ok);
    AttachChild(m_btn_cancel);
}

void ServerConnectWnd::DetachControls()
{
    DetachChild(m_cbo_available_servers);
    DetachChild(m_btn_search_more);
    DetachChild(m_btn_ok);
    DetachChild(m_btn_cancel);
}

/** Add a row to the drop down list. Return insertion index value */
int ServerConnectWnd::AddRow(const IPValue& value)
{
    GG::DropDownList::Row r;
    FormatRow(value,&r);
	return (m_cbo_available_servers->Insert(r));
}

/** Add a row to the drop down list. Return insertion index value. index specify insertion point */
int ServerConnectWnd::AddRow(const IPValue& value, int index)
{
    GG::DropDownList::Row r;
    FormatRow(value,&r);
	return (m_cbo_available_servers->Insert(r,index));
}

void ServerConnectWnd::ShowDialog(const string& msg)
{
/*
    GG::MessageDlg* dialog = new GG::MessageDlg(300,300,400,200,msg,ClientUI::FONT,ClientUI::PTS,ClientUI::WND_COLOR,ClientUI::BORDER_COLOR);
    dialog->Run();
    delete dialog;
*/
//now uses the messagebox method in ClientUI
    ClientUI::MessageBox(msg);
}


