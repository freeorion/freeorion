//GalaxySetupWnd.cpp

#ifndef _GalaxySetupWnd_h_
#include "GalaxySetupWnd.h"
#endif

#ifndef _GGApp_h_
#include "GGApp.h"
#endif

#ifndef _GGDrawUtil_h_
#include "GGDrawUtil.h"
#endif

#ifndef _GGClr_h_
#include "GGClr.h"
#endif

#ifndef _ClientUI_h_
#include "ClientUI.h"
#endif

#ifndef _GGFileDlg_h_
#include "dialogs/GGFileDlg.h"
#endif

#include <fstream>

////////////////////////////////////////////
//   CONSTANTS
////////////////////////////////////////////
const int GalaxySetupWnd::GALAXY_TYPES  =  4;   //not including "From file:"

////////////////////////////////////////////
//   CONSTRUCTION/DESTRUCTION
////////////////////////////////////////////

GalaxySetupWnd::GalaxySetupWnd():
//    GG::ModalWnd(200,200,645,360),
    CUI_ModalWnd(ClientUI::String("GSETUP_WINDOW_TITLE"),200,200,645,360, GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE),
    m_end_with_ok(false)
{
    //initialize size radio group
    m_size_buttons = new GG::RadioButtonGroup(5,7);
    GG::StateButton* temp=NULL;
    
    m_size_buttons->AddButton(temp=new GG::StateButton(5,10,100,50,ClientUI::String("GSETUP_VERYSMALL"), ClientUI::FONT, ClientUI::PTS,0,ClientUI::TEXT_COLOR, ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR, GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    m_size_buttons->AddButton(temp=new GG::StateButton(5,65,100,50,ClientUI::String("GSETUP_SMALL"), ClientUI::FONT,ClientUI::PTS,0, ClientUI::TEXT_COLOR, ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR,GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    m_size_buttons->AddButton(temp=new GG::StateButton(5,120,100,50,ClientUI::String("GSETUP_MEDIUM"), ClientUI::FONT,ClientUI::PTS,0, ClientUI::TEXT_COLOR, ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR,GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    m_size_buttons->AddButton(temp=new GG::StateButton(5,175,100,50,ClientUI::String("GSETUP_LARGE"), ClientUI::FONT,ClientUI::PTS,0, ClientUI::TEXT_COLOR, ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR,GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    m_size_buttons->AddButton(temp=new GG::StateButton(5,230,100,50,ClientUI::String("GSETUP_VERYLARGE"), ClientUI::FONT,ClientUI::PTS,0, ClientUI::TEXT_COLOR, ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR,GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    m_size_buttons->AddButton(temp=new GG::StateButton(5,285,100,50,ClientUI::String("GSETUP_ENORMOUS"), ClientUI::FONT,ClientUI::PTS,0, ClientUI::TEXT_COLOR, ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR,GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    
    m_type_buttons = new GG::RadioButtonGroup(125,7);
    
    m_type_buttons->AddButton(temp=new GG::StateButton(125,10,100,50,ClientUI::String("GSETUP_2ARM"), ClientUI::FONT,ClientUI::PTS,0,ClientUI::TEXT_COLOR, ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR,GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    m_type_vector.push_back(temp);
    m_type_buttons->AddButton(temp=new GG::StateButton(125,65,100,50,ClientUI::String("GSETUP_3ARM"), ClientUI::FONT,ClientUI::PTS,0,ClientUI::TEXT_COLOR, ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR,GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    m_type_vector.push_back(temp);
    m_type_buttons->AddButton(temp=new GG::StateButton(125,120,100,50,ClientUI::String("GSETUP_4ARM"), ClientUI::FONT,ClientUI::PTS,0,ClientUI::TEXT_COLOR, ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR,GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    m_type_vector.push_back(temp);
    m_type_buttons->AddButton(temp=new GG::StateButton(125,175,100,50,ClientUI::String("GSETUP_CLUSTER"), ClientUI::FONT,ClientUI::PTS,0,ClientUI::TEXT_COLOR, ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR,GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    m_type_vector.push_back(temp);
    m_type_buttons->AddButton(temp=new GG::StateButton(125,230,100,50,ClientUI::String("GSETUP_FROMFILE"), ClientUI::FONT,ClientUI::PTS,0,ClientUI::TEXT_COLOR, ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR,GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    m_type_vector.push_back(temp);
    
    m_galaxy_file =    new GG::Edit(385, 247, 168, 25, "", ClientUI::FONT, ClientUI::PTS, ClientUI::INNER_BORDER_COLOR/*, GG::CLR_BLACK*/);
    m_galaxy_file->SetTextColor(ClientUI::TEXT_COLOR);
    
    m_browse_button = new GG::Button(555, 247, 75, 25, ClientUI::String("BROWSE_BTN"), ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR, ClientUI::TEXT_COLOR);
    
    //create a temporary texture and static graphic  
    // this will get overwritten during construction
    boost::shared_ptr<GG::Texture> temptex(new GG::Texture());
    m_preview_image =  new GG::StaticGraphic(385, 30, 248, 186, temptex, GG::SG_FITGRAPHIC);    //create a blank graphic
 
    m_ok     = new GG::Button(555,305,75,25,ClientUI::String("OK"), ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR, ClientUI::TEXT_COLOR);
    m_cancel = new GG::Button(465,305,75,25,ClientUI::String("CANCEL"), ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR, ClientUI::TEXT_COLOR);

    Init();    //attaches children and connects signals to slots
}//GalaxySetupWnd()

GalaxySetupWnd::GalaxySetupWnd(const GG::XMLElement &elem):
//    GG::ModalWnd(elem.Child("GG::ModalWnd")),
    CUI_ModalWnd(elem.Child("CUI_ModalWnd")),
    m_end_with_ok(false)
{
    using namespace GG;
    
    if (elem.Tag() != "GalaxySetupWnd")
       throw std::invalid_argument("Attempted to construct an GalaxySetupWnd from an XMLElement that had a tag other than \"GalaxySetupWnd\"");
    
    try
    {
        m_browse_button = new Button(elem.Child("m_browse_button"));
            
        m_ok = new Button(elem.Child("m_ok"));
                   
        m_cancel = new Button(elem.Child("m_cancel"));
                            
        m_galaxy_file = new Edit(elem.Child("m_galaxy_file"));
    
        m_size_buttons = new RadioButtonGroup(elem.Child("m_size_buttons"));
            
        m_type_buttons = new RadioButtonGroup(elem.Child("m_type_buttons"));
    
        m_preview_image = new StaticGraphic(elem.Child("m_preview_image"));
        
        Init();
    }
    catch(const std::invalid_argument& e)
    {
        App::GetApp()->Logger().fatal(e.what());
        ClientUI::MessageBox(e.what());
    }
    
}//GalaxySetupWnd(XMLElement)

void GalaxySetupWnd::Init()
{
    //add children
    AttachChild(m_size_buttons);
    AttachChild(m_type_buttons);
    
    AttachChild(m_galaxy_file);
    AttachChild(m_browse_button);
    AttachChild(m_preview_image);
    
    AttachChild(m_ok);
    AttachChild(m_cancel);
    
    //attach signals
    GG::Connect(m_size_buttons->ButtonChangedSignal(), &GalaxySetupWnd::OnChangeSize, this);
    GG::Connect(m_type_buttons->ButtonChangedSignal(), &GalaxySetupWnd::OnChangeType, this);
    GG::Connect(m_browse_button->ClickedSignal(), &GalaxySetupWnd::OnBrowse, this);
    GG::Connect(m_ok->ClickedSignal(), &GalaxySetupWnd::OnOK, this);
    GG::Connect(m_cancel->ClickedSignal(), &GalaxySetupWnd::OnCancel, this);
    
    //create and load textures
    m_textures.clear();
    for(int i=0; i<GALAXY_TYPES; ++i)
    {
        m_textures.push_back(boost::shared_ptr<GG::Texture>(new GG::Texture()));
    }
    m_textures[TWO_ARM]->Load(ClientUI::ART_DIR + "gp_spiral2.png");
    m_textures[THREE_ARM]->Load(ClientUI::ART_DIR + "gp_spiral3.png");
    m_textures[FOUR_ARM]->Load(ClientUI::ART_DIR + "gp_spiral4.png");
    m_textures[CLUSTER]->Load(ClientUI::ART_DIR + "gp_cluster.png");
    
    //now change everything to default setting (large and spiral-2arm)
    m_size_buttons->SetCheck(LARGE);
    m_type_buttons->SetCheck(TWO_ARM);
}//Init()

GalaxySetupWnd::~GalaxySetupWnd()
{

}//~GalaxySetupWnd

///////////////////////////////////////////////
//   MUTATORS
///////////////////////////////////////////////

int GalaxySetupWnd::Render()
{
    CUI_ModalWnd::Render();
   // GG::BeveledRectangle(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y,ClientUI::WND_COLOR,ClientUI::BORDER_COLOR,true);
    //ClientUI::DrawWindow(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y, "Galaxy Setup");
    
    //draw spot for preview image 385, 30, 248, 186
    GG::FlatRectangle(UpperLeft().x+383, UpperLeft().y+28, UpperLeft().x+383+252, UpperLeft().y+28+190, GG::CLR_BLACK, ClientUI::INNER_BORDER_COLOR, 1);
    return true;
}//Render()

GG::XMLElement GalaxySetupWnd::XMLEncode() const
{
    using namespace GG;
    
    //DO NOT ENCODE:
    //    m_end_with_ok
    
    XMLElement retval("GalaxySetupWnd"), temp;
    
    retval.AppendChild(ModalWnd::XMLEncode());
    
    temp = XMLElement("m_browse_button");
    temp.AppendChild(m_browse_button->XMLEncode());
    retval.AppendChild(temp);
    
    temp = XMLElement("m_cancel");
    temp.AppendChild(m_cancel->XMLEncode());
    retval.AppendChild(temp);
    
    temp = XMLElement("m_ok");
    temp.AppendChild(m_ok->XMLEncode());
    retval.AppendChild(temp);
    
    temp = XMLElement("m_galaxy_file");
    temp.AppendChild(m_galaxy_file->XMLEncode());
    retval.AppendChild(temp);
    
    temp = XMLElement("m_size_buttons");
    temp.AppendChild(m_size_buttons->XMLEncode());
    retval.AppendChild(temp);
    
    temp = XMLElement("m_type_buttons");
    temp.AppendChild(m_type_buttons->XMLEncode());
    retval.AppendChild(temp);
    
    temp = XMLElement("m_preview_image");
    temp.AppendChild(m_preview_image->XMLEncode());
    retval.AppendChild(temp);
    
    return retval;
    
}//XMLEncode()

///////////////////////////////////////////////
//   ACCESSORS
///////////////////////////////////////////////

int GalaxySetupWnd::GalaxyShape() const
{
    return m_type_buttons->CheckedButton();
}//GalaxyShape()

std::string GalaxySetupWnd::GalaxyFile() const
{
    return *m_galaxy_file;
}//GalaxyFile()

///////////////////////////////////////////////
//   EVENT HANDLERS
///////////////////////////////////////////////

void GalaxySetupWnd::OnOK()
{
    //check to see if we have a valid image if file is checked
    if(m_type_buttons->CheckedButton() == FROM_FILE)
    {
        //validate filename
        std::string temp_str(*m_galaxy_file);
        //first check that it exists by trying to open it....
        try
        {
            std::ifstream f(temp_str.c_str());
            f.close();
        }
        catch(const std::exception& e)
        {
            ClientUI::MessageBox("\"" + temp_str + "\" "+ ClientUI::String("GSETUP_ERR_NOEXIST"));
            return;
        }        
        
        //now see if it is a valid image
        try
        {
            boost::shared_ptr<GG::Texture> tex(new GG::Texture);
            tex->Load(temp_str);
        }
        catch(const std::exception& e)
        {
            ClientUI::MessageBox("\"" + temp_str + "\" " + ClientUI::String("GSETUP_ERR_INVALID_GRAPHICS"));
            return;
        }
    }

    m_end_with_ok = true;
    
    m_done = true;
}//OnOK()

void GalaxySetupWnd::OnCancel()
{
    m_end_with_ok = false;
    
    m_done = true;
}//OnCancel()

void GalaxySetupWnd::OnChangeSize(int index)
{
    //enable all questionable controls for a microsecond or two....
    m_type_vector[TWO_ARM]->Disable(false);
    m_type_vector[THREE_ARM]->Disable(false);
    m_type_vector[FOUR_ARM]->Disable(false);   
    
    //...then disable invalid galaxy shapes
    switch(index)
    {
    case VERY_SMALL:
        m_type_vector[TWO_ARM]->Disable();   
    case SMALL:
        m_type_vector[THREE_ARM]->Disable();
    case MEDIUM:
        m_type_vector[FOUR_ARM]->Disable();
        
        //if a checked button is disabled, change it to cluster
        if( (m_type_vector[TWO_ARM]->Disabled() && m_type_buttons->CheckedButton() == TWO_ARM) ||
            (m_type_vector[THREE_ARM]->Disabled() && m_type_buttons->CheckedButton() == THREE_ARM) ||
            (m_type_vector[FOUR_ARM]->Disabled() && m_type_buttons->CheckedButton() == FOUR_ARM))
        {
            m_type_buttons->SetCheck(CLUSTER); 
        }
            
        break;
    default:
        break;
    }//switch
    
}//OnChangeSize()

void GalaxySetupWnd::OnChangeType(int index)
{
    //update preview image
    if(m_type_buttons->CheckedButton() == FROM_FILE)
    {
        //delete current image
        if(m_preview_image)
        {
            DetachChild(m_preview_image);
            delete(m_preview_image);
            m_preview_image = NULL;
        }//if
        
        //create temp texture
        try
        {
            boost::shared_ptr<GG::Texture> tex(new GG::Texture());
            tex->Load(*m_galaxy_file);
        
            //creat the new staticgraphic object
            m_preview_image = new GG::StaticGraphic(385, 30, 248, 186, tex, GG::SG_FITGRAPHIC);
            AttachChild(m_preview_image);
        }
        catch(const std::exception& e)
        {
            //possible if file not found or otherwise
             GG::App::GetApp()->Logger().alert("GalaxySetupWnd: Invalid texture file specified.");
        }
        
    }//if
    else if(index != -1)
    {
        //load from stored image
        if(m_preview_image)
        {
            DetachChild(m_preview_image);
            delete(m_preview_image);
            m_preview_image = NULL;
        }//if
        
        m_preview_image = new GG::StaticGraphic(385, 30, 248, 186, m_textures[index], GG::SG_FITGRAPHIC);
        AttachChild(m_preview_image);
    }//else

}//OnChangeType()

void GalaxySetupWnd::OnBrowse()
{
    //open a file dialog and allow the user to browse for a graphics file
//<<<<<<< GalaxySetupWnd.cpp
 /*   GG::FileDlg dlg(*m_galaxy_file,
=======
    std::vector<std::pair<std::string, std::string> > allowed_file_types;
    allowed_file_types.push_back(std::pair<std::string, std::string>("Graphics Files (PNG, Targa)", "*.png, *.PNG, *.tga, *.TGA"));
    GG::FileDlg dlg(*m_galaxy_file,
>>>>>>> 1.3
                    false,
                    false,
                    allowed_file_types,
                    ClientUI::FONT,
                    ClientUI::PTS,
                    GG::Clr(100,100,100,255),GG::CLR_WHITE,GG::CLR_BLACK
                    //ClientUI::WND_COLOR,
                    //ClientUI::BORDER_COLOR,
                    //ClientUI::TEXT_COLOR
                    );
<<<<<<< GalaxySetupWnd.cpp
  */
  
    std::vector<std::pair<std::string, std::string> > allowed_file_types;
    //filter for graphics files
    allowed_file_types.push_back(std::pair<std::string, std::string>(ClientUI::String("GSETUP_GRAPHICS_FILES") + " (PNG, Targa, JPG, BMP)", "*.png, *.PNG, *.tga, *.TGA, *.jpg, *.JPG, *.bmp, *.BMP"));
    
    GG::FileDlg dlg(*m_galaxy_file,
                    false, false, allowed_file_types,
                    ClientUI::FONT,
                    ClientUI::PTS,
                    GG::Clr(100,100,100,255),
                    GG::CLR_WHITE,
                    GG::CLR_BLACK); 
//=======

//>>>>>>> 1.3
    dlg.Run();
    
    if (dlg.Result().empty())
        return;
    
    //get and store result
    //retrieve it from the set
    std::string filename = *(dlg.Result().begin());   
    
    m_galaxy_file->SetText(filename);
    
    //uncheck and recheck the browse box
    m_type_buttons->SetCheck(-1);
    m_type_buttons->SetCheck(FROM_FILE);

}//OnBrowse()
