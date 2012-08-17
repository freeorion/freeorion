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
   
/** \file OISInput.h \brief Contains OISInput, the OgreGUIInputPlugin subclass
    that provides OgreGUI with OIS input. */

#include <GG/Ogre/Plugins/OgreGUIInputPlugin.h>

#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>


namespace OIS { class InputManager; }

/** \brief An Ogre plugin that provides input functionality to OgreGUI using
    OIS.

    OISInput is configured from the configuration data in
    OgreGUI::ConfigFileStream(), if any.  See OISInput.cfg for an example
    config file. */
class GG_OGRE_PLUGIN_API OISInput :
    public GG::OgreGUIInputPlugin,
    public OIS::MouseListener,
    public OIS::KeyListener
{
public:
    OISInput();
    virtual ~OISInput();

    virtual const Ogre::String& getName() const;
    virtual void install();
    virtual void initialise();
    virtual void shutdown();
    virtual void uninstall();

private:
    virtual void HandleSystemEvents();
    virtual void HandleWindowResize(GG::X width, GG::Y height);
    virtual void HandleWindowClose();

    virtual bool mouseMoved(const OIS::MouseEvent &event);
    virtual bool mousePressed(const OIS::MouseEvent &event, OIS::MouseButtonID id);
    virtual bool mouseReleased(const OIS::MouseEvent &event, OIS::MouseButtonID id);

    virtual bool keyPressed(const OIS::KeyEvent& event);
    virtual bool keyReleased(const OIS::KeyEvent& event);

    void CleanupInputManager();

    OIS::InputManager*  m_input_manager;
    OIS::Mouse*         m_mouse;
    OIS::Keyboard*      m_keyboard;
};
