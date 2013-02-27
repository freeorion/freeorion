// -*- C++ -*-
#ifndef _Moderator_Action_h_
#define _Moderator_Action_h_

#include "../universe/Enums.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

namespace Moderator {

/////////////////////////////////////////////////////
// ModeratorAction
/////////////////////////////////////////////////////
/** The abstract base class for serializable moderator actions.
  * Moderators make a change in the client, and the update is sent to the server
  * which actually implements the action, and sends back the resulting gamestate
  * (if applicable) */
class ModeratorAction {
public:
    ModeratorAction() {}
    virtual ~ModeratorAction() {}
    virtual void        Execute() const {}
    virtual std::string Dump() const { return "ModeratorAction"; }
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class DestroyUniverseObject : public ModeratorAction {
public:
    explicit DestroyUniverseObject(int object_id) :
        m_object_id(object_id)
    {}
    virtual ~DestroyUniverseObject() {}
    virtual void        Execute() const;
    virtual std::string Dump() const;
private:
    int m_object_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

}

// template implementations
template <class Archive>
void Moderator::ModeratorAction::serialize(Archive& ar, const unsigned int version)
{}

template <class Archive>
void Moderator::DestroyUniverseObject::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ModeratorAction)
        & BOOST_SERIALIZATION_NVP(m_object_id);
}

#endif // _Moderator_Action_h_
