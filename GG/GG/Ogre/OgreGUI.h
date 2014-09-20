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
   
/** \file OgreGUI.h \brief Contains OgreGUI, the input driver for using Ogre
    with GG. */

#ifndef _GG_OgreGUI_h_ 
#define _GG_OgreGUI_h_

#ifdef __APPLE__
/* prevents OpenTransportProviders.h (a system header in Mac SDKs)
    from trying to enum what's already defined by related headers */
#undef TCP_NODELAY
#undef TCP_MAXSEG
#undef TCP_NOTIFY_THRESHOLD
#undef TCP_ABORT_THRESHOLD
#undef TCP_CONN_NOTIFY_THRESHOLD
#undef TCP_CONN_ABORT_THRESHOLD
#undef TCP_OOBINLINE
#undef TCP_URGENT_PTR_TYPE
#undef TCP_KEEPALIVE
#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0 // workaround by Apple to avoid conflicting macro names, fixes compile error
#include <Carbon/Carbon.h>
#endif

#include <OgreRenderTargetListener.h>
#include <OgreTimer.h>
#include <OgreSharedPtr.h>
#include <OgreDataStream.h>
#include <OgreWindowEventUtilities.h>

#include <GG/GUI.h>

#include <boost/filesystem/path.hpp>

#ifdef _MSC_VER
# ifdef GiGiOgre_EXPORTS
#  define GG_OGRE_API __declspec(dllexport)
# else
#  define GG_OGRE_API __declspec(dllimport)
# endif
#else
# define GG_OGRE_API
#endif

namespace GG {

/** \brief This is an abstract singleton class that represents the GUI
    framework of an Ogre OpenGL application.

    Usage:<br>
    Any application including an object of this class should declare that object
    as a local variable in main(). The name of this variable will herein be
    assumed to be "gui". It should be allocated on the stack; if it is created
    dynamically, a leak may occur.
    <p>
    OgreGUI serves as a driver of the main Ogre event loop, and mediates between
    the global GL state set by Ogre and the global GL state set by GG.  It also
    provides important information to the input plugin via its public signals.
    <p>
    OgreGUI does not constitute a complete input driver for GG.  This is because
    Ogre does not provide any direct input support at all (at the time of this
    writing, Ogre uses an input library called OIS to provide keyboard, mouse,
    and joystick input).  Following the Ogre convention, OgreGUI relies upon the
    Ogre plugin system to provide the actual input system used by Ogre and GG.
    A plugin for OIS is provided in the Ogre/Plugins subtree of the GG sources.
    <p>
    Any plugin used with OgreGUI must grab input state (mouse, keyboard, etc.) 
    in response to the firing of OgreGUI::HandleSystemEventsSignal.  It is
    notified of changes in the window size via OgreGUI::WindowResizedSignal.
    OgreGUI::HandleWindowClose indicates that the Ogre::RenderWindow in which
    OgreGUI is operating has or is about to close, in case the plugin needs to
    perform cleanup.
    <p>
    To use OgreGUI, one must first create "gui", then load the input plugin
    using Ogre's plugin loading mechanism, then call "gui();".  For example:
    \verbatim
    OgreGUI gui(ogre_window, "/path/to/input_plugin.cfg");
    ogre_root->loadPlugin("/path/to/plugin");
    gui();
    \endverbatim
    <p>
    When building GG and OgreGUI statically, the input plugin needs to be
    created and managed by the application, and installed into and uninstalled
    from the Ogre::Root singleton explicitly.  The plugin must be constructed
    after Ogre::Root has been constructed:
    \verbatim
    #include <GG/Ogre/Plugins/OISIput.h>
    ...
    OgreGUI gui(ogre_window, "/path/to/input_plugin.cfg");
    OISInput* ois_input_plugin = new OISInput;
    ogre_root->installPlugin(ois_input_plugin);
    gui();
    ...
    ogre_root->uninstallPlugin(ois_input_plugin);
    delete ois_input_plugin;
    \endverbatim
    */
class GG_OGRE_API OgreGUI :
    public GUI,
    public Ogre::RenderTargetListener,
    public Ogre::WindowEventListener
{
public:
    /** Basic ctor.  A nonzero \a window and \a root are required, and an optional
        configuration filename, \a config_filename.  If \a config_filename is
        supplied, it will be available via ConfigFileStream(). */
    explicit OgreGUI(Ogre::RenderWindow* window, Ogre::Root* root,
                     const boost::filesystem::path& config_file_path = boost::filesystem::path());

    /** Dtor. */
    virtual ~OgreGUI();

    /** Creates a modal event pump suitable for use with Ogre. */
    virtual boost::shared_ptr<ModalEventPump> CreateModalEventPump(bool& done);

    virtual unsigned int Ticks() const;
    virtual X AppWidth() const;
    virtual Y AppHeight() const;

    /** Returns an Ogre::DataStream containing the contents of the \a
        config_filename ctor parameter.  This will be null if \a
        config_filename was not supplied to the ctor. */
    const Ogre::SharedPtr<Ogre::DataStream>& ConfigFileStream() const;

    Ogre::RenderWindow* GetRenderWindow() const { return m_window; }

    virtual void Exit(int code);

    /** Emitted whenever the OgreGUI is ready for human input from the
        keyboard, mouse, etc. */
    boost::signals2::signal<void ()> HandleSystemEventsSignal;

    /** Emitted when the Ogre::RenderWindow in which the OgreGUI is operating
        is about to close. */
    boost::signals2::signal<void ()> WindowClosingSignal;

    /** Emitted when the Ogre::RenderWindow in which the OgreGUI is operating
        closes or is about to close. */
    boost::signals2::signal<void ()> WindowClosedSignal;

    /** Allows any code to access the gui framework by calling GG::OgreGUI::GetGUI(). */
    static OgreGUI* GetGUI();

    /// \override
    virtual std::vector< std::string > GetSupportedResolutions() const;

protected:
    virtual void RenderBegin();
    virtual void RenderEnd();
    virtual void Run();
    virtual void HandleSystemEvents();
    virtual void Enter2DMode();
    virtual void Exit2DMode();

private:
    virtual void postRenderTargetUpdate(const Ogre::RenderTargetEvent& event);
    virtual void windowMoved(Ogre::RenderWindow* window);
    virtual void windowResized(Ogre::RenderWindow* window);
    virtual bool windowClosing(Ogre::RenderWindow* window);
    virtual void windowClosed(Ogre::RenderWindow* window);
    virtual void windowFocusChange(Ogre::RenderWindow* window);

    Ogre::RenderWindow*               m_window;
    Ogre::Root*                       m_root;
    mutable Ogre::Timer               m_timer;
    Ogre::SharedPtr<Ogre::DataStream> m_config_file_data;
};

} // namespace GG

#endif
