#ifndef _AccordionPanel_h_
#define _AccordionPanel_h_

#include <GG/GGFwd.h>
#include <GG/Wnd.h>
#include <GG/GLClientAndServerBuffer.h>
#include "GG/Control.h"

class AccordionPanel : public GG::Control {
public:
    static const int EXPAND_BUTTON_SIZE = 16;

    /** \name Structors */ //@{
    AccordionPanel(GG::X w, GG::Y h, bool is_button_on_left = false);
    virtual ~AccordionPanel();
    //@}

    virtual GG::Pt ClientUpperLeft() const;
    virtual GG::Pt ClientLowerRight() const;

    /** \name Mutators */ //@{
    virtual void Render();
    virtual void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);
    virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr);

    /** Sets the interior color of the box. */
    void SetInteriorColor(GG::Clr c);

    //@}

    typedef boost::signals2::signal<void ()> ExpandCollapseSignalType;
    mutable ExpandCollapseSignalType ExpandCollapseSignal;

protected:
    GG::GL2DVertexBuffer    m_border_buffer;

    void            SetCollapsed(bool collapsed);
    bool            IsCollapsed() const;
    virtual void    DoLayout();
    virtual void    InitBuffer();

    GG::Button*             m_expand_button;    ///< at top right/left of panel, toggles the panel open/closed to show details or minimal summary
    bool                    m_collapsed;
    bool                    m_is_left; ///< Is expand button on the left?

    GG::Clr                 m_interior_color;
};

#endif
