//GalaxySetupWnd.cpp
#include "GalaxySetupWnd.h"

#include "CUIControls.h"
#include "GGDrawUtil.h"
#include "dialogs/GGFileDlg.h"
#include "GGStaticGraphic.h"
#include "../client/human/HumanClientApp.h"

#include <fstream>


namespace {
const int GAL_SETUP_WND_WD = 645;
const int GAL_SETUP_WND_HT = 250;
const int RADIO_BN_HT = ClientUI::PTS + 4;
const int RADIO_BN_SPACING = RADIO_BN_HT + 10;
const GG::Pt PREVIEW_UL(385, 23);
const GG::Pt PREVIEW_SZ(248, 186);
}

GalaxySetupWnd::GalaxySetupWnd() : 
    CUI_Wnd(ClientUI::String("GSETUP_WINDOW_TITLE"), (HumanClientApp::GetApp()->AppWidth() - GAL_SETUP_WND_WD) / 2, 
            (HumanClientApp::GetApp()->AppHeight() - GAL_SETUP_WND_HT) / 2, GAL_SETUP_WND_WD, GAL_SETUP_WND_HT, GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE | GG::Wnd::MODAL),
    m_ended_with_ok(false)
{
    m_size_buttons = new GG::RadioButtonGroup(5, 7);
    int y = 15;
    m_size_buttons->AddButton(new CUIStateButton(5, y,                     100, RADIO_BN_HT, ClientUI::String("GSETUP_VERYSMALL"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_size_buttons->AddButton(new CUIStateButton(5, y += RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_SMALL"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_size_buttons->AddButton(new CUIStateButton(5, y += RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_MEDIUM"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_size_buttons->AddButton(new CUIStateButton(5, y += RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_LARGE"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_size_buttons->AddButton(new CUIStateButton(5, y += RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_VERYLARGE"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_size_buttons->AddButton(new CUIStateButton(5, y += RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_ENORMOUS"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));

    m_type_buttons = new GG::RadioButtonGroup(125, 7);
    y = 15;
    m_type_buttons->AddButton(new CUIStateButton(125, y,                     100, RADIO_BN_HT, ClientUI::String("GSETUP_2ARM"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_type_buttons->AddButton(new CUIStateButton(125, y += RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_3ARM"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_type_buttons->AddButton(new CUIStateButton(125, y += RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_4ARM"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_type_buttons->AddButton(new CUIStateButton(125, y += RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_CLUSTER"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_type_buttons->AddButton(new CUIStateButton(125, y += RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_ELLIPTICAL"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_type_buttons->AddButton(new CUIStateButton(125, y += RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_IRREGULAR"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
    m_type_buttons->AddButton(new CUIStateButton(125, y += RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_RING"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));
	m_type_buttons->AddButton(new CUIStateButton(125, y += RADIO_BN_SPACING, 100, RADIO_BN_HT, ClientUI::String("GSETUP_FROMFILE"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON));

    // create a temporary texture and static graphic
    boost::shared_ptr<GG::Texture> temp_tex(new GG::Texture());
    m_preview_image =  new GG::StaticGraphic(PREVIEW_UL.x, PREVIEW_UL.y, PREVIEW_SZ.x, PREVIEW_SZ.y, temp_tex, GG::GR_FITGRAPHIC); // create a blank graphic
    GG::Pt preview_lr = m_preview_image->LowerRight();

    m_galaxy_file = new CUIEdit(PREVIEW_UL.x, preview_lr.y + 7, 168, ClientUI::PTS + 10, "");
    
    m_browse_button = new CUIButton(PREVIEW_UL.x + m_galaxy_file->Size().x + 5, preview_lr.y + 7, 75, ClientUI::String("BROWSE_BTN"));
    
    m_ok     = new CUIButton(10, preview_lr.y + 10, 75, ClientUI::String("OK"));
    m_cancel = new CUIButton(10 + m_ok->Size().x + 15, preview_lr.y + 10, 75, ClientUI::String("CANCEL"));

    Init();
}

GalaxySetupWnd::GalaxySetupWnd(const GG::XMLElement &elem):
    CUI_Wnd(elem.Child("CUI_Wnd")),
    m_ended_with_ok(false)
{
    using namespace GG;

    if (elem.Tag() != "GalaxySetupWnd")
        throw std::invalid_argument("Attempted to construct an GalaxySetupWnd from an XMLElement that had a tag other than \"GalaxySetupWnd\"");

    m_browse_button = new CUIButton(elem.Child("m_browse_button"));
    m_ok = new CUIButton(elem.Child("m_ok"));
    m_cancel = new CUIButton(elem.Child("m_cancel"));
    m_galaxy_file = new CUIEdit(elem.Child("m_galaxy_file"));
    m_size_buttons = new RadioButtonGroup(elem.Child("m_size_buttons"));
    m_type_buttons = new RadioButtonGroup(elem.Child("m_type_buttons"));
    m_preview_image = new StaticGraphic(elem.Child("m_preview_image"));

    Init();
}

GalaxySetupWnd::~GalaxySetupWnd()
{
}

bool GalaxySetupWnd::Render()
{
    CUI_Wnd::Render();
    GG::FlatRectangle(UpperLeft().x + PREVIEW_UL.x - 2, UpperLeft().y + PREVIEW_UL.y - 2, UpperLeft().x + PREVIEW_UL.x + PREVIEW_SZ.x + 2, 
                      UpperLeft().y + PREVIEW_UL.y + PREVIEW_SZ.y + 2, GG::CLR_BLACK, ClientUI::WND_INNER_BORDER_COLOR, 1);
    return true;
}

void GalaxySetupWnd::Keypress (GG::Key key, Uint32 key_mods)
{
    if (key == GG::GGK_RETURN) // Same behaviour as if "OK" was pressed
    {
      OnOK();
    }
    if (key == GG::GGK_ESCAPE) // Same behaviour as if "Cancel" was pressed
    {
      OnCancel();
    }
}

GG::XMLElement GalaxySetupWnd::XMLEncode() const
{
    using namespace GG;

    XMLElement retval("GalaxySetupWnd"), temp;

    const_cast<GalaxySetupWnd*>(this)->DetachSignalChildren();
    retval.AppendChild(CUI_Wnd::XMLEncode());
    const_cast<GalaxySetupWnd*>(this)->AttachSignalChildren();

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
}

int GalaxySetupWnd::Systems() const
{
    int retval = 0;
    switch (m_size_buttons->CheckedButton()) {
    case 0: retval = 50;  break;
    case 1: retval = 100; break;
    case 2: retval = 150; break;
    case 3: retval = 200; break;
    case 4: retval = 250; break;
    case 5: retval = 300; break;
    }
    return retval;
}

Universe::Shape GalaxySetupWnd::GalaxyShape() const
{
    return Universe::Shape(m_type_buttons->CheckedButton());
}

std::string GalaxySetupWnd::GalaxyFile() const
{
    return ((m_type_buttons->CheckedButton() == Universe::FROM_FILE) ? m_galaxy_file->WindowText() : "");
}

void GalaxySetupWnd::Init()
{
    AttachSignalChildren();

    GG::Connect(m_size_buttons->ButtonChangedSignal(), &GalaxySetupWnd::OnChangeSize, this);
    GG::Connect(m_type_buttons->ButtonChangedSignal(), &GalaxySetupWnd::OnChangeType, this);
    GG::Connect(m_browse_button->ClickedSignal(), &GalaxySetupWnd::OnBrowse, this);
    GG::Connect(m_ok->ClickedSignal(), &GalaxySetupWnd::OnOK, this);
    GG::Connect(m_cancel->ClickedSignal(), &GalaxySetupWnd::OnCancel, this);

    // create and load textures
    m_textures.clear();
    for (int i = 0; i < Universe::GALAXY_SHAPES; ++i)
        m_textures.push_back(boost::shared_ptr<GG::Texture>(new GG::Texture()));
    m_textures[Universe::SPIRAL_2]->Load(ClientUI::ART_DIR + "gp_spiral2.png");
    m_textures[Universe::SPIRAL_3]->Load(ClientUI::ART_DIR + "gp_spiral3.png");
    m_textures[Universe::SPIRAL_4]->Load(ClientUI::ART_DIR + "gp_spiral4.png");
    m_textures[Universe::CLUSTER]->Load(ClientUI::ART_DIR + "gp_cluster.png");
    m_textures[Universe::ELLIPTICAL]->Load(ClientUI::ART_DIR + "gp_elliptical.png");
    m_textures[Universe::IRREGULAR]->Load(ClientUI::ART_DIR + "gp_irregular.png");
	m_textures[Universe::RING]->Load(ClientUI::ART_DIR + "gp_ring.png");
    
    // default settings (medium and 2-arm spiral)
    m_size_buttons->SetCheck(2);
    m_type_buttons->SetCheck(Universe::SPIRAL_2);
}

void GalaxySetupWnd::AttachSignalChildren()
{
    AttachChild(m_size_buttons);
    AttachChild(m_type_buttons);
    AttachChild(m_galaxy_file);
    AttachChild(m_browse_button);
    AttachChild(m_preview_image);
    AttachChild(m_ok);
    AttachChild(m_cancel);    
}

void GalaxySetupWnd::DetachSignalChildren()
{
    DetachChild(m_size_buttons);
    DetachChild(m_type_buttons);
    DetachChild(m_galaxy_file);
    DetachChild(m_browse_button);
    DetachChild(m_preview_image);
    DetachChild(m_ok);
    DetachChild(m_cancel);    
}

void GalaxySetupWnd::OnOK()
{
    //check to see if we have a valid image if file is checked
    if (m_type_buttons->CheckedButton() == Universe::FROM_FILE) {
        try {
            boost::shared_ptr<GG::Texture> tex(new GG::Texture);
            tex->Load(GalaxyFile());
        } catch (const std::exception& e) {
            ClientUI::MessageBox("\"" + GalaxyFile() + "\" " + ClientUI::String("GSETUP_ERR_INVALID_GRAPHICS"));
            return;
        }
    }

    m_ended_with_ok = true;
    m_done = true;
}

void GalaxySetupWnd::OnCancel()
{
    m_ended_with_ok = false;
    m_done = true;
}

void GalaxySetupWnd::OnChangeSize(int index)
{
    // disable invalid galaxy shapes for the chosen galaxy size
    switch (index) {
    case 0:
        if (-1 < m_type_buttons->CheckedButton() && m_type_buttons->CheckedButton() <= Universe::SPIRAL_4)
            m_type_buttons->SetCheck(Universe::CLUSTER);
        m_type_buttons->DisableButton(Universe::SPIRAL_2);
        m_type_buttons->DisableButton(Universe::SPIRAL_3);
        m_type_buttons->DisableButton(Universe::SPIRAL_4);
        break;
    case 1:
        if (-1 < m_type_buttons->CheckedButton() && 
            (m_type_buttons->CheckedButton() == Universe::SPIRAL_3 ||
             m_type_buttons->CheckedButton() == Universe::SPIRAL_4))
            m_type_buttons->SetCheck(Universe::SPIRAL_2);
        m_type_buttons->DisableButton(Universe::SPIRAL_2, false);
        m_type_buttons->DisableButton(Universe::SPIRAL_3);
        m_type_buttons->DisableButton(Universe::SPIRAL_4);
        break;
    case 2:
        if (-1 < m_type_buttons->CheckedButton() && m_type_buttons->CheckedButton() == Universe::SPIRAL_4)
            m_type_buttons->SetCheck(Universe::SPIRAL_3);
        m_type_buttons->DisableButton(Universe::SPIRAL_2, false);
        m_type_buttons->DisableButton(Universe::SPIRAL_3, false);
        m_type_buttons->DisableButton(Universe::SPIRAL_4);
        break;
    default:
        m_type_buttons->DisableButton(Universe::SPIRAL_2, false);
        m_type_buttons->DisableButton(Universe::SPIRAL_3, false);
        m_type_buttons->DisableButton(Universe::SPIRAL_4, false);
        break;
    }
}

void GalaxySetupWnd::OnChangeType(int index)
{
    if (m_preview_image) {
        DeleteChild(m_preview_image);
        m_preview_image = 0;
    }
    if (m_type_buttons->CheckedButton() == Universe::FROM_FILE) {
        try {
            boost::shared_ptr<GG::Texture> tex(new GG::Texture());
            tex->Load(GalaxyFile());
            m_preview_image = new GG::StaticGraphic(PREVIEW_UL.x, PREVIEW_UL.y, PREVIEW_SZ.x, PREVIEW_SZ.y, tex, GG::GR_FITGRAPHIC);
            AttachChild(m_preview_image);
        } catch (const std::exception& e) {
            GG::App::GetApp()->Logger().alert("GalaxySetupWnd: Invalid texture file specified.");
        }
        m_galaxy_file->Disable(false);
        m_browse_button->Disable(false);
    } else {
        if (index != -1) {
            m_preview_image = new GG::StaticGraphic(PREVIEW_UL.x, PREVIEW_UL.y, PREVIEW_SZ.x, PREVIEW_SZ.y, m_textures[index], GG::GR_FITGRAPHIC);
            AttachChild(m_preview_image);
        }
        m_galaxy_file->Disable();
        m_browse_button->Disable();
    }
}

void GalaxySetupWnd::OnBrowse()
{
    // filter for graphics files
    std::vector<std::pair<std::string, std::string> > allowed_file_types;
    allowed_file_types.push_back(std::pair<std::string, std::string>(ClientUI::String("GSETUP_GRAPHICS_FILES") + " (PNG, Targa, JPG, BMP)", 
                                                                     "*.png, *.PNG, *.tga, *.TGA, *.jpg, *.JPG, *.bmp, *.BMP"));

    GG::FileDlg dlg("", GalaxyFile(), false, false, allowed_file_types, ClientUI::FONT, ClientUI::PTS,
                    ClientUI::WND_COLOR, ClientUI::WND_OUTER_BORDER_COLOR, ClientUI::TEXT_COLOR);

    dlg.Run();    
    if (!dlg.Result().empty()) {
        m_galaxy_file->SetText(*dlg.Result().begin());
        m_type_buttons->SetCheck(-1);
        m_type_buttons->SetCheck(Universe::FROM_FILE);
    }
}
