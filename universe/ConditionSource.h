#ifndef _ConditionSource_h_
#define _ConditionSource_h_


#include "Condition.h"
#include "ValueRefs.h"
#include "../util/CheckSums.h"


/** this namespace holds Condition and its subclasses; these classes
  * represent predicates about UniverseObjects used by, for instance, the
  * Effect system. */
namespace Condition {

/** Matches the source object only. */
struct FO_COMMON_API Source final : public Condition {
    constexpr Source() noexcept : Condition(true, true, false) {}

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<decltype(this)>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const Source&) const noexcept { return true; }

    [[nodiscard]] ObjectSet GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context) const override;
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string&) noexcept override {}
    [[nodiscard]] constexpr uint32_t GetCheckSum() const noexcept(noexcept(CheckSums::GetCheckSum(""))) override
    { return CheckSums::GetCheckSum("Condition::Source"); }

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;

private:
    [[nodiscard]] bool Match(const ScriptingContext& local_context) const noexcept override
    { return local_context.source && local_context.source == local_context.condition_local_candidate; }
};

}


#endif
