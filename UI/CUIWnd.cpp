//CUIWnd.cpp

#include "CUIWnd.h"

#include "../client/human/HumanClientApp.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "Sound.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "../util/Logger.h"

#include <GG/GUI.h>
#include <GG/DrawUtil.h>

#include <limits>


namespace {
    void PlayMinimizeSound()
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.window-maximize"), true); }
    void PlayMaximizeSound()
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.window-minimize"), true); }
    void PlayCloseSound()
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.window-close"), true); }

    void AddOptions(OptionsDB& db) {
        db.AddFlag('w', "window-reset", UserStringNop("OPTIONS_DB_WINDOW_RESET"), false);
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    const double BUTTON_DIMMING_SCALE_FACTOR = 0.75;

    const GG::X::value_type INVALID_POS = std::numeric_limits<GG::X::value_type>::min();
    const GG::X INVALID_X = GG::X(INVALID_POS);
    const GG::Y INVALID_Y = GG::Y(INVALID_POS);
}

////////////////////////////////////////////////
// CUI_MinRestoreButton
////////////////////////////////////////////////
CUI_MinRestoreButton::CUI_MinRestoreButton() :
    GG::Button("", boost::shared_ptr<GG::Font>(), ClientUI::WndInnerBorderColor()),
    m_mode(MIN_BUTTON)
{ GG::Connect(LeftClickedSignal, &CUI_MinRestoreButton::Toggle, this); }

void CUI_MinRestoreButton::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::Clr color_to_use = ClientUI::WndInnerBorderColor();
    if (State() != BN_ROLLOVER)
        AdjustBrightness(color_to_use, BUTTON_DIMMING_SCALE_FACTOR);
    if (m_mode == MIN_BUTTON) {
        // draw a dash to signify the minimize command
        GG::Y middle_y = (lr.y + ul.y) / 2;
        glColor(color_to_use);
        GG::Line(ul.x, middle_y, lr.x, middle_y);
    } else {
        // draw a square to signify the restore command
        GG::FlatRectangle(ul, lr, GG::CLR_ZERO, ClientUI::WndInnerBorderColor(), 1);
    }
}

void CUI_MinRestoreButton::Toggle() {
    if (m_mode == MIN_BUTTON) {
        PlayMinimizeSound();
        m_mode = RESTORE_BUTTON;
    } else {
        PlayMaximizeSound();
        m_mode = MIN_BUTTON;
    }
}


////////////////////////////////////////////////
// CUI_PinButton
////////////////////////////////////////////////
CUI_PinButton::CUI_PinButton() :
    GG::Button("", boost::shared_ptr<GG::Font>(), ClientUI::WndInnerBorderColor())
{
    GG::Connect(LeftClickedSignal, &PlayCloseSound, -1);
    SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "pin.png"   )));
    SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "pin.png"  )));
    SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "pin_mouseover.png")));
}

void CUI_PinButton::Toggle(bool pinned) {
    if (!pinned) {
        SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "pin.png")));
        SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "pin.png")));
        SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "pin_mouseover.png")));
    } else {
        SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "pinned.png")));
        SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "pinned.png")));
        SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "pinned_mouseover.png")));
    }
}

////////////////////////////////////////////////
// CUIWnd
////////////////////////////////////////////////
GG::WndFlag MINIMIZABLE(1 << 10);
GG::WndFlag CLOSABLE(1 << 11);
GG::WndFlag PINABLE(1 << 12);

namespace {
    bool RegisterWndFlags() {
        GG::FlagSpec<GG::WndFlag>::instance().insert(MINIMIZABLE, "MINIMIZABLE");
        GG::FlagSpec<GG::WndFlag>::instance().insert(CLOSABLE, "CLOSABLE");
        GG::FlagSpec<GG::WndFlag>::instance().insert(PINABLE, "PINABLE");
        return true;
    }
    bool dummy = RegisterWndFlags();
}

const GG::Y CUIWnd::BUTTON_TOP_OFFSET(3);
const GG::X CUIWnd::MINIMIZED_WND_WIDTH(50);
const GG::X CUIWnd::BORDER_LEFT(5);
const GG::X CUIWnd::BORDER_RIGHT(5);
const GG::Y CUIWnd::BORDER_BOTTOM(5);
const int CUIWnd::OUTER_EDGE_ANGLE_OFFSET = 8;
const int CUIWnd::INNER_BORDER_ANGLE_OFFSET = 11;
const int CUIWnd::TITLE_OFFSET = 3;
const int CUIWnd::RESIZE_HASHMARK1_OFFSET = 7;
const int CUIWnd::RESIZE_HASHMARK2_OFFSET = 3;

