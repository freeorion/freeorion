//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/dialogs/ColorDlg.h
//!
//! Contains the Wnd class, upon which all GG GUI elements are based.

#ifndef _GG_dialogs_ColorDlg_h_
#define _GG_dialogs_ColorDlg_h_


#include <GG/Button.h>
#include <GG/ClrConstants.h>


namespace GG {

class Font;
template <typename T>
class Slider;

/** \brief Contains the necessary data to represent a color in HSV space, with
    an alpha value thrown in to make conversions to and from GG::Clr
    possible. */
struct GG_API HSVClr
{
    constexpr HSVClr() = default;

    constexpr HSVClr(double h_, double s_, double v_, GLubyte a_ = 255) noexcept :
        h(h_),
        s(s_),
        v(v_),
        a(a_)
    {}

    constexpr HSVClr(Clr color) noexcept :
        a(color.a)
    {
        double r = (color.r / 255.0);
        double g = (color.g / 255.0);
        double b = (color.b / 255.0);

        double min_channel = std::min(r, std::min(g, b));
        double max_channel = std::max(r, std::max(g, b));
        double channel_range = max_channel - min_channel;

        v = max_channel;

        if (max_channel < EPSILON) {
            h = 0.0;
            s = 0.0;
            return;
        }

        s = channel_range / max_channel;
        if (channel_range == 0.0) {
            h = 0.0;
            return;
        }

        double delta_r = (((max_channel - r) / 6.0) + (channel_range / 2.0)) / channel_range;
        double delta_g = (((max_channel - g) / 6.0) + (channel_range / 2.0)) / channel_range;
        double delta_b = (((max_channel - b) / 6.0) + (channel_range / 2.0)) / channel_range;

        if (r == max_channel)
            h = delta_b - delta_g;
        else if (g == max_channel)
            h = (1.0 / 3.0) + delta_r - delta_b;
        else if (b == max_channel)
            h = (2.0 / 3.0) + delta_g - delta_r;

        if (h < 0.0)
            h += 1.0;
        if (1.0 < h)
            h -= 1.0;
    }

    constexpr operator Clr() const noexcept {
        Clr retval{0, 0, 0, a};

        if (s < EPSILON) {
            retval.r = static_cast<GLubyte>(v * 255);
            retval.g = static_cast<GLubyte>(v * 255);
            retval.b = static_cast<GLubyte>(v * 255);
            return retval;
        }
        double tmph = h * 6.0;
        int tmpi = static_cast<int>(tmph);
        double tmp1 = v * (1 - s);
        double tmp2 = v * (1 - s * (tmph - tmpi));
        double tmp3 = v * (1 - s * (1 - (tmph - tmpi)));

        switch (tmpi) {
        case 0:
            retval.r = static_cast<GLubyte>(v * 255);
            retval.g = static_cast<GLubyte>(tmp3 * 255);
            retval.b = static_cast<GLubyte>(tmp1 * 255);
            break;
        case 1:
            retval.r = static_cast<GLubyte>(tmp2 * 255);
            retval.g = static_cast<GLubyte>(v * 255);
            retval.b = static_cast<GLubyte>(tmp1 * 255);
            break;
        case 2:
            retval.r = static_cast<GLubyte>(tmp1 * 255);
            retval.g = static_cast<GLubyte>(v * 255);
            retval.b = static_cast<GLubyte>(tmp3 * 255);
            break;
        case 3:
            retval.r = static_cast<GLubyte>(tmp1 * 255);
            retval.g = static_cast<GLubyte>(tmp2 * 255);
            retval.b = static_cast<GLubyte>(v * 255);
            break;
        case 4:
            retval.r = static_cast<GLubyte>(tmp3 * 255);
            retval.g = static_cast<GLubyte>(tmp1 * 255);
            retval.b = static_cast<GLubyte>(v * 255);
            break;
        default:
            retval.r = static_cast<GLubyte>(v * 255);
            retval.g = static_cast<GLubyte>(tmp1 * 255);
            retval.b = static_cast<GLubyte>(tmp2 * 255);
            break;
        }

        return retval;
    }

    double  h = 0.0; ///< hue
    double  s = 0.0; ///< saturation
    double  v = 0.0; ///< value
    GLubyte a = 0;   ///< alpha

private:
    static constexpr double EPSILON = 0.0001;
};

/** \brief A control specifically designed for ColorDlg that allows the user
    to select a point in the Hue-Saturation subspace of the HSV color
    space. */
class GG_API HueSaturationPicker : public Control
{
public:
    /** emitted whenever the hue or saturation in the picker changes */
    typedef boost::signals2::signal<void (double, double)> ChangedSignalType;

    HueSaturationPicker(X x, Y y, X w, Y h);

    void Render() override;

    void LButtonDown(Pt pt, Flags<ModKey> mod_keys) override;

    void LDrag(Pt pt, Pt move, Flags<ModKey> mod_keys) override;

    void SetHueSaturation(double hue, double saturation); ///< sets the current hue and saturation.  Note that this does not cause a signal to be emitted.

    mutable ChangedSignalType ChangedSignal; ///< emitted whenever the hue or saturation in the picker changes

private:
    void SetHueSaturationFromPt(Pt pt);

