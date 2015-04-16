// -*- C++ -*-
#ifndef _MultiIconValueIndicator_h_
#define _MultiIconValueIndicator_h_

#include <GG/Wnd.h>

#include "../universe/Enums.h"

class StatisticIcon;

/** Display icon and number for various meter-related quantities associated
  * with objects.  Typical use would be to display the resource production
  * values or population for a planet.  Given a set of MeterType, the indicator
  * will present the appropriate values for each. */
class MultiIconValueIndicator : public GG::Wnd {
public:
    MultiIconValueIndicator(GG::X w, int object_id, const std::vector<std::pair<MeterType, MeterType> >& meter_types);
    MultiIconValueIndicator(GG::X w, const std::vector<int>& object_ids, const std::vector<std::pair<MeterType, MeterType> >& meter_types);
    MultiIconValueIndicator(GG::X w); ///< initializes with no icons shown
    virtual ~MultiIconValueIndicator();

    bool            Empty();

    virtual void    Render();
    virtual void    MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys);

    void            Update();

    void            SetToolTip(MeterType meter_type, const boost::shared_ptr<GG::BrowseInfoWnd>& browse_wnd);
    void            ClearToolTip(MeterType meter_type);
    bool            EventFilter(GG::Wnd* w, const GG::WndEvent& event);

private:
    void            Init();

    std::vector<StatisticIcon*>                         m_icons;
    const std::vector<std::pair<MeterType, MeterType> > m_meter_types;
    std::vector<int>                                    m_object_ids;
};

#endif
