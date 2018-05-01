#include "ResourceBrowseWnd.h"

#include "../util/i18n.h"
#include "CUIControls.h"

namespace {
    const int   EDGE_PAD(3);
    const GG::Y ICON_BROWSE_ICON_HEIGHT(64);

    const GG::X BrowseTextWidth()
    { return GG::X(FontBasedUpscale(200)); }
}

////////////////////////////////////////////////////////////
// ResourceBrowseWnd
////////////////////////////////////////////////////////////
ResourceBrowseWnd::ResourceBrowseWnd(const std::string& title_text,
                                     const std::string& unit_label,
                                     float used,
                                     float output,
                                     float target_output,
                                     bool show_stockpile /*=false*/,
                                     float stockpile_use /*=0.0f*/,
                                     float stockpile /*=0.0f*/,
                                     float stockpile_change /*=0.0f*/,
                                     bool show_stockpile_limit /* = false*/,
                                     float stockpile_use_limit /*= 0.0f*/) :
    GG::BrowseInfoWnd(GG::X0, GG::Y0, BrowseTextWidth(), GG::Y1),
    m_title_text(GG::Wnd::Create<CUILabel>(title_text, GG::FORMAT_CENTER)),
    m_show_points(used >= 0.0f),
    m_used_points_label(GG::Wnd::Create<CUILabel>(UserString("RESOURCE_TT_USED"), GG::FORMAT_RIGHT)),
    m_used_points(GG::Wnd::Create<CUILabel>(DoubleToString(used, 3, false), GG::FORMAT_LEFT)),
    m_used_points_P_label(GG::Wnd::Create<CUILabel>(unit_label, GG::FORMAT_LEFT)),
    m_output_points_label(GG::Wnd::Create<CUILabel>(UserString("RESOURCE_TT_OUTPUT"), GG::FORMAT_RIGHT)),
    m_output_points(GG::Wnd::Create<CUILabel>(DoubleToString(output, 3, false), GG::FORMAT_LEFT)),
    m_output_points_P_label(GG::Wnd::Create<CUILabel>(unit_label, GG::FORMAT_LEFT)),
    m_target_points_label(GG::Wnd::Create<CUILabel>(UserString("RESOURCE_TT_TARGET_OUTPUT"), GG::FORMAT_RIGHT)),
    m_target_points(GG::Wnd::Create<CUILabel>(DoubleToString(target_output, 3, false), GG::FORMAT_LEFT)),
    m_target_points_P_label(GG::Wnd::Create<CUILabel>(unit_label, GG::FORMAT_LEFT)),
    m_show_stockpile(show_stockpile),
    m_stockpile_points_label(GG::Wnd::Create<CUILabel>(UserString("STOCKPILE_LABEL"), GG::FORMAT_RIGHT)),
    m_stockpile_points(GG::Wnd::Create<CUILabel>(DoubleToString(stockpile, 3, false), GG::FORMAT_LEFT)),
    m_stockpile_points_P_label(GG::Wnd::Create<CUILabel>(unit_label, GG::FORMAT_LEFT)),
    m_stockpile_used_points_label(GG::Wnd::Create<CUILabel>(UserString("STOCKPILE_USE_LABEL"), GG::FORMAT_RIGHT)),
    m_stockpile_used_points(GG::Wnd::Create<CUILabel>(DoubleToString(stockpile_use, 3, false), GG::FORMAT_LEFT)),
    m_stockpile_used_points_P_label(GG::Wnd::Create<CUILabel>(unit_label, GG::FORMAT_LEFT)),
    m_stockpile_change_points_label(GG::Wnd::Create<CUILabel>(UserString("STOCKPILE_CHANGE_LABEL"), GG::FORMAT_RIGHT)),
    m_stockpile_change_points(GG::Wnd::Create<CUILabel>(DoubleToString(stockpile_change, 3, false), GG::FORMAT_LEFT)),
    m_stockpile_change_points_P_label(GG::Wnd::Create<CUILabel>(unit_label, GG::FORMAT_LEFT)),
    m_offset(GG::X0, ICON_BROWSE_ICON_HEIGHT/2),
    m_show_stockpile_limit(show_stockpile_limit)
{
    if (!m_show_points && m_show_stockpile_limit) {
        // replace usage with with stockpile limits
        m_output_points_label->SetText(UserString("STOCKPILE_USE_LIMIT"));
        m_output_points->SetText(DoubleToString(stockpile_use_limit, 3, false));
    }
}

