//CUIControls.cpp

#include "CUIControls.h"

#include "CUIDrawUtil.h"
#include "CUISpin.h"
#include "GGApp.h"
#include "GGDrawUtil.h"
#include "GGStaticGraphic.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"

#include <boost/lexical_cast.hpp>

namespace {
    bool PlaySounds()
    {
        return GetOptionsDB().Get<bool>("UI.sound.enabled");
    }

    void PlayButtonClickSound()
    {
        if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.button-click"));
    }

    void PlayTurnButtonClickSound()
    {
        if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.turn-button-click"));
    }

    struct PlayButtonCheckSound
    {
        PlayButtonCheckSound(bool play_only_when_checked) : m_play_only_when_checked(play_only_when_checked) {}
        void operator()(bool checked) const
        {
            if ((!m_play_only_when_checked || checked) && PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.button-click"));
        }
        const bool m_play_only_when_checked;
    };

    void PlayListSelectSound(const std::set<int>&)
    {
        if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.list-select"));
    }

    void PlayItemDropSound(int, const boost::shared_ptr<GG::ListBox::Row>&)
    {
        if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.item-drop"));
    }

    void PlayTextTypingSound(const std::string&)
    {
        if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.text-typing"));
    }

    bool temp_header_bool = RecordHeaderFile(CUIControlsRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");

    bool temp_header_bool2 = RecordHeaderFile(CUISpinRevision());
}

#if 0
// TODO: finish implementing these sounds, as recommended by the sound team (the ones commented out are done):
//1) Button Click - includes clicks on regular buttons, check boxes, radio buttons
//2) Button Rollover - includes moving the mouse over regular buttons only
//3) Item Select - selecting/deselecting an item in a drop-down list or listbox
//4) Item Drop - dropping a drag-and-drop item into a listbox
//5) Alert - when a warning or error message box pops up
//6) Minimize window
//7) Maximize window
//8) Close window
//9) Turn Button Click (specialization of Button Click)
//9) Fleet Rollover (specialization of Button Rollover)
//10) Fleet Click (specialization of Button Click)
//11) Planet Click (specialization of Button Click)
//12) Text Typing
13) Planet Background Music (music, instead of sound effect, for the planet detail view)
14) Fleet-Type Sound (plays sound specific to the type of fleet in the fleet detail view)
15) Turn Progressing
#endif

///////////////////////////////////////
// class CUIButton
///////////////////////////////////////
namespace {
    const int CUIBUTTON_ANGLE_OFFSET = 5;
    const int COLOR_SELECTOR_WIDTH = 75;
    const int COLOR_SQUARE_HEIGHT = 10;

    // row type used in the EmpireColorSelector
    struct ColorRow : public GG::ListBox::Row
    {
        struct ColorSquare : GG::Control
        {
            ColorSquare(const GG::Clr& color, int h) : 
                GG::Control(0, 0, COLOR_SELECTOR_WIDTH - 40, h, 0)
            {
                SetColor(color);
            }

            virtual bool Render()
            {
                GG::Pt ul = UpperLeft(), lr = LowerRight();
                GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, Color(), GG::CLR_ZERO, 0);
                return true;
            }
        };

        ColorRow(const GG::Clr& color, int h = COLOR_SQUARE_HEIGHT)
        {
            push_back(new ColorSquare(color, h));
        }
    };
}

CUIButton::CUIButton(int x, int y, int w, const std::string& str, const std::string& font_filename/* = ClientUI::FONT*/, 
                     int pts/* = ClientUI::PTS*/, GG::Clr color/* = ClientUI::BUTTON_COLOR*/, 
                     GG::Clr border/* = ClientUI::CTRL_BORDER_COLOR*/, int thick/* = 2*/, 
                     GG::Clr text_color/* = ClientUI::TEXT_COLOR*/, Uint32 flags/* = GG::Wnd::CLICKABLE*/) : 
    Button(x, y, w, GG::App::GetApp()->GetFont(font_filename, pts)->Lineskip() + 6, str, font_filename, pts, 
           color, text_color, flags),
    m_border_color(border),
    m_border_thick(thick)
{
    GG::Connect(ClickedSignal(), &PlayButtonClickSound, -1);
}

CUIButton::CUIButton(const GG::XMLElement& elem) : 
    Button(elem.Child("GG::Button"))
{
    if (elem.Tag() != "CUIButton")
        throw std::invalid_argument("Attempted to construct a CUIButton from an XMLElement that had a tag other than \"CUIButton\"");

    const GG::XMLElement* curr_elem = &elem.Child("m_border_color");
    m_border_color = GG::Clr(curr_elem->Child("GG::Clr"));

    curr_elem = &elem.Child("m_border_thick");
    m_border_thick = boost::lexical_cast<int>(curr_elem->Attribute("value"));

    GG::Connect(ClickedSignal(), &PlayButtonClickSound, -1);
}

bool CUIButton::InWindow(const GG::Pt& pt) const
{
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    return InAngledCornerRect(pt, ul.x, ul.y, lr.x, lr.y, CUIBUTTON_ANGLE_OFFSET);
}

GG::XMLElement CUIButton::XMLEncode() const
{
    GG::XMLElement retval("CUIButton");
    retval.AppendChild(GG::Button::XMLEncode());

    GG::XMLElement temp("m_border_color");
    temp.AppendChild(m_border_color.XMLEncode());
    retval.AppendChild(temp);

    temp = GG::XMLElement("m_border_thick");
    temp.SetAttribute("value", boost::lexical_cast<std::string>(m_border_thick));
    retval.AppendChild(temp);

    return retval;
}

void CUIButton::MouseHere(const GG::Pt& pt, Uint32 keys)
{
    if (!Disabled()) {
        if (State() != BN_ROLLOVER && PlaySounds())
            HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.button-rollover"));
        SetState(BN_ROLLOVER);
    }
}