CUIWnd::CUIWnd(const std::string& wnd_name,
               GG::X x, GG::Y y,
               GG::X w, GG::Y h,
               GG::Flags<GG::WndFlag> flags,
               const std::string& config_name,
               bool visible) :
    GG::Wnd(x, y, w, h, flags & ~GG::RESIZABLE),
    m_resizable(flags & GG::RESIZABLE),
    m_closable(flags & CLOSABLE),
    m_minimizable(flags & MINIMIZABLE),
    m_minimized(false),
    m_pinable(flags & PINABLE),
    m_pinned(false),
    m_drag_offset(-GG::X1, -GG::Y1),
    m_mouse_in_resize_tab(false),
    m_config_save(true),
    m_config_name(AddWindowOptions(config_name, x, y, w, h, visible, false, false)),
    m_close_button(0),
    m_minimize_button(0),
    m_pin_button(0),
    m_vertex_buffer(),
    m_buffer_indices()
{
    Init(wnd_name);
    if (!m_config_name.empty()) {
        // Default position was already supplied
        GetOptionsDB().Set<bool>("UI.windows." + m_config_name + ".initialized", true);
    }
    ValidatePosition();
}

CUIWnd::CUIWnd(const std::string& wnd_name, GG::Flags<GG::WndFlag> flags, const std::string& config_name, bool visible) :
    GG::Wnd(INVALID_X, INVALID_Y, GG::X1, GG::Y1, flags & ~GG::RESIZABLE),
    m_resizable(flags & GG::RESIZABLE),
    m_closable(flags & CLOSABLE),
    m_minimizable(flags & MINIMIZABLE),
    m_minimized(false),
    m_pinable(flags & PINABLE),
    m_pinned(false),
    m_drag_offset(-GG::X1, -GG::Y1),
    m_mouse_in_resize_tab(false),
    m_config_save(true),
    m_config_name(AddWindowOptions(config_name, INVALID_POS, INVALID_POS, 1, 1, visible, false, false)),
    m_close_button(0),
    m_minimize_button(0),
    m_pin_button(0),
    m_vertex_buffer(),
    m_buffer_indices()
{ Init(wnd_name); }

void CUIWnd::Init(const std::string& wnd_name) {
    SetName(wnd_name);
    InitButtons();
    SetChildClippingMode(ClipToClientAndWindowSeparately);

    if (!m_config_name.empty()) {
        LoadOptions();
        GG::Connect(HumanClientApp::GetApp()->FullscreenSwitchSignal, boost::bind(&CUIWnd::LoadOptions, this));
    }

    // User-dragable windows recalculate their position only when told to (e.g.
    // auto-reposition is set or user clicks a 'reset windows' button).
    // Non-user-dragable windows are given the chance to position themselves on
    // every resize event.
    if (Dragable() || m_resizable)
        GG::Connect(HumanClientApp::GetApp()->RepositionWindowsSignal, &CUIWnd::ResetDefaultPosition, this);
    else
        GG::Connect(HumanClientApp::GetApp()->WindowResizedSignal, boost::bind(&CUIWnd::ResetDefaultPosition, this));

    // call to CUIWnd::MinimizedWidth() because MinimizedWidth is virtual
    SetMinSize(GG::Pt(CUIWnd::MinimizedSize().x, TopBorder() + INNER_BORDER_ANGLE_OFFSET + BORDER_BOTTOM + 50));
}

void CUIWnd::InitSizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    OptionsDB& db = GetOptionsDB();

    if (!m_config_name.empty()) {
        if (db.OptionExists("UI.windows." + m_config_name + ".initialized")) {
            std::string windowed = ""; // empty string in fullscreen mode, appends -windowed in windowed mode
            if (!db.Get<bool>("fullscreen"))
                windowed = "-windowed";
            // If the window has already had its default position specified
            // (either in the ctor or a previous call to this function), apply
            // this position to the window.
            if (db.Get<bool>("UI.windows." + m_config_name + ".initialized") ||
                db.Get<int>("UI.windows." + m_config_name + ".left" + windowed) == INVALID_X)
            { SizeMove(ul, lr); }
            db.Set<bool>("UI.windows."+m_config_name+".initialized", true);
        } else {
            ErrorLogger() << "CUIWnd::InitSizeMove() : attempted to check if window using name \"" << m_config_name
                          << "\" was initialized but the options do not appear to be registered in the OptionsDB.";
        }
    } else {
        SizeMove(ul, lr);
    }
}

CUIWnd::~CUIWnd() {
    try {
        if (!m_config_name.empty() && GetOptionsDB().OptionExists("UI.windows." + m_config_name + ".initialized"))
            GetOptionsDB().Remove("UI.windows."+m_config_name+".initialized");
    } catch (std::exception& e) { // catch std::runtime_error, boost::bad_any_cast
        ErrorLogger() << "CUIWnd::~CUIWnd() : caught exception while removing \"UI.windows." << m_config_name
                      << ".initialized\": " << e.what();
    }
    m_vertex_buffer.clear();
}

void CUIWnd::ValidatePosition()
{ SizeMove(RelativeUpperLeft(), RelativeLowerRight()); }

void CUIWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_sz = Size();
    if (m_config_save) {    // can write position/size to OptionsDB

        GG::Pt available_size;
        if (const GG::Wnd* parent = Parent()) {
            // Keep this CUIWnd entirely inside its parent.
            available_size = parent->ClientSize();
        } else if (const HumanClientApp* app = HumanClientApp::GetApp()) {
            // Keep this CUIWnd entirely inside the application window.
            available_size = GG::Pt(app->AppWidth(), app->AppHeight());
        } else {
            available_size = GG::Pt(GG::X(HumanClientApp::MaximumPossibleWidth()),
                                    GG::Y(HumanClientApp::MaximumPossibleHeight()));
            ErrorLogger() << "CUIWnd::SizeMove() could not get app instance!";
        }

        // Limit window size to be no larger than the containing window.
        GG::Pt new_size(std::min(lr.x - ul.x, available_size.x),
                        std::min(lr.y - ul.y, available_size.y));

        // Clamp position of this window to keep its entire area visible in the
        // containing window.
        GG::Pt new_ul(std::min(available_size.x - new_size.x, std::max(GG::X0, ul.x)),
                      std::min(available_size.y - new_size.y, std::max(GG::Y0, ul.y)));

        Wnd::SizeMove(new_ul, new_ul + new_size);

    } else {    // don't write position/size to OptionsDB
        Wnd::SizeMove(ul, lr);
    }

    if (Size() != old_sz)
        PositionButtons();

    SaveOptions();
    m_vertex_buffer.clear();    // force buffer re-init on next Render call
}

void CUIWnd::Render() {
    if (m_vertex_buffer.empty())
        InitBuffers();

    glDisable(GL_TEXTURE_2D);
    glLineWidth(1.0f);

    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    m_vertex_buffer.activate();

    // within m_vertex_buffer:
    // [0] is the start and range for minimized background triangle fan and minimized border line loop
    // [1] is ... the background fan / outer border line loop
    // [2] is ... the inner border line loop
    // [3] is ... the resize tab line list

    if (m_minimized) {
        glColor(ClientUI::WndColor());
        glDrawArrays(GL_TRIANGLE_FAN,   m_buffer_indices[0].first, m_buffer_indices[0].second);
        glColor(ClientUI::WndOuterBorderColor());
        glDrawArrays(GL_LINE_LOOP,      m_buffer_indices[0].first, m_buffer_indices[0].second);

    } else {
        glColor(ClientUI::WndColor());
        glDrawArrays(GL_TRIANGLE_FAN,   m_buffer_indices[1].first, m_buffer_indices[1].second);
        glColor(ClientUI::WndOuterBorderColor());
        glDrawArrays(GL_LINE_LOOP,      m_buffer_indices[1].first, m_buffer_indices[1].second);
        glColor(ClientUI::WndInnerBorderColor());
        glDrawArrays(GL_LINE_LOOP,      m_buffer_indices[2].first, m_buffer_indices[2].second);

        if (m_resizable) {
            GG::Clr tab_lines_colour = m_mouse_in_resize_tab ? ClientUI::WndInnerBorderColor() : ClientUI::WndOuterBorderColor();
            glColor(tab_lines_colour);
            glDrawArrays(GL_LINES,      m_buffer_indices[3].first, m_buffer_indices[3].second);
        }
    }

    glEnable(GL_TEXTURE_2D);

    glPopClientAttrib();

    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::BeginScissorClipping(ul, lr);
    glColor(ClientUI::TextColor());
    boost::shared_ptr<GG::Font> font = ClientUI::GetTitleFont();
    font->RenderText(GG::Pt(ul.x + BORDER_LEFT, ul.y + TITLE_OFFSET), Name());
    GG::EndScissorClipping();
}

void CUIWnd::LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!InResizeTab(pt))
        return;
    m_drag_offset = pt - LowerRight();
}

bool CUIWnd::InResizeTab(const GG::Pt& pt) const {
    if (!m_resizable || m_minimized)
        return false;

    GG::Pt cl_lr = LowerRight() - GG::Pt(BORDER_RIGHT, BORDER_BOTTOM);
    GG::Pt dist_from_lr = cl_lr - pt;
    if (Value(dist_from_lr.x) + Value(dist_from_lr.y) <= INNER_BORDER_ANGLE_OFFSET)
        return true;

    return false;
}

void CUIWnd::LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys) {
    if (m_pinned)
        return;

    if (m_drag_offset != GG::Pt(-GG::X1, -GG::Y1)) { // resize-dragging
        // drag offset: position of cursor relative to lower-right of window when left button was pressed
        // pt: position of cursor relative to upper-left of screen
        GG::Pt requested_lr = pt - m_drag_offset;

        GG::Pt max_lr;
        if (const GG::Wnd* parent = Parent()) {
            max_lr = parent->ClientLowerRight();
        } else {
            max_lr.x = GG::GUI::GetGUI()->AppWidth();
            max_lr.y = GG::GUI::GetGUI()->AppHeight();
        }

        GG::X new_x = std::min(max_lr.x, requested_lr.x);
        GG::Y new_y = std::min(max_lr.y, requested_lr.y);
        GG::Pt new_lr(new_x, new_y);

        Resize(new_lr - UpperLeft());

    } else { // normal-dragging
        GG::Wnd::LDrag(pt, move, mod_keys);
    }
}

void CUIWnd::LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    m_drag_offset = GG::Pt(-GG::X1, -GG::Y1);
    SaveOptions();
}

void CUIWnd::MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    m_mouse_in_resize_tab = InResizeTab(pt);
    Wnd::MouseEnter(pt, mod_keys);
}

void CUIWnd::MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    m_mouse_in_resize_tab = InResizeTab(pt);
    Wnd::MouseHere(pt, mod_keys);
}

