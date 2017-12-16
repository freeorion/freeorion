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
             GG::Flags<GG::WndFlag> flags = GG::NO_WND_FLAGS,
             GG::X x = GG::X0, GG::Y y = GG::Y0, GG::X w = GG::X1, GG::Y h = GG::Y1);

    CUILabel(const std::string& str,
             const std::vector<std::shared_ptr<GG::Font::TextElement>>& text_elements,
             GG::Flags<GG::TextFormat> format = GG::FORMAT_NONE,
             GG::Flags<GG::WndFlag> flags = GG::NO_WND_FLAGS,
             GG::X x = GG::X0, GG::Y y = GG::Y0, GG::X w = GG::X1, GG::Y h = GG::Y1);
    //@}

    /** \name Mutators */ //@{
    void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    //@}
};

/** a FreeOrion Button control */
class CUIButton : public GG::Button {
public:
    /** \name Structors */ //@{
    CUIButton(const std::string& str);

    CUIButton(const GG::SubTexture& unpressed, const GG::SubTexture& pressed, const GG::SubTexture& rollover);
    //@}

    /** \name Accessors */ //@{
    GG::Pt MinUsableSize() const override;

    bool InWindow(const GG::Pt& pt) const override;
    //@}

    /** \name Mutators */ //@{
    void MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    //@}

protected:
    /** \name Mutators control */ //@{
    void RenderPressed() override;

    void RenderRollover() override;

    void RenderUnpressed() override;
    //@}
};

class SettableInWindowCUIButton : public CUIButton {
public:
    /** \name Structors */ //@{
    SettableInWindowCUIButton(const GG::SubTexture& unpressed, const GG::SubTexture& pressed, const GG::SubTexture& rollover, boost::function<bool(const SettableInWindowCUIButton*, const GG::Pt&)> in_window_function);
    //@}
    /** \name Accessors */ //@{
    bool InWindow(const GG::Pt& pt) const override;
    //@}

private:
    boost::function<bool(const SettableInWindowCUIButton*, const GG::Pt&)>    m_in_window_func;
};

/** a FreeOrion triangular arrow button */
class CUIArrowButton : public GG::Button {
public:
    /** \name Structors */ //@{
    CUIArrowButton(ShapeOrientation orientation, bool fill_background,
                   GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE);
    //@}

    /** \name Accessors */ //@{
    bool InWindow(const GG::Pt& pt) const override;
    //@}

    /** \name Mutators */ //@{
    void MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    //@}

protected:
    /** \name Mutators control */ //@{
    void RenderPressed() override;

    void RenderRollover() override;

    void RenderUnpressed() override;
    //@}

private:
    ShapeOrientation m_orientation;
    bool             m_fill_background_with_wnd_color;
};

/** \brief A FreeOrion styled check box state button. */
class CUICheckBoxRepresenter : public GG::StateButtonRepresenter {
public:
    void Render(const GG::StateButton& button) const override;

    void OnChanged(const GG::StateButton& button, GG::StateButton::ButtonState prev_state) const override;

    void OnChecked(bool checked) const override;
};

/** \brief A FreeOrion styled radio state button. */
class CUIRadioRepresenter : public GG::StateButtonRepresenter {
public:
    void Render(const GG::StateButton& button) const override;

    void OnChanged(const GG::StateButton& button, GG::StateButton::ButtonState prev_state) const override;

    void OnChecked(bool checked) const override;
};

/** \brief A FreeOrion styled TabBar tab. */
class CUITabRepresenter : public GG::StateButtonRepresenter {
public:
    void Render(const GG::StateButton& button) const override;

    void OnChanged(const GG::StateButton& button, GG::StateButton::ButtonState prev_state) const override;

    void OnChecked(bool checked) const override;

    GG::Pt MinUsableSize(const GG::StateButton& button) const override;
};

/** @brief A FreeOrion styled label toggle button. */
class CUILabelButtonRepresenter : public GG::StateButtonRepresenter {
public:
    void Render(const GG::StateButton& button) const override;

    void OnChanged(const GG::StateButton& button, GG::StateButton::ButtonState prev_state) const override;

    void OnChecked(bool checked) const override;
};


/** @brief A FreeOrion styled icon toggle button.
 *
 * Renders a SubTexture depending on the checked state of the CUIToggleButton.
 * When the @a button has single texture and is unchecked, it renders the same
 * icon recolored towards grey.  If a second SubTexture is assigned to the
 * @a button, it will render the appropriate SubTexture.  When a mouse pointer
 * hovers over this button, the opposing SubTexture and color are used and
 * highlighted.
 */
