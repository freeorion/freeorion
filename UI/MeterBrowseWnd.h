#ifndef _MeterBrowseWnd_h_
#define _MeterBrowseWnd_h_

#include <GG/GGFwd.h>
#include <GG/BrowseInfoWnd.h>

#include "../universe/EnumsFwd.h"
#include <tuple>

class UniverseObject;

/** Gives details about what effects contribute to a meter's maximum value
  * (Effect Accounting) and shows the current turn's current meter value and the
  * predicted current meter value for next turn. */
class MeterBrowseWnd : public GG::BrowseInfoWnd {
public:
    MeterBrowseWnd(int object_id, MeterType primary_meter_type,
                   MeterType secondary_meter_type);
    MeterBrowseWnd(int object_id, MeterType primary_meter_type);

    bool WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const override;
    void Render() override;

protected:
    void UpdateImpl(std::size_t mode, const Wnd* target) override;
    void Initialize();
    void UpdateSummary();
    void UpdateEffectLabelsAndValues(GG::Y& top);

    MeterType                   m_primary_meter_type;
    MeterType                   m_secondary_meter_type;
    int                         m_object_id;

    std::shared_ptr<GG::Label>  m_summary_title = nullptr;
    std::shared_ptr<GG::Label>  m_current_label = nullptr;
    std::shared_ptr<GG::Label>  m_current_value = nullptr;
    std::shared_ptr<GG::Label>  m_next_turn_label = nullptr;
    std::shared_ptr<GG::Label>  m_next_turn_value = nullptr;
    std::shared_ptr<GG::Label>  m_change_label = nullptr;
    std::shared_ptr<GG::Label>  m_change_value = nullptr;
    std::shared_ptr<GG::Label>  m_meter_title = nullptr;

    std::vector<std::pair<std::shared_ptr<GG::Label>, std::shared_ptr<GG::Label>>>
                                m_effect_labels_and_values;

    GG::Y                       m_row_height = GG::Y1;
    bool                        m_initialized = false;
};

/** Gives details about what effects contribute to a meter's maximum value
  * (Effect Accounting) and shows the current turn's current meter value and the
  * predicted current meter value for next turn. */
class ShipDamageBrowseWnd : public MeterBrowseWnd {
public:
    ShipDamageBrowseWnd(int object_id, MeterType primary_meter_type);

private:
    void UpdateImpl(std::size_t mode, const Wnd* target) override;
    void Initialize();
    void UpdateSummary();
    void UpdateEffectLabelsAndValues(GG::Y& top);

};

class ShipFightersBrowseWnd : public MeterBrowseWnd {
public:
    ShipFightersBrowseWnd(int object_id, MeterType primary_meter_type, bool show_all_bouts = false);

private:
    void UpdateImpl(std::size_t mode, const Wnd* target) override;
    void Initialize();
    void UpdateSummary();
    void UpdateEffectLabelsAndValues(GG::Y& top);

    std::shared_ptr<GG::ListBox>    m_bay_list;
    std::shared_ptr<GG::ListBox>    m_hangar_list;
    bool                            m_show_all_bouts;
};

#endif
