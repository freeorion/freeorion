// -*- C++ -*-
//CUIControls.h
#ifndef _CUIControls_h_
#define _CUIControls_h_

#ifndef _ClientUI_h_
#include "ClientUI.h"
#endif

#ifndef _GGButton_
#include "GGButton.h"
#endif

#ifndef _CUIDrawUtil_h_
#include "CUIDrawUtil.h"
#endif

#ifndef _GGListbox_h_
#include "GGDropDownList.h"
#endif

#ifndef _GGEdit_h_
#include "GGEdit.h"
#endif

#ifndef _GGFileDlg_h_
#include "GGFileDlg.h"
#endif

#ifndef _GGMenu_h_
#include "GGMenu.h"
#endif

#ifndef _GGMultiEdit_h_
#include "GGMultiEdit.h"
#endif

#ifndef _GGScroll_h_
#include "GGScroll.h"
#endif

namespace GG {
    class StaticGraphic;
}


//! \file All CUI* classes are FreeOrion-style controls incorporating 
//! the visual theme the project requires.  Implementation may
//! depend on graphics and design team specifications.  They extend
//! GG controls.


/** a FreeOrion Button control */
class CUIButton : public GG::Button
{
public:
    /** \name Structors */ //@{
    CUIButton(int x, int y, int w, const std::string& str, const std::string& font_filename = ClientUI::FONT, int pts = ClientUI::PTS, 
              GG::Clr color = ClientUI::BUTTON_COLOR, GG::Clr border = ClientUI::CTRL_BORDER_COLOR, int thick = 1, 
              GG::Clr text_color = ClientUI::TEXT_COLOR, Uint32 flags = GG::Wnd::CLICKABLE); ///< basic ctor
    CUIButton(const GG::XMLElement& elem); ///< ctor that constructs a CUIButton object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a CUIButton object
    //@}

    /** \name Accessors */ //@{
    GG::Clr     BorderColor() const {return m_border_color;} ///< returns the color used to render the border of the button
    int         BorderThickness() const {return m_border_thick;} ///< returns the width used to render the border of the button

    virtual bool            InWindow(const GG::Pt& pt) const;
    virtual GG::XMLElement  XMLEncode() const; ///< constructs an XMLElement from a CUIButton object
    //@}

    /** \name Mutators */ //@{
    virtual void   MouseHere(const GG::Pt& pt, Uint32 keys);
    //@}

protected:
    /** \name Mutators control */ //@{
    virtual void RenderPressed();
    virtual void RenderRollover();
    virtual void RenderUnpressed();
    //@}

private:
    GG::Clr m_border_color;
    int     m_border_thick;
};


/** a FreeOrion next-turn button control */
class CUITurnButton : public CUIButton
{
public:
    /** \name Structors */ //@{
    CUITurnButton(int x, int y, int w, const std::string& str, const std::string& font_filename = ClientUI::FONT, int pts = ClientUI::PTS, 
                  GG::Clr color = ClientUI::BUTTON_COLOR, GG::Clr border = ClientUI::CTRL_BORDER_COLOR, int thick = 1, 
                  GG::Clr text_color = ClientUI::TEXT_COLOR, Uint32 flags = GG::Wnd::CLICKABLE); ///< basic ctor
    CUITurnButton(const GG::XMLElement& elem); ///< ctor that constructs a CUITurnButton object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a CUITurnButton object
    //@}

    /** \name Accessors */ //@{
    virtual GG::XMLElement  XMLEncode() const; ///< constructs an XMLElement from a CUITurnButton object
    //@}
};


/** a FreeOrion triangular arrow button */
class CUIArrowButton : public GG::Button
{
public:
    /** \name Structors */ //@{
    CUIArrowButton(int x, int y, int w, int h, ShapeOrientation orientation, GG::Clr color, Uint32 flags = GG::Wnd::CLICKABLE); ///< basic ctor
    CUIArrowButton(const GG::XMLElement& elem); ///< ctor that constructs a CUIScroll::ScrollTab object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a CUIScroll::ScrollTab object
    //@}

