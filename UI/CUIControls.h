// -*- C++ -*-
//CUIControls.h
#ifndef _CUIControls_h_
#define _CUIControls_h_

#ifndef _ClientUI_h_
#include "ClientUI.h"
#endif

#ifndef _GG_Button_h_
#include <GG/Button.h>
#endif

#ifndef _CUIDrawUtil_h_
#include "CUIDrawUtil.h"
#endif

#ifndef _GG_Listbox_h_
#include <GG/DropDownList.h>
#endif

#ifndef _GG_Edit_h_
#include <GG/Edit.h>
#endif

#ifndef _GG_FileDlg_h_
#include <GG/dialogs/FileDlg.h>
#endif

#ifndef _GG_Menu_h_
#include <GG/Menu.h>
#endif

#ifndef _GG_MultiEdit_h_
#include <GG/MultiEdit.h>
#endif

#ifndef _GG_Scroll_h_
#include <GG/Scroll.h>
#endif

#ifndef _GG_Slider_h_
#include <GG/Slider.h>
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
    CUIButton(int x, int y, int w, const std::string& str, const boost::shared_ptr<GG::Font>& font = boost::shared_ptr<GG::Font>(), 
              GG::Clr color = ClientUI::BUTTON_COLOR, GG::Clr border = ClientUI::CTRL_BORDER_COLOR, int thick = 1, 
              GG::Clr text_color = ClientUI::TEXT_COLOR, Uint32 flags = GG::CLICKABLE); ///< basic ctor
    //@}

    /** \name Accessors */ //@{
    GG::Clr      BorderColor() const {return m_border_color;} ///< returns the color used to render the border of the button
    int          BorderThickness() const {return m_border_thick;} ///< returns the width used to render the border of the button

    virtual bool InWindow(const GG::Pt& pt) const;
    //@}

    /** \name Mutators */ //@{
    virtual void MouseHere(const GG::Pt& pt, Uint32 keys);
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
                  GG::Clr color = ClientUI::BUTTON_COLOR, GG::Clr border = ClientUI::CTRL_BORDER_COLOR, int thick = 1, 
                  GG::Clr text_color = ClientUI::TEXT_COLOR, Uint32 flags = GG::CLICKABLE); ///< basic ctor
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
    //@}

    /** \name Mutators */ //@{
    virtual void MouseHere(const GG::Pt& pt, Uint32 keys);
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
                   GG::Clr color = ClientUI::STATE_BUTTON_COLOR, const boost::shared_ptr<GG::Font>& font = boost::shared_ptr<GG::Font>(),
                   GG::Clr text_color = ClientUI::TEXT_COLOR, GG::Clr interior = GG::CLR_ZERO,
                   GG::Clr border = ClientUI::CTRL_BORDER_COLOR, Uint32 flags = GG::CLICKABLE); ///< ctor
    //@}

    /** \name Accessors */ //@{
    GG::Clr      BorderColor() const {return m_border_color;} ///< returns the color used to render the border of the button
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    virtual void MouseEnter(const GG::Pt& pt, Uint32 keys);
    virtual void MouseLeave();
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
              GG::Clr border = ClientUI::CTRL_BORDER_COLOR, GG::Clr interior = GG::CLR_ZERO,
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
    CUIListBox(int x, int y, int w, int h, GG::Clr color = ClientUI::CTRL_BORDER_COLOR, GG::Clr interior = GG::CLR_ZERO, 
               Uint32 flags = GG::CLICKABLE); ///< basic ctor
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    //@}
};


/** a FreeOrion DropDownList control */
class CUIDropDownList : public GG::DropDownList
{
public:
    /** \name Structors */ //@{
    CUIDropDownList(int x, int y, int w, int h, int drop_ht, GG::Clr color = ClientUI::CTRL_BORDER_COLOR, 
                    GG::Clr interior = ClientUI::DROP_DOWN_LIST_INT_COLOR, Uint32 flags = GG::CLICKABLE); ///< basic ctor
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
            GG::Clr color = ClientUI::CTRL_BORDER_COLOR, GG::Clr text_color = ClientUI::TEXT_COLOR, 
            GG::Clr interior = ClientUI::EDIT_INT_COLOR, Uint32 flags = GG::CLICKABLE); ///< basic ctor
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
                 GG::Clr color = ClientUI::CTRL_BORDER_COLOR, GG::Clr text_color = ClientUI::TEXT_COLOR, 
                 GG::Clr interior = ClientUI::MULTIEDIT_INT_COLOR, Uint32 flags = GG::CLICKABLE); ///< basic ctor
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
    //@}
};

/** A simple GG::ListBox::Row subclass designed for use in text-only drop-down lists, such as the ones used in the game setup dialogs. */
struct CUISimpleDropDownListRow : public GG::ListBox::Row
{
    enum {DEFAULT_ROW_HEIGHT = 22};
    CUISimpleDropDownListRow(const std::string& row_text, int row_height = DEFAULT_ROW_HEIGHT) :
        GG::ListBox::Row(1, row_height, "")
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
    void Render() {}

    void SetValue(double value); ///< sets the value to be displayed

    void SetDecimalsShown(int d)     {m_decimals_to_show = d; Refresh();} ///< sets the number of places after the decimal point to be shown
    void ShowSign(bool b)            {m_show_sign = b; Refresh();}        ///< sets whether a sign should always be shown, even for positive values
    void SetPositiveColor(GG::Clr c) {m_positive_color = c; Refresh();}   ///< sets the color that will be used to display positive values
    void SetNegativeColor(GG::Clr c) {m_negative_color = c; Refresh();}   ///< sets the color that will be used to display negative values
    //@}

    static const double UNKNOWN_VALUE;

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
    void Render() {}

    void SetValue         (double value); ///< sets the value to be displayed
    void SetValueSecond   (double value); ///< sets the value to be displayed
    void SetDecimalsShown (int        d) {m_decimals_to_show  = d; UpdateTextControl();} ///< sets the number of places after the decimal point to be shown
    void SetDecimalsShownSecond (int        d) {m_decimals_to_show  = d; UpdateTextControl();} ///< sets the number of places after the decimal point to be shown
    void ShowSign         (bool       b) {m_show_sign         = b; UpdateTextControl();}        ///< sets whether a sign should always be shown, even for positive values
    void ShowSignSecond   (bool       b) {m_show_sign_second  = b; UpdateTextControl();}        ///< sets whether a sign should always be shown, even for positive values
    void SetPositiveColor (GG::Clr    c) {m_positive_color    = c; UpdateTextControl();}   ///< sets the color that will be used to display positive values
    void SetNegativeColor (GG::Clr    c) {m_negative_color    = c; UpdateTextControl();}   ///< sets the color that will be used to display negative values
   //@}

    static const double UNKNOWN_VALUE;

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

inline std::string CUIControlsRevision()
{return "$Id$";}

#endif // _CUIControls_h_