void ResourceBrowseWnd::CompleteConstruction() {
    GG::BrowseInfoWnd::CompleteConstruction();

    const GG::Y ROW_HEIGHT(ClientUI::Pts() * 4 / 3);

    GG::Pt top_left = m_offset;

    m_title_text->MoveTo(GG::Pt(top_left.x + EDGE_PAD, top_left.y));
    m_title_text->Resize(GG::Pt(BrowseTextWidth() - 2 * EDGE_PAD, ROW_HEIGHT));
    m_title_text->SetFont(ClientUI::GetBoldFont());
    top_left.y += m_title_text->Height() + EDGE_PAD;

    AttachChild(m_title_text);


    // info controls layout
    const int STAT_TEXT_PTS = ClientUI::Pts();
    const int CENTERLINE_GAP = 4;
    const GG::X LABEL_TEXT_WIDTH = (Width() - 4 - CENTERLINE_GAP) * 2 / 3;
    const GG::X VALUE_TEXT_WIDTH = Width() - 4 - CENTERLINE_GAP - LABEL_TEXT_WIDTH;
    const GG::X LEFT_TEXT_X(0);
    const GG::X RIGHT_TEXT_X = LEFT_TEXT_X + LABEL_TEXT_WIDTH + 8 + CENTERLINE_GAP;
    const GG::X P_LABEL_X = RIGHT_TEXT_X + FontBasedUpscale(40);

    std::pair<int, int> m_center_gap(Value(LABEL_TEXT_WIDTH + 2), Value(LABEL_TEXT_WIDTH + 2 + CENTERLINE_GAP));

    const GG::Pt LABEL_TEXT_SIZE(LABEL_TEXT_WIDTH, GG::Y(STAT_TEXT_PTS + 4));
    const GG::Pt VALUE_TEXT_SIZE(VALUE_TEXT_WIDTH, GG::Y(STAT_TEXT_PTS + 4));
    const GG::Pt P_LABEL_SIZE(Width() - 2 - 5 - P_LABEL_X, GG::Y(STAT_TEXT_PTS + 4));



    AttachChild(m_used_points_label);
    AttachChild(m_used_points);
    AttachChild(m_used_points_P_label);
    AttachChild(m_output_points_label);
    AttachChild(m_output_points);
    AttachChild(m_output_points_P_label);
    AttachChild(m_target_points_label);
    AttachChild(m_target_points);
    AttachChild(m_target_points_P_label);

    if (m_show_points) {
        m_used_points_label->MoveTo(GG::Pt(top_left.x + LEFT_TEXT_X, top_left.y));
        m_used_points_label->Resize(LABEL_TEXT_SIZE);
        m_used_points->MoveTo(GG::Pt(top_left.x + RIGHT_TEXT_X, top_left.y));
        m_used_points->Resize(VALUE_TEXT_SIZE);
        m_used_points_P_label->MoveTo(GG::Pt(top_left.x + P_LABEL_X, top_left.y));
        m_used_points_P_label->Resize(P_LABEL_SIZE);
        top_left.y += m_used_points_label->Height();

        m_output_points_label->MoveTo(GG::Pt(top_left.x + LEFT_TEXT_X, top_left.y));
        m_output_points_label->Resize(LABEL_TEXT_SIZE);
        m_output_points->MoveTo(GG::Pt(top_left.x + RIGHT_TEXT_X, top_left.y));
        m_output_points->Resize(VALUE_TEXT_SIZE);
        m_output_points_P_label->MoveTo(GG::Pt(top_left.x + P_LABEL_X, top_left.y));
        m_output_points_P_label->Resize(P_LABEL_SIZE);
        top_left.y += m_output_points_label->Height();

        m_target_points_label->MoveTo(GG::Pt(top_left.x + LEFT_TEXT_X, top_left.y));
        m_target_points_label->Resize(LABEL_TEXT_SIZE);
        m_target_points->MoveTo(GG::Pt(top_left.x + RIGHT_TEXT_X, top_left.y));
        m_target_points->Resize(VALUE_TEXT_SIZE);
        m_target_points_P_label->MoveTo(GG::Pt(top_left.x + P_LABEL_X, top_left.y));
        m_target_points_P_label->Resize(P_LABEL_SIZE);
        top_left.y += m_target_points_label->Height();
    } else {
        m_used_points_label->Hide();
        m_used_points->Hide();
        m_used_points_P_label->Hide();

        m_output_points_label->Hide();
        m_output_points->Hide();
        m_output_points_P_label->Hide();

        m_target_points_label->Hide();
        m_target_points->Hide();
        m_target_points_P_label->Hide();
    }
    AttachChild(m_stockpile_points_label);
    AttachChild(m_stockpile_points);
    AttachChild(m_stockpile_points_P_label);
    AttachChild(m_stockpile_used_points_label);
    AttachChild(m_stockpile_used_points);
    AttachChild(m_stockpile_used_points_P_label);
    AttachChild(m_stockpile_change_points_label);
    AttachChild(m_stockpile_change_points);
    AttachChild(m_stockpile_change_points_P_label);

    if (m_show_stockpile) {
        m_stockpile_used_points_label->MoveTo(GG::Pt(top_left.x + LEFT_TEXT_X, top_left.y));
        m_stockpile_used_points_label->Resize(LABEL_TEXT_SIZE);
        m_stockpile_used_points->MoveTo(GG::Pt(top_left.x + RIGHT_TEXT_X, top_left.y));
        m_stockpile_used_points->Resize(VALUE_TEXT_SIZE);
        m_stockpile_used_points_P_label->MoveTo(GG::Pt(top_left.x + P_LABEL_X, top_left.y));
        m_stockpile_used_points_P_label->Resize(P_LABEL_SIZE);
        top_left.y += m_stockpile_used_points_label->Height();
    } else {
        m_stockpile_used_points_label->Hide();
        m_stockpile_used_points->Hide();
        m_stockpile_used_points_P_label->Hide();
    }

    if (!m_show_points && m_show_stockpile_limit) {
        m_output_points_label->MoveTo(GG::Pt(top_left.x + LEFT_TEXT_X, top_left.y));
        m_output_points_label->Resize(LABEL_TEXT_SIZE);
        m_output_points->MoveTo(GG::Pt(top_left.x + RIGHT_TEXT_X, top_left.y));
        m_output_points->Resize(VALUE_TEXT_SIZE);
        m_output_points_P_label->MoveTo(GG::Pt(top_left.x + P_LABEL_X, top_left.y));
        m_output_points_P_label->Resize(P_LABEL_SIZE);
        top_left.y += m_output_points_label->Height();

        m_output_points_label->Show();
        m_output_points->Show();
        m_output_points_P_label->Show();
    }

    if (m_show_stockpile) {
        m_stockpile_points_label->MoveTo(GG::Pt(top_left.x + LEFT_TEXT_X, top_left.y));
        m_stockpile_points_label->Resize(LABEL_TEXT_SIZE);
        m_stockpile_points->MoveTo(GG::Pt(top_left.x + RIGHT_TEXT_X, top_left.y));
        m_stockpile_points->Resize(VALUE_TEXT_SIZE);
        m_stockpile_points_P_label->MoveTo(GG::Pt(top_left.x + P_LABEL_X, top_left.y));
        m_stockpile_points_P_label->Resize(P_LABEL_SIZE);
        top_left.y += m_stockpile_points_label->Height();

        m_stockpile_change_points_label->MoveTo(GG::Pt(top_left.x + LEFT_TEXT_X, top_left.y));
        m_stockpile_change_points_label->Resize(LABEL_TEXT_SIZE);
        m_stockpile_change_points->MoveTo(GG::Pt(top_left.x + RIGHT_TEXT_X, top_left.y));
        m_stockpile_change_points->Resize(VALUE_TEXT_SIZE);
        m_stockpile_change_points_P_label->MoveTo(GG::Pt(top_left.x + P_LABEL_X, top_left.y));
        m_stockpile_change_points_P_label->Resize(P_LABEL_SIZE);
        top_left.y += m_stockpile_change_points_label->Height();
    } else {
        m_stockpile_points_label->Hide();
        m_stockpile_points->Hide();
        m_stockpile_points_P_label->Hide();
        m_stockpile_change_points_label->Hide();
        m_stockpile_change_points->Hide();
        m_stockpile_change_points_P_label->Hide();
    }

    // background / border rendering prep
    Resize(GG::Pt(BrowseTextWidth(), top_left.y + EDGE_PAD - m_offset.y));

    InitBuffer();
}

