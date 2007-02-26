// -*- C++ -*-
//CUIControls.h
#ifndef _CUIControls_h_
#define _CUIControls_h_

#include "ClientUI.h"
#include <GG/Button.h>
#include "CUIDrawUtil.h"
#include <GG/DropDownList.h>
#include <GG/Edit.h>
#include <GG/dialogs/FileDlg.h>
#include <GG/Menu.h>
#include <GG/MultiEdit.h>
#include <GG/Scroll.h>
#include <GG/Slider.h>

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
    CUIButton(int x, int y, int w, const std::string& str, const boost::shared_ptr<GG::Font>& font = boost::shared_ptr<GG::Font>(), 
              GG::Clr color = ClientUI::ButtonColor(), GG::Clr border = ClientUI::CtrlBorderColor(), int thick = 1, 
              GG::Clr text_color = ClientUI::TextColor(), Uint32 flags = GG::CLICKABLE); ///< basic ctor
    //@}

    /** \name Accessors */ //@{
    GG::Clr      BorderColor() const {return m_border_color;} ///< returns the color used to render the border of the button
    int          BorderThickness() const {return m_border_thick;} ///< returns the width used to render the border of the button

    virtual bool InWindow(const GG::Pt& pt) const;
    //@}

    /** \name Mutators */ //@{
    virtual void MouseHere(const GG::Pt& pt, Uint32 keys);

    void SetBorderColor(GG::Clr clr);   ///< sets the color used to render the border of the button
    void SetBorderThick(int thick);     ///< sets the thickness of the rendered the border of the button

    void MarkNotSelected();             ///< sets button colours to standard UI colours
    void MarkSelectedGray();            ///< sets button colours to lighter grey background and border to indicate selection or activation
    void MarkSelectedTechCategoryColor(std::string category);   ///< sets button background and border colours to variants of the colour of the tech category specified
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
    CUITurnButton(int x, int y, int w, const std::string& str, const boost::shared_ptr<GG::Font>& font = boost::shared_ptr<GG::Font>(), 
                  GG::Clr color = ClientUI::ButtonColor(), GG::Clr border = ClientUI::CtrlBorderColor(), int thick = 1, 
                  GG::Clr text_color = ClientUI::TextColor(), Uint32 flags = GG::CLICKABLE); ///< basic ctor
    //@}
};


/** a FreeOrion triangular arrow button */
class CUIArrowButton : public GG::Button
{
public:
    /** \name Structors */ //@{
    CUIArrowButton(int x, int y, int w, int h, ShapeOrientation orientation, GG::Clr color, Uint32 flags = GG::CLICKABLE); ///< basic ctor
    //@}

    /** \name Accessors */ //@{
    virtual bool InWindow(const GG::Pt& pt) const;

    bool FillBackgroundWithWndColor() const;
    //@}

    /** \name Mutators */ //@{
    virtual void MouseHere(const GG::Pt& pt, Uint32 keys);

    void FillBackgroundWithWndColor(bool fill);
    //@}

protected:
    /** \name Mutators control */ //@{
    virtual void RenderPressed();
    virtual void RenderRollover();
    virtual void RenderUnpressed();
    //@}

private:
    ShapeOrientation m_orientation;
    bool             m_fill_background_with_wnd_color;
};


/** a FreeOrion StateButton control */
class CUIStateButton : public GG::StateButton
{
public:
    /** \name Structors */ //@{
    CUIStateButton(int x, int y, int w, int h, const std::string& str, Uint32 text_fmt, GG::StateButtonStyle style = GG::SBSTYLE_3D_CHECKBOX,
                   GG::Clr color = ClientUI::StateButtonColor(), const boost::shared_ptr<GG::Font>& font = boost::shared_ptr<GG::Font>(),
                   GG::Clr text_color = ClientUI::TextColor(), GG::Clr interior = GG::CLR_ZERO,
                   GG::Clr border = ClientUI::CtrlBorderColor(), Uint32 flags = GG::CLICKABLE); ///< ctor
    //@}

    /** \name Accessors */ //@{
    virtual GG::Pt MinUsableSize() const;

    GG::Clr        BorderColor() const {return m_border_color;} ///< returns the color used to render the border of the button
    //@}

    /** \name Mutators */ //@{
    virtual void   Render();
    virtual void   MouseEnter(const GG::Pt& pt, Uint32 keys);
    virtual void   MouseLeave();
    //@}

private:
    GG::Clr m_border_color;
    bool    m_mouse_here;
};


