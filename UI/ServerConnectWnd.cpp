#include "ServerConnectWnd.h"

#include "ClientUI.h"
#include "CUI_Wnd.h"
#include "CUIControls.h"
#include "GGClr.h"
#include "GGControl.h"
#include "GGDrawUtil.h"

#include <sstream>
#include <iomanip>

ServerConnectWnd::ServerConnectWnd(int x, int y, int w, int h) : 
    CUI_Wnd(ClientUI::String("SCONNECT_WINDOW_TITLE"), x, y, w, h, GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE | GG::Wnd::MODAL),
	m_ended_with_ok(false)
{
    m_btn_search_more = new CUIButton(10, 150, 220, ClientUI::String("SCONNECT_BTN_SEARCH_MORE"));
    m_cbo_available_servers = new CUIDropDownList(120, 80, 300, ClientUI::PTS + 4, 120);
    m_btn_ok = new CUIButton(350, 300, 80, ClientUI::String("OK"));
    m_btn_cancel = new CUIButton(230, 300, 80, ClientUI::String("CANCEL"));
    
    // static labels
    AttachChild(new GG::TextControl(10, 80, ClientUI::String("SCONNECT_LBL_SERVER"), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR));
    
    // signal connections
    InitControls();
}

ServerConnectWnd::ServerConnectWnd(const GG::XMLElement& elem) : CUI_Wnd(elem.Child("CUI_Wnd"))
{
 
    m_ended_with_ok = false;
    
    const GG::XMLElement*  curr_elem  = &elem.Child("m_cbo_available_servers");
    m_cbo_available_servers = new CUIDropDownList(curr_elem->Child("CUIDropDownList"));
    
    curr_elem  = &elem.Child("m_btn_search_more");
    m_btn_search_more = new CUIButton(curr_elem->Child("CUIButton"));
    
    curr_elem  = &elem.Child("m_btn_ok");
    m_btn_ok = new CUIButton(curr_elem->Child("CUIButton"));
    
    curr_elem  = &elem.Child("m_btn_cancel");
    m_btn_cancel = new CUIButton(curr_elem->Child("CUIButton"));
    
    // Attach children and signal connections
    InitControls();
}

const std::string& ServerConnectWnd::GetSelectedServer() const
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
    return Wnd::Run(); 
}

void ServerConnectWnd::InitControls()
{    
    m_cbo_available_servers->SetStyle(GG::LB_NOSORT);
    
    // Attach & connect signal controls
    AttachControls();
    GG::Connect(m_btn_search_more->ClickedSignal(), &ServerConnectWnd::SearchMore, this);
    GG::Connect(m_btn_ok->ClickedSignal(), &ServerConnectWnd::OnOk, this);
    GG::Connect(m_btn_cancel->ClickedSignal(), &ServerConnectWnd::OnCancel, this);
}

/** Take server list (m_server_list) and populate dropdownlist with the values */
void ServerConnectWnd::PopulateServerList()
{
    int lastindex=-1;
    if (m_server_list.size()>0)
    {  
	    for(RowsIterator i=m_server_list.begin(); i!=m_server_list.end(); i++)
	    {
	         lastindex = AddRow((*i), lastindex);
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

    ShowDialog(ClientUI::String("SCONNECT_RETRIEVING"));
    
    int n = UpdateServerList(4);  
    std::string feedback(ClientUI::String("SCONNECT_FOUND"));
    if (n>0)
    {
        int x = feedback.find("xx");
        std::ostringstream cntr;
        cntr << std::setfill(' ') << std::setw(2) << n;
        feedback.replace(x, 2 ,cntr.str());
        PopulateServerList();
    }
    else
    {
        feedback = ClientUI::String("SCONNECT_NOMORE");
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
      retval.AppendChild(CUI_Wnd::XMLEncode());
      
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

void ServerConnectWnd::ShowDialog(const std::string& msg)
{
    ClientUI::MessageBox(msg);
}

void ServerConnectWnd::FormatRow(const IPValue& rstr, GG::DropDownList::Row* row)
{
	row->push_back(rstr.Name(), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);  // display the name/URL of server
	row->data_type = rstr.Address();             // store the real ip address
}
