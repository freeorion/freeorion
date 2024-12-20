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

#include "LinkText.h"

/** \file
 *
 * All CUI* classes are FreeOrion-style controls incorporating
 * the visual theme the project requires.  Implementation may
 * depend on graphics and design team specifications.  They extend
 * GG controls.
 */

struct ScriptingContext;

/** a FreeOrion Label control */
class CUILabel final : public GG::TextControl {
public:
    CUILabel(std::string str,
             GG::Flags<GG::TextFormat> format,
             GG::Flags<GG::WndFlag> flags,
             std::shared_ptr<GG::Font> font,
             GG::X x = GG::X0, GG::Y y = GG::Y0, GG::X w = GG::X1, GG::Y h = GG::Y1);

    CUILabel(std::string str,
             GG::Flags<GG::TextFormat> format = GG::FORMAT_NONE,
             GG::Flags<GG::WndFlag> flags = GG::NO_WND_FLAGS,
             GG::X x = GG::X0, GG::Y y = GG::Y0, GG::X w = GG::X1, GG::Y h = GG::Y1);

    CUILabel(std::string str,
             std::vector<GG::Font::TextElement> text_elements,
             GG::Flags<GG::TextFormat> format = GG::FORMAT_NONE,
             GG::Flags<GG::WndFlag> flags = GG::NO_WND_FLAGS,
             GG::X x = GG::X0, GG::Y y = GG::Y0, GG::X w = GG::X1, GG::Y h = GG::Y1);

    void RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
};

/** a FreeOrion Button control */
class CUIButton : public GG::Button {
public:
    CUIButton(std::string str);

    CUIButton(GG::SubTexture unpressed, GG::SubTexture pressed, GG::SubTexture rollover);

    GG::Pt MinUsableSize() const override;

    bool InWindow(GG::Pt pt) const override;

protected:
    void MouseEnter(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;

    void RenderPressed() override;
    void RenderRollover() override;
    void RenderUnpressed() override;
};

class SettableInWindowCUIButton final : public CUIButton {
public:
    using TestFuncT = std::function<bool (const SettableInWindowCUIButton*, GG::Pt)>;

    SettableInWindowCUIButton(GG::SubTexture unpressed, GG::SubTexture pressed, GG::SubTexture rollover,
                              TestFuncT in_window_function);

    bool InWindow(GG::Pt pt) const override;

private:
    TestFuncT m_in_window_func;
};

/** a FreeOrion triangular arrow button */
class CUIArrowButton final : public GG::Button {
public:
    CUIArrowButton(ShapeOrientation orientation, bool fill_background,
                   GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE);

    bool InWindow(GG::Pt pt) const noexcept override;

    void MouseHere(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;

protected:
    void RenderPressed() override;
    void RenderRollover() override;
    void RenderUnpressed() override;

private:
    ShapeOrientation m_orientation;
    bool             m_fill_background_with_wnd_color;
};

/** \brief A FreeOrion styled check box state button. */
class CUICheckBoxRepresenter final : public GG::StateButtonRepresenter {
public:
    void Render(const GG::StateButton& button) const override;

    void OnChanged(const GG::StateButton& button, GG::StateButton::ButtonState prev_state) const override;

    void OnChecked(bool checked) const override;
};

/** \brief A FreeOrion styled radio state button. */
class CUIRadioRepresenter final : public GG::StateButtonRepresenter {
public:
    void Render(const GG::StateButton& button) const override;

    void OnChanged(const GG::StateButton& button, GG::StateButton::ButtonState prev_state) const override;

    void OnChecked(bool checked) const override;
};

/** \brief A FreeOrion styled TabBar tab. */
class CUITabRepresenter final : public GG::StateButtonRepresenter {
public:
    void Render(const GG::StateButton& button) const override;

    void OnChanged(const GG::StateButton& button, GG::StateButton::ButtonState prev_state) const override;

    void OnChecked(bool checked) const override;

    GG::Pt MinUsableSize(const GG::StateButton& button) const override;
};

/** @brief A FreeOrion styled label toggle button. */
class CUILabelButtonRepresenter final : public GG::StateButtonRepresenter {
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
class CUIIconButtonRepresenter final : public GG::StateButtonRepresenter {
public:
    CUIIconButtonRepresenter(std::shared_ptr<GG::SubTexture> icon,
                             GG::Clr highlight_clr);

