#ifndef _CUIControls_h_
#define _CUIControls_h_

#include "ClientUI.h"
#include "CUIDrawUtil.h"
#include "CUIWnd.h"
#include "TextBrowseWnd.h"
#include <GG/Button.h>
#include <GG/DropDownList.h>
#include <GG/Edit.h>
#include <GG/Menu.h>
#include <GG/MultiEdit.h>
#include <GG/Scroll.h>
#include <GG/Slider.h>
#include <GG/StaticGraphic.h>
#include <GG/TabWnd.h>
#include <GG/dialogs/FileDlg.h>
#include <GG/GLClientAndServerBuffer.h>

#include <boost/function.hpp>

#include "LinkText.h"

/** \file
 *
 * All CUI* classes are FreeOrion-style controls incorporating
 * the visual theme the project requires.  Implementation may
 * depend on graphics and design team specifications.  They extend
 * GG controls.
 */

/** a FreeOrion Label control */
class CUILabel : public GG::TextControl {
public:
    /** \name Structors */ //@{
    CUILabel(const std::string& str,
             GG::Flags<GG::TextFormat> format = GG::FORMAT_NONE,
             GG::Flags<GG::WndFlag> flags = GG::NO_WND_FLAGS);
    //@}
};

/** a FreeOrion Button control */
class CUIButton : public GG::Button {
public:
    /** \name Structors */ //@{
    CUIButton(const std::string& str); ///< basic ctor

    CUIButton(const std::string& str, GG::Clr background, GG::Clr border);

    CUIButton(const GG::SubTexture& unpressed, const GG::SubTexture& pressed, const GG::SubTexture& rollover);
    //@}

    /** \name Accessors */ //@{
    virtual GG::Pt MinUsableSize() const;

    virtual bool InWindow(const GG::Pt& pt) const;
    //@}

    /** \name Mutators */ //@{
    virtual void MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    void         SetCheck(bool b = true);
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
    bool    m_checked;     ///< true when this button in a checked, active state
};

class SettableInWindowCUIButton : public CUIButton {
public:
    /** \name Structors */ //@{
    SettableInWindowCUIButton(const GG::SubTexture& unpressed, const GG::SubTexture& pressed, const GG::SubTexture& rollover, boost::function<bool(const SettableInWindowCUIButton*, const GG::Pt&)> in_window_function);
    //@}

    /** \name Accessors */ //@{
    virtual bool    InWindow(const GG::Pt& pt) const;
    //@}

private:
    boost::function<bool(const SettableInWindowCUIButton*, const GG::Pt&)>    m_in_window_func;
};

/** a FreeOrion triangular arrow button */
class CUIArrowButton : public GG::Button {
public:
    /** \name Structors */ //@{
    CUIArrowButton(ShapeOrientation orientation, bool fill_background,
                   GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE); ///< basic ctor
    //@}

    /** \name Accessors */ //@{
    virtual bool    InWindow(const GG::Pt& pt) const;
    //@}

    /** \name Mutators */ //@{
    virtual void    MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
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


/** \brief A FreeOrion styled check box state button. */
class CUICheckBoxRepresenter : public GG::StateButtonRepresenter {
public:
    virtual void Render(const GG::StateButton& button) const;

    virtual void OnChecked(bool checked) const;
};


/** \brief A FreeOrion styled radio state button. */
class CUIRadioRepresenter : public GG::StateButtonRepresenter {
public:
    virtual void Render(const GG::StateButton& button) const;

    virtual void OnChecked(bool checked) const;
};


/** \brief A FreeOrion styled TabBar tab. */
class CUITabRepresenter : public GG::StateButtonRepresenter {
public:
    virtual void Render(const GG::StateButton& button) const;

    virtual void OnChecked(bool checked) const;

