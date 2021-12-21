#include "PopulationPool.h"

#include "../universe/Enums.h"
#include "../universe/PopCenter.h"
#include "../util/AppInterface.h"

float PopulationPool::Population() const
{ return m_population; }

void PopulationPool::SetPopCenters(std::vector<int> pop_center_ids)
{ m_pop_center_ids = std::move(pop_center_ids); }

void PopulationPool::Update(const ObjectMap& objects) {
    m_population = 0.0f;
    // sum population from all PopCenters in this pool
    for (const auto& center : objects.find<PopCenter>(m_pop_center_ids)) {
        if (!center)
            continue;
        m_population += center->GetMeter(MeterType::METER_POPULATION)->Current();
    }
    ChangedSignal();
}
