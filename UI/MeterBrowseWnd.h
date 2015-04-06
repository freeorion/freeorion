// -*- C++ -*-
#ifndef _MeterBrowseWnd_h_
#define _MeterBrowseWnd_h_

#include <GG/BrowseInfoWnd.h>

#include "../universe/Enums.h"

class CUILabel;

/** Gives details about what effects contribute to a meter's maximum value (Effect Accounting) and
  * shows the current turn's current meter value and the predicted current meter value for next turn. */
class MeterBrowseWnd : public GG::BrowseInfoWnd {
public:
    MeterBrowseWnd(int object_id, MeterType primary_meter_type, MeterType secondary_meter_type = INVALID_METER_TYPE);

    virtual bool    WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const;
    virtual void    Render();

private:
    void            Initialize();

    virtual void    UpdateImpl(std::size_t mode, const Wnd* target);
    void            UpdateSummary();
    void            UpdateEffectLabelsAndValues(GG::Y& top);

    MeterType               m_primary_meter_type;
    MeterType               m_secondary_meter_type;
    int                     m_object_id;

    CUILabel*               m_summary_title;

    CUILabel*               m_current_label;
    CUILabel*               m_current_value;
    CUILabel*               m_next_turn_label;
    CUILabel*               m_next_turn_value;
    CUILabel*               m_change_label;
    CUILabel*               m_change_value;

    CUILabel*               m_meter_title;

    std::vector<std::pair<CUILabel*, CUILabel*> >
                            m_effect_labels_and_values;

    GG::Y                   m_row_height;
    bool                    m_initialized;
};

#endif