    virtual GG::Pt MinUsableSize(const GG::StateButton& button) const;
};


/** a FreeOrion StateButton control */
class CUIStateButton : public GG::StateButton {
public:
    /** \name Structors */ //@{
    CUIStateButton(const std::string& str, GG::Flags<GG::TextFormat> format, boost::shared_ptr<GG::StateButtonRepresenter> representer); ///< ctor
    //@}
};

/** Tab bar with buttons for selecting tabbed windows. */
class CUITabBar : public GG::TabBar {
public:
    /** \name Structors */ ///@{
    /** Basic ctor. */
    CUITabBar(const boost::shared_ptr<GG::Font>& font, GG::Clr color,
              GG::Clr text_color);
    //@}

private:
    virtual void DistinguishCurrentTab(const std::vector<GG::StateButton*>& tab_buttons);
};

/** a FreeOrion Scroll control */
class CUIScroll : public GG::Scroll {
public:
    /** represents the tab button for a CUIScroll */
    class ScrollTab : public GG::Button {
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
    CUIScroll(GG::Orientation orientation); ///< basic ctor
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    //@}

protected:
    GG::Clr m_border_color;
};

/** a FreeOrion ListBox control */
class CUIListBox : public GG::ListBox {
public:
    /** \name Structors */ //@{
    CUIListBox(void); ///< basic ctor
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    //@}
};

/** a FreeOrion DropDownList control */
class CUIDropDownList : public GG::DropDownList {
public:
    /** \name Structors */ //@{
    explicit CUIDropDownList(size_t num_shown_elements); ///< basic ctor
    //@}

    /** \name Mutators */ //@{
    virtual void    Render();
    virtual void    LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    MouseLeave();

    void            DisableDropArrow();  ///< disables rendering of the small downward-facing arrow on the right of the control
    void            EnableDropArrow();   ///< enables rendering of the small downward-facing arrow on the right of the control
    //@}

private:
    virtual void    InitBuffer();

    bool    m_render_drop_arrow;
    bool    m_mouse_here;
};

/** a FreeOrion Edit control */
class CUIEdit : public GG::Edit {
public:
    /** \name Structors */ //@{
    CUIEdit(const std::string& str); ///< basic ctor
    //@}

    /** \name Mutators */ //@{
    virtual void GainingFocus();
    virtual void LosingFocus();
    virtual void Render();
    //@}

    mutable boost::signals2::signal<void ()> GainingFocusSignal;
    mutable boost::signals2::signal<void ()> LosingFocusSignal;
};

/** a FreeOrion MultiEdit control */
class CUIMultiEdit : public GG::MultiEdit {
public:
    /** \name Structors */ //@{
    CUIMultiEdit(const std::string& str, GG::Flags<GG::MultiEditStyle> style = GG::MULTI_LINEWRAP); ///< basic ctor
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    //@}
};

/** a FreeOrion MultiEdit control that parses its text and makes links within clickable */
class CUILinkTextMultiEdit : public CUIMultiEdit, public TextLinker {
public:
    /** \name Structors */ //@{
    CUILinkTextMultiEdit(const std::string& str, GG::Flags<GG::MultiEditStyle> style = GG::MULTI_LINEWRAP); ///< basic ctor
    //@}
    /** \name Accessors */ //@{
    virtual const std::vector<GG::Font::LineData>&  GetLineData() const;
    virtual const boost::shared_ptr<GG::Font>&      GetFont() const;
    virtual GG::Pt                                  TextUpperLeft() const;
    virtual GG::Pt                                  TextLowerRight() const;
    virtual const std::string&                      RawText() const;
    //@}

    /** \name Mutators */ //@{
    virtual void    Render();
    virtual void    LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    MouseLeave();
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);   // needed primarily so the SetText call will take a RawText

    /** sets the text to \a str; may resize the window.  If the window was
        constructed to fit the size of the text (i.e. if the second ctor type
        was used), calls to this function cause the window to be resized to
        whatever space the newly rendered text occupies. */
    virtual void    SetText(const std::string& str);
    //@}

private:
    virtual void    SetLinkedText(const std::string& str);

