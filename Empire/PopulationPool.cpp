#include "PopulationPool.h"

#include "../universe/Enums.h"
#include "../universe/Planet.h"
#include "../util/AppInterface.h"

void PopulationPool::SetPopCenters(std::vector<int> pop_center_ids)
{ m_pop_center_ids = std::move(pop_center_ids); }

void PopulationPool::Update(const ObjectMap& objects) {
    m_population = 0.0f;
    // sum population from all PopCenters in this pool
    for (const auto& planet : objects.find<Planet>(m_pop_center_ids)) {
        if (planet)
            m_population += planet->UniverseObject::GetMeter(MeterType::METER_POPULATION)->Current();
    }
    ChangedSignal();
}