    /** \name Accessors */ //@{
    virtual bool            InWindow(const GG::Pt& pt) const;
    virtual GG::XMLElement  XMLEncode() const; ///< constructs an XMLElement from a CUIScroll::ScrollTab object
    //@}

    /** \name Mutators */ //@{
    virtual void   MouseHere(const GG::Pt& pt, Uint32 keys);
    //@}

protected:
    /** \name Mutators control */ //@{
    virtual void RenderPressed();
    virtual void RenderRollover();
    virtual void RenderUnpressed();
    //@}

private:
    ShapeOrientation m_orientation;
};


/** a FreeOrion StateButton control */
class CUIStateButton : public GG::StateButton
{
public:
    /** styles used to render CUIStateButtons; these are in addition to the values in the GG::StateButton::StateButtonStyle enum.*/
    enum CUIStateButtonStyle {SBSTYLE_CUI_CHECKBOX = 5, SBSTYLE_CUI_RADIO_BUTTON};

    /** \name Structors */ //@{
    CUIStateButton(int x, int y, int w, int h, const std::string& str, Uint32 text_fmt, Uint32 style = SBSTYLE_CUI_CHECKBOX, 
                   GG::Clr color = ClientUI::STATE_BUTTON_COLOR, const std::string& font_filename = ClientUI::FONT, 
                   int pts = ClientUI::PTS, GG::Clr text_color = ClientUI::TEXT_COLOR, GG::Clr interior = GG::CLR_ZERO, 
                   GG::Clr border = ClientUI::CTRL_BORDER_COLOR, int bn_x = -1, int bn_y = -1, int bn_w = -1, int bn_h = -1, 
                   Uint32 flags = CLICKABLE); ///< ctor
    CUIStateButton(const GG::XMLElement& elem); ///< ctor that constructs a CUIStateButton object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a CUIStateButton object
    //@}

    /** \name Accessors */ //@{
    GG::Clr     BorderColor() const {return m_border_color;} ///< returns the color used to render the border of the button

    virtual GG::XMLElement  XMLEncode() const; ///< constructs an XMLElement from a Button object
    //@}

    /** \name Mutators */ //@{
    virtual bool  Render();
    //@}

private:
    GG::Clr m_border_color;
};


/** a FreeOrion Scroll control */
class CUIScroll : public GG::Scroll
{
public:
    using Wnd::SizeMove;

    /** represents the tab button for a CUIScroll */
    class ScrollTab : public GG::Button
    {
    public:
        ScrollTab(GG::Scroll::Orientation orientation, int scroll_width, GG::Clr color, GG::Clr border_color); ///< basic ctor
        ScrollTab(const GG::XMLElement& elem); ///< ctor that constructs a CUIScroll::ScrollTab object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a CUIScroll::ScrollTab object
        virtual GG::XMLElement  XMLEncode() const; ///< constructs an XMLElement from a CUIScroll::ScrollTab object
        virtual bool Render();
    private:
        GG::Clr m_border_color;
    };

    /** \name Structors */ //@{
    CUIScroll(int x, int y, int w, int h, Orientation orientation, GG::Clr color = GG::CLR_ZERO, 
              GG::Clr border = ClientUI::CTRL_BORDER_COLOR, GG::Clr interior = GG::CLR_ZERO, Uint32 flags = CLICKABLE); ///< basic ctor
    CUIScroll(const GG::XMLElement& elem); ///< ctor that constructs a CUIScroll object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a CUIScroll object
    //@}

    /** \name Accessors */ //@{
    GG::Clr BorderColor() const {return m_border_color;} ///< returns the color used to render the border of the control

    virtual GG::XMLElement  XMLEncode() const; ///< constructs an XMLElement from a CUIScroll object
    //@}

    /** \name Mutators */ //@{
    virtual bool    Render();
    virtual void    SizeMove(int x1, int y1, int x2, int y2);
    //@}

private:
    GG::Clr m_border_color;