void CUIButton::RenderPressed()
{
    GG::Clr color_to_use = Color();
    AdjustBrightness(color_to_use, 25);
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    AngledCornerRectangle(ul.x, ul.y, lr.x, lr.y, color_to_use, m_border_color, CUIBUTTON_ANGLE_OFFSET, m_border_thick);
    OffsetMove(1,1);
    TextControl::Render();
    OffsetMove(-1,-1);
}

void CUIButton::RenderRollover()
{
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    GG::Clr border_color_to_use = m_border_color;
    AdjustBrightness(border_color_to_use, 100);
    if (Disabled())
        border_color_to_use = DisabledColor(m_border_color);
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    AngledCornerRectangle(ul.x, ul.y, lr.x, lr.y, color_to_use, border_color_to_use, CUIBUTTON_ANGLE_OFFSET, m_border_thick);
    TextControl::Render();
}

void CUIButton::RenderUnpressed()
{
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    GG::Clr border_color_to_use = Disabled() ? DisabledColor(m_border_color) : m_border_color;
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    AngledCornerRectangle(ul.x, ul.y, lr.x, lr.y, color_to_use, border_color_to_use, CUIBUTTON_ANGLE_OFFSET, m_border_thick);
    TextControl::Render();
}


///////////////////////////////////////
// class CUITurnButton
///////////////////////////////////////
CUITurnButton::CUITurnButton(int x, int y, int w, const std::string& str, const std::string& font_filename/* = ClientUI::FONT*/, 
                     int pts/* = ClientUI::PTS*/, GG::Clr color/* = ClientUI::BUTTON_COLOR*/, 
                     GG::Clr border/* = ClientUI::CTRL_BORDER_COLOR*/, int thick/* = 2*/, 
                     GG::Clr text_color/* = ClientUI::TEXT_COLOR*/, Uint32 flags/* = GG::Wnd::CLICKABLE*/) : 
    CUIButton(x, y, w, str, font_filename, pts, color, border, thick, text_color, flags)
{
    GG::Connect(ClickedSignal(), &PlayTurnButtonClickSound, -1);
}

CUITurnButton::CUITurnButton(const GG::XMLElement& elem) : 
    CUIButton(elem.Child("CUIButton"))
{
    if (elem.Tag() != "CUITurnButton")
        throw std::invalid_argument("Attempted to construct a CUITurnButton from an XMLElement that had a tag other than \"CUITurnButton\"");

    GG::Connect(ClickedSignal(), &PlayTurnButtonClickSound, -1);
}

GG::XMLElement CUITurnButton::XMLEncode() const
{
    GG::XMLElement retval("CUITurnButton");
    retval.AppendChild(CUIButton::XMLEncode());
    return retval;
}


///////////////////////////////////////
// class CUIArrowButton
///////////////////////////////////////
CUIArrowButton::CUIArrowButton(int x, int y, int w, int h, ShapeOrientation orientation, GG::Clr color, Uint32 flags/* = GG::Wnd::CLICKABLE*/) :
    Button(x, y, w, h, "", "", 0, color, GG::CLR_ZERO, flags),
    m_orientation(orientation)
{
    GG::Connect(ClickedSignal(), &PlayButtonClickSound, -1);
}

CUIArrowButton::CUIArrowButton(const GG::XMLElement& elem) : 
    Button(elem.Child("GG::Button"))
{
    if (elem.Tag() != "CUIArrowButton")
        throw std::invalid_argument("Attempted to construct a CUIArrowButton from an XMLElement that had a tag other than \"CUIArrowButton\"");

    const GG::XMLElement* curr_elem = &elem.Child("m_orientation");
    m_orientation = ShapeOrientation(boost::lexical_cast<int>(curr_elem->Attribute("value")));

    GG::Connect(ClickedSignal(), &PlayButtonClickSound, -1);
}

bool CUIArrowButton::InWindow(const GG::Pt& pt) const
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    return InIsoscelesTriangle(pt, ul.x, ul.y, lr.x, lr.y, m_orientation);
}

GG::XMLElement CUIArrowButton::XMLEncode() const
{
    GG::XMLElement retval("CUIArrowButton");
    retval.AppendChild(GG::Button::XMLEncode());

    GG::XMLElement temp("m_orientation");
    temp.SetAttribute("value", boost::lexical_cast<std::string>(m_orientation));
    retval.AppendChild(temp);

    return retval;
}

void CUIArrowButton::MouseHere(const GG::Pt& pt, Uint32 keys)
{
    if (!Disabled()) {
        if (State() != BN_ROLLOVER && PlaySounds())
            HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.button-rollover"));
        SetState(BN_ROLLOVER);
    }
}

void CUIArrowButton::RenderPressed()
{
    OffsetMove(1, 1);
    RenderUnpressed();
    OffsetMove(-1, -1);
}

void CUIArrowButton::RenderRollover()
{
    RenderUnpressed();
}

void CUIArrowButton::RenderUnpressed()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    IsoscelesTriangle(ul.x, ul.y, lr.x, lr.y, m_orientation, color_to_use);
}


///////////////////////////////////////
// class CUIStateButton
///////////////////////////////////////
CUIStateButton::CUIStateButton(int x, int y, int w, int h, const std::string& str, Uint32 text_fmt, Uint32 style/* = SBSTYLE_CUI_CHECKBOX*/,
                               GG::Clr color/* = ClientUI::STATE_BUTTON_COLOR*/, const std::string& font_filename/* = ClientUI::FONT*/,
                               int pts/* = ClientUI::PTS*/, GG::Clr text_color/* = ClientUI::TEXT_COLOR*/, GG::Clr interior/* = CLR_ZERO*/,
                               GG::Clr border/* = ClientUI::CTRL_BORDER_COLOR*/, int bn_x/* = -1*/, int bn_y/* = -1*/,
                               int bn_w/* = -1*/, int bn_h/* = -1*/, Uint32 flags/* = CLICKABLE*/) : 
    StateButton(x, y, w, h, str, font_filename, pts, text_fmt, color, text_color, interior, StateButtonStyle(style), 
                bn_x, bn_y, bn_w, bn_h, flags),
    m_border_color(border)
{
    // HACK! radio buttons should only emit sounds when they are checked, and *not* when they are unchecked; currently, there's no 
    // other way to detect the difference between these two kinds of CUIStateButton within the CUIStateButton ctor other than
    // checking the redering style
    GG::Connect(CheckedSignal(), PlayButtonCheckSound(style == CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON), -1);
}

