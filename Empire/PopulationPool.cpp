#include "PopulationPool.h"

#include "universe/Enums.h"
#include "universe/PopCenter.h"
#include "util/AppInterface.h"

PopulationPool::PopulationPool()
{}

float PopulationPool::Population() const
{ return m_population; }

void PopulationPool::SetPopCenters(const std::vector<int>& pop_center_ids) {
    if (m_pop_center_ids == pop_center_ids)
        return;
    m_pop_center_ids = pop_center_ids;
}

void PopulationPool::Update() {
    m_population = 0.0f;
    // sum population from all PopCenters in this pool
    for (int pop_center_id : m_pop_center_ids) {
        if (auto center = GetPopCenter(pop_center_id)) {
            m_population += center->CurrentMeterValue(METER_POPULATION);
        }
    }
    ChangedSignal();
}
