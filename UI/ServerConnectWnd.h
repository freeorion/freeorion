#ifndef _ServerConnectWnd_h_
#define _ServerConnectWnd_h_

#ifndef _SDLGGApp_h_
#include "SDLGGApp.h"
#endif

#ifndef _HumanClientApp_h_
#include "../client/human/HumanClientApp.h"
#endif

#ifndef _GGWnd_h_
#include "../GG/GGWnd.h"
#endif

#ifndef _GGTextControl_h_
#include "../GG/GGTextControl.h"
#endif

#ifndef _GGButton_h_
#include "../GG/GGButton.h"
#endif

#ifndef _GGDropDownList_h_
#include "../GG/GGDropDownList.h"
#endif

#ifndef _ClientUI_h_
#include "ClientUI.h"
#endif

#ifndef _CUI_Wnd_h_
#include "CUI_Wnd.h"
#endif

#include <vector>
#include <string>

/** utility class to collect IP address and name */
class IPValue
{
public:
    /** \name Structors */ //@{
    /** construct a new IPValue consisting of the sole IP address */
    /*!
      \param ip_address IP address of the server
    */
    IPValue(const std::string& ip_address): il_address(ip_address),il_name(ip_address) {}
    /** construct a new IPValue consisting of the IP address and an associated name that can be an URL, a logical name, or the server name */
    /*!
      \param ip_address ip address of the server
      \param ip_name name of server (ie: "localhost", "http://wwww.freeorion.org", "LAN Server #1")
    */
    IPValue(const std::string& ip_address, const std::string& ip_name) : il_address(ip_address),il_name(ip_name) {}
    //@}
    /** \name Accessors */ //@{
    /** get the IP address of this server */
    /*!
      \return the IP address of this server (ie "127.0.0.1")
    */
    const std::string& Address() const { return il_address; }
    /** get the name of this server */
    /*!
      \return the name of this server (ie: "localhost", "http://wwww.freeorion.org", "LAN Server #1")
    */
    const std::string& Name() const { return il_name; }
    //@}

private:
    std::string il_address; // ip address
    std::string il_name;    // ip name or URL
};


/** server connections window  */
class ServerConnectWnd : public CUI_Wnd
{
public:
        /** \name Structors */ //@{
    /** display a new dialog for server connection */
    /*!
      \param x upper left x coordinate of screen
      \param y upper left y coordinate of screen
      \param w width of screen
      \param h height of screen
    */
    ServerConnectWnd(int x, int y, int w, int h);
    /** this constructor creates the dialog from an XML file. During the loading of the definition, all 
    children are created. */
    /*!
      \param elem parent xml element
    */ 
    ServerConnectWnd(const GG::XMLElement& elem);
    /** destructor */
    virtual ~ServerConnectWnd() {}
    //@}
    /** \name Accessors */ //@{
    /** get the IP address of the selected server  */
    /*!
      \return the IP address of the selected server 
    */
    virtual const std::string& GetSelectedServer() const;
    /** if user selects one server returns true, else false if user selected cancel */
    /*!
      \return true if user exit with OK selecting one server, or false if user selected cancel
    */
    bool IsServerSelected() const { return m_ended_with_ok; }
    //@}
    /** \name Mutators */ //@{
    /** Add a new IP server address, as a std::string, to the server list shown in the drop downlist */
    /*!
      \param ipaddress IP address of the server to add (ie: "127.0.0.1")
      \param ipvalue IP name/URL of server to add (ie: "localhost")
    */
    void AddServer(const std::string& ipaddress, const std::string& ipname);
    /** fill the dropdownlist box and then executes the modal window */
	virtual int	Run(); 
	/** render window */
//    virtual int Render(); 
    //@}

private:
/*
    static const GG::Clr BACKCOLOR; 
    static const std::string DEF_FONT;
*/ 
    
    typedef std::vector<IPValue> RowsVector;
    typedef RowsVector::iterator RowsIterator;
    
    void InitControls();
    void AttachControls();
    void DetachControls();
    void PopulateServerList();
    void SearchMore();
    void OnOk();
    void OnCancel();
    inline virtual void OnClose() {m_ended_with_ok=false;}
    int UpdateServerList(int maxitems);  
    /** Add row to dropdownlist. Each row contains name and address of server */ 
    int AddRow(const IPValue& value);   
    /** Add row to dropdownlist at index position. Each row contains name and address of server */  
    int AddRow(const IPValue& value, int index);
    void ShowDialog(const std::string& msg);
    GG::XMLElement XMLEncode();

   
    // inline
    void FormatRow(const IPValue& rstr,GG::DropDownList::Row* row)
	{
	    row->push_back(rstr.Name(),ClientUI::GetClientUI()->FONT,ClientUI::GetClientUI()->PTS);  // display the name/URL of server
	    row->data_type = rstr.Address();             // store the real ip address
	}
   
    bool m_ended_with_ok;
    RowsVector m_server_list;
    GG::DropDownList* m_cbo_available_servers;
    GG::Button* m_btn_search_more;
    GG::Button* m_btn_ok;
    GG::Button* m_btn_cancel;
};


#endif // _ServerConnect_h_
