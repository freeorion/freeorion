#ifndef _focs_ConditionAll_h_
#define _focs_ConditionAll_h_


#include "Condition.h"


/** Matches all objects. */
struct FO_COMMON_API focs::All final : public Condition {
    All();

    bool operator==(const Condition& rhs) const override;
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches,
              ObjectSet& non_matches, SearchDomain search_domain = NON_MATCHES) const override;
    std::string Description(bool negated = false) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override
    {}
    unsigned int GetCheckSum() const override;
};


#endif
