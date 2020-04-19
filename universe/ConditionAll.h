#ifndef _ConditionAll_h_
#define _ConditionAll_h_

#include "Condition.h"

/** this namespace holds Condition and its subclasses; these classes
  * represent predicates about UniverseObjects used by, for instance, the
  * Effect system. */
namespace Condition {

/** Matches all objects. */
struct FO_COMMON_API All final : public Condition {
    All() : Condition() {}

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    bool RootCandidateInvariant() const override
    { return true; }
    bool TargetInvariant() const override
    { return true; }
    bool SourceInvariant() const override
    { return true; }
    void SetTopLevelContent(const std::string& content_name) override
    {}
    unsigned int GetCheckSum() const override;

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

} // namespace Condition

#endif // _ConditionAll_h_