    CUIIconButtonRepresenter(std::shared_ptr<GG::SubTexture> unchecked_icon,
                             GG::Clr unchecked_clr,
                             std::shared_ptr<GG::SubTexture> checked_icon,
                             GG::Clr checked_clr);

    void Render(const GG::StateButton& button) const override;

    void OnChecked(bool checked) const override;

private:
    std::shared_ptr<GG::SubTexture> m_unchecked_icon;
    std::shared_ptr<GG::SubTexture> m_checked_icon;
    GG::Clr                         m_unchecked_color;
    GG::Clr                         m_checked_color;
};


/** a FreeOrion StateButton control */
class CUIStateButton final : public GG::StateButton {
public:
    CUIStateButton(std::string str, GG::Flags<GG::TextFormat> format,
                   std::shared_ptr<GG::StateButtonRepresenter> representer);

};

/** Tab bar with buttons for selecting tabbed windows. */
class CUITabBar final : public GG::TabBar {
public:
    CUITabBar(const std::shared_ptr<GG::Font>& font, GG::Clr color,
              GG::Clr text_color);

private:
    void DistinguishCurrentTab(const std::vector<GG::StateButton*>& tab_buttons) override;
};

/** a FreeOrion Scroll control */
class CUIScroll final : public GG::Scroll {
public:
    /** represents the tab button for a CUIScroll */
    class ScrollTab final : public GG::Button {
    public:
        ScrollTab(GG::Orientation orientation, int scroll_width, GG::Clr color, GG::Clr border_color);

        void SetColor(GG::Clr) noexcept override {} // intentionally ignored

        void Render() override;
        void LButtonDown(GG::Pt, GG::Flags<GG::ModKey>) noexcept override { m_being_dragged = true; }
        void LButtonUp(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
        void LClick(GG::Pt, GG::Flags<GG::ModKey>) noexcept override { m_being_dragged = false; }
        void MouseEnter(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
        void MouseLeave() noexcept override  { if (!m_being_dragged) m_mouse_here = false; }

    private:
        const GG::Clr m_border_color;
        const GG::Orientation m_orientation;
        bool m_mouse_here = false;
        bool m_being_dragged = false;
    };

    CUIScroll(GG::Orientation orientation);

    void Render() override;

    void SizeMove(GG::Pt ul, GG::Pt lr) override;

protected:
    GG::Clr m_border_color;
};

/** a FreeOrion ListBox control */
class CUIListBox : public GG::ListBox {
public:
    CUIListBox();
    void Render() override;
};

/** a FreeOrion DropDownList control */
class CUIDropDownList : public GG::DropDownList {
public:
    explicit CUIDropDownList(std::size_t num_shown_elements);

    /** Return the width of the dropped row which excludes the DropArrow. */
    GG::X DroppedRowWidth() const override;

    GG::Pt ClientLowerRight() const noexcept override;

    void Render() override;
    void LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseEnter(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseLeave() override;

    void DisableDropArrow();  ///< disables rendering of the small downward-facing arrow on the right of the control
    void EnableDropArrow();   ///< enables rendering of the small downward-facing arrow on the right of the control

private:
    void InitBuffer() override;

    bool m_render_drop_arrow = false;
    bool m_mouse_here = false;
};

/** a FreeOrion Edit control */
class CUIEdit : public GG::Edit {
public:
    explicit CUIEdit(std::string str);

    void CompleteConstruction() override;

    void RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void KeyPress(GG::Key key, uint32_t key_code_point,
                  GG::Flags<GG::ModKey> mod_keys) override;
    void AcceptPastedText(const std::string& text) override;
    void GainingFocus() override;
    void LosingFocus() override;
    void Render() override;
    virtual bool AutoComplete() { return false; };
    void DisallowChars(std::string_view chars) { m_disallowed_chars = chars; }

