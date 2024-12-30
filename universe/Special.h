#ifndef _Special_h_
#define _Special_h_


#include <map>
#include <memory>
#include <string>
#include <vector>
#include <boost/optional/optional.hpp>
#include "../util/Export.h"
#include "../util/Pending.h"


namespace Effect {
    class EffectsGroup;
}
namespace Condition {
    struct Condition;
}
namespace ValueRef {
    template <typename T>
    struct ValueRef;
}
struct ScriptingContext;

/** A predefined set of EffectsGroups that can be attached to a UniverseObject
  * (often referred to as the "source" object).  The effects of a Special are
  * not limited to the object to which it is attached.  Each kind of Special
  * must have a \a unique name string, by which it can be looked up using
  * GetSpecial(). */
class FO_COMMON_API Special {
public:
    Special(std::string&& name, std::string&& description,
            std::unique_ptr<ValueRef::ValueRef<double>>&& stealth,
            std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
            double spawn_rate = 1.0, int spawn_limit = 99999,
            std::unique_ptr<ValueRef::ValueRef<double>>&& initial_capaicty = nullptr,
            std::unique_ptr<Condition::Condition>&& location = nullptr,
            const std::string& graphic = "");
    Special(Special&&) = default;

    ~Special();

    [[nodiscard]] bool operator==(const Special& rhs) const;

    [[nodiscard]] auto&       Name() const noexcept            { return m_name; }           ///< unique name for this type of special
    [[nodiscard]] std::string Description() const;           ///< text description of this type of special
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const; ///< data file format representation of this object
    [[nodiscard]] auto*       Stealth() const noexcept         { return m_stealth.get(); } ///< stealth of the special, which determines how easily it is seen by empires
    [[nodiscard]] auto&       Effects() const noexcept         { return m_effects; }       ///< EffectsGroups that encapsulate the effects that specials of this type have.
    [[nodiscard]] float       SpawnRate() const noexcept       { return m_spawn_rate; }
    [[nodiscard]] int         SpawnLimit() const noexcept      { return m_spawn_limit; }
    [[nodiscard]] auto*       InitialCapacity() const noexcept { return m_initial_capacity.get(); } ///< ValueRef to use to set the initial capacity of the special when placed
    [[nodiscard]] float       InitialCapacity(int object_id, const ScriptingContext& context) const; ///< evaluates initial capacity ValueRef using the object with specified \a object_id as the object on which the special will be placed
    [[nodiscard]] const auto* Location() const noexcept        { return m_location.get(); }///< condition that determines whether an UniverseObject can have this special applied during universe creation
    [[nodiscard]] auto&       Graphic() const noexcept         { return m_graphic; };      ///< name of the grapic file for this special

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    [[nodiscard]] uint32_t GetCheckSum() const;

private:
    void Init();

    std::string                                 m_name;
    std::string                                 m_description;
    std::unique_ptr<ValueRef::ValueRef<double>> m_stealth;
    std::vector<Effect::EffectsGroup>           m_effects;
    float                                       m_spawn_rate = 0.0f;
    int                                         m_spawn_limit = 99999;
    std::unique_ptr<ValueRef::ValueRef<double>> m_initial_capacity;
    std::unique_ptr<Condition::Condition>       m_location;
    std::string                                 m_graphic;
};

/** Returns the Special object used to represent specials of type \a name.
  * If no such Special exists, 0 is returned instead. */
[[nodiscard]] FO_COMMON_API const Special* GetSpecial(std::string_view name);

/** Returns names of all specials. */
[[nodiscard]] FO_COMMON_API std::vector<std::string_view> SpecialNames();


/** Look up table for specials.*/
class FO_COMMON_API SpecialsManager {
public:
    using SpecialsTypeMap = std::map<std::string, std::unique_ptr<Special>, std::less<>>;

    [[nodiscard]] auto                          NumSpecials() const noexcept { return m_specials.size(); }
    [[nodiscard]] std::vector<std::string_view> SpecialNames() const;
    [[nodiscard]] const Special*                GetSpecial(std::string_view name) const;
    [[nodiscard]] uint32_t                      GetCheckSum() const;

    /** Sets types to the value of \p future. */
    void SetSpecialsTypes(Pending::Pending<SpecialsTypeMap>&& future);

private:
    /** Assigns any m_pending_types to m_specials. */
    void CheckPendingSpecialsTypes() const;

    /** Future types being parsed by parser.  mutable so that it can
        be assigned to m_species_types when completed.*/
    mutable boost::optional<Pending::Pending<SpecialsTypeMap>> m_pending_types = boost::none;

    mutable std::string                   m_concatenated_special_names;
    mutable std::vector<std::string_view> m_special_names;
    mutable std::vector<Special>          m_specials;
};

FO_COMMON_API SpecialsManager& GetSpecialsManager();


#endif