void CUIWnd::MouseLeave() {
    m_mouse_in_resize_tab = false;
    Wnd::MouseLeave();
}

GG::Pt CUIWnd::ClientUpperLeft() const
{ return m_minimized ? UpperLeft() : UpperLeft() + GG::Pt(BORDER_LEFT, TopBorder()); }

GG::Pt CUIWnd::ClientLowerRight() const
{ return m_minimized ? LowerRight() : LowerRight() - GG::Pt(BORDER_RIGHT, BORDER_BOTTOM); }

bool CUIWnd::InWindow(const GG::Pt& pt) const {
    GG::Pt lr = LowerRight();
    if (m_resizable) {
        return UpperLeft() <= pt && pt < lr;
    } else {
        GG::Pt dist_from_lr = lr - pt;
        bool inside_lower_right_corner = OUTER_EDGE_ANGLE_OFFSET < Value(dist_from_lr.x) + Value(dist_from_lr.y);
        return (UpperLeft() <= pt && pt < lr && inside_lower_right_corner);
    }
}

void CUIWnd::PositionButtons() {
    // The buttons are to be positioned based on the presence of other buttons
    GG::Pt button_ul = GG::Pt(Width() - ClientUI::Pts() - BORDER_RIGHT, BUTTON_TOP_OFFSET) + UpperLeft() - ClientUpperLeft();

    if (m_close_button) {
        m_close_button->MoveTo(GG::Pt(button_ul.x, button_ul.y));
        button_ul -= GG::Pt(m_close_button->Width(), GG::Y0) + GG::Pt(GG::X(ClientUI::TitlePts()/3), GG::Y0);
    }

    if (m_minimize_button) {
        m_minimize_button->MoveTo(GG::Pt(button_ul.x, button_ul.y));
        button_ul -= GG::Pt(m_minimize_button->Width(), GG::Y0) + GG::Pt(GG::X(ClientUI::TitlePts()/3), GG::Y0);
    }

    if (m_pin_button)
        m_pin_button->MoveTo(GG::Pt(button_ul.x, button_ul.y));
}

void CUIWnd::InitButtons() {
    boost::filesystem::path button_texture_dir = ClientUI::ArtDir() / "icons" / "buttons";

    // create the close button
    if (m_closable) {
        m_close_button = new CUIButton(
            GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "close.png")),
            GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "close_clicked.png")),
            GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "close_mouseover.png")));
        m_close_button->SetColor(ClientUI::WndInnerBorderColor());
        GG::Connect(m_close_button->LeftClickedSignal, &PlayCloseSound, -1);
        m_close_button->Resize(GG::Pt(GG::X(ClientUI::TitlePts()), GG::Y(ClientUI::TitlePts())));
        GG::Connect(m_close_button->LeftClickedSignal, &CUIWnd::CloseClicked, this);
        AttachChild(m_close_button);
        m_close_button->NonClientChild(true);
    }

    // create the minimize button
    if (m_minimizable) {
        m_minimize_button = new CUI_MinRestoreButton();
        m_minimize_button->Resize(GG::Pt(GG::X(ClientUI::TitlePts()), GG::Y(ClientUI::TitlePts())));
        GG::Connect(m_minimize_button->LeftClickedSignal, &CUIWnd::MinimizeClicked, this);
        AttachChild(m_minimize_button);
        m_minimize_button->NonClientChild(true);
    }

    // create the pin button
    if (m_pinable) {
        m_pin_button = new CUI_PinButton();
        m_pin_button->Resize(GG::Pt(GG::X(ClientUI::TitlePts()), GG::Y(ClientUI::TitlePts())));
        GG::Connect(m_pin_button->LeftClickedSignal, &CUIWnd::PinClicked, this);
        AttachChild(m_pin_button);
        m_pin_button->NonClientChild(true);
    }

    // All buttons were created at the same spot, position them to the correct spot
    PositionButtons();
}

GG::Pt CUIWnd::MinimizedSize() const
{ return GG::Pt(MINIMIZED_WND_WIDTH, TopBorder()); }

GG::X CUIWnd::LeftBorder() const
{ return BORDER_LEFT; }

GG::Y CUIWnd::TopBorder() const
{ return GG::Y(ClientUI::TitlePts()*3/2); }

GG::X CUIWnd::RightBorder() const
{ return BORDER_RIGHT; }

GG::Y CUIWnd::BottomBorder() const
{ return BORDER_BOTTOM; }

int CUIWnd::InnerBorderAngleOffset() const
{ return INNER_BORDER_ANGLE_OFFSET; }

void CUIWnd::CloseClicked() {
    m_done = true;
    if (Parent())
        Parent()->DetachChild(this);
    else
        GG::GUI::GetGUI()->Remove(this);

    //m_minimized_buffer.clear();
    //m_outer_border_buffer.clear();
    //m_inner_border_buffer.clear();
    //m_resize_corner_lines_buffer.clear();
}

void CUIWnd::PinClicked() {
    m_pinned = !m_pinned;
    m_resizable = !m_pinned;
    m_pin_button->Toggle(m_pinned); // Change the icon on the pin button
    m_vertex_buffer.clear();        // force buffer re-init on next Render call
    SaveOptions();
}

