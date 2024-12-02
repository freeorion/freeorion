#ifndef _Moderator_Action_h_
#define _Moderator_Action_h_

#include "../universe/ConstantsFwd.h"
#include "../universe/EnumsFwd.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "Export.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include <map>
#include <string>

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
    ModeratorAction() = default;
    virtual ~ModeratorAction() = default;

    virtual void Execute() const {}

    [[nodiscard]] virtual std::string Dump() const { return "ModeratorAction"; }

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class FO_COMMON_API DestroyUniverseObject final : public ModeratorAction {
public:
    DestroyUniverseObject() = default;
    explicit DestroyUniverseObject(int object_id) :
        m_object_id(object_id)
    {}

    void Execute() const override;
    [[nodiscard]] std::string Dump() const override;

private:
    int m_object_id = INVALID_OBJECT_ID;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class FO_COMMON_API SetOwner final : public ModeratorAction {
public:
    SetOwner() = default;
    SetOwner(int object_id, int new_owner_empire_id) :
        m_object_id(object_id),
        m_new_owner_empire_id(new_owner_empire_id)
    {}

    void Execute() const override;
    [[nodiscard]] std::string Dump() const override;

private:
    int m_object_id = INVALID_OBJECT_ID;
    int m_new_owner_empire_id = ALL_EMPIRES;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class FO_COMMON_API AddStarlane final : public ModeratorAction {
public:
    AddStarlane() = default;
    AddStarlane(int system_1_id, int system_2_id) :
        m_id_1(system_1_id),
        m_id_2(system_2_id)
    {}

    void Execute() const override;
    [[nodiscard]] std::string Dump() const override;

private:
    int m_id_1 = INVALID_OBJECT_ID;
    int m_id_2 = INVALID_OBJECT_ID;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class FO_COMMON_API RemoveStarlane final : public ModeratorAction {
public:
    RemoveStarlane() = default;
    RemoveStarlane(int system_1_id, int system_2_id) :
        m_id_1(system_1_id),
        m_id_2(system_2_id)
    {}

    void Execute() const override;
    [[nodiscard]] std::string Dump() const override;

private:
    int m_id_1 = INVALID_OBJECT_ID;
    int m_id_2 = INVALID_OBJECT_ID;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class FO_COMMON_API CreateSystem final : public ModeratorAction {
public:
    CreateSystem() = default;
    CreateSystem(double x, double y, StarType star_type) :
        m_x(x),
        m_y(y),
        m_star_type(star_type)
    {}

    void Execute() const override;
    [[nodiscard]] std::string Dump() const override;

private:
    double   m_x = UniverseObject::INVALID_POSITION;
    double   m_y = UniverseObject::INVALID_POSITION;
    StarType m_star_type = StarType::STAR_NONE;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class FO_COMMON_API CreatePlanet final : public ModeratorAction {
public:
    CreatePlanet() = default;
    CreatePlanet(int system_id, PlanetType planet_type, PlanetSize planet_size) :
        m_system_id(system_id),
        m_planet_type(planet_type),
        m_planet_size(planet_size)
    {}

    void Execute() const override;
    [[nodiscard]] std::string Dump() const override;

private:
    int         m_system_id = INVALID_OBJECT_ID;
    PlanetType  m_planet_type = PlanetType::PT_SWAMP;
    PlanetSize  m_planet_size = PlanetSize::SZ_MEDIUM;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

}


#endif
