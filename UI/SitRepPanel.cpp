#include "SitRepPanel.h"
 
#include "../client/human/HumanClientApp.h"
#include "CUIControls.h"
#include "LinkText.h"
#include "Sound.h"
#include "../util/MultiplayerCommon.h"
#include "../Empire/Empire.h"

#include <GG/DrawUtil.h>


namespace {
    const int    SITREP_LB_MARGIN_X = 5;
    const int    SITREP_LB_MARGIN_Y = 5;
}


SitRepPanel::SitRepPanel(int x, int y, int w, int h) : 
    CUIWnd(UserString("SITREP_PANEL_TITLE"), x, y, w, h, GG::ONTOP | GG::CLICKABLE | GG::DRAGABLE | GG::RESIZABLE | CLOSABLE)
{
    Sound::TempUISoundDisabler sound_disabler;
    m_sitreps_lb = new CUIListBox(SITREP_LB_MARGIN_X, SITREP_LB_MARGIN_Y,
                                  ClientWidth() - SITREP_LB_MARGIN_X, ClientHeight() - SITREP_LB_MARGIN_Y);
    m_sitreps_lb->SetStyle(GG::LIST_NOSORT);

    AttachChild(m_sitreps_lb);
    EnableChildClipping(false);

    Hide();
}

void SitRepPanel::KeyPress (GG::Key key, GG::Flags<GG::ModKey> mod_keys)
{
    switch (key) {
    case GG::GGK_RETURN:
    case GG::GGK_KP_ENTER:
    case GG::GGK_ESCAPE:
    case GG::GGK_F2: {
        Hide();
        break;
    }
    default:
        break;
    }
}

void SitRepPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    GG::Pt old_size = GG::Wnd::LowerRight() - GG::Wnd::UpperLeft();

    CUIWnd::SizeMove(ul, lr);
    m_sitreps_lb->SizeMove(GG::Pt(SITREP_LB_MARGIN_X, SITREP_LB_MARGIN_Y),
                           GG::Pt(ClientWidth() - SITREP_LB_MARGIN_X, ClientHeight() - SITREP_LB_MARGIN_Y));
    if (Visible() && old_size != GG::Wnd::Size())
        Update();
}

void SitRepPanel::OnClose()
{
    Hide();
}

void SitRepPanel::CloseClicked()
{
    ClosingSignal();
}

void SitRepPanel::Update()
{
    Empire *empire = HumanClientApp::GetApp()->Empires().Lookup(HumanClientApp::GetApp()->EmpireID());

    if (!empire)
        return;

    m_sitreps_lb->Clear();

    boost::shared_ptr<GG::Font> font = GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts());
    GG::Flags<GG::TextFormat> format = GG::FORMAT_LEFT | GG::FORMAT_WORDBREAK;
    int width = m_sitreps_lb->Width() - 8;

    // loop through sitreps and display
    for (Empire::SitRepItr sitrep_it = empire->SitRepBegin(); sitrep_it != empire->SitRepEnd(); ++sitrep_it) {
        LinkText* link_text = new LinkText(0, 0, width, (*sitrep_it)->GetText(), font, format, ClientUI::TextColor());
        GG::ListBox::Row *row = new GG::ListBox::Row(link_text->Width(), link_text->Height(), "");
        row->push_back(link_text);
        m_sitreps_lb->Insert(row);
    }
}
