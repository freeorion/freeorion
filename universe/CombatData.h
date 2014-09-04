// -*- C++ -*-

#ifndef _CombatData_h_
#define _CombatData_h_

#include "../combat/PathingEngine.h"

class System;
struct CombatSetupGroup;

/** The state of combat (units, planets, their health, etc.) at the start of a
    round of combat. */
struct FO_COMMON_API CombatData {
    CombatData() :
        m_combat_turn_number(0)
    {}
    CombatData(TemporaryPtr<System> system, std::map<int, std::vector<CombatSetupGroup> >& setup_groups);

    unsigned int m_combat_turn_number;
    TemporaryPtr<System> m_system;
    std::map<int, TemporaryPtr<UniverseObject> > m_combat_universe;
    PathingEngine m_pathing_engine;

    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const;
    template<class Archive>
    void load(Archive & ar, const unsigned int version);
    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

#endif // _CombatData_h_
