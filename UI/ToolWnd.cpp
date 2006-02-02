#include "ToolWnd.h"

#include "GGDrawUtil.h"
#include "../util/MultiplayerCommon.h"


namespace {
    bool temp_header_bool = RecordHeaderFile(ToolWndRevision());
    bool temp_source_bool = RecordSourceFile("$Id$");
}


ToolWnd::ToolWnd(int x, int y, const std::string& text, const GG::Clr& wnd_color, const GG::Clr& border_color,
                 const GG::Clr& text_color, const std::string& font_name /* = "arial.ttf"*/, int pts /*= 10*/) :
    GG::Wnd(x, y, 10, 10, 0),
    m_color(wnd_color),
    m_border_color(border_color)
{
    m_text = new GG::TextControl(4, 4, text.c_str(), font_name, pts, text_color);
    Resize(m_text->GG::Wnd::Width() + 8, m_text->GG::Wnd::Height() + 8);
    AttachChild(m_text);
    Hide(); 
}

bool ToolWnd::Render()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, m_color, m_border_color, 1);
    GG::FlatRectangle(ul.x + 2, ul.y + 2, lr.x - 2, lr.y - 2, GG::CLR_ZERO, m_border_color);
    return true;
}


