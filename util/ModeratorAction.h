#ifndef _Moderator_Action_h_
#define _Moderator_Action_h_

#include "../universe/EnumsFwd.h"
#include "Export.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include <map>
#include <string>

FO_COMMON_API extern const int ALL_EMPIRES;
FO_COMMON_API extern const int INVALID_OBJECT_ID;

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
    explicit ModeratorAction();

    virtual ~ModeratorAction()
    {}

    virtual void Execute() const
    {}

    virtual std::string Dump() const
    { return "ModeratorAction"; }

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class FO_COMMON_API DestroyUniverseObject : public ModeratorAction {
public:
    explicit DestroyUniverseObject();
    explicit DestroyUniverseObject(int object_id);

    void Execute() const override;
    std::string Dump() const override;

private:
    int m_object_id = INVALID_OBJECT_ID;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class FO_COMMON_API SetOwner : public ModeratorAction {
public:
    explicit SetOwner();
    SetOwner(int object_id, int new_owner_empire_id);

    void Execute() const override;
    std::string Dump() const override;

private:
    int m_object_id = INVALID_OBJECT_ID;
    int m_new_owner_empire_id = ALL_EMPIRES;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class FO_COMMON_API AddStarlane : public ModeratorAction {
public:
    explicit AddStarlane();
    AddStarlane(int system_1_id, int system_2_id);

    void Execute() const override;
    std::string Dump() const override;

private:
    int m_id_1 = INVALID_OBJECT_ID;
    int m_id_2 = INVALID_OBJECT_ID;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class FO_COMMON_API RemoveStarlane : public ModeratorAction {
public:
    explicit RemoveStarlane();
    RemoveStarlane(int system_1_id, int system_2_id);

    void Execute() const override;
    std::string Dump() const override;

private:
    int m_id_1 = INVALID_OBJECT_ID;
    int m_id_2 = INVALID_OBJECT_ID;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class FO_COMMON_API CreateSystem : public ModeratorAction {
public:
    explicit CreateSystem();
    CreateSystem(double x, double y, StarType star_type);

    void Execute() const override;
    std::string Dump() const override;

private:
    double      m_x = 0.0;
    double      m_y = 0.0;
    StarType    m_star_type;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class FO_COMMON_API CreatePlanet : public ModeratorAction {
public:
    explicit CreatePlanet();
    CreatePlanet(int system_id, PlanetType planet_type, PlanetSize planet_size);

    void Execute() const override;
    std::string Dump() const override;

private:
    int         m_system_id = INVALID_OBJECT_ID;
    PlanetType  m_planet_type;
    PlanetSize  m_planet_size;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

}

#endif // _Moderator_Action_h_
