#ifndef _ServerConnectWnd_h_
#define _ServerConnectWnd_h_

#ifndef _SDLGGApp_h_
#include "SDLGGApp.h"
#endif

#ifndef _HumanClientApp_h_
#include "HumanClientApp.h"
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

#include <vector>
#include <string>

//using namespace std;

typedef std::vector<std::string> RowsVector;
typedef RowsVector::iterator RowsIterator;

/** server connections window  */
class ServerConnectWnd : public GG::ModalWnd
{
public:
    /** \name Structors */ //@{
    /** display a new dialog for server connection. The dialog is a modal window composed by a dropdown list 
    of the available servers. User may choose the server or search for more in the net. When done user may press OK, 
    which means the server is selected, or cancel, which means go back to previous screen. */
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
    /** get the IP address or URL of the selected server as a string */
    /*!
      \return the IP address or URL of the selected server as a const string
    */
    virtual const std::string& GetSelectedServer() const;
    /** if user selects one server returns true, else false if user selected cancel */
    /*!
      \return true if user exit with OK selecting one server, or false if user selected cancel
    */
    bool IsServerSelected() const { return this->m_ended_with_ok; }
    //@}
    /** \name Mutators */ //@{
    /** Add a new IP server address, as a string, to the server list shown in the drop downlist */
    /*!
      \param ipaddress IP address of the server to add
    */
    void AddServer(const std::string& ipaddress);
    /** fill the dropdownlist box and then executes the modal window */
	virtual int	Run();  
    //@}
protected:
    virtual int Render();
private:
/*
    static const GG::Clr BACKCOLOR; 
    static const std::string DEF_FONT;
*/ 
    
    void InitControls();
    void AttachControls();
    void DetachControls();
    void PopulateServerList();
    void SearchMore();
    void OnOk();
    void OnCancel();
    int UpdateServerList(int maxitems);   
    int AddRow(const std::string& value);
    int AddRow(const std::string& value, int index);
    void ShowDialog(const std::string& msg);
    GG::XMLElement XMLEncode();

   
    // inline
    inline void FormatRow(const std::string& rstr,GG::DropDownList::Row* row)
	{
	    row->push_back(rstr,ClientUI::FONT,12);
	    row->data_type = rstr;
	}
   
    bool m_ended_with_ok;
    RowsVector m_server_list;
    GG::DropDownList* m_cbo_available_servers;
    GG::Button* m_btn_search_more;
    GG::Button* m_btn_ok;
    GG::Button* m_btn_cancel;
};


#endif // _ServerConnect_h_
