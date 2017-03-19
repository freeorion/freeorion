#ifndef _MultiMeterStatusBar_h_
#define _MultiMeterStatusBar_h_

#include <GG/GGFwd.h>
#include <GG/Wnd.h>

#include "../universe/EnumsFwd.h"


/** Graphically represets current value and projected changes to single meters
  * or pairs of meters, using horizontal indicators. */
class MultiMeterStatusBar : public GG::Wnd {
public:
    MultiMeterStatusBar(GG::X w, int object_id, const std::vector<std::pair<MeterType, MeterType>>& meter_types);

    void Render() override;

    void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys) override;

    void            Update();

private:
    std::shared_ptr<GG::Texture> m_bar_shading_texture;

    std::vector<std::pair<MeterType, MeterType>>    m_meter_types;      // list of <MeterType, MeterType> pairs, where .first is the "actual" meter value, and .second is the the "target" or "max" meter value.  Either may be INVALID_METER_TYPE, in which case nothing will be shown for that MeterType
    std::vector<float>                              m_initial_values;   // initial value of .first MeterTypes at the start of this turn
    std::vector<float>                              m_projected_values; // projected current value of .first MeterTypes for the start of next turn
    std::vector<float>                              m_target_max_values;// current values of the .second MeterTypes in m_meter_types

    int                                             m_object_id;

    std::vector<GG::Clr>                            m_bar_colours;
};

#endif