class CUIIconButtonRepresenter : public GG::StateButtonRepresenter {
public:
    CUIIconButtonRepresenter(std::shared_ptr<GG::SubTexture> icon,
                             const GG::Clr& highlight_clr);

    CUIIconButtonRepresenter(std::shared_ptr<GG::SubTexture> unchecked_icon,
                             const GG::Clr& unchecked_clr,
                             std::shared_ptr<GG::SubTexture> checked_icon,
                             const GG::Clr& checked_clr);

    void Render(const GG::StateButton& button) const override;

    void OnChecked(bool checked) const override;

private:
    std::shared_ptr<GG::SubTexture> m_unchecked_icon;
    std::shared_ptr<GG::SubTexture> m_checked_icon;
    GG::Clr                             m_unchecked_color;
    GG::Clr                             m_checked_color;
};


/** a FreeOrion StateButton control */
class CUIStateButton : public GG::StateButton {
public:
    /** \name Structors */ //@{
    CUIStateButton(const std::string& str, GG::Flags<GG::TextFormat> format, std::shared_ptr<GG::StateButtonRepresenter> representer);
    //@}

};

/** Tab bar with buttons for selecting tabbed windows. */
class CUITabBar : public GG::TabBar {
public:
    /** \name Structors */ ///@{
    CUITabBar(const std::shared_ptr<GG::Font>& font, GG::Clr color,
              GG::Clr text_color);
    //@}

private:
    void DistinguishCurrentTab(const std::vector<GG::StateButton*>& tab_buttons) override;
};

/** a FreeOrion Scroll control */
class CUIScroll : public GG::Scroll {
public:
    /** represents the tab button for a CUIScroll */
    class ScrollTab : public GG::Button {
    public:
        ScrollTab(GG::Orientation orientation, int scroll_width, GG::Clr color, GG::Clr border_color);

        void SetColor(GG::Clr c) override;

        void Render() override;

        void LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

        void LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

        void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

        void MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

        void MouseLeave() override;

    private:
        GG::Clr m_border_color;
        GG::Orientation m_orientation;
        bool m_mouse_here;
        bool m_being_dragged;
    };

    /** \name Structors */ //@{
    CUIScroll(GG::Orientation orientation);
    //@}

    /** \name Mutators */ //@{
    void Render() override;

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;
    //@}

protected:
    GG::Clr m_border_color;
};

/** a FreeOrion ListBox control */
class CUIListBox : public GG::ListBox {
public:
    /** \name Structors */ //@{
    CUIListBox(void);
    //@}

    /** \name Mutators */ //@{
    void Render() override;
    //@}
};

/** a FreeOrion DropDownList control */
class CUIDropDownList : public GG::DropDownList {
public:
    /** \name Structors */ //@{
    explicit CUIDropDownList(size_t num_shown_elements);
    //@}

    /** Return the width of the dropped row which excludes the DropArrow. */
    GG::X DroppedRowWidth() const override;

    GG::Pt ClientLowerRight() const override;

    /** \name Mutators */ //@{
    void Render() override;

    void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void MouseLeave() override;

    void            DisableDropArrow();  ///< disables rendering of the small downward-facing arrow on the right of the control
    void            EnableDropArrow();   ///< enables rendering of the small downward-facing arrow on the right of the control
    //@}

private:
    void InitBuffer() override;

    bool    m_render_drop_arrow;
    bool    m_mouse_here;
};

/** a FreeOrion Edit control */
class CUIEdit : public GG::Edit {
public:
    /** \name Structors */ //@{
    explicit CUIEdit(const std::string& str);
    //@}

    /** \name Mutators */ //@{
    void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void KeyPress(GG::Key key, std::uint32_t key_code_point,
                  GG::Flags<GG::ModKey> mod_keys) override;
    void GainingFocus() override;
    void LosingFocus() override;
    void Render() override;
    //@}

    mutable boost::signals2::signal<void ()> GainingFocusSignal;
    mutable boost::signals2::signal<void ()> LosingFocusSignal;
};

/** a FreeOrion Edit control that replaces its displayed characters with a
  * placeholder. Useful for password entry.*/
class CensoredCUIEdit : public CUIEdit {
public:
    /** \name Structors */ //@{
    explicit CensoredCUIEdit(const std::string& str, char display_placeholder = '*');
    //@}

    /** \name Accessors */ //@{
    const std::string& RawText() const;
    //@}

