#include "MarkupBox.h"

#include "CUIControls.h"

#include <GG/StaticGraphic.h>
#include <GG/MultiEdit.h>
#include <GG/WndEvent.h>
#include <GG/DrawUtil.h>
#include <GG/StyleFactory.h>

namespace {
    static const GG::X SCROLL_WIDTH(14);
    static const int EDGE_PAD(3);
}

//////////////////////////////
// MarkupBox::MarkupSurface //
//////////////////////////////
class MarkupBox::MarkupSurface : public GG::Control {
public:
    /** \name Structors */ ///@{
    /** Ctor. */
    MarkupSurface(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str);

    MarkupSurface();            ///< default ctor
    ~MarkupSurface();           ///< dtor
    //@}

    /** \name Accessors */ ///@{
    const std::string&  Text() const;
    //@}

    /** \name Mutators */ ///@{

    /** Removes old controls and recreates new contents of surface from passed
        \a str text. If PreserveScrollPositionOnNextTextSet() has been called,
        does not alter scroll position.  Otherwise, resets scroll position to
        top of contents. */
    void                SetText(const std::string& str);

    void                Clear();                    ///< Removes all controls from surface.
    void                Refresh();                  ///< Removes and recreates all controls on surface using existing text

    virtual void        Render();
    //@}

private:
    std::string                 m_text;

    std::vector<GG::Control*>   m_controls;     ///< GG::Controls used to display marked up text
};

MarkupBox::MarkupSurface::MarkupSurface(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str) :
    GG::Control(x, y, w, h),
    m_text(""),
    m_controls()
{
    SetText(str);
    EnableChildClipping(true);
}

MarkupBox::MarkupSurface::MarkupSurface() :
    m_text(""),
    GG::Control(),
    m_controls()
{
    EnableChildClipping(true);
}

MarkupBox::MarkupSurface::~MarkupSurface() {
    Clear();
}

const std::string& MarkupBox::MarkupSurface::Text() const {
    return m_text;
}

void MarkupBox::MarkupSurface::SetText(const std::string& str) {
    Clear();
    m_text = str;
    Refresh();
}

void MarkupBox::MarkupSurface::Clear() {
    for (std::vector<GG::Control*>::iterator it = m_controls.begin(); it != m_controls.end(); ++it)
        delete *it;
    m_controls.clear();
    m_text = "";
}

void MarkupBox::MarkupSurface::Refresh() {
    // remove old controls / contents
    for (std::vector<GG::Control*>::iterator it = m_controls.begin(); it != m_controls.end(); ++it)
        delete *it;
    m_controls.clear();

    // recreate controls / content based on current m_str

    // TEMP / TEST~!
    GG::Y top = GG::Y0;
    GG::Control* control = new GG::TextControl(GG::X0, top, m_text, ClientUI::GetFont(), ClientUI::TextColor());
    m_controls.push_back(control);
    AttachChild(control);

    control->MoveTo(GG::Pt(GG::X0, top));
    top += control->Height() + EDGE_PAD;

    std::cout << "MarkupSurface::Refresh control (x,y): " << control->UpperLeft().x << ", " << control->UpperLeft().y <<
                                               " (w,h): " << control->Width() << ", " << control->Height() << std::endl;

    Resize(GG::Pt(Width(), GG::Y(m_text.length()*ClientUI::Pts())));
    // END TEMP / TEST
}

void MarkupBox::MarkupSurface::Render() {
    // do nothing.  controls on surface give it appearance.  background that contains surface give it a background.
}


//////////////////////////////
//        MarkupBox         //
//////////////////////////////
namespace {
    static const std::string EMPTY_STRING("");
}

MarkupBox::MarkupBox(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str, GG::Flags<GG::WndFlag> flags) :
    GG::Control(x, y, w, h, flags),
    m_vscroll(0),
    m_surface(0),
    m_preserve_scroll_position_on_next_text_set(false)
{
    m_surface = new MarkupSurface(GG::X0 + EDGE_PAD, GG::Y0 + EDGE_PAD, w - 2*EDGE_PAD, h - 2*EDGE_PAD, str);
    AttachChild(m_surface);
    EnableChildClipping(true);
    Refresh();
    AdjustScrolls();
    VScrolled(0, 0, 0, 0);
}