/** a FreeOrion Scroll control */
class CUIScroll : public GG::Scroll
{
public:
    /** represents the tab button for a CUIScroll */
    class ScrollTab : public GG::Button
    {
    public:
        ScrollTab(GG::Orientation orientation, int scroll_width, GG::Clr color, GG::Clr border_color); ///< basic ctor
        virtual void SetColor(GG::Clr c);
        virtual void Render();
        virtual void LButtonDown(const GG::Pt& pt, Uint32 keys);
        virtual void LButtonUp(const GG::Pt& pt, Uint32 keys);
        virtual void LClick(const GG::Pt& pt, Uint32 keys);
        virtual void MouseEnter(const GG::Pt& pt, Uint32 keys);
        virtual void MouseLeave();
    private:
        GG::Clr m_border_color;
        GG::Orientation m_orientation;
        bool m_mouse_here;
        bool m_being_dragged;
    };

    /** \name Structors */ //@{
    CUIScroll(int x, int y, int w, int h, GG::Orientation orientation, GG::Clr color = GG::CLR_ZERO, 
              GG::Clr border = ClientUI::CtrlBorderColor(), GG::Clr interior = GG::CLR_ZERO,
              Uint32 flags = GG::CLICKABLE | GG::REPEAT_BUTTON_DOWN); ///< basic ctor
    //@}

    /** \name Accessors */ //@{
    GG::Clr      BorderColor() const {return m_border_color;} ///< returns the color used to render the border of the control
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    //@}

private:
    GG::Clr m_border_color;
};


/** a FreeOrion ListBox control */
class CUIListBox : public GG::ListBox
{
public:
    /** \name Structors */ //@{
    CUIListBox(int x, int y, int w, int h, GG::Clr color = ClientUI::CtrlBorderColor(), GG::Clr interior = GG::CLR_ZERO, 
               Uint32 flags = GG::CLICKABLE); ///< basic ctor
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    //@}
};

/** a ListBox with user-sortable columns */
class CUISortListBox : public GG::ListBox
{
public:
    /** \name Structors */ //@{
    CUISortListBox(int x, int y, int w, int h, GG::Clr color = ClientUI::CtrlBorderColor(), GG::Clr interior = GG::CLR_ZERO,
                   Uint32 flags = GG::CLICKABLE);
    //@}

    /** \name Mutators */ //@{
    virtual void Render();

    void SetSortCol(int n);                 ///< sets column used to sort rows, updates sort-button appearance
    void SetSortDirecion(bool ascending);   ///< sets whether to sort ascending (true) or descending (false), updates sort-button appearance
    void SetColHeaders(std::map<int, boost::shared_ptr<GG::Texture> > textures_map);    ///< indexed by column number, provides texture with which to label column
    void SetColHeaders(std::map<int, std::string> label_text_map);    ///< indexed by column number, provides text with which to label column
    //@}

private:
    Row* m_header_row;  ///< need to keep own pointer to header row, since ListBox doesn't provide non-const access to its header row pointer
};

/** a FreeOrion DropDownList control */
class CUIDropDownList : public GG::DropDownList
{
public:
    /** \name Structors */ //@{
    CUIDropDownList(int x, int y, int w, int h, int drop_ht, GG::Clr color = ClientUI::CtrlBorderColor(), 
                    GG::Clr interior = ClientUI::DropDownListIntColor(), Uint32 flags = GG::CLICKABLE); ///< basic ctor
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    virtual void LClick(const GG::Pt& pt, Uint32 keys);
    virtual void MouseEnter(const GG::Pt& pt, Uint32 keys);
    virtual void MouseLeave();

    void         DisableDropArrow();  ///< disables rendering of the small downward-facing arrow on the right of the control
    void         EnableDropArrow();   ///< enables rendering of the small downward-facing arrow on the right of the control
    //@}

private:
    bool m_render_drop_arrow;
    bool m_mouse_here;
};


/** a FreeOrion Edit control */
class CUIEdit : public GG::Edit
{
public:
    /** \name Structors */ //@{
    CUIEdit(int x, int y, int w, const std::string& str, const boost::shared_ptr<GG::Font>& font = boost::shared_ptr<GG::Font>(),
            GG::Clr color = ClientUI::CtrlBorderColor(), GG::Clr text_color = ClientUI::TextColor(), 
            GG::Clr interior = ClientUI::EditIntColor(), Uint32 flags = GG::CLICKABLE); ///< basic ctor
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    //@}
};


/** a FreeOrion MultiEdit control */
class CUIMultiEdit : public GG::MultiEdit
{
public:
    /** \name Structors */ //@{
    CUIMultiEdit(int x, int y, int w, int h, const std::string& str, Uint32 style = GG::TF_LINEWRAP,
                 const boost::shared_ptr<GG::Font>& font = boost::shared_ptr<GG::Font>(),
                 GG::Clr color = ClientUI::CtrlBorderColor(), GG::Clr text_color = ClientUI::TextColor(), 
                 GG::Clr interior = ClientUI::MultieditIntColor(), Uint32 flags = GG::CLICKABLE); ///< basic ctor
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    //@}
};