void CUIWnd::MinimizeClicked() {
    if (!m_minimized) {
        m_minimized = true;
        m_original_size = Size();
        SetMinSize(MinimizedSize());
        Resize(MinimizedSize());

        // hide all children, re-showing only position/size controls
        Hide();
        Show(false);
        if (m_close_button)
            m_close_button->Show();
        if (m_minimize_button)
            m_minimize_button->Show();
        if (m_pin_button)
            m_pin_button->Show();

    } else {
        m_minimized = false;
        SetMinSize(GG::Pt(MinimizedSize().x, TopBorder() + INNER_BORDER_ANGLE_OFFSET + BORDER_BOTTOM + 10));
        Resize(GG::Pt(m_original_size));
        Show();
    }
    SaveOptions();
}

void CUIWnd::InitBuffers() {
    m_vertex_buffer.clear();
    m_vertex_buffer.reserve(17);
    m_buffer_indices.resize(4);
    std::size_t previous_buffer_size = m_vertex_buffer.size();

    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Pt cl_ul = ClientUpperLeft(), cl_lr = ClientLowerRight();

    // within m_vertex_buffer:
    // [0] is the start and range for minimized background triangle fan and minimized border line loop
    // [1] is ... the background fan / outer border line loop
    // [2] is ... the inner border line loop
    // [3] is ... the resize tab line list

    // minimized background fan and border line loop
    m_vertex_buffer.store(Value(ul.x),  Value(ul.y));
    m_vertex_buffer.store(Value(lr.x),  Value(ul.y));
    m_vertex_buffer.store(Value(lr.x),  Value(lr.y));
    m_vertex_buffer.store(Value(ul.x),  Value(lr.y));
    m_buffer_indices[0].first = previous_buffer_size;
    m_buffer_indices[0].second = m_vertex_buffer.size() - previous_buffer_size;
    previous_buffer_size = m_vertex_buffer.size();

    // outer border, with optional corner cutout
    m_vertex_buffer.store(Value(ul.x),  Value(ul.y));
    m_vertex_buffer.store(Value(lr.x),  Value(ul.y));
    if (!m_resizable) {
        m_vertex_buffer.store(Value(lr.x),                            Value(lr.y) - OUTER_EDGE_ANGLE_OFFSET);
        m_vertex_buffer.store(Value(lr.x) - OUTER_EDGE_ANGLE_OFFSET,  Value(lr.y));
    } else {
        m_vertex_buffer.store(Value(lr.x),  Value(lr.y));
    }
    m_vertex_buffer.store(Value(ul.x),      Value(lr.y));
    m_buffer_indices[1].first = previous_buffer_size;
    m_buffer_indices[1].second = m_vertex_buffer.size() - previous_buffer_size;
    previous_buffer_size = m_vertex_buffer.size();

    // inner border, with optional corner cutout
    m_vertex_buffer.store(Value(cl_ul.x),       Value(cl_ul.y));
    m_vertex_buffer.store(Value(cl_lr.x),       Value(cl_ul.y));
    if (m_resizable) {
        m_vertex_buffer.store(Value(cl_lr.x),                             Value(cl_lr.y) - INNER_BORDER_ANGLE_OFFSET);
        m_vertex_buffer.store(Value(cl_lr.x) - INNER_BORDER_ANGLE_OFFSET, Value(cl_lr.y));
    } else {
        m_vertex_buffer.store(Value(cl_lr.x),   Value(cl_lr.y));
    }
    m_vertex_buffer.store(Value(cl_ul.x),       Value(cl_lr.y));
    m_buffer_indices[2].first = previous_buffer_size;
    m_buffer_indices[2].second = m_vertex_buffer.size() - previous_buffer_size;
    previous_buffer_size = m_vertex_buffer.size();

    // resize hash marks
    m_vertex_buffer.store(Value(cl_lr.x),                           Value(cl_lr.y) - RESIZE_HASHMARK1_OFFSET);
    m_vertex_buffer.store(Value(cl_lr.x) - RESIZE_HASHMARK1_OFFSET, Value(cl_lr.y));
    m_vertex_buffer.store(Value(cl_lr.x),                           Value(cl_lr.y) - RESIZE_HASHMARK2_OFFSET);
    m_vertex_buffer.store(Value(cl_lr.x) - RESIZE_HASHMARK2_OFFSET, Value(cl_lr.y));
    m_buffer_indices[3].first = previous_buffer_size;
    m_buffer_indices[3].second = m_vertex_buffer.size() - previous_buffer_size;
    //previous_buffer_size = m_vertex_buffer.size();

    m_vertex_buffer.createServerBuffer();

    //std::cout << "CUIWnd vertex buffer final size: " << previous_buffer_size << std::endl;
}

void CUIWnd::Hide(bool children) {
    GG::Wnd::Hide(children);
    SaveOptions();
}

void CUIWnd::Show(bool children) {
    GG::Wnd::Show(children);
    SaveOptions();
}

void CUIWnd::ResetDefaultPosition() {
    GG::Rect default_position = CalculatePosition();
    if (default_position.ul.x != INVALID_X) // do nothing if not overridden
        InitSizeMove(default_position.ul, default_position.lr);
}