MarkupBox::MarkupBox() :
    GG::Control(),
    m_vscroll(0),
    m_surface(0),
    m_preserve_scroll_position_on_next_text_set(false)
{
    m_surface = new MarkupSurface(GG::X0, GG::Y0, GG::X0, GG::Y0, "");
    AttachChild(m_surface);
    EnableChildClipping(true);
    Refresh();
    VScrolled(0, 0, 0, 0);
    // shouldn't need to adjust scrolls since text is empty
}

MarkupBox::~MarkupBox() {
    delete m_vscroll;
    //delete m_surface; // should be unnecessary
}

const std::string& MarkupBox::Text() const {
    if (m_surface)
        return m_surface->Text();
    else
        return EMPTY_STRING;
}

void MarkupBox::Render() {
    // Draw outline and background...

    // copied from CUIWnd
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::Pt cl_ul = ClientUpperLeft();
    GG::Pt cl_lr = ClientLowerRight();

    // use GL to draw the lines
    glDisable(GL_TEXTURE_2D);
    GLint initial_modes[2];
    glGetIntegerv(GL_POLYGON_MODE, initial_modes);

    // draw background
    glPolygonMode(GL_BACK, GL_FILL);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndColor());
        glVertex(ul.x, ul.y);
        glVertex(lr.x, ul.y);
        glVertex(lr.x, lr.y);
        glVertex(ul.x, lr.y);
        glVertex(ul.x, ul.y);
    glEnd();

    // draw outer border on pixel inside of the outer edge of the window
    glPolygonMode(GL_BACK, GL_LINE);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndOuterBorderColor());
        glVertex(ul.x, ul.y);
        glVertex(lr.x, ul.y);
        glVertex(lr.x, lr.y);
        glVertex(ul.x, lr.y);
        glVertex(ul.x, ul.y);
    glEnd();

    // reset this to whatever it was initially
    glPolygonMode(GL_BACK, initial_modes[1]);

    glEnable(GL_TEXTURE_2D);
}

void MarkupBox::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = Size();
    GG::Control::SizeMove(ul, lr);
    if (Size() != old_size) {
        if (m_vscroll)
            m_surface->SizeMove(GG::Pt(GG::X0 + EDGE_PAD,                   GG::Y0 + EDGE_PAD),
                                GG::Pt(Width() - SCROLL_WIDTH - 2*EDGE_PAD, Height() - 2*EDGE_PAD));
        else
            m_surface->SizeMove(GG::Pt(GG::X0 + EDGE_PAD,                   GG::Y0 + EDGE_PAD),
                                GG::Pt(Width() - 2*EDGE_PAD,                Height() - 2*EDGE_PAD));

        Refresh();
    }
}

void MarkupBox::SetText(const std::string& str) {
    if (m_preserve_scroll_position_on_next_text_set) {
        m_preserve_scroll_position_on_next_text_set = false;
        m_surface->SetText(str);
        return;
    }

    // save old surface size
    GG::Pt old_surface_size = m_surface->Size();

    // update text, which might change size of surface
    m_surface->SetText(str);

    // check new surface size.  if it has changed, check if scrollbar positions and layout need changing
    if (m_surface->Size() != old_surface_size)
        AdjustScrolls();
}

void MarkupBox::PreserveScrollPositionOnNextTextSet() {
    m_preserve_scroll_position_on_next_text_set = true;
}

void MarkupBox::Refresh() {
    m_surface->Refresh();
}

void MarkupBox::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys) {
    if (!Disabled() && m_vscroll) {
        for (int i = 0; i < move; ++i)
            m_vscroll->ScrollLineDecr();
        for (int i = 0; i < -move; ++i)
            m_vscroll->ScrollLineIncr();
    }
}