/** a FreeOrion slider, much feared in the forums */
class CUISlider : public GG::Slider
{
public:
    /** \name Structors */ //@{
    CUISlider(int x, int y, int w, int h, int min, int max, GG::Orientation orientation, Uint32 flags = GG::CLICKABLE);
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    //@}
};

/** A simple GG::ListBox::Row subclass designed for use in text-only drop-down lists, such as the ones used in the game setup dialogs. */
struct CUISimpleDropDownListRow : public GG::ListBox::Row
{
    CUISimpleDropDownListRow(const std::string& row_text, int row_height = DEFAULT_ROW_HEIGHT);
    static const int DEFAULT_ROW_HEIGHT;
};

/** Encapsulates an icon and text that goes with it in a single control.  For example, "[food icon] +1" or
    "[population icon] 66 (+5)", where [... icon] is an icon image, not text.
    The icon may have one or two numerical values.  If one, just that number is displayed.  If two, the first
    number is displayed followed by the second in brackets "()"
    
    The number of significant figures to display may be specified for each value.  In order to represent
    numbers which would normally require more digits than the allowed sig figs, SI unit prefixes indicate
    powers of 1000.  Possible prefixes include k = 10^3, M = 10^6, G = 10^9, T = 10^12, m = 10^-3,
    µ = 10^-6, n = 10^-9, p = 10^-12.  The smallest prefix possible for the number and sig figs limit is used.

    Each value may optionally be displayed rounded to the nearest integer, and/or with signs always indicated
    (including + for positive numbers or zero) or only for negative numbers (-)
    
    Note: PositiveColor(), NegativeColor() and ZeroColor() are all the same color by default, specifically the one
    \a text_color parameter specified in the ctor. 
  */
class StatisticIcon : public GG::Control
{
public:
    /** \name Structors */ //@{
    StatisticIcon(int x, int y, int w, int h, const std::string& icon_filename, GG::Clr text_color,
                  double value, int digits, bool integerize, bool showsign); ///< initializes with one value
    
    StatisticIcon(int x, int y, int w, int h, const std::string& icon_filename, GG::Clr text_color,
                  double value0, double value1, int digits0, int digits1,
                  bool integerize0, bool integerize1, bool showsign0, bool showsign1);  ///< initializes with two values
    //@}

    /** \name Accessors */ //@{
    int NumValues() const {return m_num_values;}                ///< returns the number of values displayed
    std::vector<double> Values() const {return m_values;}       ///< returns a vector containing the values displayed
    double Value(int index) const;                              ///< returns the value with \a index
    std::vector<int> SigFigs() const {return m_digits;}         ///< returns a vector of int containing the number of digits to display for each value
    int SigFigs(int index) const;                               ///< returns the number of digits for value with \a index
    std::vector<bool> Integerize() const {return m_integerize;} ///< returns a vector of bool containing whether each value should be rounded to an integer
    bool Integerize(int index) const;                           ///< returns whether the the value with \a index should be rounded to an integer
    std::vector<bool> ShowsSigns() const {return m_show_signs;} ///< returns a vector of bool containing whether the sign of each value should be shown
    bool ShowSigns(int index) const;                            ///< returns whether the sign of the value with \a index should always be shown
    GG::Clr PositiveColor() const {return m_positive_color;}    ///< returns the color that will be used to display positive values
    GG::Clr ZeroColor() const {return m_zero_color;}            ///< returns the color that will be used to display values of 0
    GG::Clr NegativeColor() const {return m_negative_color;}    ///< returns the color that will be used to display negative values
    //@}
    
    GG::Clr ValueColor(int index) const;                        ///< returns colour in which to draw value

    /** \name Mutators */ //@{
    void Render() {}

    void SetValue(double value, int index = 0);             ///< sets displayed \a value with \a index

    /** sets the number of values displayed to \a num.  If \a num is greater than the current number of values, extra values are
      * initilaized to 0.  If \a num is less than the current number of values, the extra values are discarded, and the first
      * \a num values are kept unchanged
      */
    void SetNumValues(int num); 

    void SetSigFigs(int digits, int index = 0);       ///< sets the significant figures of value with \a index.  Negative or zero digits are displayed to full precision.
    void SetIntegerize(bool integerize, int index = 0); ///< sets whether value with \a index should be rounded to the nearest integer when displayed
    void SetShowSigns(bool sign, int index = 0);        ///< sets whether the sign of value with \a index should always be shown, even for positive or zero values