GG::Rect CUIWnd::CalculatePosition() const
{ return GG::Rect(INVALID_X, INVALID_Y, INVALID_X, INVALID_Y); }

void CUIWnd::SaveOptions() const {
    OptionsDB& db = GetOptionsDB();

    // The default empty string means 'do not save/load properties'
    // Also do not save while the window is being dragged.
    if (m_config_name.empty() || !m_config_save || GG::GUI::GetGUI()->DragWnd(this, 0)) {
        return;
    } else if (!db.OptionExists("UI.windows."+m_config_name+".initialized")) {
        ErrorLogger() << "CUIWnd::SaveOptions() : attempted to save window options using name \"" << m_config_name << "\" but the options do not appear to be registered in the OptionsDB.";
        return;
    } else if (!db.Get<bool>("UI.windows."+m_config_name+".initialized")) {
        // Don't save until the window has been given its proper default values
        return;
    }

    GG::Pt size;
    if (m_minimized)
        size = m_original_size;
    else
        size = Size();

    std::string windowed = ""; // empty string in fullscreen mode, appends -windowed in windowed mode
    if (!db.Get<bool>("fullscreen"))
        windowed = "-windowed";

    db.Set<int>("UI.windows."+m_config_name+".left"+windowed,   Value(RelativeUpperLeft().x));
    db.Set<int>("UI.windows."+m_config_name+".top"+windowed,    Value(RelativeUpperLeft().y));
    db.Set<int>("UI.windows."+m_config_name+".width"+windowed,  Value(size.x));
    db.Set<int>("UI.windows."+m_config_name+".height"+windowed, Value(size.y));

    if (!Modal()) {
        db.Set<bool>("UI.windows."+m_config_name+".visible", Visible());
        db.Set<bool>("UI.windows."+m_config_name+".pinned", m_pinned);
        db.Set<bool>("UI.windows."+m_config_name+".minimized", m_minimized);
    }

    db.Commit();
}

void CUIWnd::LoadOptions() {
    OptionsDB& db = GetOptionsDB();

    // The default empty string means 'do not save/load properties'
    if (m_config_name.empty()) {
        return;
    } else if (!db.OptionExists("UI.windows."+m_config_name+".initialized")) {
        ErrorLogger() << "CUIWnd::LoadOptions() : attempted to load window options using name \"" << m_config_name << "\" but the options do not appear to be registered in the OptionsDB.";
        return;
    }

    // These functions are only called in certain circumstances, could pass in
    // things like the fullscreen/windowed mode instead of using global program
    // state like this?
    std::string windowed = ""; // empty string in fullscreen mode, appends -windowed in windowed mode
    if (!db.Get<bool>("fullscreen"))
        windowed = "-windowed";

    GG::Pt ul   = GG::Pt(GG::X(db.Get<int>("UI.windows."+m_config_name+".left"+windowed)),
                         GG::Y(db.Get<int>("UI.windows."+m_config_name+".top"+windowed)));
    GG::Pt size = GG::Pt(GG::X(db.Get<int>("UI.windows."+m_config_name+".width"+windowed)),
                         GG::Y(db.Get<int>("UI.windows."+m_config_name+".height"+windowed)));

    m_config_save = false;

    if (m_minimized) {
        MinimizeClicked();
    }

    if (ul.x == INVALID_X || ul.y == INVALID_Y) {
        // If no options have been saved yet, allow the window to calculate its
        // own position.  Note that when this is first called from the CUIWnd
        // constructor it will call CUIWnd::CalculatePosition() (a no-op) but
        // afterwards will call derived overrides.  This will still do nothing
        // for windows that don't override CalculatePosition() but they should
        // be positioned by their owners in any case.
        ResetDefaultPosition();
    } else {
        SizeMove(ul, ul + size);
    }

    if (!Modal()) {
        if (db.Get<bool>("UI.windows."+m_config_name+".visible")) {
            Show();
        } else {
            Hide();
        }

        if (db.Get<bool>("UI.windows."+m_config_name+".pinned") != m_pinned) {
            PinClicked();
        }

        if (db.Get<bool>("UI.windows."+m_config_name+".minimized") != m_minimized) {
            MinimizeClicked();
        }
    }

    m_config_save = true;
}