bool ResourceBrowseWnd::WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const {
    assert(mode <= wnd->BrowseModes().size());
    return true;
}

void ResourceBrowseWnd::InitBuffer() {
    GG::Pt sz = Size();
    const GG::Y ROW_HEIGHT(ClientUI::Pts()*4/3);

    m_buffer.clear();

    m_buffer.store(Value(sz.x), Value(ROW_HEIGHT));
    m_buffer.store(0.0f,        Value(ROW_HEIGHT));

    m_buffer.store(0.0f,        0.0f);
    m_buffer.store(Value(sz.x), 0.0f);
    m_buffer.store(Value(sz.x), Value(sz.y));
    m_buffer.store(0.0f,        Value(sz.y));
    m_buffer.createServerBuffer();
}

void ResourceBrowseWnd::Render() {
    GG::Pt ul = UpperLeft();

    glPushMatrix();
    glLoadIdentity();
    glTranslatef(static_cast<GLfloat>(Value(m_offset.x + ul.x)),
                 static_cast<GLfloat>(Value(m_offset.y + ul.y)),
                 0.0f);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(1.0);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);

    m_buffer.activate();
    glColor(ClientUI::WndColor());
    glDrawArrays(GL_TRIANGLE_FAN,   2, 4);
    glColor(ClientUI::WndOuterBorderColor());
    glDrawArrays(GL_LINE_LOOP,      2, 4);
    glDrawArrays(GL_TRIANGLE_FAN,   0, 4);

    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
    glPopClientAttrib();
}


