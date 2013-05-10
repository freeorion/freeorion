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

/** \file PluginInterface.h \brief Contains the PluginInterface class, an
    interface to custom-control plugins that allow runtime control
    selection. */

#ifndef _GG_PluginInterface_h_
#define _GG_PluginInterface_h_

#include <GG/GUI.h>

#if defined(__APPLE__) && defined(__MACH__)
# include "../libltdl/ltdl.h"
#else
# include <ltdl.h>
#endif

#include <string>


namespace GG {

/** \brief The interface to custom-control plugins.

    This class is used to access derived GG controls and dialogs that are
    unknown until runtime, but are available for dynamic loading in shared
    libraries/DLLs.  The interface basically allows you to create custom
    controls and dialogs (anything a StyleFactory can produce) from a
    dynamic-link library, which in turn allows you to change the styles of the
    controls in an application without recompiling, or even relinking.  While
    the interface is in an unloaded state, the functions in the interface are
    all null, and calling any of them will crash your app.  Once a plugin has
    been loaded, all the functions in the interface should be valid (if the
    plugin author did everything correctly). */
class GG_API PluginInterface
{
private:
    struct ConvertibleToBoolDummy {int _;};

public:
    /** The type of function that supplies the name of the plugin. */
    typedef const char*                     (*PluginNameFn)();
    /** The type of function that supplies the name of the plugin's default
        font. */
    typedef const char*                     (*DefaultFontNameFn)();
    /** The type of function that supplies the plugin's default font size. */
    typedef unsigned int                    (*DefaultFontSizeFn)();
    /** The type of function that supplies the plugin's StyleFactory. */
    typedef boost::shared_ptr<StyleFactory> (*GetStyleFactoryFn)();

    /** \name Structors */ ///@{
    PluginInterface(); ///< default ctor.

    /** Ctor that loads the plugin file \a lib_name.  The base filename should
        be provided, without the extension (i.e. "foo", not "foo.so" or
        "foo.dll"). */
    PluginInterface(const std::string& lib_name);

    ~PluginInterface(); ///< dtor.
    //@}

    /** \name Accessors */ ///@{
    /** Returns true iff this PluginInterface has a loaded plugin.  This is a
        conversion operator, allowing you to test the validity of the
        interface, as you would a pointer (e.g. if (my_interface)
        my_interface.PluginName();).  \warning If this method returns false,
        the functions in the interface are invalid. */
    operator int ConvertibleToBoolDummy::* () const;
    //@}

    /** \name Mutators */ ///@{
    /** Loads the plugin \a lib_name, unloading the currently-loaded plugin if
        necessary. */
    bool Load(const std::string& lib_name);

    PluginNameFn                PluginName;             ///< returns the name of this plugin
    DefaultFontNameFn           DefaultFontName;        ///< returns the default font name that should be used to create controls using this plugin.
    DefaultFontSizeFn           DefaultFontSize;        ///< returns the default font point size that should be used to create controls using this plugin.
    GetStyleFactoryFn           GetStyleFactory;        ///< returns a shared_ptr to the plugin's style factory, which creates controls and dialogs
    //@}

private:
    lt_dlhandle m_handle;
    boost::archive::xml_oarchive* m_out_archive;
    boost::archive::xml_iarchive* m_in_archive;
};

/** \brief A singleton that loads and stores textures for use by GG.

    This class is essentially a very thin wrapper around a map of
    PluginInterface smart pointers, keyed on std::string plugin names.  The
    user need only request a plugin through GetPlugin(); if the plugin is not
    already resident, it will be loaded.*/
class GG_API PluginManager
{
public:
    /** \name Mutators */ ///@{
    /** Returns a shared_ptr to the plugin interface created from plugin \a
        name. If the plugin is not present in the manager's pool, it will be
        loaded from disk. */
    boost::shared_ptr<PluginInterface> GetPlugin(const std::string& name);

    /** Removes the manager's shared_ptr to the plugin created from file \a
        name, if it exists.  \note Due to shared_ptr semantics, the plugin may
        not be deleted until much later. */
    void FreePlugin(const std::string& name);
    //@}

    /** Initializes the dynamic loader system that loads and unloads plugins.
        This is available as a convenience only; it will be called
        automatically as needed. */
    static void InitDynamicLoader();

    /** Adds a directory which should be searched for plugins. */
    static void AddSearchDirectory(const std::string& dir);

    /** Cleans up the dynamic loader system that loads and unloads plugins.
        This should be called manually when desiredl it will never be called
        by other PluginInterface code. */
    static void CleanupDynamicLoader();

private:
    PluginManager();

    std::map<std::string, boost::shared_ptr<PluginInterface> > m_plugins;

    static bool s_lt_dl_initialized;

    friend GG_API PluginManager& GetPluginManager();
};

/** Returns the singleton PluginManager instance. */
GG_API PluginManager& GetPluginManager();

} // namespace GG

#endif
