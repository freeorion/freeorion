// -*- C++ -*-
/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

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
   whatwasthataddress@gmail.com */
   
/** \file Exception.h \brief Contains the GG::Exception base class and macros
    that ease declaring subclasses. */

#ifndef _GG_Exception_h_
#define _GG_Exception_h_

#include <GG/Export.h>

#include <stdexcept>
#include <string>


namespace GG {

/** \brief The base class for all GG exceptions.

    It is based on the std::exception class.  As required by its inheritance
    from std::exceptions not throw other exceptions, the no-throw exception
    specification has been added to every member function. */
class ExceptionBase : public std::exception
{
public:
    ExceptionBase() noexcept
    {}

    /** Create an exception with the given @p msg. */
    ExceptionBase(const std::string& msg) noexcept :
        m_msg(msg)
    {}

    /** Dtor required by std::exception. */
    ~ExceptionBase() noexcept
    {}

    /** Returns a string representation of the type this exception. */
    virtual const char* type() const throw() = 0;

    const char* what() const throw() override
    { return m_msg.c_str(); }

private:
    std::string m_msg; ///< the text message associated with this Exception (may be "")
};

/** Declares a GG exception class.  This should be used to declare GG
    exceptions at namespace scope. */
#define GG_EXCEPTION( name )                                            \
    class name : public ExceptionBase                                   \
    {                                                                   \
    public:                                                             \
        name () throw() : ExceptionBase() {}                            \
        name (const std::string& msg) throw() : ExceptionBase(msg) {}   \
        const char* type() const noexcept override                      \
            {return "GG::" # name ;}                                    \
    };

/** Declares an abstract base for further GG exception class inheritance.
    This should be used along with GG_CONCRETE_EXCEPTION to group all
    exceptions from a single GG class under one subhierarchy. */
#define GG_ABSTRACT_EXCEPTION( name )                                   \
    class name : public ExceptionBase                                   \
    {                                                                   \
    public:                                                             \
        name () throw() : ExceptionBase() {}                            \
        name (const std::string& msg) throw() : ExceptionBase(msg) {}   \
        const char* type() const noexcept override = 0;                 \
    };

/** Declares a concrete exception class derived from \a superclass.  This
    should be used along with GG_ABSTRACT_EXCEPTION to group all exceptions
    from a single GG class under one subhierarchy. */
#define GG_CONCRETE_EXCEPTION( name, class_name, superclass )           \
    class name : public superclass                                      \
    {                                                                   \
    public:                                                             \
        name () throw() : superclass () {}                              \
        name (const std::string& msg) throw() : superclass (msg) {}     \
        const char* type() const noexcept override                      \
            {return # class_name "::" # name ;}                         \
    };

} // namespace GG

#endif
