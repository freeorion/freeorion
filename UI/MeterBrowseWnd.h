#ifndef _MeterBrowseWnd_h_
#define _MeterBrowseWnd_h_

#include <GG/GGFwd.h>
#include <GG/BrowseInfoWnd.h>

#include "../universe/Enums.h"
#include <boost/tuple/tuple.hpp>

class UniverseObject;

namespace DualMeter {

    /** Return the triplet of {Current, Projected, Target} meter value for the pair of meters \p
        actual_meter_type and \p target_meter_type associated with \p obj. */
    boost::tuple<float, float, float> CurrentProjectedTarget(
        const UniverseObject& obj, const MeterType& actual_meter_type, const MeterType& target_meter_type);
}

/** Gives details about what effects contribute to a meter's maximum value (Effect Accounting) and
  * shows the current turn's current meter value and the predicted current meter value for next turn. */
class MeterBrowseWnd : public GG::BrowseInfoWnd {
public:
    MeterBrowseWnd(int object_id, MeterType primary_meter_type, MeterType secondary_meter_type = INVALID_METER_TYPE);

    virtual bool    WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const;
    virtual void    Render();

protected:
    void            Initialize();

    virtual void    UpdateImpl(std::size_t mode, const Wnd* target);
    void            UpdateSummary();
    void            UpdateEffectLabelsAndValues(GG::Y& top);

    MeterType               m_primary_meter_type;
    MeterType               m_secondary_meter_type;
    int                     m_object_id;

    GG::Label*              m_summary_title;

    GG::Label*              m_current_label;
    GG::Label*              m_current_value;
    GG::Label*              m_next_turn_label;
    GG::Label*              m_next_turn_value;
    GG::Label*              m_change_label;
    GG::Label*              m_change_value;

    GG::Label*              m_meter_title;

    std::vector<std::pair<GG::Label*, GG::Label*> >
                            m_effect_labels_and_values;

    GG::Y                   m_row_height;
    bool                    m_initialized;
};

/** Gives details about what effects contribute to a meter's maximum value (Effect Accounting) and
  * shows the current turn's current meter value and the predicted current meter value for next turn. */
class ShipDamageBrowseWnd : public MeterBrowseWnd {
public:
    ShipDamageBrowseWnd(int object_id, MeterType primary_meter_type);

private:
    void            Initialize();

    virtual void    UpdateImpl(std::size_t mode, const Wnd* target);
    void            UpdateSummary();
    void            UpdateEffectLabelsAndValues(GG::Y& top);

};

class ShipFightersBrowseWnd : public MeterBrowseWnd {
public:
    ShipFightersBrowseWnd(int object_id, MeterType primary_meter_type, bool show_all_bouts = false);
    static const int NUM_COMBAT_BOUTS = 3;

private:
    void            Initialize();

    virtual void    UpdateImpl(std::size_t mode, const Wnd* target);
    void            UpdateSummary();
    void            UpdateEffectLabelsAndValues(GG::Y& top);

    GG::ListBox*    m_bay_list;
    GG::ListBox*    m_hangar_list;
    bool            m_show_all_bouts;

};

#endif
