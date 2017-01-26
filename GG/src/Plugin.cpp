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

/** \file GGPlugin.cpp
    This is a sample plugin implementation.  It can only create the default types of GG controls.  Extend
    it to create a plugin suitable for a user-defined GG control a hierarchy. */

#include <GG/Button.h>
#include <GG/DropDownList.h>
#include <GG/DynamicGraphic.h>
#include <GG/Edit.h>
#include <GG/Layout.h>
#include <GG/ListBox.h>
#include <GG/Menu.h>
#include <GG/MultiEdit.h>
#include <GG/Scroll.h>
#include <GG/Slider.h>
#include <GG/Spin.h>
#include <GG/StaticGraphic.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>

#ifdef _MSC_VER
# define GG_PLUGIN_API __declspec(dllexport)
#else
# define GG_PLUGIN_API
#endif

namespace {
    // TODO: edit these constants to reflect your plugin's requirements
    const char* PLUGIN_NAME = "GG Basic Controls";
    const char* DEFAULT_FONT_NAME = "Vera.ttf";
    const int DEFAULT_FONT_SIZE = 12;
}

extern "C" {
    // provides the name of the plugin
    GG_PLUGIN_API 
    const char* PluginName()
    { return PLUGIN_NAME; }

    // provides the name of the default font to be used with this plugin
    GG_PLUGIN_API 
    const char* DefaultFontName()
    { return DEFAULT_FONT_NAME; }

    // provides the size of the default font to be used with this plugin
    GG_PLUGIN_API 
    int DefaultFontSize()
    { return DEFAULT_FONT_SIZE; }


    //  TODO: Override this with your own StyleFactory subclass.
    GG_PLUGIN_API
    std::shared_ptr<GG::StyleFactory> GetStyleFactory()
    {
        static auto style_factor = std::make_shared<GG::StyleFactory>();
        return style_factory;
    }
}
