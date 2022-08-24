#ifndef _CommonParams_h_
#define _CommonParams_h_


#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include "EnumsFwd.h"
#include "../util/Export.h"


namespace Condition {
    struct Condition;
}
namespace Effect {
    class EffectsGroup;
}
namespace ValueRef {
    template <typename T>
    struct ValueRef;
}

template <typename T>
using ConsumptionMap = std::map<T, std::pair<std::unique_ptr<ValueRef::ValueRef<double>>,
                                             std::unique_ptr<Condition::Condition>>>;

/** @content_tag{PEDIA_} Defines an encyclopedia category for
  * the generated article of the containing content definition.
  * The category name should be postfixed to this tag. **/
constexpr std::string_view TAG_PEDIA_PREFIX = "PEDIA_";

//! Common parameters for ShipPart, ShipHull, and BuildingType constructors.
//!
//! Used as temporary storage for parsing to reduce number of sub-items parsed
//! per item.
struct FO_COMMON_API CommonParams {
    CommonParams() = default;
    CommonParams(std::unique_ptr<ValueRef::ValueRef<double>>&& production_cost_,
                 std::unique_ptr<ValueRef::ValueRef<int>>&& production_time_,
                 bool producible_,
                 std::set<std::string>& tags_,
                 std::unique_ptr<Condition::Condition>&& location_,
                 std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects_,
                 ConsumptionMap<MeterType>&& production_meter_consumption_,
                 ConsumptionMap<std::string>&& production_special_consumption_,
                 std::unique_ptr<Condition::Condition>&& enqueue_location_);
    ~CommonParams(); // needed due to forward-declared Condition held in unique_ptr

    std::unique_ptr<ValueRef::ValueRef<double>>         production_cost;
    std::unique_ptr<ValueRef::ValueRef<int>>            production_time;
    bool                                                producible = true;
    std::vector<std::string>                            tags;
    ConsumptionMap<MeterType>                           production_meter_consumption;
    ConsumptionMap<std::string>                         production_special_consumption;
    std::unique_ptr<Condition::Condition>               location;
    std::unique_ptr<Condition::Condition>               enqueue_location;
    std::vector<std::unique_ptr<Effect::EffectsGroup>>  effects;
};


#endif
