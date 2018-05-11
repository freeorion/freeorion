#ifndef _AccordionPanel_h_
#define _AccordionPanel_h_

#include <GG/GGFwd.h>
#include <GG/Wnd.h>
#include <GG/GLClientAndServerBuffer.h>
#include "GG/Control.h"

#include <boost/signals2/signal.hpp>


class AccordionPanel : public GG::Control {
public:
    /** \name Structors */ //@{
    AccordionPanel(GG::X w, GG::Y h, bool is_button_on_left = false);
    virtual ~AccordionPanel();
    //@}

    void CompleteConstruction() override;
    GG::Pt ClientUpperLeft() const override;
    GG::Pt ClientLowerRight() const override;

    /** \name Mutators */ //@{
    void Render() override;
    void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys) override;
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    /** Sets the interior color of the box. */
    void SetInteriorColor(GG::Clr c);

    /** Set the number of pixels between the expansion symbol and the
        client area. */
    void SetBorderMargin(unsigned int margin);
    //@}

    typedef boost::signals2::signal<void ()> ExpandCollapseSignalType;
    mutable ExpandCollapseSignalType ExpandCollapseSignal;

protected:
    void            SetCollapsed(bool collapsed);
    bool            IsCollapsed() const;
    virtual void    DoLayout();
    virtual void    InitBuffer();

    GG::GL2DVertexBuffer        m_border_buffer;
    std::shared_ptr<GG::Button> m_expand_button;    ///< at top right/left of panel, toggles the panel open/closed to show details or minimal summary;
    bool                        m_collapsed = true;
    bool                        m_is_left = false;  ///< Is expand button on the left?
    GG::Clr                     m_interior_color;
    unsigned int                m_border_margin = 1;///< The number of pixels between the expansion button and the client area.
};

#endif