    /** \name Mutators */ //@{
    void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void KeyPress(GG::Key key, std::uint32_t key_code_point,
                  GG::Flags<GG::ModKey> mod_keys) override;
    void SetText(const std::string& str) override;
    void AcceptPastedText(const std::string& text) override;
    //@}

protected:
    char m_placeholder = '*';

private:
    void ClearSelected();

    std::string m_raw_text = "";
};

/** a FreeOrion MultiEdit control */
class CUIMultiEdit : public GG::MultiEdit {
public:
    /** \name Structors */ //@{
    explicit CUIMultiEdit(const std::string& str,
                          GG::Flags<GG::MultiEditStyle> style = GG::MULTI_LINEWRAP);
    //@}
    void CompleteConstruction() override;

    /** \name Mutators */ //@{
    void Render() override;
    void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    //@}
};

/** a FreeOrion MultiEdit control that parses its text and makes links within clickable */
class CUILinkTextMultiEdit : public CUIMultiEdit, public TextLinker {
public:
    /** \name Structors */ //@{
    CUILinkTextMultiEdit(const std::string& str, GG::Flags<GG::MultiEditStyle> style = GG::MULTI_LINEWRAP);
    //@}
    void CompleteConstruction() override;

    /** \name Accessors */ //@{
    const std::vector<GG::Font::LineData>& GetLineData() const override;
    const std::shared_ptr<GG::Font>& GetFont() const override;
    GG::Pt TextUpperLeft() const override;
    GG::Pt TextLowerRight() const override;
    const std::string& RawText() const override;
    //@}

    /** \name Mutators */ //@{
    void Render() override;
    void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseLeave() override;

    /** Needed primarily so the SetText call will take a RawText. */
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    /** sets the text to \a str; may resize the window.  If the window was
        constructed to fit the size of the text (i.e. if the second ctor type
        was used), calls to this function cause the window to be resized to
        whatever space the newly rendered text occupies. */
    void SetText(const std::string& str) override;
    //@}

private:
    void SetLinkedText(const std::string& str) override;

    bool            m_already_setting_text_so_dont_link;
    std::string     m_raw_text;
};

/** A simple GG::ListBox::Row subclass designed for use in text-only drop-down
  * lists, such as the ones used in the game setup dialogs. */
struct CUISimpleDropDownListRow : public GG::ListBox::Row {
    CUISimpleDropDownListRow(const std::string& row_text, GG::Y row_height = DEFAULT_ROW_HEIGHT);
    void CompleteConstruction() override;
    static const GG::Y DEFAULT_ROW_HEIGHT;
private:
    std::shared_ptr<CUILabel> m_row_label;
};

/** Encapsulates an icon and text that goes with it in a single control.  For
  * example, "[trade icon] +1" or "[population icon] 66 (+5)", where [... icon]
  * is an icon image, not text.
  * The icon may have one or two numerical values.  If one, just that number is
  * displayed.  If two, the first number is displayed followed by the second in
  * brackets "()"

  * Sizing StatisticIcon correctly in the constructor saves time because resizing the value string
  * is processor intensive.
  */
class StatisticIcon : public GG::Control {
public:
    /** \name Structors */ //@{
    StatisticIcon(const std::shared_ptr<GG::Texture> texture,
                  GG::X w = GG::X1, GG::Y h = GG::Y1); ///< initialized with no value (just an icon)

    StatisticIcon(const std::shared_ptr<GG::Texture> texture,
                  double value, int digits, bool showsign,
                  GG::X w = GG::X1, GG::Y h = GG::Y1); ///< initializes with one value

    StatisticIcon(const std::shared_ptr<GG::Texture> texture,
                  double value0, double value1, int digits0, int digits1,
                  bool showsign0, bool showsign1,
                  GG::X w = GG::X1, GG::Y h = GG::Y1); ///< initializes with two values
    //@}

    void CompleteConstruction() override;

    /** \name Accessors */ //@{
    double          GetValue(int index = 0) const;
    GG::Pt          MinUsableSize() const override;
    //@}

    /** \name Mutators */ //@{
    void PreRender() override;

    void Render() override
    {}

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void RButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys) override;

    void            SetValue(double value, int index = 0);  ///< sets displayed \a value with \a index
    //@}

    mutable boost::signals2::signal<void ()>    LeftClickedSignal;
    mutable boost::signals2::signal<void ()>    RightClickedSignal;

private:
    void            DoLayout();
    GG::Clr         ValueColor(int index) const;        ///< returns colour in which to draw value

    int                 m_num_values;

    std::vector<double> m_values;
    std::vector<int>    m_digits;
    std::vector<bool>   m_show_signs;

    std::shared_ptr<GG::StaticGraphic>  m_icon;
    std::shared_ptr<GG::Label>          m_text;
};