    bool            m_already_setting_text_so_dont_link;
    std::string     m_raw_text;
};

/** A simple GG::ListBox::Row subclass designed for use in text-only drop-down
  * lists, such as the ones used in the game setup dialogs. */
struct CUISimpleDropDownListRow : public GG::ListBox::Row {
    CUISimpleDropDownListRow(const std::string& row_text, GG::Y row_height = DEFAULT_ROW_HEIGHT);
    static const GG::Y DEFAULT_ROW_HEIGHT;
};

/** Encapsulates an icon and text that goes with it in a single control.  For
  * example, "[trade icon] +1" or "[population icon] 66 (+5)", where [... icon]
  * is an icon image, not text.
  * The icon may have one or two numerical values.  If one, just that number is
  * displayed.  If two, the first number is displayed followed by the second in
  * brackets "()" */
class StatisticIcon : public GG::Control {
public:
    /** \name Structors */ //@{
    StatisticIcon(const boost::shared_ptr<GG::Texture> texture);///< initialized with no value (just an icon)

    StatisticIcon(const boost::shared_ptr<GG::Texture> texture,
                  double value, int digits, bool showsign);     ///< initializes with one value

    StatisticIcon(const boost::shared_ptr<GG::Texture> texture,
                  double value0, double value1, int digits0, int digits1,
                  bool showsign0, bool showsign1);              ///< initializes with two values
    //@}

    /** \name Accessors */ //@{
    double          GetValue(int index = 0) const;
    //@}

    /** \name Mutators */ //@{
    virtual void    Render() {}

    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    virtual void    LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    RButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);

    void            SetValue(double value, int index = 0);  ///< sets displayed \a value with \a index
    //@}

    mutable boost::signals2::signal<void ()>    LeftClickedSignal;
    mutable boost::signals2::signal<void ()>    RightClickedSignal;

private:
    void            DoLayout();
    void            Refresh();
    GG::Clr         ValueColor(int index) const;        ///< returns colour in which to draw value

    int                 m_num_values;

    std::vector<double> m_values;
    std::vector<int>    m_digits;
    std::vector<bool>   m_show_signs;

    GG::StaticGraphic*  m_icon;
    GG::Label*          m_text;
};

class CUIToolBar : public GG::Control {
public:
    /** \name Structors */ //@{
    CUIToolBar();
    //@}

    /** \name Accessors */ //@{
    virtual bool    InWindow(const GG::Pt& pt) const;
    //@}

    /** \name Mutators */ //@{
    void            Render();
    //@}
private:
};

/** A control used to pick from at list of species names. */
class SpeciesSelector : public CUIDropDownList {
public:
    /** \name Structors */ //@{
    SpeciesSelector(GG::X w, GG::Y h);                                                  ///< populates with all species in SpeciesManager
    SpeciesSelector(GG::X w, GG::Y h, const std::vector<std::string>& species_names);   ///< populates with the species in \a species_names
    //@}

    /** \name Accessors */ //@{
    const std::string&          CurrentSpeciesName() const;     ///< returns the name of the species that is currently selected
    std::vector<std::string>    AvailableSpeciesNames() const;  ///< returns the names of species in the selector
    //@}

    /** \name Mutators */ //@{
    void SelectSpecies(const std::string& species_name);
    void SetSpecies(const std::vector<std::string>& species_names);         ///< sets the species that can be selected
    //@}

    mutable boost::signals2::signal<void (const std::string&)> SpeciesChangedSignal;

private:
    void SelectionChanged(GG::DropDownList::iterator it);
};

/** A control used to pick from the empire colors returned by EmpireColors(). */
class EmpireColorSelector : public CUIDropDownList {
public:
    /** \name Structors */ //@{
    explicit EmpireColorSelector(GG::Y h);
    //@}

    /** \name Accessors */ //@{
    GG::Clr CurrentColor() const; ///< returns the color that is currently selected, or GG::CLR_ZERO if none is selected
    //@}

    /** \name Mutators */ //@{
    void SelectColor(const GG::Clr& clr);
    //@}