    static GG::Button* NewDummyButton(); ///< creates "dummy up and down buttons, not actually used in FreeOrion's scrolls
};


/** a FreeOrion ListBox control */
class CUIListBox : public GG::ListBox
{
public:
    /** \name Structors */ //@{
    CUIListBox(int x, int y, int w, int h, GG::Clr color = ClientUI::CTRL_BORDER_COLOR, GG::Clr interior = GG::CLR_ZERO, 
               Uint32 flags = CLICKABLE | DRAG_KEEPER); ///< basic ctor

    /** ctor that allows the specification of column widths */
    CUIListBox(int x, int y, int w, int h, const std::vector<int>& col_widths, GG::Clr color = ClientUI::CTRL_BORDER_COLOR, 
               GG::Clr interior = GG::CLR_ZERO, Uint32 flags = CLICKABLE | DRAG_KEEPER);

    CUIListBox(const GG::XMLElement& elem); ///< ctor that constructs a CUIListBox object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a CUIListBox object
    //@}

    /** \name Accessors */ //@{
    virtual GG::XMLElement  XMLEncode() const; ///< constructs an XMLElement from a CUIListBox object
    //@}

    /** \name Mutators */ //@{
    virtual bool   Render();
    //@}

protected:
    /** \name Mutators */ //@{
    virtual GG::Scroll* NewVScroll(bool horz_scroll);
    virtual GG::Scroll* NewHScroll(bool vert_scroll);
    //@}
};


/** a FreeOrion DropDownList control */
class CUIDropDownList : public GG::DropDownList
{
public:
    /** \name Structors */ //@{
    CUIDropDownList(int x, int y, int w, int row_ht, int drop_ht, GG::Clr color = ClientUI::CTRL_BORDER_COLOR, 
                    GG::Clr interior = ClientUI::DROP_DOWN_LIST_INT_COLOR, GG::Clr drop_list_interior = ClientUI::DROP_DOWN_LIST_INT_COLOR, 
                    Uint32 flags = CLICKABLE); ///< basic ctor

    CUIDropDownList(const GG::XMLElement& elem); ///< ctor that constructs a CUIDropDownList object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a CUIDropDownList object
    //@}

    /** \name Accessors */ //@{
    virtual GG::XMLElement  XMLEncode() const; ///< constructs an XMLElement from a CUIDropDownList object
    //@}

    /** \name Mutators */ //@{
    virtual bool   Render();

    void DisableDropArrow();  ///< disables rendering of the small downward-facing arrow on the right of the control
    void EnableDropArrow();   ///< enables rendering of the small downward-facing arrow on the right of the control
    //@}

private:
    bool m_render_drop_arrow;
    GG::Clr m_interior_color;
};


/** a FreeOrion Edit control */
class CUIEdit : public GG::Edit
{
public:
    /** \name Structors */ //@{
    CUIEdit(int x, int y, int w, int h, const std::string& str, const std::string& font_filename = ClientUI::FONT, 
            int pts = ClientUI::PTS, GG::Clr color = ClientUI::CTRL_BORDER_COLOR, GG::Clr text_color = ClientUI::TEXT_COLOR, 
            GG::Clr interior = ClientUI::EDIT_INT_COLOR, Uint32 flags = CLICKABLE | DRAG_KEEPER); ///< basic ctor

    CUIEdit(const GG::XMLElement& elem); ///< ctor that constructs a CUIEdit object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a CUIEdit object
    //@}

    /** \name Accessors */ //@{
    virtual GG::XMLElement  XMLEncode() const; ///< constructs an XMLElement from a CUIDropDownList object
    //@}

    /** \name Mutators */ //@{
    virtual bool   Render();
    //@}
};