////////////////////////////////////////////////////////////
// WastedStockpiledResourceBrowseWnd
////////////////////////////////////////////////////////////
WastedStockpiledResourceBrowseWnd::WastedStockpiledResourceBrowseWnd(
    const std::string& title_text, const std::string& unit_label,
    float capacity, float excess, bool show_stockpile/* = false*/,
    float stockpile_effic/* = 0.0f*/, float to_stockpile/* = 0.0f*/,
    float wasted/* = 0.0f*/, const std::string& bottom_message/* = ""*/) :
    GG::BrowseInfoWnd(GG::X0, GG::Y0, BrowseTextWidth(), GG::Y1),
    m_buffer(),
    m_title_text(GG::Wnd::Create<CUILabel>(title_text, GG::FORMAT_CENTER)),

    m_output_label(GG::Wnd::Create<CUILabel>(UserString("RESOURCE_TT_OUTPUT"), GG::FORMAT_RIGHT)),
    m_output(GG::Wnd::Create<CUILabel>(DoubleToString(capacity, 3, false), GG::FORMAT_LEFT)),
    m_output_P_label(GG::Wnd::Create<CUILabel>(unit_label, GG::FORMAT_LEFT)),
    m_used_label(GG::Wnd::Create<CUILabel>(UserString("RESOURCE_TT_USED"), GG::FORMAT_RIGHT)),
    m_used(GG::Wnd::Create<CUILabel>(DoubleToString(std::max(0.0f, capacity - excess), 3, false), GG::FORMAT_LEFT)),
    m_used_P_label(GG::Wnd::Create<CUILabel>(unit_label, GG::FORMAT_LEFT)),
    m_excess_label(GG::Wnd::Create<CUILabel>(UserString("RESOURCE_TT_EXCESS"), GG::FORMAT_RIGHT)),
    m_excess(GG::Wnd::Create<CUILabel>(DoubleToString(excess, 3, false), GG::FORMAT_LEFT)),
    m_excess_P_label(GG::Wnd::Create<CUILabel>(unit_label, GG::FORMAT_LEFT)),

    m_show_stockpile(show_stockpile),

    m_stockpile_label(GG::Wnd::Create<CUILabel>(UserString("RESOURCE_TT_TO_STOCKPILE"), GG::FORMAT_RIGHT)),
    m_stockpile(GG::Wnd::Create<CUILabel>(DoubleToString(to_stockpile, 3, false), GG::FORMAT_LEFT)),
    m_stockpile_P_label(GG::Wnd::Create<CUILabel>(unit_label, GG::FORMAT_LEFT)),

    m_wasted_label(GG::Wnd::Create<CUILabel>(UserString("RESOURCE_TT_WASTED"), GG::FORMAT_RIGHT)),
    m_wasted(GG::Wnd::Create<CUILabel>(DoubleToString(wasted, 3, false), GG::FORMAT_LEFT)),
    m_wasted_P_label(GG::Wnd::Create<CUILabel>(unit_label, GG::FORMAT_LEFT)),

    m_bottom_message_text(GG::Wnd::Create<CUILabel>(
        bottom_message,
        GG::FORMAT_LEFT | GG::FORMAT_WORDBREAK | GG::FORMAT_LINEWRAP | GG::FORMAT_TOP)),

    m_offset(GG::X0, ICON_BROWSE_ICON_HEIGHT/2)
{}

