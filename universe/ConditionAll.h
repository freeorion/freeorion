#ifndef _ConditionAll_h_
#define _ConditionAll_h_


#include "Condition.h"


/** this namespace holds Condition and its subclasses; these classes
  * represent predicates about UniverseObjects used by, for instance, the
  * Effect system. */
namespace Condition {

/** Matches all objects. */
struct FO_COMMON_API All final : public Condition {
    constexpr All() noexcept : Condition(true, true, true) {};

    [[nodiscard]] constexpr bool operator==(const Condition& rhs) const noexcept override
    { return this == &rhs || dynamic_cast<const All*>(&rhs); }
    [[nodiscard]] constexpr bool operator==(const All&) const noexcept { return true; }

    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    [[nodiscard]] bool EvalAny(const ScriptingContext&, const ObjectSet& candidates) const noexcept override { return !candidates.empty(); }
    [[nodiscard]] std::string Description(bool negated = false) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override {}
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Condition> Clone() const override;
};

}


#endif
