#include "FleetPlan.h"

#include "../util/i18n.h"


const std::string& FleetPlan::Name() const {
    if (m_name_in_stringtable)
        return UserString(m_name);
    else
        return m_name;
}
