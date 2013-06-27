// -*- C++ -*-
#ifndef _Moderator_Action_h_
#define _Moderator_Action_h_

#include "../universe/Enums.h"
#include "Export.h"

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
    ModeratorAction();
    virtual ~ModeratorAction() {}
    virtual void        Execute() const {}
    virtual std::string Dump() const { return "ModeratorAction"; }
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class FO_COMMON_API DestroyUniverseObject : public ModeratorAction {
public:
    DestroyUniverseObject();
    explicit DestroyUniverseObject(int object_id);
    virtual ~DestroyUniverseObject() {}
    virtual void        Execute() const;
    virtual std::string Dump() const;
private:
    int m_object_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class FO_COMMON_API SetOwner : public ModeratorAction {
public:
    SetOwner();
    SetOwner(int object_id, int new_owner_empire_id);
    virtual ~SetOwner() {}
    virtual void        Execute() const;
    virtual std::string Dump() const;
private:
    int m_object_id;
    int m_new_owner_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class FO_COMMON_API AddStarlane : public ModeratorAction {
public:
    AddStarlane();
    AddStarlane(int system_1_id, int system_2_id);
    virtual ~AddStarlane() {}
    virtual void        Execute() const;
    virtual std::string Dump() const;
private:
    double      m_id_1;
    double      m_id_2;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class FO_COMMON_API RemoveStarlane : public ModeratorAction {
public:
    RemoveStarlane();
    RemoveStarlane(int system_1_id, int system_2_id);
    virtual ~RemoveStarlane() {}
    virtual void        Execute() const;
    virtual std::string Dump() const;
private:
    double      m_id_1;
    double      m_id_2;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class FO_COMMON_API CreateSystem : public ModeratorAction {
public:
    CreateSystem();
    CreateSystem(double x, double y, StarType star_type);
    virtual ~CreateSystem() {}
    virtual void        Execute() const;
    virtual std::string Dump() const;
private:
    double      m_x;
    double      m_y;
    StarType    m_star_type;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class FO_COMMON_API CreatePlanet : public ModeratorAction {
public:
    CreatePlanet();
    CreatePlanet(int system_id, PlanetType planet_type, PlanetSize planet_size);
    virtual ~CreatePlanet() {}
    virtual void        Execute() const;
    virtual std::string Dump() const;
private:
    int         m_system_id;
    PlanetType  m_planet_type;
    PlanetSize  m_planet_size;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

}

#endif // _Moderator_Action_h_
