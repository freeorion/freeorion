// -*- C++ -*-
/** \file Layout.h \brief Contains the Layout class, which is used to size and
    align GG windows. */

#ifndef _GG_DeferredLayout_h_
#define _GG_DeferredLayout_h_

#include <GG/Layout.h>

namespace GG {

/** \brief An invisible Wnd subclass that arranges its child Wnds during PreRender.

    A DeferredLayout is a layout that does all of the expensive layout operations once per frame
    during PreRender().
*/
class GG_API DeferredLayout : public Layout
{
public:
    /** \name Structors */ ///@{
    /** Ctor. */
    DeferredLayout(X x, Y y, X w, Y h, std::size_t rows, std::size_t columns,
           unsigned int border_margin = 0, unsigned int cell_margin = INVALID_CELL_MARGIN);
    //@}

    /** \name Accessors */ ///@{
    //@}

    /** \name Mutators */ ///@{
    virtual void SizeMove(const Pt& ul, const Pt& lr);
    virtual void PreRender();
    //@}

protected:
    /** \name Mutators */ ///@{
    virtual void   RedoLayout();
    //@}

private:
    Pt                              m_ul_prerender;    ///< ul for PreRender
    Pt                              m_lr_prerender;    ///< lr for PreRender
};

} // namespace GG

#endif