class CUIToolBar : public GG::Control {
public:
    /** \name Structors */ //@{
    CUIToolBar();
    //@}

    /** \name Accessors */ //@{
    bool InWindow(const GG::Pt& pt) const override;
    //@}

    /** \name Mutators */ //@{
    void Render() override;
    //@}
private:
};

/** A control used to pick from at list of species names. */
class SpeciesSelector : public CUIDropDownList {
public:
    /** \name Structors */ //@{
    SpeciesSelector(const std::string& preselect_species, GG::X w, GG::Y h);                          ///< populates with all species in SpeciesManager
    //@}

    /** \name Accessors */ //@{
    const std::string&          CurrentSpeciesName() const;     ///< returns the name of the species that is currently selected
    //@}

    mutable boost::signals2::signal<void (const std::string&)> SpeciesChangedSignal;
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
};

/** A control used to pick arbitrary colors using GG::ColorDlg. */
class ColorSelector : public GG::Control {
public:
    /** \name Structors */ //@{
    ColorSelector(GG::Clr color, GG::Clr default_color);

    virtual ~ColorSelector();
    //@}

    /** \name Mutators */ //@{
    void Render() override;

    void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;
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
            std::vector<std::pair<std::string, std::string>> types);
    //@}
    void CompleteConstruction() override;
private:
    const std::vector<std::pair<std::string, std::string>> m_init_file_filters;
};

/** Displays resource and stockpile info on the Researach and Production
    screens. */
class ResourceInfoPanel : public CUIWnd {
public:
    /** \name Structors */ //@{
    ResourceInfoPanel(const std::string& title, const std::string& point_units_str,
                      const GG::X x, const GG::Y y, const GG::X w, const GG::Y h,
                      const std::string& config_name);
    //@}
    void    CompleteConstruction() override;

    /** \name Accessors */ //@{
    GG::Pt  MinUsableSize() const override;
    //@}

    /** \name Mutators */ //@{
    void    SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void    SetTotalPointsCost(float total_points, float total_cost);
    void    SetStockpileCost(float stockpile, float stockpile_use, float stockpile_use_max);
    void    SetLocalPointsCost(float local_points, float local_cost, float local_stockpile_use,
                               float local_stockpile_use_max, const std::string& location_name);
    void    SetEmpireID(int empire_id);
    void    ClearLocalInfo();
    void    Clear();
    //@}

private:
    void    DoLayout();

    std::string m_units_str;
    std::string m_title_str;
    int         m_empire_id;

    std::shared_ptr<GG::Label>  m_empire_column_label;
    std::shared_ptr<GG::Label>  m_local_column_label;
    std::shared_ptr<GG::Label>  m_total_points_label;
    std::shared_ptr<GG::Label>  m_total_points;
    std::shared_ptr<GG::Label>  m_total_points_P_label;
    std::shared_ptr<GG::Label>  m_stockpile_points_label;
    std::shared_ptr<GG::Label>  m_stockpile_points;
    std::shared_ptr<GG::Label>  m_stockpile_points_P_label;
    std::shared_ptr<GG::Label>  m_stockpile_use_label;
    std::shared_ptr<GG::Label>  m_stockpile_use;
    std::shared_ptr<GG::Label>  m_stockpile_use_P_label;
    std::shared_ptr<GG::Label>  m_local_stockpile_use;
    std::shared_ptr<GG::Label>  m_local_stockpile_use_P_label;
    std::shared_ptr<GG::Label>  m_stockpile_max_use_label;
    std::shared_ptr<GG::Label>  m_stockpile_max_use;
    std::shared_ptr<GG::Label>  m_stockpile_max_use_P_label;
    std::shared_ptr<GG::Label>  m_wasted_points_label;
    std::shared_ptr<GG::Label>  m_wasted_points;
    std::shared_ptr<GG::Label>  m_wasted_points_P_label;
    std::shared_ptr<GG::Label>  m_local_points;
    std::shared_ptr<GG::Label>  m_local_points_P_label;
    std::shared_ptr<GG::Label>  m_local_wasted_points;
    std::shared_ptr<GG::Label>  m_local_wasted_points_P_label;
};