const std::string CUIWnd::AddWindowOptions(const std::string& config_name,
                                           int left, int top,
                                           int width, int height,
                                           bool visible, bool pinned, bool minimized)
{
    OptionsDB& db = GetOptionsDB();
    std::string new_name = "";

    if (db.OptionExists("UI.windows."+config_name+".left")) {
        // If the option has already been added, a window was previously created with this name...
        if (config_name.empty()) {
            // Should never happen, but just in case.
            db.Remove("UI.windows..left");
            db.Remove("UI.windows..top");
            db.Remove("UI.windows..left-windowed");
            db.Remove("UI.windows..top-windowed");
            db.Remove("UI.windows..width");
            db.Remove("UI.windows..height");
            db.Remove("UI.windows..width-windowed");
            db.Remove("UI.windows..height-windowed");
            db.Remove("UI.windows..visible");
            db.Remove("UI.windows..pinned");
            db.Remove("UI.windows..minimized");
            ErrorLogger() << "CUIWnd::AddWindowOptions() : Found window options with a blank name, removing those options.";
        } else if (db.OptionExists("UI.windows."+config_name+".initialized")) {
            // If the window's still there, shouldn't use the same name (but the window can still be created so don't throw)
            ErrorLogger() << "CUIWnd::AddWindowOptions() : Attempted to create a window with config_name = " << config_name << " but one already exists with that name.";
        } else {
            // Old window has been destroyed, use the properties it had.
            db.Add<bool>("UI.windows."+config_name+".initialized",      UserStringNop("OPTIONS_DB_UI_WINDOWS_EXISTS"),          false,      Validator<bool>(),              false);
            new_name = config_name;
        }
    } else if (!config_name.empty()) {
        const int max_width_plus_one = HumanClientApp::MaximumPossibleWidth() + 1;
        const int max_height_plus_one = HumanClientApp::MaximumPossibleHeight() + 1;

        db.Add<bool>("UI.windows."+config_name+".initialized",      UserStringNop("OPTIONS_DB_UI_WINDOWS_EXISTS"),          false,      Validator<bool>(),              false);

        db.Add<int> ("UI.windows."+config_name+".left",             UserStringNop("OPTIONS_DB_UI_WINDOWS_LEFT"),            left,       OrValidator<int>(RangedValidator<int>(0, max_width_plus_one), DiscreteValidator<int>(INVALID_POS)));
        db.Add<int> ("UI.windows."+config_name+".top",              UserStringNop("OPTIONS_DB_UI_WINDOWS_TOP"),             top,        OrValidator<int>(RangedValidator<int>(0, max_height_plus_one), DiscreteValidator<int>(INVALID_POS)));
        db.Add<int> ("UI.windows."+config_name+".left-windowed",    UserStringNop("OPTIONS_DB_UI_WINDOWS_LEFT_WINDOWED"),   left,       OrValidator<int>(RangedValidator<int>(0, max_width_plus_one), DiscreteValidator<int>(INVALID_POS)));
        db.Add<int> ("UI.windows."+config_name+".top-windowed",     UserStringNop("OPTIONS_DB_UI_WINDOWS_TOP_WINDOWED"),    top,        OrValidator<int>(RangedValidator<int>(0, max_height_plus_one), DiscreteValidator<int>(INVALID_POS)));

        db.Add<int> ("UI.windows."+config_name+".width",            UserStringNop("OPTIONS_DB_UI_WINDOWS_WIDTH"),           width,      RangedValidator<int>(0, max_width_plus_one));
        db.Add<int> ("UI.windows."+config_name+".height",           UserStringNop("OPTIONS_DB_UI_WINDOWS_HEIGHT"),          height,     RangedValidator<int>(0, max_height_plus_one));
        db.Add<int> ("UI.windows."+config_name+".width-windowed",   UserStringNop("OPTIONS_DB_UI_WINDOWS_WIDTH_WINDOWED"),  width,      RangedValidator<int>(0, max_width_plus_one));
        db.Add<int> ("UI.windows."+config_name+".height-windowed",  UserStringNop("OPTIONS_DB_UI_WINDOWS_HEIGHT_WINDOWED"), height,     RangedValidator<int>(0, max_height_plus_one));

        db.Add<bool>("UI.windows."+config_name+".visible",          UserStringNop("OPTIONS_DB_UI_WINDOWS_VISIBLE"),         visible,    Validator<bool>());
        db.Add<bool>("UI.windows."+config_name+".pinned",           UserStringNop("OPTIONS_DB_UI_WINDOWS_PINNED"),          pinned,     Validator<bool>());
        db.Add<bool>("UI.windows."+config_name+".minimized",        UserStringNop("OPTIONS_DB_UI_WINDOWS_MINIMIZED"),       minimized,  Validator<bool>());

        new_name = config_name;
    }

    return new_name;
}

const std::string CUIWnd::AddWindowOptions(const std::string& config_name,
                                           GG::X left, GG::Y top,
                                           GG::X width, GG::Y height,
                                           bool visible, bool pinned, bool minimized)
{
    return AddWindowOptions(config_name,
                            Value(left), Value(top),
                            Value(width), Value(height),
                            visible, pinned, minimized);
}

void CUIWnd::InvalidateWindowOptions(const std::string& config_name) {
    OptionsDB& db = GetOptionsDB();
    if (db.OptionExists("UI.windows."+config_name+".initialized")) {
        // Should be removed in window dtor.
        ErrorLogger() << "CUIWnd::RemoveWindowOptions() : attempted to remove window options using name \"" << config_name << "\" but they appear to be in use by a window.";
        return;
    } else if (!db.OptionExists("UI.windows."+config_name+".left")) {
        ErrorLogger() << "CUIWnd::RemoveWindowOptions() : attempted to remove window options using name \"" << config_name << "\" but the options do not appear to be registered in the OptionsDB.";
        return;
    }

    std::string windowed = ""; // empty string in fullscreen mode, appends -windowed in windowed mode
    if (!db.Get<bool>("fullscreen"))
        windowed = "-windowed";

    db.Set<int>("UI.windows."+config_name+".left"+windowed, INVALID_POS);
    db.Set<int>("UI.windows."+config_name+".top"+windowed,  INVALID_POS);
    db.Set<bool>("UI.windows."+config_name+".visible", db.GetDefault<bool>("UI.windows."+config_name+".visible"));
    db.Set<bool>("UI.windows."+config_name+".pinned", db.GetDefault<bool>("UI.windows."+config_name+".pinned"));
    db.Set<bool>("UI.windows."+config_name+".minimized", db.GetDefault<bool>("UI.windows."+config_name+".minimized"));
}