CUIStateButton::CUIStateButton(const GG::XMLElement& elem) : 
    StateButton(elem.Child("GG::StateButton"))
{
    if (elem.Tag() != "CUIStateButton")
        throw std::invalid_argument("Attempted to construct a CUIStateButton from an XMLElement that had a tag other than \"CUIStateButton\"");

    const GG::XMLElement* curr_elem = &elem.Child("m_border_color");
    m_border_color = GG::Clr(curr_elem->Child("GG::Clr"));

    // HACK! radio buttons should only emit sounds when they are checked, and *not* when they are unchecked; currently, there's no 
    // other way to detect the difference between these two kinds of CUIStateButton within the CUIStateButton ctor other than
    // checking the redering style
    GG::Connect(CheckedSignal(), PlayButtonCheckSound(StateButton::Style() == CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON), -1);
}

GG::XMLElement CUIStateButton::XMLEncode() const
{
    GG::XMLElement retval("CUIStateButton");
    retval.AppendChild(StateButton::XMLEncode());

    GG::XMLElement temp("m_border_color");
    temp.AppendChild(m_border_color.XMLEncode());
    retval.AppendChild(temp);

    return retval;
}

bool CUIStateButton::Render()
{
    if (static_cast<int>(Style()) == SBSTYLE_CUI_CHECKBOX || 
        static_cast<int>(Style()) == SBSTYLE_CUI_RADIO_BUTTON) {
        // draw button
        GG::Pt bn_ul = UpperLeft() + GG::Pt(ButtonX(), ButtonY());
        GG::Pt bn_lr = bn_ul + GG::Pt(ButtonWd(), ButtonHt());
        GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
        GG::Clr int_color_to_use = Disabled() ? DisabledColor(InteriorColor()) : InteriorColor();
        GG::Clr border_color_to_use = Disabled() ? DisabledColor(BorderColor()) : BorderColor();

        if (static_cast<int>(Style()) == SBSTYLE_CUI_CHECKBOX) {
            const int MARGIN = 3;
            FlatRectangle(bn_ul.x, bn_ul.y, bn_lr.x, bn_lr.y, int_color_to_use, border_color_to_use, 1);
            if (Checked()) {
                GG::Clr inside_color = color_to_use;
                GG::Clr outside_color = color_to_use;
                AdjustBrightness(outside_color, 50);
                bn_ul += GG::Pt(MARGIN, MARGIN);
                bn_lr -= GG::Pt(MARGIN, MARGIN);
                const int OFFSET = (bn_lr.y - bn_ul.y) / 2;
                glDisable(GL_TEXTURE_2D);
                glColor4ubv(inside_color.v);
                glBegin(GL_QUADS);
                glVertex2i(bn_lr.x, bn_ul.y);
                glVertex2i(bn_ul.x + OFFSET, bn_ul.y);
                glVertex2i(bn_ul.x, bn_ul.y + OFFSET);
                glVertex2i(bn_ul.x, bn_lr.y);
                glVertex2i(bn_ul.x, bn_lr.y);
                glVertex2i(bn_lr.x - OFFSET, bn_lr.y);
                glVertex2i(bn_lr.x, bn_lr.y - OFFSET);
                glVertex2i(bn_lr.x, bn_ul.y);
                glEnd();
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glColor4ubv(outside_color.v);
                glBegin(GL_POLYGON);
                glVertex2i(bn_lr.x, bn_ul.y);
                glVertex2i(bn_ul.x + OFFSET, bn_ul.y);
                glVertex2i(bn_ul.x, bn_ul.y + OFFSET);
                glVertex2i(bn_ul.x, bn_lr.y);
                glVertex2i(bn_ul.x, bn_lr.y);
                glVertex2i(bn_lr.x - OFFSET, bn_lr.y);
                glVertex2i(bn_lr.x, bn_lr.y - OFFSET);
                glVertex2i(bn_lr.x, bn_ul.y);
                glEnd();
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glEnable(GL_TEXTURE_2D);
            } else {
                GG::Clr inside_color = border_color_to_use;
                AdjustBrightness(inside_color, -75);
                GG::Clr outside_color = inside_color;
                AdjustBrightness(outside_color, 40);
                glTranslated((bn_ul.x + bn_lr.x) / 2.0, -(bn_ul.y + bn_lr.y) / 2.0, 0.0);
                glScaled(-1.0, 1.0, 1.0);
                glTranslated(-(bn_ul.x + bn_lr.x) / 2.0, (bn_ul.y + bn_lr.y) / 2.0, 0.0);
                AngledCornerRectangle(bn_ul.x + MARGIN, bn_ul.y + MARGIN, bn_lr.x - MARGIN, bn_lr.y - MARGIN, 
                                      inside_color, outside_color, (bn_lr.y - bn_ul.y - 2 * MARGIN) / 2, 1);
                glTranslated((bn_ul.x + bn_lr.x) / 2.0, -(bn_ul.y + bn_lr.y) / 2.0, 0.0);
                glScaled(-1.0, 1.0, 1.0);
                glTranslated(-(bn_ul.x + bn_lr.x) / 2.0, (bn_ul.y + bn_lr.y) / 2.0, 0.0);
            }
        } else if (static_cast<int>(Style()) == SBSTYLE_CUI_RADIO_BUTTON) {
            const int MARGIN = 2;
            FlatCircle(bn_ul.x, bn_ul.y, bn_lr.x, bn_lr.y, int_color_to_use, border_color_to_use, 1);
            if (Checked()) {
                GG::Clr inside_color = color_to_use;
                GG::Clr outside_color = color_to_use;
                AdjustBrightness(outside_color, 50);
                FlatCircle(bn_ul.x + MARGIN, bn_ul.y + MARGIN, bn_lr.x - MARGIN, bn_lr.y - MARGIN, GG::CLR_ZERO, 
                           outside_color, 1);
                FlatCircle(bn_ul.x + MARGIN + 1, bn_ul.y + MARGIN + 1, bn_lr.x - MARGIN - 1, bn_lr.y - MARGIN - 1, 
                           inside_color, outside_color, 1);
            } else {
                GG::Clr inside_color = border_color_to_use;
                AdjustBrightness(inside_color, -75);
                GG::Clr outside_color = inside_color;
                AdjustBrightness(outside_color, 40);
                FlatCircle(bn_ul.x + MARGIN, bn_ul.y + MARGIN, bn_lr.x - MARGIN, bn_lr.y - MARGIN, inside_color, 
                           outside_color, 1);
            }
        }
        // draw text
        OffsetMove(TextX(), TextY());
        TextControl::Render();
        OffsetMove(-TextX(), -TextY());
    } else {
        StateButton::Render();
    }
    return true;
}