    double m_hue = 0.0;
    double m_saturation = 0.0;
    std::vector<std::vector<std::pair<double, double>>> m_vertices;
    std::vector<std::vector<Clr>>                       m_colors;
};


/** \brief A control specifically designed for ColorDlg that allows the user
    to select a point in the Value subspace of the HSV color space. */
class GG_API ValuePicker : public Control
{
public:
    /** emitted whenever the hue or saturation in the picker changes */
    typedef boost::signals2::signal<void (double)> ChangedSignalType;

    ValuePicker(X x, Y y, X w, Y h, Clr arrow_color);

    void Render() override;
    void LButtonDown(Pt pt, Flags<ModKey> mod_keys) override;
    void LDrag(Pt pt, Pt move, Flags<ModKey> mod_keys) override;

    /** Sets the current hue and saturation.  These are only used to render
        the control, and do not otherwise influence its operation. */
    void SetHueSaturation(double hue, double saturation);
    void SetValue(double value); ///< sets the current value.  Note that this does not cause a signal to be emitted.

    mutable ChangedSignalType ChangedSignal; ///< emitted whenever the hue or saturation in the picker changes

private:
    void SetValueFromPt(Pt pt);

    double  m_hue;
    double  m_saturation;
    double  m_value;

    Clr m_arrow_color;
};


/** \brief A dialog box used to get a color selection from the user.

    The user may select a certain number of custom colors, which will remain
    available for the duration of that run of the application in the
    ColorDlg's static space.  If desired, an optional previous color can be
    provided to the ColorDlg ctor, which will cause this previous color to be
    shown next to the new color for comparison purposes. */
class GG_API ColorDlg : public Wnd
{
public:
    /** \brief The button used to select the custom colors in ColorDlg. */
    class GG_API ColorButton : public Button
    {
    public:
        explicit ColorButton(Clr color);

        /** returns the custom color represented by the button */
        Clr RepresentedColor() const noexcept { return m_represented_color; }

        /** sets the custom color represented by the button */
        void SetRepresentedColor(Clr color) { m_represented_color = color; }

    protected:
        void RenderUnpressed() override;
        void RenderPressed() override;
        void RenderRollover() override;

    private:
        Clr m_represented_color;
    };

    /** \brief A simple control that only displays a rectangle filled with the
        given color.

        The color is shown in full alpha in the upper-left portion of the
        rectangle, and the color is shown in its given alpha in the lower-left
        of the rectangle. */
    class GG_API ColorDisplay : public Control
    {
    public:
        explicit ColorDisplay(Clr color);

        void Render() override;
    };

    ColorDlg(X x, Y y, Clr original_color, const std::shared_ptr<Font>& font,
             Clr dialog_color, Clr border_color, Clr text_color = CLR_BLACK);
    void CompleteConstruction() override;

    /** Returns true iff the user selected a color and then clicked the "Ok"
        button.  Otherwise, the color returned by Result() will be the
        original color if one was selected, or undefined if one was not. */
    bool ColorWasSelected() const;

    /** returns the color selected by the user, if the "Ok" button was used to close the dialog. */
    Clr Result() const;

    void Render() override;
    void KeyPress(Key key, uint32_t key_code_point, Flags<ModKey> mod_keys) override;

    static constexpr std::size_t INVALID_COLOR_BUTTON{std::numeric_limits<std::size_t>::max()};

private:
    void ColorChanged(HSVClr color);
    void HueSaturationPickerChanged(double hue, double saturation);
    void ValuePickerChanged(double value);
    void UpdateRGBSliders();
    void UpdateHSVSliders();
    void ColorChangeFromRGBSlider();
    void ColorButtonClicked(std::size_t i);
    void RedSliderChanged(int value, int low, int high);
    void GreenSliderChanged(int value, int low, int high);
    void BlueSliderChanged(int value, int low, int high);
    void AlphaSliderChanged(int value, int low, int high);
    void HueSliderChanged(int value, int low, int high);
    void SaturationSliderChanged(int value, int low, int high);
    void ValueSliderChanged(int value, int low, int high);
    void OkClicked();
    void CancelClicked();

    HSVClr  m_current_color;
    Clr     m_original_color;
    bool    m_original_color_specified = true;
    bool    m_color_was_picked = false;

    std::shared_ptr<HueSaturationPicker>      m_hue_saturation_picker;
    std::shared_ptr<ValuePicker>              m_value_picker;
    std::shared_ptr<Layout>                   m_pickers_layout;
    std::shared_ptr<ColorDisplay>             m_new_color_square;
    std::shared_ptr<ColorDisplay>             m_old_color_square;
    std::shared_ptr<TextControl>              m_new_color_square_text;
    std::shared_ptr<TextControl>              m_old_color_square_text;
    std::shared_ptr<Layout>                   m_color_squares_layout;
    std::vector<std::shared_ptr<ColorButton>> m_color_buttons;
    std::shared_ptr<Layout>                   m_color_buttons_layout;
    std::size_t                               m_current_color_button;
    std::vector<std::shared_ptr<TextControl>> m_slider_labels;
    std::vector<std::shared_ptr<TextControl>> m_slider_values;
    std::vector<std::shared_ptr<Slider<int>>> m_sliders;
    std::shared_ptr<Button>                   m_ok;
    std::shared_ptr<Button>                   m_cancel;
    std::shared_ptr<Layout>                   m_sliders_ok_cancel_layout;

    Clr m_color;
    Clr m_border_color;
    Clr m_text_color;
};

}


#endif
