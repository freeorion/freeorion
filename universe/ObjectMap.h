// -*- C++ -*-
#ifndef _Object_Map_h_
#define _Object_Map_h_

#include <map>
#include <vector>
#include <string>

#include <boost/serialization/access.hpp>

class Universe;
class UniverseObject;
class UniverseObjectVisitor;

extern const int ALL_EMPIRES;

/** Contains a set of objects that make up a (known or complete) Universe. */
class ObjectMap {
public:
    typedef std::map<int, UniverseObject*>::const_iterator          iterator;       ///< iterator that allows modification of pointed-to UniverseObjects
    typedef std::map<int, const UniverseObject*>::const_iterator    const_iterator; ///< iterator that does not allow modification of UniverseObjects

    /** \name Structors */ //@{
    ObjectMap();            ///< default ctor
    ~ObjectMap();           ///< dtor

    /** Copies contents of this ObjectMap to a new ObjectMap, which is
      * returned.  Copies are limited to only duplicate information that the
      * empire with id \a empire_id would know about the copied objects. */
    ObjectMap*              Clone(int empire_id = ALL_EMPIRES) const;
    //@}

    /** \name Accessors */ //@{
    /** Returns number of objects in this ObjectMap */
    int                     NumObjects() const;

    /** Returns true if this ObjectMap contains no objects */
    bool                    Empty() const;

    /** Returns a pointer to the universe object with ID number \a id, or 0 if
      * none exists */
    const UniverseObject*   Object(int id) const;

    /** Returns a pointer to the universe object with ID number \a id, or 0 if
      * none exists */
    UniverseObject*         Object(int id);

    /** Returns a pointer to the object of type T with ID number \a id.
      * Returns 0 if none exists or the object with ID \a id is not of
      * type T. */
    template <class T>
    const T*                Object(int id) const;

    /** Returns a pointer to the object of type T with ID number \a id.
      * Returns 0 if none exists or the object with ID \a id is not of
      * type T */
    template <class T>
    T*                      Object(int id);

    /** Returns a vector containing the objects with ids in \a object_ids */
    std::vector<const UniverseObject*>  FindObjects(const std::vector<int>& object_ids) const;

    /** Returns a vector containing the objects with ids in \a object_ids */
    std::vector<UniverseObject*>        FindObjects(const std::vector<int>& object_ids);

    /** Returns all the objects that match \a visitor */
    std::vector<const UniverseObject*>  FindObjects(const UniverseObjectVisitor& visitor) const;

    /** Returns all the objects that match \a visitor */
    std::vector<UniverseObject*>        FindObjects(const UniverseObjectVisitor& visitor);

    /** Returns all the objects of type T */
    template <class T>
    std::vector<const T*>   FindObjects() const;

    /** Returns all the objects of type T */
    template <class T>
    std::vector<T*>         FindObjects();

    /** Returns the IDs of all the objects that match \a visitor */
    std::vector<int>        FindObjectIDs(const UniverseObjectVisitor& visitor) const;

    /** Returns the IDs of all the objects of type T */
    template <class T>
    std::vector<int>        FindObjectIDs() const;

    /** Returns the IDs of all objects in this ObjectMap */
    std::vector<int>        FindObjectIDs() const;

    /** iterators */
    iterator            begin();
    iterator            end();
    const_iterator      const_begin() const;
    const_iterator      const_end() const;

    std::string         Dump() const;
    //@}

    /** \name Mutators */ //@{

    /** Copies the contents of the ObjectMap \a copied_map into this ObjectMap.
      * Each object in \a copied_map has information transferred to this map.
      * If there already is a version of an object in \a copied_map in this map
      * then information is copied onto this map's version of the object using
      * the UniverseObject::Copy function.  If there is no corresponding object
      * in this map, a new object is created using the UinverseObject::Clone
      * function.  The copied objects are complete copies if \a empire_id is
      * ALL_EMPIRES, but if another \a empire_id is specified, the copied
      * information is limited by passing \a empire_id to are limited to the
      * Copy or Clone functions of the copied UniverseObjects.  Any objects
      * in this ObjectMap that have no corresponding object in \a copied_map
      * are left unchanged. */
    void                Copy(const ObjectMap& copied_map, int empire_id = ALL_EMPIRES);

