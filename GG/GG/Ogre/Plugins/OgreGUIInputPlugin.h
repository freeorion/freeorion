// -*- C++ -*-
/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2007 T. Zachary Laine

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
   
/** \file OgreGUIInputPlugin.h \brief Contains OgreGUIInputPlugin, the
    Ogre::Plugin base class for making plugins that provide OgreGUI with
    keyboard and mouse input. */

#ifndef _GG_OgreGUIInputPlugin_h_ 
#define _GG_OgreGUIInputPlugin_h_

#include <OgrePlugin.h>

#include <GG/PtRect.h>

#include <boost/signals.hpp>


#ifdef _MSC_VER
# ifdef GiGiOgrePlugin_OIS_EXPORTS
#  define GG_OGRE_PLUGIN_API __declspec(dllexport)
# else
#  define GG_OGRE_PLUGIN_API __declspec(dllimport)
# endif
#else
# define GG_OGRE_PLUGIN_API
#endif

namespace Ogre { class RenderWindow; }

namespace GG {

/** \brief The base class for Ogre plugins that provides input functionality
    to OgreGUI.

    Derived classes must implement HandleWindowResize(), HandleWindowClose(),
    and HandleSystemEvents().  The last of these is where the input-system
    specific code resides.  This function must obtain mouse moves and clicks,
    keyboard events, etc., from the underlying system and translate them to
    the associated GG events.  Derived classes must also call
    ConnectHandlers() sometime in their contructors.  See OISInput for a
    sample implementation. */
class GG_OGRE_PLUGIN_API OgreGUIInputPlugin :
    public Ogre::Plugin
{
public:
    OgreGUIInputPlugin();  ///< Default ctor.
    virtual ~OgreGUIInputPlugin(); ///< Dtor.

    /** Overrides the use of the Ogre::Root-autocreated window. */
    static void SetRenderWindow(Ogre::RenderWindow* window);

    /** Returns the Ogre::RenderWindow in which the OgreGUI is operating. */
    static Ogre::RenderWindow* GetRenderWindow();

protected:
    void ConnectHandlers();
    void DisconnectHandlers();

private:
    virtual void HandleSystemEvents() = 0;
    virtual void HandleWindowResize(X width, Y height);
    virtual void HandleWindowClose();

    boost::signals::connection m_handle_events_connection;
    boost::signals::connection m_resize_connection;
    boost::signals::connection m_close_connection;

    static Ogre::RenderWindow* s_render_window;
};

} // namespace GG

#endif