/** a FreeOrion MultiEdit control */
class CUIMultiEdit : public GG::MultiEdit
{
public:
    /** \name Structors */ //@{
    CUIMultiEdit(int x, int y, int w, int h, const std::string& str, Uint32 style = GG::TF_LINEWRAP, const std::string& font_filename = ClientUI::FONT, 
                 int pts = ClientUI::PTS, GG::Clr color = ClientUI::CTRL_BORDER_COLOR, GG::Clr text_color = ClientUI::TEXT_COLOR, 
                 GG::Clr interior = ClientUI::MULTIEDIT_INT_COLOR, Uint32 flags = CLICKABLE | DRAG_KEEPER); ///< basic ctor

    CUIMultiEdit(const GG::XMLElement& elem); ///< ctor that constructs a CUIListBox object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a CUIListBox object
    //@}

    /** \name Accessors */ //@{
    virtual GG::XMLElement  XMLEncode() const; ///< constructs an XMLElement from a CUIListBox object
    //@}

    /** \name Mutators */ //@{
    virtual bool   Render();
    //@}

protected:
    /** \name Mutators */ //@{
    virtual GG::Scroll* NewVScroll(bool horz_scroll);
    virtual GG::Scroll* NewHScroll(bool vert_scroll);
    //@}
};


/** A simple GG::ListBox::Row subclass designed for use in text-only drop-down lists, such as the ones used in the game setup dialogs. */
struct CUISimpleDropDownListRow : public GG::ListBox::Row
{
    enum {DEFAULT_ROW_HEIGHT = 22};
    CUISimpleDropDownListRow(const std::string& row_text, int row_height = DEFAULT_ROW_HEIGHT) :
        GG::ListBox::Row("", row_height)
    {
        push_back(row_text, ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
    }
};



/** Encapsulates an icon and text that goes with it in a single control.  For example, "[food icon] +1" or "[population icon] 66", 
    where [... icon] is an icon image, not text.  Note that no rounding is done when DecimalsShown() = 0; in this state, the number 
    displayed will be the truncated integer version of the current value.  Also not that PositiveColor() and NegativeColor() are
    the same color by default, specifically the one \a text_color parameter specified in the ctor. */
class StatisticIcon : public GG::Control
{
public:
    /** \name Structors */ //@{
    StatisticIcon(int x, int y, int w, int h, const std::string& icon_filename, GG::Clr text_color, double value, 
                  int decimals_to_show = 0, bool show_sign = false);
    //@}

    /** \name Accessors */ //@{
    double  Value() const         {return m_value;}            ///< returns the value displayed
    int     DecimalsShown() const {return m_decimals_to_show;} ///< returns the number of places after the decimal point to be shown
    bool    ShowsSign() const     {return m_show_sign;}        ///< returns true iff a sign should always be shown, even for positive values
    GG::Clr PositiveColor() const {return m_positive_color;}   ///< returns the color that will be used to display positive values
    GG::Clr NegativeColor() const {return m_negative_color;}   ///< returns the color that will be used to display negative values
    //@}

    /** \name Mutators */ //@{
    bool Render() {return true;}

    void SetValue(double value); ///< sets the value to be displayed

    void SetDecimalsShown(int d)     {m_decimals_to_show = d; Refresh();} ///< sets the number of places after the decimal point to be shown
    void ShowSign(bool b)            {m_show_sign = b; Refresh();}        ///< sets whether a sign should always be shown, even for positive values
    void SetPositiveColor(GG::Clr c) {m_positive_color = c; Refresh();}   ///< sets the color that will be used to display positive values
    void SetNegativeColor(GG::Clr c) {m_negative_color = c; Refresh();}   ///< sets the color that will be used to display negative values
    //@}

private:
    void Refresh();

    double m_value;
    int m_decimals_to_show;
    bool m_show_sign;
    GG::Clr m_positive_color;
    GG::Clr m_negative_color;
    GG::StaticGraphic* m_icon;
    GG::TextControl* m_text;
};


class StatisticIconDualValue : public GG::Control
{
public:
    /** \name Structors */ //@{
    StatisticIconDualValue(int x, int y, int w, int h, const std::string& icon_filename, GG::Clr text_color, 
                           double value, double value_second,  int decimals_to_show = 0,int decimals_to_show_second = 0,
                           bool show_sign = false, bool show_sign_second = false);
    //@}