    void SetPositiveColor(GG::Clr c) {m_positive_color = c; Refresh();} ///< sets the color that will be used to display positive values
    void SetZeroColor(GG::Clr c) {m_zero_color = c; Refresh();} ///< sets the color that will be used to display values of zero
    void SetNegativeColor(GG::Clr c) {m_negative_color = c; Refresh();} ///< sets the color that will be used to display negative values
    //@}

    static const double UNKNOWN_VALUE;
    static const double SMALL_VALUE;    ///< smallest (absolute value) number displayed as nonzero, with nonzero colour
    static const double LARGE_VALUE;    ///< largest (absolute value) number displayed.  larger values rounded down to this

    static std::string DoubleToString(double val, int digits, bool integerize, bool showsign); ///< converts double to string with \a digits significant figures.  Represents large numbers with SI prefixes.

private:
    void Refresh();    
    static int EffectiveSign(double val, bool integerize);  // returns sign of value, accounting for SMALL_VALUE: +1 for positive values and -1 for negative values if their absolute value is larger than SMALL VALUE, and returns 0 for zero values or values with absolute value less than SMALL_VALUE

    int                 m_num_values;

    std::vector<double> m_values;
    std::vector<int>    m_digits;
    std::vector<bool>   m_integerize;
    std::vector<bool>   m_show_signs;
    
    GG::Clr             m_positive_color;
    GG::Clr             m_zero_color;
    GG::Clr             m_negative_color;
    GG::StaticGraphic*  m_icon;
    GG::TextControl*    m_text;
};

class CUIToolBar : public GG::Control
{
public:
    /** \name Structors */ //@{
    CUIToolBar(int x, int y, int w, int h);
    //@}

    /** \name Mutators */ //@{
    void Render();
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

    mutable ColorChangedSignalType ColorChangedSignal;
    //@}

private:
    void SelectionChanged(int i);
};

/** A control used to pick arbitrary colors using GG::ColorDlg. */
class ColorSelector : public GG::Control
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void (const GG::Clr&)> ColorChangedSignalType;
    //@}

    /** \name Structors */ //@{
    ColorSelector(int x, int y, int w, int h, GG::Clr color);
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    virtual void LClick(const GG::Pt& pt, Uint32 keys);
    //@}

    mutable ColorChangedSignalType ColorChangedSignal;
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

/** Despite the name, this is actually used to display info in both the Research and Production screens. */
class ProductionInfoPanel : public GG::Wnd
{
public:
    /** \name Structors */ //@{
    ProductionInfoPanel(int w, int h, const std::string& title, const std::string& points_str,
                        double border_thickness, const GG::Clr& color, const GG::Clr& text_and_border_color);
    //@}

    /** \name Mutators */ //@{
    virtual void Render();

    void Reset(double total_points, double total_queue_cost, int projects_in_progress, double points_to_underfunded_projects, int queue_size);
    //@}

private:
    void Draw(GG::Clr clr, bool fill);

    GG::TextControl* m_title;
    GG::TextControl* m_total_points_label;
    GG::TextControl* m_total_points;
    GG::TextControl* m_total_points_P_label;
    GG::TextControl* m_wasted_points_label;
    GG::TextControl* m_wasted_points;
    GG::TextControl* m_wasted_points_P_label;
    GG::TextControl* m_projects_in_progress_label;
    GG::TextControl* m_projects_in_progress;
    GG::TextControl* m_points_to_underfunded_projects_label;
    GG::TextControl* m_points_to_underfunded_projects;
    GG::TextControl* m_points_to_underfunded_projects_P_label;
    GG::TextControl* m_projects_in_queue_label;
    GG::TextControl* m_projects_in_queue;

    std::pair<int, int> m_center_gap;
    double m_border_thickness;
    GG::Clr m_color;
    GG::Clr m_text_and_border_color;

    static const int CORNER_RADIUS;
    static const int VERTICAL_SECTION_GAP;
};

/** Displays progress that is divided over mulitple turns, as in the Research and Production screens. */
class MultiTurnProgressBar : public GG::Control
{
public:
    /** \name Structors */ //@{
    /** ctor */
    MultiTurnProgressBar(int w, int h, int total_turns, int turns_completed, double partially_complete_turn,
                         const GG::Clr& bar_color, const GG::Clr& background, const GG::Clr& outline_color);
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    //@}

private:
    void LeftEndVertices(double x1, double y1, double x2, double y2);
    void RightEndVertices(double x1, double y1, double x2, double y2);

    int     m_total_turns;
    int     m_turns_completed;
    double  m_partially_complete_turn;
    GG::Clr m_bar_color;
    GG::Clr m_background;
    GG::Clr m_outline_color;
};

#endif // _CUIControls_h_
