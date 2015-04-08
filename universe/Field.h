// -*- C++ -*-
#ifndef _Field_h_
#define _Field_h_

#include "UniverseObject.h"

#include "../util/Export.h"

namespace Effect {
    class EffectsGroup;
}

/** a class representing a region of space */
class FO_COMMON_API Field : public UniverseObject {
public:
    /** \name Accessors */ //@{
    virtual std::set<std::string>
                                Tags() const;                                       ///< returns all tags this object has
    virtual bool                HasTag(const std::string& name) const;              ///< returns true iff this object has the tag with the indicated \a name

    virtual UniverseObjectType  ObjectType() const;
    virtual std::string         Dump() const;

    const std::string&          FieldTypeName() const { return m_type_name; }
    virtual TemporaryPtr<UniverseObject>
                                Accept(const UniverseObjectVisitor& visitor) const;

    virtual int                 ContainerObjectID() const;                          ///< returns id of the object that directly contains this object, if any, or INVALID_OBJECT_ID if this object is not contained by any other
    virtual bool                ContainedBy(int object_id) const;                   ///< returns true if there is an object with id \a object_id that contains this UniverseObject

    bool                        InField(TemporaryPtr<const UniverseObject> obj) const;
    bool                        InField(double x, double y) const;

    virtual const std::string&  PublicName(int empire_id) const;
    //@}

    /** \name Mutators */ //@{
    virtual void                Copy(TemporaryPtr<const UniverseObject> copied_object, int empire_id = ALL_EMPIRES);

    virtual void                ResetTargetMaxUnpairedMeters();
    //@}

protected:
    friend class Universe;
    /** \name Structors */ //@{
    Field();                                        ///< default ctor
    Field(const std::string& field_type, double x, double y, double radius);
    
    template <class T> friend void boost::python::detail::value_destroyer<false>::execute(T const volatile* p);
    template <class T> friend void boost::checked_delete(T* x);
    ~Field() {}

    virtual Field*              Clone(int empire_id = ALL_EMPIRES) const;   ///< returns new copy of this Field
    //@}

private:
    virtual void                ClampMeters();

    std::string     m_type_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/** A specification for a type of field. */
class FO_COMMON_API FieldType {
public:
    /** \name Structors */ //@{
    FieldType(const std::string& name, const std::string& description,
              float stealth, const std::set<std::string>& tags,
              const std::vector<boost::shared_ptr<Effect::EffectsGroup> >& effects,
              const std::string& graphic);
    ~FieldType();
    //@}

    /** \name Accessors */ //@{
    const std::string&              Name() const            { return m_name; }          ///< returns the unique name for this type of field
    const std::string&              Description() const     { return m_description; }   ///< returns a text description of this type of building
    std::string                     Dump() const;                                       ///< returns a data file format representation of this object
    float                           Stealth() const         { return m_stealth; }       ///< returns stealth of field type
    const std::set<std::string>&    Tags() const            { return m_tags; }
    const std::vector<boost::shared_ptr<Effect::EffectsGroup> >&
                                    Effects() const         { return m_effects; }       ///< returns the EffectsGroups that encapsulate the effects of this FieldType
    const std::string&              Graphic() const         { return m_graphic; }       ///< returns the name of the grapic file for this field type
    //@}

private:
    std::string                                             m_name;
    std::string                                             m_description;
    float                                                   m_stealth;
    std::set<std::string>                                   m_tags;
    std::vector<boost::shared_ptr<Effect::EffectsGroup> >   m_effects;
    std::string                                             m_graphic;
};


class FieldTypeManager {
public:
    typedef std::map<std::string, FieldType*>::const_iterator iterator;

    /** \name Accessors */ //@{
    /** returns the field type with the name \a name; you should use the
      * free function GetFieldType(...) instead, mainly to save some typing. */
    const FieldType*            GetFieldType(const std::string& name) const;

    /** iterator to the first field type */
    iterator                    begin() const   { return m_field_types.begin(); }

    /** iterator to the last + 1th field type */
    iterator                    end() const     { return m_field_types.end(); }

    /** returns the instance of this singleton class; you should use the free
      * function GetFieldTypeManager() instead */
    static FieldTypeManager&    GetFieldTypeManager();
    //@}
private:
    FieldTypeManager();
    ~FieldTypeManager();
    std::map<std::string, FieldType*>   m_field_types;
    static FieldTypeManager*            s_instance;
};

/** returns the singleton field type manager */
FO_COMMON_API FieldTypeManager& GetFieldTypeManager();

/** Returns the BuildingType specification object for a field of
  * type \a name.  If no such FieldType exists, 0 is returned instead. */
FO_COMMON_API const FieldType* GetFieldType(const std::string& name);


#endif // _Ship_h_
