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

#include "LinkText.h"

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
              GG::Clr text_color = ClientUI::TextColor(), GG::Flags<GG::WndFlag> flags = GG::CLICKABLE); ///< basic ctor
    //@}

    /** \name Accessors */ //@{
    GG::Clr      BorderColor() const {return m_border_color;} ///< returns the color used to render the border of the button
    int          BorderThickness() const {return m_border_thick;} ///< returns the width used to render the border of the button

    virtual bool InWindow(const GG::Pt& pt) const;
    //@}

    /** \name Mutators */ //@{
    virtual void MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);

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
                  GG::Clr text_color = ClientUI::TextColor(), GG::Flags<GG::WndFlag> flags = GG::CLICKABLE); ///< basic ctor
    //@}
};


/** a FreeOrion triangular arrow button */
class CUIArrowButton : public GG::Button
{
public:
    /** \name Structors */ //@{
    CUIArrowButton(int x, int y, int w, int h, ShapeOrientation orientation, GG::Clr color, GG::Flags<GG::WndFlag> flags = GG::CLICKABLE); ///< basic ctor
    //@}

    /** \name Accessors */ //@{
    virtual bool InWindow(const GG::Pt& pt) const;

    bool FillBackgroundWithWndColor() const;
    //@}

    /** \name Mutators */ //@{
    virtual void MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);

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
    CUIStateButton(int x, int y, int w, int h, const std::string& str, GG::Flags<GG::TextFormat> format, GG::StateButtonStyle style = GG::SBSTYLE_3D_CHECKBOX,
                   GG::Clr color = ClientUI::StateButtonColor(), const boost::shared_ptr<GG::Font>& font = boost::shared_ptr<GG::Font>(),
                   GG::Clr text_color = ClientUI::TextColor(), GG::Clr interior = GG::CLR_ZERO,
                   GG::Clr border = ClientUI::CtrlBorderColor(), GG::Flags<GG::WndFlag> flags = GG::CLICKABLE); ///< ctor
    //@}

    /** \name Accessors */ //@{
    virtual GG::Pt MinUsableSize() const;

    GG::Clr        BorderColor() const {return m_border_color;} ///< returns the color used to render the border of the button
    //@}

    /** \name Mutators */ //@{
    virtual void   Render();
    virtual void   MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
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
        virtual void LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
        virtual void LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
        virtual void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
        virtual void MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
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
              GG::Flags<GG::WndFlag> flags = GG::CLICKABLE | GG::REPEAT_BUTTON_DOWN); ///< basic ctor
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
               GG::Flags<GG::WndFlag> flags = GG::CLICKABLE); ///< basic ctor
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
                   GG::Flags<GG::WndFlag> flags = GG::CLICKABLE);
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
                    GG::Clr interior = ClientUI::DropDownListIntColor(), GG::Flags<GG::WndFlag> flags = GG::CLICKABLE); ///< basic ctor
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    virtual void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
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
            GG::Clr interior = ClientUI::EditIntColor(), GG::Flags<GG::WndFlag> flags = GG::CLICKABLE); ///< basic ctor
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
    CUIMultiEdit(int x, int y, int w, int h, const std::string& str, GG::Flags<GG::MultiEditStyle> style = GG::MULTI_LINEWRAP,
                 const boost::shared_ptr<GG::Font>& font = boost::shared_ptr<GG::Font>(),
                 GG::Clr color = ClientUI::CtrlBorderColor(), GG::Clr text_color = ClientUI::TextColor(), 
                 GG::Clr interior = ClientUI::MultieditIntColor(), GG::Flags<GG::WndFlag> flags = GG::CLICKABLE); ///< basic ctor
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    //@}
};

/** a FreeOrion MultiEdit control that parses its text and makes links within clickable */
class CUILinkTextMultiEdit : public CUIMultiEdit, public TextLinker
{
public:
    /** \name Structors */ //@{
    CUILinkTextMultiEdit(int x, int y, int w, int h, const std::string& str, GG::Flags<GG::MultiEditStyle> style = GG::MULTI_LINEWRAP,
                         const boost::shared_ptr<GG::Font>& font = boost::shared_ptr<GG::Font>(),
                         GG::Clr color = ClientUI::CtrlBorderColor(), GG::Clr text_color = ClientUI::TextColor(), 
                         GG::Clr interior = ClientUI::MultieditIntColor(), GG::Flags<GG::WndFlag> flags = GG::CLICKABLE); ///< basic ctor
    //@}
    /** \name Accessors */ //@{
    virtual const std::vector<GG::Font::LineData>&  GetLineData() const;
    virtual const boost::shared_ptr<GG::Font>&      GetFont() const;
    virtual GG::Pt                                  TextUpperLeft() const;
    virtual GG::Pt                                  TextLowerRight() const;
    virtual const std::string&                      WindowText() const;
    //@}

    /** \name Mutators */ //@{
    virtual void    Render();
    virtual void    LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    MouseLeave();

    /** sets the text to \a str; may resize the window.  If the window was constructed to fit the size of the text 
        (i.e. if the second ctor type was used), calls to this function cause the window to be resized to whatever 
        space the newly rendered text occupies. */
    virtual void    SetText(const std::string& str);
    //@}

private:
    virtual void    SetLinkedText(const std::string& str);
    bool            m_already_setting_text_so_dont_link;
};

/** a FreeOrion slider, much feared in the forums */
class CUISlider : public GG::Slider
{
public:
    /** \name Structors */ //@{
    CUISlider(int x, int y, int w, int h, int min, int max, GG::Orientation orientation, GG::Flags<GG::WndFlag> flags = GG::CLICKABLE);
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
  */
class StatisticIcon : public GG::Control
{
public:
    /** \name Structors */ //@{
    StatisticIcon(int x, int y, int w, int h, const boost::shared_ptr<GG::Texture> texture, 
                  double value, int digits, bool integerize, bool showsign,
                  GG::Flags<GG::WndFlag> flags = GG::CLICKABLE); ///< initializes with one value
    
    StatisticIcon(int x, int y, int w, int h, const boost::shared_ptr<GG::Texture> texture,
                  double value0, double value1, int digits0, int digits1,
                  bool integerize0, bool integerize1, bool showsign0, bool showsign1,
                  GG::Flags<GG::WndFlag> flags = GG::CLICKABLE);  ///< initializes with two values
    //@}

    /** \name Mutators */ //@{
    void Render() {}
    void SetValue(double value, int index = 0); ///< sets displayed \a value with \a index
    //@}

private:
    void Refresh();
    GG::Clr ValueColor(int index) const;        ///< returns colour in which to draw value

    int                 m_num_values;

    std::vector<double> m_values;
    std::vector<int>    m_digits;
    std::vector<bool>   m_integerize;
    std::vector<bool>   m_show_signs;
    
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
    virtual void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
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

/** Displays current rendering frames per second. */
class FPSIndicator : public GG::TextControl
{
public:
    FPSIndicator(int x, int y);
    virtual void Render();
private:
    void UpdateEnabled();
    bool m_enabled;
};
#endif // _CUIControls_h_
