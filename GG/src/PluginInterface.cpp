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

#include <GG/PluginInterface.h>

#include <GG/DropDownList.h>
#include <GG/DynamicGraphic.h>
#include <GG/Edit.h>
#include <GG/ListBox.h>
#include <GG/Menu.h>
#include <GG/MultiEdit.h>
#include <GG/StaticGraphic.h>
#include <GG/TextControl.h>

#include <iostream>

using namespace GG;

///////////////////////////////////////
// class GG::PluginInterface
///////////////////////////////////////
PluginInterface::PluginInterface() : 
    PluginName(0),
    DefaultFontName(0),
    DefaultFontSize(0),
    GetStyleFactory(0),
    m_handle(0),
    m_out_archive(0),
    m_in_archive(0)
{}

PluginInterface::PluginInterface(const std::string& lib_name) : 
    PluginName(0),
    DefaultFontName(0),
    DefaultFontSize(0),
    GetStyleFactory(0),
    m_handle(0),
    m_out_archive(0),
    m_in_archive(0)
{ Load(lib_name); }

PluginInterface::~PluginInterface()
{
    if (m_handle) {
        PluginName = 0;
        DefaultFontName = 0;
        DefaultFontSize = 0;
        GetStyleFactory = 0;
        lt_dlclose(m_handle);
        m_handle = 0;
    }
}

PluginInterface::operator int PluginInterface::ConvertibleToBoolDummy::* () const
{ return m_handle ? &ConvertibleToBoolDummy::_ : 0; }

bool PluginInterface::Load(const std::string& lib_name)
{
    PluginManager::InitDynamicLoader();

    bool retval = true;

    int err = 0;
    if (m_handle) {
        if ((err = lt_dlclose(m_handle))) {
            retval = false;
            std::cerr << "PluginInterface::Load : lt_dlclose() call failed; load of new dynamic library aborted (error #" 
                      << err << ": " << lt_dlerror() << ").";
        }
    }

    if (!err) {
        m_handle = lt_dlopenext(lib_name.c_str());
        if (m_handle) {
            PluginName = (PluginNameFn)(lt_dlsym(m_handle, "PluginName"));
            DefaultFontName = (DefaultFontNameFn)(lt_dlsym(m_handle, "DefaultFontName"));
            DefaultFontSize = (DefaultFontSizeFn)(lt_dlsym(m_handle, "DefaultFontSize"));
            GetStyleFactory = (GetStyleFactoryFn)(lt_dlsym(m_handle, "GetStyleFactory"));
        } else {
            retval = false;
            std::cerr << "PluginInterface::Load : Failed to load dynamic library \"" << lib_name << "\" (error was: " << lt_dlerror() << ").";
        }
    }
    return retval;
}


///////////////////////////////////////
// class GG::PluginManager
///////////////////////////////////////
// static member(s)
bool PluginManager::s_lt_dl_initialized = false;

PluginManager::PluginManager()
{}

boost::shared_ptr<PluginInterface> PluginManager::GetPlugin(const std::string& name)
{
    std::map<std::string, boost::shared_ptr<PluginInterface> >::iterator it = m_plugins.find(name);
    if (it == m_plugins.end()) { // if no such plugin was found, attempt to load it now, using name as the filename
        m_plugins[name].reset(new PluginInterface());
        m_plugins[name]->Load(name);
        return m_plugins[name];
    } else { // otherwise, just return the plugin we found
        return it->second;
    }
}

void PluginManager::FreePlugin(const std::string& name)
{
    std::map<std::string, boost::shared_ptr<PluginInterface> >::iterator it = m_plugins.find(name);
    if (it != m_plugins.end())
        m_plugins.erase(it);
}

void PluginManager::InitDynamicLoader()
{
    if (s_lt_dl_initialized)
        return;

    int err = lt_dlinit();
    if (err) {
        std::cerr << "PluginManager::InitDynamicLoader : lt_dlinit() call failed. (error #" << err << ": " << lt_dlerror() << ").";
    } else {
        s_lt_dl_initialized = true;
    }
}

void PluginManager::AddSearchDirectory(const std::string& dir)
{
    if (!s_lt_dl_initialized)
        InitDynamicLoader();

    int err = lt_dladdsearchdir(dir.c_str());
    if (err) {
        std::cerr << "PluginManager::AddSearchDirectory : lt_dladdsearchdir() call failed for directory \"" << dir << "\". (error #" << err 
                  << ": " << lt_dlerror() << ").";
    }
}

void PluginManager::CleanupDynamicLoader()
{
    if (!s_lt_dl_initialized)
        return;

    int err = lt_dlexit();
    if (err) {
        std::cerr << "PluginManager::CleanupDynamicLoader : lt_dlexit() call failed. (error #" << err << ": " << lt_dlerror() << ").";
    } else {
        s_lt_dl_initialized = false;
    }
}

PluginManager& GG::GetPluginManager()
{
    static PluginManager manager;
    return manager;
}