    mutable boost::signals2::signal<void (const GG::Clr&)> ColorChangedSignal;

private:
    void SelectionChanged(GG::DropDownList::iterator it);
};

/** A control used to pick arbitrary colors using GG::ColorDlg. */
class ColorSelector : public GG::Control {
public:
    /** \name Structors */ //@{
    ColorSelector(GG::Clr color, GG::Clr default_color);
    virtual ~ColorSelector();
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    virtual void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    //@}

    mutable boost::signals2::signal<void (const GG::Clr&)> ColorChangedSignal;

private:
    virtual void InitBuffer();
    GG::GL2DVertexBuffer    m_border_buffer;
    GG::Clr                 m_default_color;
};

/** A GG file dialog in the FreeOrion style. */
class FileDlg : public GG::FileDlg {
public:
    /** \name Structors */ //@{
    FileDlg(const std::string& directory, const std::string& filename, bool save, bool multi,
            const std::vector<std::pair<std::string, std::string> >& types);
    //@}
};

/** Despite the name, this is actually used to display info in both the Research and Production screens. */
class ProductionInfoPanel : public CUIWnd {
public:
    /** \name Structors */ //@{
    ProductionInfoPanel(const std::string& title, const std::string& point_units_str, const GG::X x, const GG::Y y,
                        const GG::X w, const GG::Y h, const std::string& config_name);
    //@}

    /** \name Accessors */ //@{
    virtual GG::Pt  MinUsableSize() const;
    //@}

    /** \name Mutators */ //@{
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);

    void            SetTotalPointsCost(float total_points, float total_cost);
    void            SetLocalPointsCost(float local_points, float local_cost, const std::string& location_name);
    void            SetEmpireID(int empire_id);
    void            ClearLocalInfo();
    void            Clear();
    //@}

private:
    void DoLayout();

    std::string m_units_str;
    std::string m_title_str;
    int         m_empire_id;

    GG::Label*  m_total_points_label;
    GG::Label*  m_total_points;
    GG::Label*  m_total_points_P_label;
    GG::Label*  m_wasted_points_label;
    GG::Label*  m_wasted_points;
    GG::Label*  m_wasted_points_P_label;

    GG::Label*  m_local_points_label;
    GG::Label*  m_local_points;
    GG::Label*  m_local_points_P_label;
    GG::Label*  m_local_wasted_points_label;
    GG::Label*  m_local_wasted_points;
    GG::Label*  m_local_wasted_points_P_label;
};

/** Displays progress that is divided over mulitple turns, as in the Research and Production screens. */
class MultiTurnProgressBar : public GG::Control {
public:
    /** \name Structors */ //@{
    /** ctor */
    MultiTurnProgressBar(int total_turns, double turns_completed, double total_cost, double turn_spending,
                         const GG::Clr& bar_color, const GG::Clr& background, const GG::Clr& outline_color);
    //@}

    /** \name Mutators */ //@{
    virtual void Render();
    //@}

private:
    int     m_total_turns;
    double  m_turns_completed;
    double  m_total_cost;
    double  m_turn_spending;
    GG::Clr m_bar_color;
    GG::Clr m_background;
    GG::Clr m_outline_color;
};

/** Displays current rendering frames per second. */
class FPSIndicator : public GG::Label {
public:
    FPSIndicator();
    virtual void Render();
private:
    void UpdateEnabled();
    bool m_enabled;
};

/** Functions like a StaticGraphic, except can have multiple textures rendered
  * on top of eachother, rather than just a single texture. */
class MultiTextureStaticGraphic : public GG::Control {
public:
    /** \name Structors */ ///@{

    /** creates a MultiTextureStaticGraphic from multiple pre-existing Textures which are rendered back-to-front in the
      * order they are specified in \a textures with GraphicStyles specified in the same-indexed value of \a styles.
      * if \a styles is not specified or contains fewer entres than \a textures, entries in \a textures without 
      * associated styles use the style GRAPHIC_CENTER. */
    MultiTextureStaticGraphic(const std::vector<boost::shared_ptr<GG::Texture> >& textures,
                              const std::vector<GG::Flags<GG::GraphicStyle> >& styles = std::vector<GG::Flags<GG::GraphicStyle> >());