void WastedStockpiledResourceBrowseWnd::CompleteConstruction() {
    GG::BrowseInfoWnd::CompleteConstruction();

    const GG::Y ROW_HEIGHT(ClientUI::Pts() * 4 / 3);

    GG::Pt top_left = m_offset;

    m_title_text->MoveTo(GG::Pt(top_left.x + EDGE_PAD, top_left.y));
    m_title_text->Resize(GG::Pt(BrowseTextWidth() - 2 * EDGE_PAD, ROW_HEIGHT));
    m_title_text->SetFont(ClientUI::GetBoldFont());
    top_left.y += m_title_text->Height() + EDGE_PAD;

    AttachChild(m_title_text);


    // info controls layout
    const int STAT_TEXT_PTS = ClientUI::Pts();
    const int CENTERLINE_GAP = 4;
    const GG::X LABEL_TEXT_WIDTH = (Width() - 4 - CENTERLINE_GAP) * 2 / 3;
    const GG::X VALUE_TEXT_WIDTH = Width() - 4 - CENTERLINE_GAP - LABEL_TEXT_WIDTH;
    const GG::X LEFT_TEXT_X(0);
    const GG::X RIGHT_TEXT_X = LEFT_TEXT_X + LABEL_TEXT_WIDTH + 8 + CENTERLINE_GAP;
    const GG::X P_LABEL_X = RIGHT_TEXT_X + FontBasedUpscale(40);

    std::pair<int, int> m_center_gap(Value(LABEL_TEXT_WIDTH + 2), Value(LABEL_TEXT_WIDTH + 2 + CENTERLINE_GAP));

    const GG::Pt LABEL_TEXT_SIZE(LABEL_TEXT_WIDTH, GG::Y(STAT_TEXT_PTS + 4));
    const GG::Pt VALUE_TEXT_SIZE(VALUE_TEXT_WIDTH, GG::Y(STAT_TEXT_PTS + 4));
    const GG::Pt P_LABEL_SIZE(Width() - 2 - 5 - P_LABEL_X, GG::Y(STAT_TEXT_PTS + 4));


    AttachChild(m_output_label);
    AttachChild(m_output);
    AttachChild(m_output_P_label);
    AttachChild(m_used_label);
    AttachChild(m_used);
    AttachChild(m_used_P_label);
    AttachChild(m_excess_label);
    AttachChild(m_excess);
    AttachChild(m_excess_P_label);

    m_output_label->MoveTo(GG::Pt(top_left.x + LEFT_TEXT_X, top_left.y));
    m_output_label->Resize(LABEL_TEXT_SIZE);
    m_output->MoveTo(GG::Pt(top_left.x + RIGHT_TEXT_X, top_left.y));
    m_output->Resize(VALUE_TEXT_SIZE);
    m_output_P_label->MoveTo(GG::Pt(top_left.x + P_LABEL_X, top_left.y));
    m_output_P_label->Resize(P_LABEL_SIZE);
    top_left.y += m_output_P_label->Height();

    m_used_label->MoveTo(GG::Pt(top_left.x + LEFT_TEXT_X, top_left.y));
    m_used_label->Resize(LABEL_TEXT_SIZE);
    m_used->MoveTo(GG::Pt(top_left.x + RIGHT_TEXT_X, top_left.y));
    m_used->Resize(VALUE_TEXT_SIZE);
    m_used_P_label->MoveTo(GG::Pt(top_left.x + P_LABEL_X, top_left.y));
    m_used_P_label->Resize(P_LABEL_SIZE);
    top_left.y += m_used_P_label->Height();

    m_excess_label->MoveTo(GG::Pt(top_left.x + LEFT_TEXT_X, top_left.y));
    m_excess_label->Resize(LABEL_TEXT_SIZE);
    m_excess->MoveTo(GG::Pt(top_left.x + RIGHT_TEXT_X, top_left.y));
    m_excess->Resize(VALUE_TEXT_SIZE);
    m_excess_P_label->MoveTo(GG::Pt(top_left.x + P_LABEL_X, top_left.y));
    m_excess_P_label->Resize(P_LABEL_SIZE);
    top_left.y += m_excess_P_label->Height();

    AttachChild(m_stockpile_label);
    AttachChild(m_stockpile);
    AttachChild(m_stockpile_P_label);
    AttachChild(m_wasted_label);
    AttachChild(m_wasted);
    AttachChild(m_wasted_P_label);

    if (m_show_stockpile) {
        m_stockpile_label->MoveTo(GG::Pt(top_left.x + LEFT_TEXT_X, top_left.y));
        m_stockpile_label->Resize(LABEL_TEXT_SIZE);
        m_stockpile->MoveTo(GG::Pt(top_left.x + RIGHT_TEXT_X, top_left.y));
        m_stockpile->Resize(VALUE_TEXT_SIZE);
        m_stockpile_P_label->MoveTo(GG::Pt(top_left.x + P_LABEL_X, top_left.y));
        m_stockpile_P_label->Resize(P_LABEL_SIZE);
        top_left.y += m_stockpile_P_label->Height();

        m_wasted_label->MoveTo(GG::Pt(top_left.x + LEFT_TEXT_X, top_left.y));
        m_wasted_label->Resize(LABEL_TEXT_SIZE);
        m_wasted->MoveTo(GG::Pt(top_left.x + RIGHT_TEXT_X, top_left.y));
        m_wasted->Resize(VALUE_TEXT_SIZE);
        m_wasted_P_label->MoveTo(GG::Pt(top_left.x + P_LABEL_X, top_left.y));
        m_wasted_P_label->Resize(P_LABEL_SIZE);
        top_left.y += m_wasted_P_label->Height();

    } else {
        m_stockpile_label->Hide();
        m_stockpile->Hide();
        m_stockpile_P_label->Hide();
        m_wasted_label->Hide();
        m_wasted->Hide();
        m_wasted_P_label->Hide();
    }

    AttachChild(m_bottom_message_text);
    if (!m_bottom_message_text->Text().empty()) {
        m_bottom_message_text->MoveTo(GG::Pt(top_left.x + LEFT_TEXT_X, top_left.y));
        m_bottom_message_text->Resize(GG::Pt(BrowseTextWidth() - 2 * EDGE_PAD, GG::Y0));
        //m_bottom_message_text->Resize(m_bottom_message_text->MinUsableSize());
        top_left.y += m_bottom_message_text->MinUsableSize().y;
    } else {
        m_bottom_message_text->Hide();
    }

    // background / border rendering prep
    Resize(GG::Pt(BrowseTextWidth(), top_left.y + EDGE_PAD - m_offset.y));

    InitBuffer();
}

