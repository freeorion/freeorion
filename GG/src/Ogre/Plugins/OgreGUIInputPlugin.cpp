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
   
#include <GG/Ogre/Plugins/OgreGUIInputPlugin.h>

#include <GG/Ogre/OgreGUI.h>

#include <OgreRoot.h>


using namespace GG;

Ogre::RenderWindow* OgreGUIInputPlugin::s_render_window = 0;

OgreGUIInputPlugin::OgreGUIInputPlugin()
{}

OgreGUIInputPlugin::~OgreGUIInputPlugin()
{ DisconnectHandlers(); }

void OgreGUIInputPlugin::SetRenderWindow(Ogre::RenderWindow* window)
{ s_render_window = window; }

Ogre::RenderWindow* OgreGUIInputPlugin::GetRenderWindow()
{ return s_render_window ? s_render_window : Ogre::Root::getSingleton().getAutoCreatedWindow(); }

void OgreGUIInputPlugin::ConnectHandlers()
{
    OgreGUI* gui = OgreGUI::GetGUI();
    assert(gui);

    m_handle_events_connection = Connect(gui->HandleSystemEventsSignal, &OgreGUIInputPlugin::HandleSystemEvents, this);
    m_resize_connection = Connect(gui->WindowResizedSignal, &OgreGUIInputPlugin::HandleWindowResize, this);
    m_close_connection = Connect(gui->WindowClosedSignal, &OgreGUIInputPlugin::HandleWindowClose, this);
}

void OgreGUIInputPlugin::DisconnectHandlers()
{
    m_handle_events_connection.disconnect();
    m_resize_connection.disconnect();
    m_close_connection.disconnect();
}

void OgreGUIInputPlugin::HandleWindowResize(X width, Y height)
{}

void OgreGUIInputPlugin::HandleWindowClose()
{}
