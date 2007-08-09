// -*- C++ -*-
#ifndef _FocusSelector_h_
#define _FocusSelector_h_

#include "../universe/Enums.h"

#include <GG/Button.h>

class ResourceCenter;

class FocusButton : public GG::Button
{
public:
    FocusButton(GG::Clr color, const boost::shared_ptr<GG::Texture>& image);

    virtual void Render();
    virtual void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void MouseLeave();
    void SetImage(const boost::shared_ptr<GG::Texture>& image);

    mutable ClickedSignalType RightClickedSignal;

private:
    boost::shared_ptr<GG::Texture> m_texture;
    bool m_mouse_here;
};

class MeterStatusBar : public GG::Wnd
{
public:
    MeterStatusBar(int w, int h, double initial_max, double initial_current, double max, double current);

    virtual void Render();

    void SetProjectedCurrent(double current);
    void SetProjectedMax(double max);

private:
    double m_initial_max;
    double m_initial_current;
    double m_projected_max;
    double m_projected_current;
};

class FocusSelector : public GG::Control
{
public:
    typedef boost::signal<void (FocusType)>   FocusChangedSignalType; ///< emitted when either the primary or secondary focus changes

    FocusSelector(int w, const ResourceCenter& resource_center);

    virtual void Render();
    virtual void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void MouseLeave();
    void Update();

    mutable FocusChangedSignalType PrimaryFocusChangedSignal;
    mutable FocusChangedSignalType SecondaryFocusChangedSignal;

private:
    struct FocusClickFunctor
    {
        FocusClickFunctor(int id, bool primary_, FocusSelector* wnd_);
        void operator()();
        const int button_id;
        const bool primary;
        FocusSelector* wnd;
    };

    FocusType ButtonFocus(int button) const;
    bool MonoImage(int button) const;
    void AdjustButtonImages();
    void AdjustMeterStatusBars(const ResourceCenter& resource_center);
    void PrimaryFocusClicked(int button);
    void SecondaryFocusClicked(int button);

    const ResourceCenter& m_res_center;

    FocusType m_primary_focus;
    FocusType m_secondary_focus;
    FocusType m_first_button_focus;
    std::map<FocusType, MeterStatusBar*> m_meter_status_bars;
    std::vector<FocusButton*> m_focus_buttons;
    std::vector<GG::TextControl*> m_meter_deltas;
    bool m_mouse_here;
};

#endif
