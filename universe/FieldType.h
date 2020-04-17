#ifndef _FieldType_h_
#define _FieldType_h_

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
    FieldType(const std::string& name, const std::string& description,
              float stealth, const std::set<std::string>& tags,
              std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
              const std::string& graphic);

    //! Returns the unique name for this type of field
    auto Name() const -> const std::string&
    { return m_name; }

    //! Returns a text description of this type of building
    auto Description() const -> const std::string&
    { return m_description; }

    //! Returns a data file format representation of this object
    auto Dump(unsigned short ntabs = 0) const -> std::string;

    //! Returns stealth of field type
    auto Stealth() const -> float
    { return m_stealth; }

    auto Tags() const -> const std::set<std::string>&
    { return m_tags; }

    //! Returns the EffectsGroups that encapsulate the effects of this
    //! FieldType.
    auto Effects() const -> const std::vector<std::shared_ptr<Effect::EffectsGroup>>&
    { return m_effects; }

    //! Returns the name of the grapic file for this field type
    auto Graphic() const -> const std::string&
    { return m_graphic; }

    //! Returns a number, calculated from the contained data, which should be
    //! different for different contained data, and must be the same for
    //! the same contained data, and must be the same on different platforms
    //! and executions of the program and the function. Useful to verify that
    //! the parsed content is consistent without sending it all between
    //! clients and server.
    auto GetCheckSum() const -> unsigned int;

private:
    std::string                                         m_name;
    std::string                                         m_description;
    float                                               m_stealth;
    std::set<std::string>                               m_tags;
    std::vector<std::shared_ptr<Effect::EffectsGroup>>  m_effects;
    std::string                                         m_graphic;
};


class FieldTypeManager {
public:
    using FieldTypeMap = std::map<std::string, std::unique_ptr<FieldType>>;
    using iterator = FieldTypeMap::const_iterator;

    //! Returns the field type with the name \a name; you should use the free
    //! free function GetFieldType(...) instead, mainly to save some typing.
    auto GetFieldType(const std::string& name) const -> const FieldType*;

    //! iterator to the first field type
    FO_COMMON_API auto begin() const -> iterator;

    //! iterator to the last + 1th field type
    FO_COMMON_API auto end() const -> iterator;

    //! Returns the instance of this singleton class; you should use the free
    //! function GetFieldTypeManager() instead
    static auto GetFieldTypeManager() -> FieldTypeManager&;

    //! Returns a number, calculated from the contained data, which should be
    //! different for different contained data, and must be the same for
    //! the same contained data, and must be the same on different platforms
    //! and executions of the program and the function. Useful to verify that
    //! the parsed content is consistent without sending it all between
    //! clients and server.
    auto GetCheckSum() const -> unsigned int;

    //! Sets types to the value of @p future.
    FO_COMMON_API void SetFieldTypes(Pending::Pending<FieldTypeMap>&& future);

private:
    FieldTypeManager();

    //! Assigns any m_pending_types to m_field_types.
    void CheckPendingFieldTypes() const;

    //! Future types being parsed by parser.
    //! mutable so that it can be assigned to m_field_types when completed.
    mutable boost::optional<Pending::Pending<FieldTypeMap>> m_pending_types = boost::none;

    mutable FieldTypeMap m_field_types;

    static FieldTypeManager* s_instance;
};


//! Returns the singleton field type manager
FO_COMMON_API auto GetFieldTypeManager() -> FieldTypeManager&;

//! Returns the BuildingType specification object for a field of
//! type @p name.  If no such FieldType exists, nullptr is returned instead.
FO_COMMON_API auto GetFieldType(const std::string& name) -> const FieldType*;

#endif // _FieldType_h_