void MarkupBox::KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) {
    if (Disabled()) {
        GG::Control::KeyPress(key, key_code_point, mod_keys);
        return;
    }

    if (!m_vscroll)
        return;

    int bottom = 0;

    switch (key) {
    case GG::GGK_UP:
        m_vscroll->ScrollLineDecr();
        break;
    case GG::GGK_DOWN:
        m_vscroll->ScrollLineIncr();
        break;
    case GG::GGK_HOME:
        m_vscroll->ScrollTo(0);
        break;
    case GG::GGK_END:
        bottom = Value(m_surface->Height() - Height()); // find top position so that bottom of surface is at bottom of this box
        m_vscroll->ScrollTo(std::max(0, bottom));       // but don't allow negative scroll positions, in case this height is larger than surface height
        break;
    case GG::GGK_PAGEUP:
        m_vscroll->ScrollPageDecr();
        break;
    case GG::GGK_PAGEDOWN:
        m_vscroll->ScrollPageIncr();
        break;
    }
}

void MarkupBox::Clear() {
    m_surface->Clear();
    VScrolled(0, 0, 0, 0);
}

void MarkupBox::AdjustScrolls() {
    if (!m_surface) {
        delete m_vscroll;   m_vscroll = NULL;
        return;
    }

    GG::Y surface_height = m_surface->Height();
    GG::Y this_height = Height();

    int line_height = ClientUI::Pts();

    if (m_vscroll) {
        if (surface_height <= this_height) {
            // there is a scrollbar, but it's not needed.  Remove it.
            delete m_vscroll;   m_vscroll = NULL;

            // resize surface to account for extra horizontal space due to lack of scrollbars
            m_surface->SizeMove(UpperLeft(), LowerRight());

            // redo text layout, since removing scrollbar added extra width
            m_surface->Refresh();

            // ensure surface top is at top of this control
            VScrolled(0, 0, 0, 0);

        } else {
            // there is a scrollbar already and it's still needed.  adjust scroll range
            m_vscroll->SizeScroll(0 /* min value scrollbar can take */, Value(surface_height) /* max value scrollbar can take */,
                                  static_cast<unsigned int>(line_height) /* size of one line tick of bar */,
                                  static_cast<unsigned int>(Value(this_height)) /* size of one page tick of bar */);
        }
    } else if (!m_vscroll) {
        if (surface_height <= this_height) {
            // there is no scrollbar, and no bar is needed.  Make sure surface is showing from top.
            VScrolled(0, 0, 0, 0);

        } else {
            // there is no scrollbar, but one is needed.  Create one
            boost::shared_ptr<GG::StyleFactory> style = GetStyleFactory();

            m_vscroll = style->NewMultiEditVScroll(Width() - SCROLL_WIDTH, GG::Y0, SCROLL_WIDTH, Height(),
                                                          ClientUI::TextColor(), GG::CLR_ZERO);

            // adjust size of surface since creating a scrollbar takes up some horizontal space, which affects layout
            m_surface->SizeMove(UpperLeft(), LowerRight() - GG::Pt(SCROLL_WIDTH, GG::Y0));

            // redo layout after changing surface size
            m_surface->Refresh();

            // adjust range of scrolling and size of increments and pages after potentially resizing surface during refresh
            m_vscroll->SizeScroll(0, Value(surface_height),
                                  static_cast<unsigned int>(line_height),
                                  static_cast<unsigned int>(Value(this_height)));

            // show new scrollbar and connect it functionally to this control
            AttachChild(m_vscroll);
            GG::Connect(m_vscroll->ScrolledSignal, &MarkupBox::VScrolled, this);
        }
    }
}

void MarkupBox::VScrolled(int upper, int ignored1, int ignored2, int ignored3) {
    // move surface in opposite direction to distance scroll is scrolled in vertical direction.  Thus, scrolling down
    // reveals more of lower parts of surface, and vice versa.
    m_surface->MoveTo(GG::Pt(GG::X0, GG::Y(-upper)));
}