///////////////////////////////////////
// class CUIScroll
///////////////////////////////////////
namespace {
const int CUISCROLL_ANGLE_OFFSET = 3;
}

///////////////////////////////////////
// class CUIScroll::ScrollTab
CUIScroll::ScrollTab::ScrollTab(GG::Scroll::Orientation orientation, int scroll_width, GG::Clr color, 
                                GG::Clr border_color) : 
    Button(0, 2, scroll_width, scroll_width, "", "", 0, color),
    m_border_color(border_color)
{
    SetMinSize(GG::Pt(orientation == GG::Scroll::VERTICAL ? MinSize().x : 10, orientation == GG::Scroll::VERTICAL ? 10 : MinSize().y));
}

CUIScroll::ScrollTab::ScrollTab(const GG::XMLElement& elem) : 
    Button(elem.Child("GG::Button"))
{
    if (elem.Tag() != "CUIScroll::ScrollTab")
        throw std::invalid_argument("Attempted to construct a CUIScroll::ScrollTab from an XMLElement that had a tag other than \"CUIScroll::ScrollTab\"");
    m_border_color = GG::Clr(elem.Child("m_border_color").Child("GG::Clr"));
}

GG::XMLElement CUIScroll::ScrollTab::XMLEncode() const
{
    GG::XMLElement retval("CUIScroll::ScrollTab");
    retval.AppendChild(Button::XMLEncode());

    GG::XMLElement temp("m_border_color");
    temp.AppendChild(m_border_color.XMLEncode());
    retval.AppendChild(temp);

    return retval;
}

bool CUIScroll::ScrollTab::Render()
{
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    ul.x += 3;
    lr.x -= 3;
    // basic shape, no border
    AngledCornerRectangle(ul.x, ul.y, lr.x, lr.y, Color(), GG::CLR_ZERO, CUISCROLL_ANGLE_OFFSET, 0);
   // upper left diagonal stripe
    GG::Clr light_color = Color();
    AdjustBrightness(light_color, 35);
    glColor4ubv(light_color.v);
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_POLYGON);
    glVertex2i(lr.x, ul.y);
    glVertex2i(ul.x + CUISCROLL_ANGLE_OFFSET, ul.y);
    glVertex2i(ul.x, ul.y + CUISCROLL_ANGLE_OFFSET);
    glVertex2i(ul.x, ul.y + (lr.x - ul.x));
    glEnd();
    // lower right diagonal stripe
    glBegin(GL_POLYGON);
    glVertex2i(lr.x, lr.y - (lr.x - ul.x));
    glVertex2i(ul.x, lr.y);
    glVertex2i(lr.x - CUISCROLL_ANGLE_OFFSET, lr.y);
    glVertex2i(lr.x, lr.y - CUISCROLL_ANGLE_OFFSET);
    glEnd();
    glEnable(GL_TEXTURE_2D);
    // border
    AngledCornerRectangle(ul.x, ul.y, lr.x, lr.y, GG::CLR_ZERO, m_border_color, CUISCROLL_ANGLE_OFFSET, 1);
    return true;
}


///////////////////////////////////////
// class CUIScroll
///////////////////////////////////////
CUIScroll::CUIScroll(int x, int y, int w, int h, GG::Scroll::Orientation orientation, GG::Clr color/* = GG::CLR_ZERO*/, 
                     GG::Clr border/* = ClientUI::CTRL_BORDER_COLOR*/, GG::Clr interior/* = GG::CLR_ZERO*/, 
                     Uint32 flags/* = CLICKABLE*/) : 
    Scroll(x, y, w, h, orientation, color, interior, NewDummyButton(), NewDummyButton(), 
           new ScrollTab(orientation, orientation == VERTICAL ? w : h, 
                         (color == GG::CLR_ZERO) ? ClientUI::SCROLL_TAB_COLOR : color, border), 
               flags),
    m_border_color(border)
{
}

CUIScroll::CUIScroll(const GG::XMLElement& elem) : 
    Scroll(elem.Child("GG::Scroll"))
{
    if (elem.Tag() != "CUIScroll")
        throw std::invalid_argument("Attempted to construct a CUIScroll from an XMLElement that had a tag other than \"CUIScroll\"");
    m_border_color = GG::Clr(elem.Child("m_border_color").Child("GG::Clr"));
}

