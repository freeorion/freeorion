#ifndef _ConditionSource_h_
#define _ConditionSource_h_


#include "Condition.h"


/** this namespace holds Condition and its subclasses; these classes
  * represent predicates about UniverseObjects used by, for instance, the
  * Effect system. */
namespace Condition {

/** Matches the source object only. */
struct FO_COMMON_API Source final : public Condition {
    Source();

    bool operator==(const Condition& rhs) const override;
    void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                           ObjectSet& condition_non_targets) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override
    {}
    unsigned int GetCheckSum() const override;

private:
    bool Match(const ScriptingContext& local_context) const override;
};

} // namespace Condition

#endif // _ConditionSource_h_