/** Displays progress that is divided over mulitple turns, as in the Research and Production screens. */
class MultiTurnProgressBar : public GG::Control {
public:
    /** @name Structors */ //@{
    /** Ctor
    * @param[in] num_segments Number of segments in the bar
    * @param[in] percent_completed Percent(0.0-1.0) of bar to fill,
    * @param[in] percent_predicted Percent(0.0-1.0) of bar to fill after
    *               @p percent_completed.  Not to exceed 1.0 after addition
    *               to @p percent_completed
    * @param[in] bar_color Color for @p percent_completed, adjusted to lighter
    *               color for @p percent_predicted
    * @param[in] bg_color Color for background of this control
    * @param[in] outline_color Color for the border.  Adjusted to darker color
    *               for segment lines within @p percent_completed portion.
    *               Adjusted to lighter color for segment lines after
    *               @p percent_completed portion
    */
    MultiTurnProgressBar(int num_segments, float percent_completed,
                         float percent_predicted, const GG::Clr& bar_color,
                         const GG::Clr& bg_color, const GG::Clr& outline_color);
    //@}

    /** @name Mutators */ //@{
    void Render() override;
    //@}

private:
    int m_num_segments;
    float m_perc_completed;
    float m_perc_predicted;
    GG::Clr m_clr_bar;
    GG::Clr m_clr_bg;
    GG::Clr m_clr_outline;
};

/** Displays current rendering frames per second. */
class FPSIndicator : public GG::Label {
public:
    FPSIndicator();

    void Render() override;

    void PreRender() override;
private:
    void UpdateEnabled();
    bool m_enabled;
    int m_displayed_FPS;
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
    MultiTextureStaticGraphic(const std::vector<std::shared_ptr<GG::Texture>>& textures,
                              const std::vector<GG::Flags<GG::GraphicStyle>>& styles = std::vector<GG::Flags<GG::GraphicStyle>>());

    /** creates a MultiTextureStaticGraphic from multiple pre-existing SubTextures which are rendered back-to-front in the
      * order they are specified in \a subtextures with GraphicStyles specified in the same-indexed value of \a styles.
      * if \a styles is not specified or contains fewer entres than \a subtextures, entries in \a subtextures without 
      * associated styles use the style GRAPHIC_CENTER. */
    MultiTextureStaticGraphic(const std::vector<GG::SubTexture>& subtextures,
                              const std::vector<GG::Flags<GG::GraphicStyle>>& styles = std::vector<GG::Flags<GG::GraphicStyle>>());
    //@}

    /** \name Mutators */ ///@{
    /** Renders textures in order specified in constructor, back-to-front. */
    void Render() override;
    //@}

protected:
    MultiTextureStaticGraphic();

    /** Returns the area in which the graphic is actually rendered, in
        UpperLeft()-relative coordinates.  This may not be the entire area of
        the StaticGraphic, based on the style being used. */
    GG::Rect        RenderedArea(const GG::SubTexture& subtexture, GG::Flags<GG::GraphicStyle> style) const;

private:
    void            Init();
    void            ValidateStyles();      ///< ensures that the style flags are consistent

    std::vector<GG::SubTexture>                 m_graphics;
    std::vector<GG::Flags<GG::GraphicStyle>>    m_styles;   ///< position of texture wrt the window area
};

/** Functions like a StaticGraphic, except can be rotated with a fixed phase
  * and/or at a continuous angular rate. */
class RotatingGraphic : public GG::StaticGraphic {
public:
    /** \name Structors */ ///@{
    RotatingGraphic(const std::shared_ptr<GG::Texture>& texture, GG::Flags<GG::GraphicStyle> style = GG::GRAPHIC_NONE,
                    GG::Flags<GG::WndFlag> flags = GG::NO_WND_FLAGS);
    //@}

    /** \name Mutators */ ///@{
    void Render() override;

    void            SetRPM(float rpm)               { m_rpm = std::max(-3600.0f, std::min(3600.0f, rpm)); }
    void            SetPhaseOffset(float degrees)   { m_phase_offset = degrees; }
    //@}

private:
    float   m_rpm;
    float   m_phase_offset;
};

/** Renders scanlines over its area. */
class ScanlineControl : public GG::Control {
public:
    ScanlineControl(GG::X x, GG::Y y, GG::X w, GG::Y h, bool square = false, GG::Clr clr = GG::CLR_BLACK);

    /** Changes the color used to draw the scanlines. */
    void Render() override;

    void SetColor(GG::Clr clr) override
    { m_color = clr; };

private:
    bool m_square;
    GG::Clr m_color;
};

/** Consistently rendered popup menu */
class CUIPopupMenu : public GG::PopupMenu {
public:
    CUIPopupMenu(GG::X x, GG::Y y);
};

#endif // _CUIControls_h_
