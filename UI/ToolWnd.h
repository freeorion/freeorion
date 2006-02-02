// -*- C++ -*-
//ToolWnd.h
#ifndef _ToolWnd_h_
#define _ToolWnd_h_

#ifndef _GGWnd_h_
#include "GGWnd.h"
#endif

#ifndef _GGTextControl_h_
#include "GGTextControl.h"
#endif

/** Implements a Tooltip window containing a string. This window appears when the mouse hovers over its parent
    window for a specified amount of time. */
class ToolWnd : public GG::Wnd
{
public:
    //! \name Structors
    //!@{
    //! Constructs a Tooltip window at (x,y) containing text with the specified background color.
    //! @param x the x coordinate of the tooltip.
    //! @param y the y coordinate of the tooltip.
    //! @param text the string that the tooltip will display
    //! @param clr the background color of the window.  Text is always black.
    ToolWnd(int x, int y, const std::string& text, const GG::Clr& wnd_color, const GG::Clr& border_color,
            const GG::Clr& text_color, const std::string& font_name = "arial.ttf", int pts = 10);
    //!@}

    //! \name Mutators
    //!@{
    virtual bool Render();
    //!@}

private:
    GG::TextControl* m_text;         //!< where the text is contained
    GG::Clr          m_color;        //!< the color of the tool window
    GG::Clr          m_border_color; //!< the color of the border of the tool window
};

inline std::string ToolWndRevision()
{return "$Id$";}

#endif // _ToolWnd_h_