    /** Copies the passed \a object into this ObjectMap, overwriting any
      * existing information about that object or creating a new object in this
      * map as appropriate with UniverseObject::Copy or UniverseObject::Clone.
      * The object is fully copied if \a empire_id is ALL_EMPIRES, but if
      * another empire id is specified, then the copied informatio is limited
      * by passed that \a empire_id to Copy or Clone of the object.  The
      * passed object is unchanged. */
    void                Copy(const UniverseObject* obj, int empire_id = ALL_EMPIRES);

    /** Copies the objects of the ObjectMap \a copied_map that are visible to
      * the empire with id \a empire_id into this ObjectMap.  Copied objects
      * are complete copies of all information in \a copied_map about objects
      * that are visible, and no information about not-visible objects is
      * copied.  Any existing objects in this ObjectMap that are not visible to
      * the empire with id \a empire_id are left unchanged.  If \a empire_id is
      * ALL_EMPIRES, then all objects in \a copied_map are copied completely
      * and this function acts just like ObjectMap::Copy .*/
    void                CompleteCopyVisible(const ObjectMap& copied_map, int empire_id = ALL_EMPIRES);

    /** Adds object \a obj to the map under id \a id if id is a valid object id
      * and obj is an object with that id set.  If there already was an object
      * in the map with the id \a id then that object is first removed, and
      * is returned. This ObjectMap takes ownership of the passed
      * UniverseObject. The caller takes ownership of any returned
      * UniverseObject. */
    UniverseObject*     Insert(int id, UniverseObject* obj);

    /** Removes object with id \a id from map, and returns that object, if
      * there was an object under that ID in the map.  If no such object
      * existed in the map, 0 is returned and nothing is removed. The caller
      * takes ownership of any returned UniverseObject. */
    UniverseObject*     Remove(int id);

    /** Removes object with id \a id from map, and deletes that object, if
      * there was an object under that ID in the map.  If no such object
      * existed in the map, nothing is done. */
    void                Delete(int id);

    /** Empties map and deletes all objects within. */
    void                Clear();

    /** Swaps the contents of *this with \a rhs. */
    void                swap(ObjectMap& rhs);
    //@}

private:
    void                CopyObjectsToConstObjects();

    std::map<int, UniverseObject*>          m_objects;
    std::map<int, const UniverseObject*>    m_const_objects;

    friend class Universe;
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
#if (10 * __GNUC__ + __GNUC_MINOR__ > 33) && (!defined _UniverseObject_h_)
#  include "UniverseObject.h"
#endif

template <class T>
const T* ObjectMap::Object(int id) const
{
    const_iterator it = m_const_objects.find(id);
    return (it != m_const_objects.end() ?
            static_cast<T*>(it->second->Accept(UniverseObjectSubclassVisitor<typename boost::remove_const<T>::type>())) :
            0);
}

template <class T>
T* ObjectMap::Object(int id)
{
    iterator it = m_objects.find(id);
    return (it != m_objects.end() ?
            static_cast<T*>(it->second->Accept(UniverseObjectSubclassVisitor<typename boost::remove_const<T>::type>())) :
            0);
}

template <class T>
std::vector<const T*> ObjectMap::FindObjects() const
{
    std::vector<const T*> retval;
    for (ObjectMap::const_iterator it = m_const_objects.begin(); it != m_const_objects.end(); ++it) {
        if (const T* obj = static_cast<T*>(it->second->Accept(UniverseObjectSubclassVisitor<typename boost::remove_const<T>::type>())))
            retval.push_back(obj);
    }
    return retval;
}

template <class T>
std::vector<T*> ObjectMap::FindObjects()
{
    std::vector<T*> retval;
    for (ObjectMap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (T* obj = static_cast<T*>(it->second->Accept(UniverseObjectSubclassVisitor<typename boost::remove_const<T>::type>())))
            retval.push_back(obj);
    }
    return retval;
}

template <class T>
std::vector<int> ObjectMap::FindObjectIDs() const
{
    std::vector<int> retval;
    for (ObjectMap::const_iterator it = m_const_objects.begin(); it != m_const_objects.end(); ++it) {
        if (static_cast<T*>(it->second->Accept(UniverseObjectSubclassVisitor<typename boost::remove_const<T>::type>())))
            retval.push_back(it->first);
    }
    return retval;
}

#endif