bool WastedStockpiledResourceBrowseWnd::WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const {
    assert(mode <= wnd->BrowseModes().size());
    return true;
}

void WastedStockpiledResourceBrowseWnd::InitBuffer() {
    GG::Pt sz = Size();
    const GG::Y ROW_HEIGHT(ClientUI::Pts()*4/3);

    m_buffer.clear();

    m_buffer.store(Value(sz.x), Value(ROW_HEIGHT));
    m_buffer.store(0.0f,        Value(ROW_HEIGHT));

    m_buffer.store(0.0f,        0.0f);
    m_buffer.store(Value(sz.x), 0.0f);
    m_buffer.store(Value(sz.x), Value(sz.y));
    m_buffer.store(0.0f,        Value(sz.y));
    m_buffer.createServerBuffer();
}

void WastedStockpiledResourceBrowseWnd::Render() {
    GG::Pt ul = UpperLeft();

    glPushMatrix();
    glLoadIdentity();
    glTranslatef(static_cast<GLfloat>(Value(m_offset.x + ul.x)),
                 static_cast<GLfloat>(Value(m_offset.y + ul.y)),
                 0.0f);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(1.0);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);

    m_buffer.activate();
    glColor(ClientUI::WndColor());
    glDrawArrays(GL_TRIANGLE_FAN,   2, 4);
    glColor(ClientUI::WndOuterBorderColor());
    glDrawArrays(GL_LINE_LOOP,      2, 4);
    glDrawArrays(GL_TRIANGLE_FAN,   0, 4);

    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
    glPopClientAttrib();
}