GG::XMLElement CUIScroll::XMLEncode() const
{
    GG::XMLElement retval("CUIScroll");
    retval.AppendChild(Scroll::XMLEncode());

    GG::XMLElement temp("m_border_color");
    temp.AppendChild(m_border_color.XMLEncode());
    retval.AppendChild(temp);

    return retval;
}

bool CUIScroll::Render()
{
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    GG::Clr border_color_to_use = Disabled() ? DisabledColor(m_border_color) : m_border_color;
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    FlatRectangle(ul.x, ul.y, lr.x, lr.y, color_to_use, border_color_to_use, 1);
    if (!Disabled()) { // hide tab if disabled
        TabButton()->OffsetMove(UpperLeft());
        TabButton()->Render();
        TabButton()->OffsetMove(-UpperLeft());
    }
    return true;
}

void CUIScroll::SizeMove(int x1, int y1, int x2, int y2)
{
    Wnd::SizeMove(x1, y1, x2, y2);
    int bn_width = (ScrollOrientation() == VERTICAL) ? Size().x : Size().y;
    TabButton()->SizeMove(TabButton()->UpperLeft(), 
                          (ScrollOrientation() == VERTICAL) ? GG::Pt(bn_width, TabButton()->LowerRight().y) :
                          GG::Pt(TabButton()->LowerRight().x, bn_width));
    SizeScroll(ScrollRange().first, ScrollRange().second, LineSize(), PageSize()); // update tab size and position
}

GG::Button* CUIScroll::NewDummyButton()
{
    return new GG::Button(-2, -2, 2, 2, "", "", 0, GG::CLR_ZERO);
}


///////////////////////////////////////
// class CUIListBox
///////////////////////////////////////
CUIListBox::CUIListBox(int x, int y, int w, int h, GG::Clr color/* = ClientUI::CTRL_BORDER_COLOR*/, 
                       GG::Clr interior/* = GG::CLR_ZERO*/, Uint32 flags/* = CLICKABLE | DRAG_KEEPER*/) : 
    ListBox(x, y, w, h, color, interior, flags)
{
    RecreateScrolls();
    EnableChildClipping(false); // this is already done by GG::ListBox, and setting this would interfere
    GG::Connect(SelChangedSignal(), &PlayListSelectSound, -1);
    GG::Connect(DroppedSignal(), &PlayItemDropSound, -1);
}

CUIListBox::CUIListBox(int x, int y, int w, int h, const std::vector<int>& col_widths, 
                       GG::Clr color/* = ClientUI::CTRL_BORDER_COLOR*/, GG::Clr interior/* = GG::CLR_ZERO*/, 
                       Uint32 flags/* = CLICKABLE | DRAG_KEEPER*/) : 
    ListBox(x, y, w, h, color, col_widths, interior, flags)
{
    RecreateScrolls();
    EnableChildClipping(false); // this is already done by GG::ListBox, and setting this would interfere
}

CUIListBox::CUIListBox(const GG::XMLElement& elem) : 
    ListBox(elem.Child("GG::ListBox"))
{
    RecreateScrolls();
    EnableChildClipping(false); // this is already done by GG::ListBox, and setting this would interfere
}

GG::XMLElement CUIListBox::XMLEncode() const
{
    GG::XMLElement retval("CUIListBox");
    retval.AppendChild(ListBox::XMLEncode());
    return retval;
}

bool CUIListBox::Render()
{
    GG::Clr color = Color(); // save color
    SetColor(GG::CLR_ZERO); // disable the default border by rendering it transparently
    ListBox::Render();
    SetColor(color); // restore color
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    FlatRectangle(ul.x, ul.y, lr.x, lr.y, GG::CLR_ZERO, color_to_use, 1);
    return false;
}

GG::Scroll* CUIListBox::NewVScroll(bool horz_scroll)
{
    GG::Pt cl_sz = ((LowerRight() - GG::Pt(BORDER_THICK, BORDER_THICK)) -
                    (UpperLeft() + GG::Pt(BORDER_THICK, BORDER_THICK + (ColHeaders().size() ? ColHeaders().Height() : 0))));
    return new CUIScroll(cl_sz.x - ClientUI::SCROLL_WIDTH, 0, ClientUI::SCROLL_WIDTH, 
                         cl_sz.y - (horz_scroll ? ClientUI::SCROLL_WIDTH : 0), GG::Scroll::VERTICAL);
}

GG::Scroll* CUIListBox::NewHScroll(bool vert_scroll)
{
    GG::Pt cl_sz = ((LowerRight() - GG::Pt(BORDER_THICK, BORDER_THICK)) -
                    (UpperLeft() + GG::Pt(BORDER_THICK, BORDER_THICK + (ColHeaders().size() ? ColHeaders().Height() : 0))));
    return new CUIScroll(0, cl_sz.y - ClientUI::SCROLL_WIDTH, cl_sz.x - (vert_scroll ? ClientUI::SCROLL_WIDTH : 0), 
                         ClientUI::SCROLL_WIDTH, GG::Scroll::HORIZONTAL);
}


///////////////////////////////////////
// class CUIDropDownList
///////////////////////////////////////
namespace {
const int CUIDROPDOWNLIST_ANGLE_OFFSET = 5;
}

CUIDropDownList::CUIDropDownList(int x, int y, int w, int row_ht, int drop_ht, GG::Clr color/* = ClientUI::CTRL_BORDER_COLOR*/,
                                 GG::Clr interior/* = ClientUI::DROP_DOWN_LIST_INT_COLOR*/, 
                                 GG::Clr drop_list_interior/* = ClientUI::DROP_DOWN_LIST_INT_COLOR*/, Uint32 flags/* = CLICKABLE*/) : 
    DropDownList(x, y, w, row_ht, drop_ht, color, interior, new CUIListBox(x, y, w, drop_ht, color, drop_list_interior, flags)),
    m_render_drop_arrow(true),
    m_interior_color(interior)
{
}