    /** creates a MultiTextureStaticGraphic from multiple pre-existing SubTextures which are rendered back-to-front in the
      * order they are specified in \a subtextures with GraphicStyles specified in the same-indexed value of \a styles.
      * if \a styles is not specified or contains fewer entres than \a subtextures, entries in \a subtextures without 
      * associated styles use the style GRAPHIC_CENTER. */
    MultiTextureStaticGraphic(const std::vector<GG::SubTexture>& subtextures,
                              const std::vector<GG::Flags<GG::GraphicStyle> >& styles = std::vector<GG::Flags<GG::GraphicStyle> >());
    //@}

    /** \name Mutators */ ///@{
    virtual void    Render();       ///< renders textures in order specified in constructor, back-to-front
    //@}

protected:
    MultiTextureStaticGraphic();    ///< default ctor

    /** Returns the area in which the graphic is actually rendered, in
        UpperLeft()-relative coordinates.  This may not be the entire area of
        the StaticGraphic, based on the style being used. */
    GG::Rect        RenderedArea(const GG::SubTexture& subtexture, GG::Flags<GG::GraphicStyle> style) const;

private:
    void            Init();
    void            ValidateStyles();      ///< ensures that the style flags are consistent

    std::vector<GG::SubTexture>                 m_graphics;
    std::vector<GG::Flags<GG::GraphicStyle> >   m_styles;   ///< position of texture wrt the window area
};

/** Functions like a StaticGraphic, except can be rotated with a fixed phase
  * and/or at a continuous angular rate. */
class RotatingGraphic : public GG::StaticGraphic {
public:
    /** \name Structors */ ///@{
    RotatingGraphic(const boost::shared_ptr<GG::Texture>& texture, GG::Flags<GG::GraphicStyle> style = GG::GRAPHIC_NONE,
                    GG::Flags<GG::WndFlag> flags = GG::NO_WND_FLAGS);
    //@}

    /** \name Mutators */ ///@{
    void            SetRPM(float rpm)               { m_rpm = std::max(-3600.0f, std::min(3600.0f, rpm)); }
    void            SetPhaseOffset(float degrees)   { m_phase_offset = degrees; }
    virtual void    Render();
    //@}

private:
    float   m_rpm;
    float   m_phase_offset;
};


/** \brief Represents a CUIToggleButton
 * 
 * Renders a SubTexture depending on the checked state of the CUIToggleButton.
 * When the \a button has single texture and is unchecked, it renders the same icon recolored towards grey.
 * If a second SubTexture is assigned to the \a button, it will render the appropriate SubTexture.
 * When a mouse pointer hovers over this button, the opposing SubTexture and color are used and highlighted
 */
class CUIToggleRepresenter : public GG::StateButtonRepresenter {
public:
    /** \name Structors */ //@{
    CUIToggleRepresenter(boost::shared_ptr<GG::SubTexture> icon, const GG::Clr& highlight_clr); ///< ctor
    CUIToggleRepresenter(boost::shared_ptr<GG::SubTexture> unchecked_icon, const GG::Clr& unchecked_clr,
                         boost::shared_ptr<GG::SubTexture> checked_icon, const GG::Clr& checked_clr); ///< ctor
    //@}

    /** \name Mutators */ //@{
    virtual void                        OnChecked(bool checked) const;
    virtual void                        Render(const GG::StateButton& button) const;
    //@}

private:
    boost::shared_ptr<GG::SubTexture>   m_unchecked_icon;
    boost::shared_ptr<GG::SubTexture>   m_checked_icon;
    GG::Clr                             m_unchecked_color;
    GG::Clr                             m_checked_color;
};

#endif // _CUIControls_h_
