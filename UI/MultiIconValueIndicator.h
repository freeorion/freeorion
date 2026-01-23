#ifndef _MultiIconValueIndicator_h_
#define _MultiIconValueIndicator_h_

#include <GG/Wnd.h>

#include "../universe/EnumsFwd.h"

class StatisticIcon;
class ObjectMap;

/** Display icon and number for various meter-related quantities associated
  * with objects.  Typical use would be to display the resource production
  * values or population for a planet.  Given a set of MeterType, the indicator
  * will present the appropriate values for each. */
class MultiIconValueIndicator : public GG::Wnd {
public:
    /** Initializes with no icons shown. */
    MultiIconValueIndicator(GG::X w);
    MultiIconValueIndicator(GG::X w, int object_id,
                            std::vector<std::pair<MeterType, MeterType>> meter_types);
    MultiIconValueIndicator(GG::X w, std::vector<int> object_ids,
                            std::vector<std::pair<MeterType, MeterType>> meter_types);

    void CompleteConstruction() override;

    [[nodiscard]] bool Empty() const noexcept { return m_object_ids.empty(); }

    void Render() override;
    void MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys) override;
    void Update(const ObjectMap& objects);
    void SetToolTip(MeterType meter_type, const std::shared_ptr<GG::BrowseInfoWnd>& browse_wnd);
    void ClearToolTip(MeterType meter_type);

private:
    std::vector<std::shared_ptr<StatisticIcon>>         m_icons;
    const std::vector<std::pair<MeterType, MeterType>>  m_meter_types;
    std::vector<int>                                    m_object_ids;
};

#endif