CUIDropDownList::CUIDropDownList(const GG::XMLElement& elem) : 
    DropDownList(elem.Child("GG::DropDownList"))
{
    if (elem.Tag() != "CUIDropDownList")
        throw std::invalid_argument("Attempted to construct a CUIDropDownList from an XMLElement that had a tag other than \"CUIDropDownList\"");

    m_render_drop_arrow = boost::lexical_cast<bool>(elem.Child("m_render_drop_arrow").Text());
    m_interior_color = GG::Clr(elem.Child("m_interior_color"));
}

GG::XMLElement CUIDropDownList::XMLEncode() const
{
    GG::XMLElement retval("CUIDropDownList");
    retval.AppendChild(DropDownList::XMLEncode());
    retval.AppendChild(GG::XMLElement("m_render_drop_arrow", boost::lexical_cast<std::string>(m_render_drop_arrow)));
    retval.AppendChild(m_interior_color.XMLEncode());
    return retval;
}

bool CUIDropDownList::Render()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Clr lb_color = LB()->Color();
    GG::Clr lb_interior_color = LB()->InteriorColor();
    GG::Clr color_to_use = Disabled() ? DisabledColor(lb_color) : lb_color;
    GG::Clr int_color_to_use = Disabled() ? DisabledColor(m_interior_color) : m_interior_color;

    AngledCornerRectangle(ul.x, ul.y, lr.x, lr.y, int_color_to_use, GG::CLR_ZERO, CUIDROPDOWNLIST_ANGLE_OFFSET, 3, false);

    LB()->SetColor(GG::CLR_ZERO);
    LB()->SetInteriorColor(GG::CLR_ZERO);
    DropDownList::Render();
    LB()->SetInteriorColor(lb_interior_color);
    LB()->SetColor(lb_color);

    AngledCornerRectangle(ul.x, ul.y, lr.x, lr.y, GG::CLR_ZERO, color_to_use, CUIDROPDOWNLIST_ANGLE_OFFSET, 1, false);

    int margin = 3;
    int triangle_width = lr.y - ul.y - 4 * margin;
    int outline_width = triangle_width + 3 * margin;

    if (m_render_drop_arrow) {
        IsoscelesTriangle(lr.x - triangle_width - margin * 5 / 2, ul.y + 2 * margin, lr.x - margin * 5 / 2, lr.y - 2 * margin, 
                          SHAPE_DOWN, ClientUI::DROP_DOWN_LIST_ARROW_COLOR);
    }

    AngledCornerRectangle(lr.x - outline_width - margin, ul.y + margin, lr.x - margin, lr.y - margin, GG::CLR_ZERO, 
                          color_to_use, CUIDROPDOWNLIST_ANGLE_OFFSET, 1, false);

    return true;
}

void CUIDropDownList::DisableDropArrow()
{
    m_render_drop_arrow = false;
}

void CUIDropDownList::EnableDropArrow()
{
    m_render_drop_arrow = true;
}



///////////////////////////////////////
// class CUIEdit
///////////////////////////////////////
CUIEdit::CUIEdit(int x, int y, int w, int h, const std::string& str, const std::string& font_filename/* = ClientUI::FONT*/, 
                 int pts/* = ClientUI::PTS*/, GG::Clr color/* = ClientUI::CTRL_BORDER_COLOR*/, 
                 GG::Clr text_color/* = ClientUI::TEXT_COLOR*/, GG::Clr interior/* = ClientUI::EDIT_INT_COLOR*/, 
                 Uint32 flags/* = CLICKABLE | DRAG_KEEPER*/) : 
    Edit(x, y, w, h, str, font_filename, pts, color, text_color, interior, flags)
{
    GG::Connect(EditedSignal(), &PlayTextTypingSound, -1);
}

CUIEdit::CUIEdit(const GG::XMLElement& elem) : 
    Edit(elem.Child("GG::Edit"))
{
    if (elem.Tag() != "CUIEdit")
        throw std::invalid_argument("Attempted to construct a CUIEdit from an XMLElement that had a tag other than \"CUIEdit\"");

    GG::Connect(EditedSignal(), &PlayTextTypingSound, -1);
}

GG::XMLElement CUIEdit::XMLEncode() const
{
    GG::XMLElement retval("CUIEdit");
    retval.AppendChild(Edit::XMLEncode());
    return retval;
}

bool CUIEdit::Render()
{
    GG::Clr color = Color();
    GG::Clr color_to_use = Disabled() ? DisabledColor(color) : color;
    GG::Clr int_color_to_use = Disabled() ? DisabledColor(InteriorColor()) : InteriorColor();

    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Pt client_ul = ClientUpperLeft(), client_lr = ClientLowerRight();

    FlatRectangle(ul.x, ul.y, lr.x, lr.y, int_color_to_use, color_to_use, 1);

    SetColor(GG::CLR_ZERO);
    Edit::Render();
    SetColor(color);
    
    return true;
}

///////////////////////////////////////
// class CUIMultiEdit
///////////////////////////////////////
CUIMultiEdit::CUIMultiEdit(int x, int y, int w, int h, const std::string& str, Uint32 style/* = TF_LINEWRAP*/, 
                           const std::string& font_filename/* = ClientUI::FONT*/, int pts/* = ClientUI::PTS*/, 
                           GG::Clr color/* = ClientUI::CTRL_BORDER_COLOR*/, GG::Clr text_color/* = ClientUI::TEXT_COLOR*/, 
                           GG::Clr interior/* = ClientUI::MULTIEDIT_INT_COLOR*/, Uint32 flags/* = CLICKABLE | DRAG_KEEPER*/) : 
    MultiEdit(x, y, w, h, str, font_filename, pts, color, style, text_color, interior, flags)
{
    RecreateScrolls();
}

