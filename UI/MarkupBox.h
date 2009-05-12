// -*- C++ -*-
//MarkupBox.h
#ifndef _MARKUP_BOX_H_
#define _MARKUP_BOX_H_

#include <GG/Control.h>
namespace GG {
    class Scroll;
}
namespace {
    struct MarkupTextBlock;
}


/** A control similar to GG::MultiEdit that displayed text, links, and images with layout determined
  * from HTML-like markup in the provided text. */
class MarkupBox : public GG::Control
{
public:
    /** \name Structors */ ///@{
    /** Ctor. */
    MarkupBox(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str, GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE);

    MarkupBox();                ///< default ctor

    virtual ~MarkupBox();       ///< dtor
    //@}

    /** \name Accessors */ ///@{
    const std::string&  Text() const;
    //@}

    /** \name Mutators */ ///@{
    virtual void        Render();
    virtual void        SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    virtual void        MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);
    virtual void        KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys);

    virtual void        SetText(const std::string& str);
    void                Clear();

    void                Refresh();
    //@}

private:
    class MarkupSurface;

    void                AdjustScrolls();    ///< sets the sizes of the scroll-space and the screen-space of the scrolls
    void                VScrolled(int upper, int ignored1, int ignored2, int ignored3);

    GG::Scroll*                     m_vscroll;      ///< scrollbar used to scroll through marked up text
    MarkupSurface*                  m_surface;      ///< all contents are attached as children of surface so that scrolling only needs to update the surface position to move all contents

    GG::Y                           m_surface_top;  ///< position, relative to top of MarkupBox where MarkupSurface is located.  Used to keep track of scrolling position.
    std::vector<MarkupTextBlock>    m_text_blocks;  ///< result of parsing raw text.  each entry can be rendered as a single GG::Control
};


#endif