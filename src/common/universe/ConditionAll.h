#ifndef _ConditionAll_h_
#define _ConditionAll_h_


#include "Condition.h"


/** this namespace holds Condition and its subclasses; these classes
  * represent predicates about UniverseObjects used by, for instance, the
  * Effect system. */
namespace Condition {

/** Matches all objects. */
struct FO_COMMON_API All final : public Condition {
    All();

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = SearchDomain::NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override
    {}
    unsigned int GetCheckSum() const override;
};

}


#endif
