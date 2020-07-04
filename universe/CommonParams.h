#ifndef _CommonParams_h_
#define _CommonParams_h_


#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include "EnumsFwd.h"
#include "../focs/focs.hpp"
#include "../util/Export.h"


template <typename T>
using ConsumptionMap = std::map<T, std::pair<std::unique_ptr<focs::ValueRef<double>>,
                                             std::unique_ptr<focs::Condition>>>;

//! Common parameters for ShipPart, ShipHull, and BuildingType constructors.
//!
//! Used as temporary storage for parsing to reduce number of sub-items parsed
//! per item.
struct FO_COMMON_API CommonParams {
    CommonParams();
    CommonParams(std::unique_ptr<focs::ValueRef<double>>&& production_cost_,
                 std::unique_ptr<focs::ValueRef<int>>&& production_time_,
                 bool producible_,
                 const std::set<std::string>& tags_,
                 std::unique_ptr<focs::Condition>&& location_,
                 std::vector<std::unique_ptr<focs::EffectsGroup>>&& effects_,
                 ConsumptionMap<MeterType>&& production_meter_consumption_,
                 ConsumptionMap<std::string>&& production_special_consumption_,
                 std::unique_ptr<focs::Condition>&& enqueue_location_);
    ~CommonParams();

    std::unique_ptr<focs::ValueRef<double>>         production_cost;
    std::unique_ptr<focs::ValueRef<int>>            production_time;
    bool                                                producible = true;
    std::set<std::string>                               tags;
    ConsumptionMap<MeterType>                           production_meter_consumption;
    ConsumptionMap<std::string>                         production_special_consumption;
    std::unique_ptr<focs::Condition>               location;
    std::unique_ptr<focs::Condition>               enqueue_location;
    std::vector<std::unique_ptr<focs::EffectsGroup>>  effects;
};


#endif