CUIMultiEdit::CUIMultiEdit(const GG::XMLElement& elem) : 
    MultiEdit(elem.Child("GG::MultiEdit"))
{
    if (elem.Tag() != "CUIMultiEdit")
        throw std::invalid_argument("Attempted to construct a CUIMultiEdit from an XMLElement that had a tag other than \"CUIMultiEdit\"");
    RecreateScrolls();
}

GG::XMLElement CUIMultiEdit::XMLEncode() const
{
    GG::XMLElement retval("CUIMultiEdit");
    retval.AppendChild(MultiEdit::XMLEncode());
    return retval;
}

bool CUIMultiEdit::Render()
{
    GG::Clr color = Color();
    GG::Clr color_to_use = Disabled() ? DisabledColor(color) : color;
    GG::Clr int_color_to_use = Disabled() ? DisabledColor(InteriorColor()) : InteriorColor();

    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Pt client_ul = ClientUpperLeft(), client_lr = ClientLowerRight();

    FlatRectangle(ul.x, ul.y, lr.x, lr.y, int_color_to_use, color_to_use, 1);

    SetColor(GG::CLR_ZERO);
    MultiEdit::Render();
    SetColor(color);
    
    return true;
}

GG::Scroll* CUIMultiEdit::NewVScroll(bool horz_scroll)
{
    const int GAP = PIXEL_MARGIN - 2; // the space between the client area and the border
    GG::Pt cl_sz = Edit::ClientLowerRight() - Edit::ClientUpperLeft();
    return new CUIScroll(cl_sz.x + GAP - SCROLL_WIDTH, -GAP, SCROLL_WIDTH, cl_sz.y + 2 * GAP - (horz_scroll ? SCROLL_WIDTH : 0), GG::Scroll::VERTICAL);
}

GG::Scroll* CUIMultiEdit::NewHScroll(bool vert_scroll)
{
    const int GAP = PIXEL_MARGIN - 2; // the space between the client area and the border
    GG::Pt cl_sz = Edit::ClientLowerRight() - Edit::ClientUpperLeft();
    return new CUIScroll(-GAP, cl_sz.y + GAP - SCROLL_WIDTH, cl_sz.x + 2 * GAP - (vert_scroll ? SCROLL_WIDTH : 0), SCROLL_WIDTH, GG::Scroll::HORIZONTAL);
}

///////////////////////////////////////
// class CUISlider
///////////////////////////////////////
CUISlider::CUISlider(int x, int y, int w, int h, int min, int max, Orientation orientation, Uint32 flags/* = CLICKABLE*/) :
    Slider(x, y, w, h, min, max, orientation, FLAT, ClientUI::CTRL_COLOR,
           new CUIScroll::ScrollTab(GG::Scroll::Orientation(orientation), orientation == VERTICAL ? w : h, 
                                    ClientUI::SCROLL_TAB_COLOR, ClientUI::CTRL_BORDER_COLOR),
           5, flags)
{
}

CUISlider::CUISlider(const GG::XMLElement& elem) :
    Slider(elem.Child("GG::Slider"))
{
    if (elem.Tag() != "CUISlider")
        throw std::invalid_argument("Attempted to construct a CUISlider from an XMLElement that had a tag other than \"CUISlider\"");
}

GG::XMLElement CUISlider::XMLEncode() const
{
    GG::XMLElement retval("CUISlider");
    retval.AppendChild(Slider::XMLEncode());
    return retval;
}

bool CUISlider::Render()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Clr border_color_to_use = Disabled() ? GG::DisabledColor(ClientUI::CTRL_BORDER_COLOR) : ClientUI::CTRL_BORDER_COLOR;
    int tab_width = GetOrientation() == VERTICAL ? Tab()->Height() : Tab()->Width();
    int x_start, x_end, y_start, y_end;
    if (GetOrientation() == VERTICAL) {
        x_start = ((lr.x + ul.x) - LineWidth()) / 2;
        x_end   = x_start + LineWidth();
        y_start = ul.y + tab_width / 2;
        y_end   = lr.y - tab_width / 2;
    } else {
        x_start = ul.x + tab_width / 2;
        x_end   = lr.x - tab_width / 2;
        y_start = ((lr.y + ul.y) - LineWidth()) / 2;
        y_end   = y_start + LineWidth();
    }
    GG::FlatRectangle(x_start, y_start, x_end, y_end, GG::CLR_ZERO, border_color_to_use, 1);
    Tab()->OffsetMove(UpperLeft());
    Tab()->Render();
    Tab()->OffsetMove(-UpperLeft());
    return true;
}

///////////////////////////////////////
// class StatisticIcon
///////////////////////////////////////
StatisticIcon::StatisticIcon(int x, int y, int w, int h, const std::string& icon_filename, GG::Clr text_color, double value,
                             int decimals_to_show/* = 0*/, bool show_sign/* = false*/) :
    GG::Control(x, y, w, h, 0),
    m_value(value),
    m_decimals_to_show(decimals_to_show),
    m_show_sign(show_sign),
    m_positive_color(text_color),
    m_negative_color(text_color),
    m_icon(new GG::StaticGraphic(0, 0, h, h, GG::App::GetApp()->GetTexture(icon_filename), GG::GR_FITGRAPHIC)),
    m_text(new GG::TextControl(h, 0, w - h, h, "", ClientUI::FONT, ClientUI::PTS, text_color, GG::TF_LEFT | GG::TF_VCENTER))
{
    AttachChild(m_icon);
    AttachChild(m_text);
    Refresh();
}