    mutable boost::signals2::signal<void ()> GainingFocusSignal;
    mutable boost::signals2::signal<void ()> LosingFocusSignal;

private:
    std::string_view m_disallowed_chars = "";
};

/** a FreeOrion Edit control that replaces its displayed characters with a
  * placeholder. Useful for password entry.*/
class CensoredCUIEdit final : public CUIEdit {
public:
    explicit CensoredCUIEdit(std::string str, char display_placeholder = '*');

    const auto& RawText() const noexcept { return m_raw_text; }

    void RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void SetText(std::string str) override;
    void AcceptPastedText(const std::string& text) override;

protected:
    char m_placeholder = '*';

private:
    void ClearSelected();

    std::string m_raw_text;
};

/** a FreeOrion MultiEdit control */
class CUIMultiEdit : public GG::MultiEdit {
public:
    explicit CUIMultiEdit(std::string str,
                          GG::Flags<GG::MultiEditStyle> style = GG::MULTI_LINEWRAP);
    void CompleteConstruction() override;

    void Render() override;
    void RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
};

/** a FreeOrion MultiEdit control that parses its text and makes links within clickable */
class CUILinkTextMultiEdit final : public CUIMultiEdit, public TextLinker {
public:
    CUILinkTextMultiEdit(std::string str, GG::Flags<GG::MultiEditStyle> style = GG::MULTI_LINEWRAP);
    void CompleteConstruction() override;

    const GG::Font::LineVec& GetLineData() const noexcept override { return CUIMultiEdit::GetLineData(); }
    const std::shared_ptr<GG::Font>& GetFont() const noexcept override { return CUIMultiEdit::GetFont(); }

    GG::Pt TextUpperLeft() const override;
    GG::Pt TextLowerRight() const override;
    const std::string& RawText() const noexcept override { return m_raw_text; }

    void Render() override;
    void LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseHere(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseLeave() override;

    /** Needed primarily so the SetText call will take a RawText. */
    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    /** sets the text to \a str; may resize the window.  If the window was
        constructed to fit the size of the text (i.e. if the second ctor type
        was used), calls to this function cause the window to be resized to
        whatever space the newly rendered text occupies. */
    void SetText(std::string str) override;

private:
    void SetLinkedText(std::string str) override;

    bool        m_already_setting_text_so_dont_link = false;
    std::string m_raw_text;
};

/** A simple GG::ListBox::Row subclass designed for use in text-only drop-down
  * lists, such as the ones used in the game setup dialogs. */
struct CUISimpleDropDownListRow final : public GG::ListBox::Row {
    CUISimpleDropDownListRow(std::string row_text, GG::Y row_height = DEFAULT_ROW_HEIGHT);
    void CompleteConstruction() override;
    static constexpr GG::Y DEFAULT_ROW_HEIGHT{22};
private:
    std::shared_ptr<CUILabel> m_row_label;
};

/** Encapsulates an icon and text that goes with it in a single control.  For
  * example, "[research icon] +1" or "[population icon] 66 (+5)", where [... icon]
  * is an icon image, not text.
  * The icon may have one or two numerical values.  If one, just that number is
  * displayed.  If two, the first number is displayed followed by the second in
  * brackets "()"

  * Sizing StatisticIcon correctly in the constructor saves time because resizing the value string
  * is processor intensive.
  */
class StatisticIcon final : public GG::Control {
public:
    StatisticIcon(std::shared_ptr<GG::Texture> texture,
                  GG::X w = GG::X1, GG::Y h = GG::Y1); ///< initialized with no value (just an icon)

    StatisticIcon(std::shared_ptr<GG::Texture> texture,
                  double value, int digits, bool showsign,
                  GG::X w = GG::X1, GG::Y h = GG::Y1); ///< initializes with one value

    void CompleteConstruction() override;

    double GetValue(std::size_t index = 0) const;
    GG::Pt MinUsableSize() const override;