void CUIWnd::InvalidateUnusedOptions() {
    OptionsDB& db = GetOptionsDB();
    std::string prefix("UI.windows.");
    std::string suffix_used(".initialized"); // this is present if the options are being used by a window
    std::string suffix_exist(".left");

    // Remove unrecognized options from the DB so that their values aren't
    // applied when they are eventually registered.
    db.RemoveUnrecognized(prefix);

    // Removed registered options for windows that aren't currently
    // instantiated so they fall back on defaults when they are re-constructed.
    std::set<std::string> window_options;
    db.FindOptions(window_options, prefix);
    for (const std::string& option : window_options) {
        // If the ".left" option is registered, the rest are implied to be
        // there.
        if (option.rfind(suffix_exist) == option.length() - suffix_exist.length() &&
            db.OptionExists(option))
        {
            std::string name = option.substr(prefix.length(), option.length() - prefix.length() - suffix_exist.length());
            // If the ".initialized" option isn't present under this name,
            // remove the options.
            if (window_options.find(prefix + name + suffix_used) == window_options.end()) {
                InvalidateWindowOptions(name);
            }
        }
    }

    db.Commit();
}

void CUIWnd::SetParent(GG::Wnd* wnd) {
    GG::Wnd::SetParent(wnd);
    m_vertex_buffer.clear();    // force buffer re-init on next Render call, so background is properly positioned for new parent-relative position
}

///////////////////////////////////////
// class CUIEditWnd
///////////////////////////////////////
const GG::X CUIEditWnd::BUTTON_WIDTH(75);
const int CUIEditWnd::CONTROL_MARGIN = 5;

CUIEditWnd::CUIEditWnd(GG::X w, const std::string& prompt_text, const std::string& edit_text, GG::Flags<GG::WndFlag> flags/* = Wnd::MODAL*/) :
    CUIWnd(prompt_text, GG::X0, GG::Y0, w, GG::Y1, flags)
{
    m_edit = new CUIEdit(edit_text);
    m_ok_bn = new CUIButton(UserString("OK"));
    m_cancel_bn = new CUIButton(UserString("CANCEL"));

    m_edit->MoveTo(GG::Pt(LeftBorder() + 3, TopBorder() + 3));
    m_edit->Resize(GG::Pt(ClientWidth() - 2 * BUTTON_WIDTH - 2 * CONTROL_MARGIN - 6 - LeftBorder() - RightBorder(), m_edit->MinUsableSize().y));

    m_ok_bn->MoveTo(GG::Pt(m_edit->Right() + CONTROL_MARGIN, TopBorder() + 3));
    m_ok_bn->Resize(GG::Pt(BUTTON_WIDTH, m_ok_bn->MinUsableSize().y));
    m_ok_bn->OffsetMove(GG::Pt(GG::X0, (m_edit->Height() - m_ok_bn->Height()) / 2));

    m_cancel_bn->MoveTo(GG::Pt(m_ok_bn->Right() + CONTROL_MARGIN, TopBorder() + 3));
    m_cancel_bn->Resize(GG::Pt(BUTTON_WIDTH, m_cancel_bn->MinUsableSize().y));
    m_cancel_bn->OffsetMove(GG::Pt(GG::X0, (m_edit->Height() - m_ok_bn->Height()) / 2));

    Resize(GG::Pt(w, std::max(m_edit->Bottom(), m_cancel_bn->Bottom()) + BottomBorder() + 3));
    MoveTo(GG::Pt((GG::GUI::GetGUI()->AppWidth() - w) / 2, (GG::GUI::GetGUI()->AppHeight() - Height()) / 2));

    AttachChild(m_edit);
    AttachChild(m_ok_bn);
    AttachChild(m_cancel_bn);

    GG::Connect(m_ok_bn->LeftClickedSignal,     &CUIEditWnd::OkClicked, this);
    GG::Connect(m_cancel_bn->LeftClickedSignal, &CUIWnd::CloseClicked, static_cast<CUIWnd*>(this));

    m_edit->SelectAll();
}

void CUIEditWnd::ModalInit()
{ GG::GUI::GetGUI()->SetFocusWnd(m_edit); }

void CUIEditWnd::KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) {
    switch (key) {
    case GG::GGK_RETURN: if (!m_ok_bn->Disabled()) OkClicked(); break;
    case GG::GGK_ESCAPE: CloseClicked(); break;
    default: break;
    }
}

const std::string& CUIEditWnd::Result() const
{ return m_result; }

void CUIEditWnd::OkClicked() {
    m_result = m_edit->Text();
    CloseClicked();
}