    /** \name Accessors */ //@{
    double  Value         () const {return m_value;}            ///< returns the value displayed
    double  ValueSecond   () const {return m_value_second;}     ///< returns the second value displayed
    int     DecimalsShown () const {return m_decimals_to_show;} ///< returns the number of places after the decimal point to be shown
    int     DecimalsShownSecond () const {return m_decimals_to_show_second;} ///< returns the number of places after the decimal point to be shown
    bool    ShowsSign     () const {return m_show_sign;}        ///< returns true iff a sign should always be shown, even for positive values
    bool    ShowsSignSecond() const {return m_show_sign_second;}///< returns true iff a sign should always be shown, even for positive values
    GG::Clr PositiveColor () const {return m_positive_color;}   ///< returns the color that will be used to display positive values
    GG::Clr NegativeColor () const {return m_negative_color;}   ///< returns the color that will be used to display negative values
    //@}

    /** \name Mutators */ //@{
    bool Render() {return true;}

    void SetValue         (double value); ///< sets the value to be displayed
    void SetValueSecond   (double value); ///< sets the value to be displayed
    void SetDecimalsShown (int        d) {m_decimals_to_show  = d; UpdateTextControl();} ///< sets the number of places after the decimal point to be shown
    void SetDecimalsShownSecond (int        d) {m_decimals_to_show  = d; UpdateTextControl();} ///< sets the number of places after the decimal point to be shown
    void ShowSign         (bool       b) {m_show_sign         = b; UpdateTextControl();}        ///< sets whether a sign should always be shown, even for positive values
    void ShowSignSecond   (bool       b) {m_show_sign_second  = b; UpdateTextControl();}        ///< sets whether a sign should always be shown, even for positive values
    void SetPositiveColor (GG::Clr    c) {m_positive_color    = c; UpdateTextControl();}   ///< sets the color that will be used to display positive values
    void SetNegativeColor (GG::Clr    c) {m_negative_color    = c; UpdateTextControl();}   ///< sets the color that will be used to display negative values
   //@}

private:
    ///< sets the textcontrol form cuurent state
    virtual void UpdateTextControl();

    double m_value,m_value_second;

    int m_decimals_to_show,m_decimals_to_show_second;
    bool m_show_sign,m_show_sign_second;
    GG::Clr m_positive_color;
    GG::Clr m_negative_color;
    GG::StaticGraphic* m_icon;
    GG::TextControl* m_text;
};

class CUIToolBar : public GG::Control
{
public:
    /** \name Structors */ //@{
    CUIToolBar(int x, int y, int w, int h);
    //@}

    /** \name Accessors */ //@{
    //@}

    /** \name Mutators */ //@{
    bool Render();
    //@}

private:
};

/** A control used to pick from the empire colors returned by EmpireColors(). */
class EmpireColorSelector : public CUIDropDownList
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void (const GG::Clr&)> ColorChangedSignalType;
    //@}

    /** \name Structors */ //@{
    EmpireColorSelector(int h);
    //@}

    /** \name Accessors */ //@{
    GG::Clr CurrentColor() const; ///< returns the color that is currently selected, or GG::CLR_ZERO if none is selected
    //@}

    /** \name Mutators */ //@{
    void SelectColor(const GG::Clr& clr);

    ColorChangedSignalType& ColorChangedSignal() const {return color_changed_sig;}
    //@}

private:
    void SelectionChanged(int i);

    mutable ColorChangedSignalType color_changed_sig;
};

/** A GG file dialog in the FreeOrion style. */
class FileDlg : public GG::FileDlg
{
public:
    /** \name Structors */ //@{
    FileDlg(const std::string& directory, const std::string& filename, bool save, bool multi,
            const std::vector<std::pair<std::string, std::string> >& types);
    //@}
};

inline std::pair<std::string, std::string> CUIControlsRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _CUIControls_h_