    void PreRender() override;
    void Render() override {}

    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    void LButtonDown(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void RButtonDown(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys) override;

    void AcceptDrops(GG::Pt pt, std::vector<std::shared_ptr<GG::Wnd>> wnds, GG::Flags<GG::ModKey> mod_keys) override;
    void DragDropEnter(GG::Pt pt, std::map<const GG::Wnd*, bool>& drop_wnds_acceptable,
                       GG::Flags<GG::ModKey> mod_keys) override;
    void DragDropHere(GG::Pt pt, std::map<const GG::Wnd*, bool>& drop_wnds_acceptable,
                      GG::Flags<GG::ModKey> mod_keys) override;
    void CheckDrops(GG::Pt pt, std::map<const GG::Wnd*, bool>& drop_wnds_acceptable,
                    GG::Flags<GG::ModKey> mod_keys) override;
    void DragDropLeave() override;

    void SetValue(double value, std::size_t index = 0);  ///< sets displayed \a value with \a index

    mutable boost::signals2::signal<void (GG::Pt)> LeftClickedSignal;
    mutable boost::signals2::signal<void (GG::Pt)> RightClickedSignal;

private:
    void DoLayout();

    /// The value, precision and sign of the statistic value(s)
    std::shared_ptr<GG::StaticGraphic>           m_icon;
    std::shared_ptr<GG::Label>                   m_text;
    std::array<std::tuple<double, int, bool>, 2> m_values{{{0.0, 0, false}, {0.0, 0, false}}};
    bool                                         m_have_two = false;
};

class CUIToolBar final : public GG::Control {
public:
    CUIToolBar();

    bool InWindow(GG::Pt pt) const override;

    void Render() override;
private:
};

/** A control used to pick from at list of species names. */
class SpeciesSelector final : public CUIDropDownList {
public:
    SpeciesSelector(const std::string& preselect_species, GG::X w, GG::Y h);    ///< populates with all species in SpeciesManager

    const std::string& CurrentSpeciesName() const;  ///< returns the name of the species that is currently selected

    mutable boost::signals2::signal<void (const std::string&)> SpeciesChangedSignal;
};

/** A control used to pick from the empire colors returned by EmpireColors(). */
class EmpireColorSelector final : public CUIDropDownList {
public:
    explicit EmpireColorSelector(GG::Y h);

    GG::Clr CurrentColor() const; ///< returns the color that is currently selected, or GG::CLR_ZERO if none is selected

    void SelectColor(GG::Clr clr);

    mutable boost::signals2::signal<void (GG::Clr)> ColorChangedSignal;
};

/** A control used to pick arbitrary colors using GG::ColorDlg. */
class ColorSelector final : public GG::Control {
public:
    ColorSelector(GG::Clr color, GG::Clr default_color);

    void Render() override;
    void LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    mutable boost::signals2::signal<void (GG::Clr)> ColorChangedSignal;

private:
    virtual void InitBuffer();
    GG::GL2DVertexBuffer    m_border_buffer;
    GG::Clr                 m_default_color;
};

/** A GG file dialog in the FreeOrion style. */
class FileDlg final : public GG::FileDlg {
public:
    FileDlg(const std::string& directory, const std::string& filename, bool save, bool multi,
            std::vector<std::pair<std::string, std::string>> types);
    void CompleteConstruction() override;

private:
    const std::vector<std::pair<std::string, std::string>> m_init_file_filters;
};

/** Displays resource and stockpile info on the Researach and Production
    screens. */
class ResourceInfoPanel final : public CUIWnd {
public:
    ResourceInfoPanel(std::string title, std::string point_units_str,
                      const GG::X x, const GG::Y y, const GG::X w, const GG::Y h,
                      std::string_view config_name);
    void    CompleteConstruction() override;

    GG::Pt  MinUsableSize() const override;

    void    SizeMove(GG::Pt ul, GG::Pt lr) override;

    void    SetTotalPointsCost(float total_points, float total_cost, const ScriptingContext& context);
    void    SetStockpileCost(float stockpile, float stockpile_use, float stockpile_use_max);
    void    SetLocalPointsCost(float local_points, float local_cost, float local_stockpile_use,
                               float local_stockpile_use_max, const std::string& location_name,
                               const ScriptingContext& context);
    void    SetEmpireID(int empire_id);
    void    ClearLocalInfo();
    void    Clear();

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
class MultiTurnProgressBar final : public GG::Control {
public:
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
    MultiTurnProgressBar(int num_segments, float percent_completed, float percent_predicted,
                         GG::Clr bar_color, GG::Clr bg_color, GG::Clr outline_color);

