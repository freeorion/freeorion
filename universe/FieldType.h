#ifndef _FieldType_h_
#define _FieldType_h_

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include "../util/Export.h"
#include "../util/Pending.h"


namespace Effect {
    class EffectsGroup;
}


//! A specification for a type of Field.
class FO_COMMON_API FieldType {
public:
    FieldType(std::string&& name, std::string&& description,
              float stealth, const std::set<std::string>& tags,
              std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
              std::string&& graphic);

    [[nodiscard]] bool operator==(const FieldType& rhs) const;

    //! Returns the unique name for this type of field
    const auto& Name() const noexcept { return m_name; }

    //! Returns a text description of this type of building
    const auto& Description() const noexcept { return m_description; }

    //! Returns a data file format representation of this object
    auto Dump(uint8_t ntabs = 0) const -> std::string;

    //! Returns stealth of field type
    auto Stealth() const noexcept { return m_stealth; }

    const auto& Tags() const noexcept { return m_tags; }

    auto HasTag(std::string_view tag) const
    { return std::any_of(m_tags.begin(), m_tags.end(), [tag](const auto& t) { return t == tag; }); }

    const auto& Effects() const noexcept { return m_effects; }

    const auto& Graphic() const noexcept { return m_graphic; }

    //! Returns a number, calculated from the contained data, which should be
    //! different for different contained data, and must be the same for
    //! the same contained data, and must be the same on different platforms
    //! and executions of the program and the function. Useful to verify that
    //! the parsed content is consistent without sending it all between
    //! clients and server.
    auto GetCheckSum() const -> uint32_t;

private:
    std::string                         m_name;
    std::string                         m_description;
    float                               m_stealth;
    const std::string                   m_tags_concatenated;
    const std::vector<std::string_view> m_tags;
    std::vector<Effect::EffectsGroup>   m_effects;
    std::string                         m_graphic;
};


class FieldTypeManager {
public:
    using container_type = std::map<std::string, std::unique_ptr<FieldType>, std::less<>>;
    using iterator = container_type::const_iterator;

    //! Returns the field type with the name \a name; you should use the free
    //! free function GetFieldType(...) instead, mainly to save some typing.
    [[nodiscard]] auto GetFieldType(std::string_view name) const -> const FieldType*;

    //! iterator to the first field type
    [[nodiscard]] FO_COMMON_API auto begin() const -> iterator;

    //! iterator to the last + 1th field type
    [[nodiscard]] FO_COMMON_API auto end() const -> iterator;

    //! How many types are known?
    [[nodiscard]] FO_COMMON_API auto size() const -> std::size_t;

    //! Returns the instance of this singleton class; you should use the free
    //! function GetFieldTypeManager() instead
    [[nodiscard]] static auto GetFieldTypeManager() -> FieldTypeManager&;

    //! Returns a number, calculated from the contained data, which should be
    //! different for different contained data, and must be the same for
    //! the same contained data, and must be the same on different platforms
    //! and executions of the program and the function. Useful to verify that
    //! the parsed content is consistent without sending it all between
    //! clients and server.
    [[nodiscard]] auto GetCheckSum() const -> uint32_t;

    //! Sets types to the value of @p future.
    FO_COMMON_API void SetFieldTypes(Pending::Pending<container_type>&& future);

private:
    FieldTypeManager();

    //! Assigns any m_pending_types to m_field_types.
    void CheckPendingFieldTypes() const;

    //! Future types being parsed by parser.
    //! mutable so that it can be assigned to m_field_types when completed.
    mutable boost::optional<Pending::Pending<container_type>> m_pending_types = boost::none;

    mutable container_type m_field_types;

    static FieldTypeManager* s_instance;
};


//! Returns the singleton field type manager
[[nodiscard]] FO_COMMON_API auto GetFieldTypeManager() -> FieldTypeManager&;

//! Returns the BuildingType specification object for a field of
//! type @p name.  If no such FieldType exists, nullptr is returned instead.
[[nodiscard]] FO_COMMON_API auto GetFieldType(std::string_view name) -> const FieldType*;


#endif