void StatisticIcon::SetValue(double value) 
{
    m_value = value;
    if (m_decimals_to_show) {
        char buf[128];
        sprintf(buf, (m_show_sign ? "%+#.*g" : "%#.*g"), m_decimals_to_show, value);
        m_text->SetText(buf);
    } else {
        m_text->SetText((m_show_sign && 0.0 <= value ? "+" : "") + boost::lexical_cast<std::string>(static_cast<int>(value)));
    }
}

void StatisticIcon::Refresh()
{
    SetValue(m_value);
    m_text->SetColor(m_value < 0.0 ? m_negative_color : m_positive_color);
}

///////////////////////////////////////
// class StatisticIconDualValue
///////////////////////////////////////
StatisticIconDualValue::StatisticIconDualValue(int x, int y, int w, int h, const std::string& icon_filename, GG::Clr text_color, 
                                               double value,double value_second,
                                               int decimals_to_show/* = 0*/,int decimals_to_show_second/* = 0*/,
                                               bool show_sign/* = false*/, bool show_sign_second/* = false*/) :
    GG::Control(x, y, w, h, 0),
    m_value(value),m_value_second(value_second),
    m_decimals_to_show(decimals_to_show),m_decimals_to_show_second(decimals_to_show_second),
    m_show_sign(show_sign),m_show_sign_second(show_sign_second),
    m_positive_color(text_color),m_negative_color(text_color),
    m_icon(new GG::StaticGraphic(0, 0, h, h, GG::App::GetApp()->GetTexture(icon_filename), GG::GR_FITGRAPHIC)),
    m_text(new GG::TextControl(h, 0, w - h, h, "", ClientUI::FONT, ClientUI::PTS, text_color, GG::TF_LEFT | GG::TF_VCENTER))
{
    AttachChild(m_icon);
    AttachChild(m_text);
    UpdateTextControl();
}

void StatisticIconDualValue::UpdateTextControl()
{
    std::string value        = (ShowsSign      () && 0.0 <= Value      () ? "+" : "") + boost::lexical_cast<std::string>(static_cast<int>(Value      ()));
    std::string value_second = (ShowsSignSecond() && 0.0 <= ValueSecond() ? "+" : "") + boost::lexical_cast<std::string>(static_cast<int>(ValueSecond()));

    char buf[128];

    if (DecimalsShown()) 
    {
        sprintf(buf, (ShowsSign() ? "%+.*f" : "%.*f"), DecimalsShown(), Value());
        value = buf;
    } 

    if (DecimalsShownSecond()) 
    {
        sprintf(buf, (ShowsSignSecond() ? "%+.*f" : "%.*f"), DecimalsShownSecond(), ValueSecond());
        value_second = buf;
    } 

    std::string text;

    text+=value + " (";
    if(ValueSecond()<0.0) text+= GG::RgbaTag(NegativeColor());
    else                  text+= GG::RgbaTag(PositiveColor());
    text+=value_second + "</rgba>)";

    m_text->SetText(text);

    m_text->SetColor(GG::CLR_WHITE);
    //SetColor(Value() < 0.0 ? NegativeColor() : PositiveColor());
}

void StatisticIconDualValue::SetValue(double value)
{
  m_value = value;
  UpdateTextControl();
}

void StatisticIconDualValue::SetValueSecond(double value)
{
  m_value_second = value;
  UpdateTextControl();
}

///////////////////////////////////////
// class CUIToolBar
///////////////////////////////////////
CUIToolBar::CUIToolBar(int x, int y, int w, int h) :
    GG::Control(x, y, w, h, 0)
{}

bool CUIToolBar::Render()
{
  GG::Pt ul(UpperLeft()),lr(LowerRight());
  GG::FlatRectangle(ul.x,ul.y,lr.x,lr.y,GG::Clr(0.0,0.0,0.0,0.8),GG::CLR_ZERO,0);
  return true;
}

///////////////////////////////////////
// class EmpireColorSelector
///////////////////////////////////////
EmpireColorSelector::EmpireColorSelector(int h) : 
    CUIDropDownList(0, 0, COLOR_SELECTOR_WIDTH, h, 5 * h)
{
    const std::vector<GG::Clr>& colors = EmpireColors();
    for (unsigned int i = 0; i < colors.size(); ++i) {
        Insert(new ColorRow(colors[i], h - 2));
    }
    GG::Connect(SelChangedSignal(), &EmpireColorSelector::SelectionChanged, this);
}

GG::Clr EmpireColorSelector::CurrentColor() const
{
    return (*CurrentItem())[0]->Color();
}

void EmpireColorSelector::SelectColor(const GG::Clr& clr)
{
    Select(0);
    const std::vector<GG::Clr>& colors = EmpireColors();
    for (unsigned int i = 0; i < colors.size(); ++i) {
        if (colors[i] == clr) {
            Select(i);
            break;
        }
    }
}

void EmpireColorSelector::SelectionChanged(int i)
{
    const std::vector<GG::Clr>& colors = EmpireColors();
    color_changed_sig(colors[i]);
}

///////////////////////////////////////
// class FileDlg
///////////////////////////////////////
FileDlg::FileDlg(const std::string& directory, const std::string& filename, bool save, bool multi,
                 const std::vector<std::pair<std::string, std::string> >& types) :
    GG::FileDlg(directory, filename, save, multi, types, ClientUI::FONT, ClientUI::PTS,
                ClientUI::CTRL_COLOR, ClientUI::CTRL_BORDER_COLOR, ClientUI::TEXT_COLOR,
                new CUIButton(3 * WIDTH / 4, HEIGHT - (ClientUI::PTS + 14) * 2, WIDTH / 4 - 10, UserString(save ? "SAVE" : "OPEN")),
                new CUIButton(3 * WIDTH / 4, HEIGHT - (ClientUI::PTS + 14), WIDTH / 4 - 10, UserString("CANCEL")))
{
    SetFilesString(UserString("FILE_DLG_FILES"));
    SetFileTypesString(UserString("FILE_DLG_FILE_TYPES"));
}