    void Render() override;

private:
    unsigned int m_num_segments = 0;
    float m_perc_completed = 0.0f;
    float m_perc_predicted = 0.0f;
    GG::Clr m_clr_bar;
    GG::Clr m_clr_bg;
    GG::Clr m_clr_outline;
};

/** Displays current rendering frames per second. */
class FPSIndicator final : public GG::Label {
public:
    FPSIndicator();

    void Render() override;
    void PreRender() override;

private:
    void UpdateEnabled();
    bool m_enabled = false;
    int m_displayed_FPS = 0;
};

/** Functions like a StaticGraphic, except can have multiple textures rendered
  * on top of eachother, rather than just a single texture. */
class MultiTextureStaticGraphic final : public GG::Control {
public:
    /** creates a MultiTextureStaticGraphic from multiple pre-existing Textures which are rendered back-to-front in the
      * order they are specified in \a textures with GraphicStyles specified in the same-indexed value of \a styles.
      * if \a styles is not specified or contains fewer entres than \a textures, entries in \a textures without 
      * associated styles use the style GRAPHIC_CENTER. */
    MultiTextureStaticGraphic(std::vector<std::shared_ptr<GG::Texture>> textures,
                              std::vector<GG::Flags<GG::GraphicStyle>> styles = {});

    /** creates a MultiTextureStaticGraphic from multiple pre-existing SubTextures which are rendered back-to-front in the
      * order they are specified in \a subtextures with GraphicStyles specified in the same-indexed value of \a styles.
      * if \a styles is not specified or contains fewer entres than \a subtextures, entries in \a subtextures without 
      * associated styles use the style GRAPHIC_CENTER. */
    MultiTextureStaticGraphic(std::vector<GG::SubTexture> subtextures,
                              std::vector<GG::Flags<GG::GraphicStyle>> styles = {});

    /** Renders textures in order specified in constructor, back-to-front. */
    void Render() override;

protected:
    MultiTextureStaticGraphic() = default;

    /** Returns the area in which the graphic is actually rendered, in
        UpperLeft()-relative coordinates.  This may not be the entire area of
        the StaticGraphic, based on the style being used. */
    GG::Rect RenderedArea(const GG::SubTexture& subtexture, GG::Flags<GG::GraphicStyle> style) const;

private:
    void Init();
    void ValidateStyles();      ///< ensures that the style flags are consistent

    std::vector<GG::SubTexture>                 m_graphics;
    std::vector<GG::Flags<GG::GraphicStyle>>    m_styles;   ///< position of texture wrt the window area
};

/** Functions like a StaticGraphic, except can be rotated with a fixed phase
  * and/or at a continuous angular rate. */
class RotatingGraphic final : public GG::StaticGraphic {
public:
    RotatingGraphic(std::shared_ptr<GG::Texture> texture,
                    GG::Flags<GG::GraphicStyle> style = GG::GRAPHIC_NONE,
                    GG::Flags<GG::WndFlag> flags = GG::NO_WND_FLAGS);

    void Render() override;

    void SetRPM(float rpm)               { m_rpm = std::max(-3600.0f, std::min(3600.0f, rpm)); }
    void SetPhaseOffset(float degrees)   { m_phase_offset = degrees; }

private:
    float m_rpm = 20.0f;
    float m_phase_offset = 0.0f;
    GG::GL2DVertexBuffer verts;
    GG::Rect last_rendered_area = {};
};

/** Renders scanlines over its area. */
class ScanlineControl final : public GG::Control {
public:
    ScanlineControl(GG::X x = GG::X0, GG::Y y = GG::Y0, GG::X w = GG::X1, GG::Y h = GG::Y1,
                    bool square = false, GG::Clr clr = GG::CLR_BLACK) :
        Control(x, y, w, h, GG::NO_WND_FLAGS),
        m_color(clr),
        m_square(square)
    {}

    void Render() override;

    /** Changes the color used to draw the scanlines. */
    void SetColor(GG::Clr clr) noexcept override { m_color = clr; };

private:
    GG::Clr m_color = GG::CLR_WHITE;
    const bool m_square = false;
};

/** Consistently rendered popup menu */
class CUIPopupMenu : public GG::PopupMenu {
public:
    CUIPopupMenu(GG::X x, GG::Y y);
};


#endif
