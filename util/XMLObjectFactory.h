// -*- C++ -*-
/* Copyright (C) 2006 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
    
   Zach Laine
   whatwasthataddress@hotmail.com */

/** \file XMLObjectFactory.h
    Contains the XMLObjectFactory class template, which allows the user to register object-creation functions and then 
    automatically create objects from XML encodings. */

#ifndef _XMLObjectFactory_h_
#define _XMLObjectFactory_h_

#include "XMLDoc.h"

#include <map>
#include <string>
#include <stdexcept>

/** Thrown when a generated object is requested with an unknown tag. */
class NoSuchGenerator : public GG::ExceptionBase
{
public:
    NoSuchGenerator () throw() : ExceptionBase() {}
    NoSuchGenerator (const std::string& msg) throw() : ExceptionBase(msg) {}
    virtual const char* type() const throw()
        {return "NoSuchGenerator";}
};

/** This class creates polymorphic subclasses of base class T from XML-formatted text.  For any polymorphic class hierarchy,
    you can instantiate XMLObjectFactory with the type of the hierarchy's base class, and provide functions that can each 
    construct one specific class in the hierarchy.  By providing a string that identifies each class, and creating XML objects
    with that string as a tag, you can ensure a method for creating the correct polymorphic subclass object at run-time. */
template <class T>
class XMLObjectFactory
{
public:
    typedef T* (*Generator)(const XMLElement&); ///< this defines the function signature for XMLObjectFactory object generators

    /** \name Structors */ //@{
    XMLObjectFactory(); ///< ctor
    //@}

    /** \name Accessors */ //@{
    /** returns a heap-allocated subclass object of the appropriate type \throw NoSuchGenerator Throws if \a elem
        has an unknown tag. */
    T* GenerateObject(const XMLElement& elem) const;
    //@}
   
    /** \name Mutators */ //@{
    /** adds a new generator (or replaces one that already exists) that can generate subclass objects described by \a
        name */
    void AddGenerator(const std::string& name, Generator gen);

    /** removes the generator that can generate subclass objects described by \a name, if one exists */
    void RemoveGenerator(const std::string& name);
    //@}

private:
    typedef typename std::map<std::string, Generator> GeneratorMap;

    /** mapping from strings to functions that can create the type of object that corresponds to the string */
    GeneratorMap m_generators;
};


// template implementations
template <class T>
XMLObjectFactory<T>::XMLObjectFactory()
{
}

template <class T>
T* XMLObjectFactory<T>::GenerateObject(const XMLElement& elem) const ///< returns a heap-allocated subclass object of the appropriate type
{
    typename GeneratorMap::const_iterator it = m_generators.find(elem.Tag());

    if (it == m_generators.end())
        throw NoSuchGenerator("XMLObjectFactory::GenerateObject(): No generator exists for XMLElements with the tag \"" + elem.Tag() + "\".");

    return it->second(elem);
}

template <class T>
void XMLObjectFactory<T>::AddGenerator(const std::string& name, Generator gen)
{
    m_generators[name] = gen;
}

template <class T>
void XMLObjectFactory<T>::RemoveGenerator(const std::string& name)
{
    m_generators.erase(name);
}

#endif // _XMLObjectFactory_h_

