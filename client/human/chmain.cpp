#ifdef OGRE_STATIC_LIB
#include "OgrePlugins/OgreCgPlugin.h"
#include "OgrePlugins/OgreOctreePlugin.h"
#include "OgrePlugins/OgreParticleFXPlugin.h"
#include "OgrePlugins/OgreGLPlugin.h"
#include <GG/Ogre/Plugins/OISInput.h>
#endif

#include "HumanClientApp.h"
#include "../../util/OptionsDB.h"
#include "../../util/Directories.h"
#include "../../util/Version.h"
#include "../../util/XMLDoc.h"
#include "../../util/MultiplayerCommon.h"

#include <OgreCamera.h>
#include <OgreLogManager.h>
#include <OgreRenderSystem.h>
#include <OgreRenderWindow.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreViewport.h>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <fstream>
#include <iostream>


#ifdef FREEORION_WIN32
#  define OGRE_INPUT_PLUGIN_NAME "GiGiOgrePlugin_OIS.dll"
#else
#  define OGRE_INPUT_PLUGIN_NAME "libGiGiOgrePlugin_OIS.so"
#endif

int main(int argc, char* argv[])
{
    InitDirs();

    // read and process command-line arguments, if any
    try {
        GetOptionsDB().AddFlag('h', "help", "OPTIONS_DB_HELP");
        GetOptionsDB().AddFlag('g', "generate-config-xml", "OPTIONS_DB_GENERATE_CONFIG_XML");
        GetOptionsDB().AddFlag('m', "music-off", "OPTIONS_DB_MUSIC_OFF");
        GetOptionsDB().Add<std::string>("bg-music", "OPTIONS_DB_BG_MUSIC", "artificial_intelligence_v3.ogg");

        // The false/true parameter below controls whether this option is stored in the XML config file.  On Win32 it is
        // not, because the installed version of FO is run with the command-line flag added in as appropriate.
        GetOptionsDB().AddFlag('f', "fullscreen", "OPTIONS_DB_FULLSCREEN",
#ifdef FREEORION_WIN32
                               false
#else
                               true
#endif
            );
        XMLDoc doc;
        boost::filesystem::ifstream ifs(GetConfigPath());
        doc.ReadDoc(ifs);
        ifs.close();
        GetOptionsDB().SetFromXML(doc);
        GetOptionsDB().SetFromCommandLine(argc, argv);
        bool early_exit = false;
        if (GetOptionsDB().Get<bool>("help")) {
            GetOptionsDB().GetUsage(std::cerr);
            early_exit = true;
        }
#ifdef FREEORION_MACOSX
        // Handle the case where the settings-dir does not exist anymore gracefully by resetting it to the standard path
        // into the application bundle this may happen if a previous installed version of FreeOrion was residing in a
        // different directory.
        if (!boost::filesystem::exists(boost::filesystem::path(GetOptionsDB().Get<std::string>("settings-dir"))))
            GetOptionsDB().Set<std::string>("settings-dir", (GetGlobalDir() / "default").directory_string());
#endif
        
        if (GetOptionsDB().Get<bool>("generate-config-xml")) {
            GetOptionsDB().Remove("generate-config-xml");
            boost::filesystem::ofstream ofs(GetConfigPath());
            GetOptionsDB().GetXML().WriteDoc(ofs);
            ofs.close();
        }
        if (early_exit)
            return 0;
    } catch (const std::invalid_argument& e) {
        std::cerr << "main() caught exception(std::invalid_arg): " << e.what() << std::endl;
        Sleep(3000);
        return 1;
    } catch (const std::runtime_error& e) {
        std::cerr << "main() caught exception(std::runtime_error): " << e.what() << std::endl;
        Sleep(3000);
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "main() caught exception(std::exception): " << e.what() << std::endl;
        Sleep(3000);
        return 1;
    } catch (...) {
        std::cerr << "main() caught unknown exception." << std::endl;
        return 1;
    }

    Ogre::LogManager* log_manager = 0;
    Ogre::Root* root = 0;

#ifdef OGRE_STATIC_LIB
    OISInput* ois_input_plugin = 0;
    Ogre::CgPlugin* cg_plugin = 0;
    Ogre::OctreePlugin* octree_plugin = 0;
    Ogre::ParticleFXPlugin* particle_fx_plugin = 0;
    Ogre::GLPlugin* gl_plugin = 0;
#endif

    try {
        using namespace Ogre;

        log_manager = new LogManager();
        log_manager->createLog((GetLocalDir() / "ogre.log").string(), true, false);

        root = new Root((GetGlobalDir() / "ogre_plugins.cfg").string());

        RenderSystemList* renderers_list = root->getAvailableRenderers();
        bool failed = true;
        RenderSystem* selected_render_system = 0;
        for (unsigned int i = 0; i < renderers_list->size(); ++i) {
            selected_render_system = renderers_list->at(i);
            String name = selected_render_system->getName();
            if (name.compare("OpenGL Rendering Subsystem") == 0) {
                failed = false;
                break;
            }
        }
        if (failed)
            throw std::runtime_error("Failed to find an Ogre GL render system.");

        root->setRenderSystem(selected_render_system);

        selected_render_system->setConfigOption("Full Screen", GetOptionsDB().Get<bool>("fullscreen") ? "Yes" : "No");
        std::string video_mode_str =
            boost::io::str(boost::format("%1% x %2% @ %3%-bit colour") %
                           GetOptionsDB().Get<int>("app-width") %
                           GetOptionsDB().Get<int>("app-height") %
                           GetOptionsDB().Get<int>("color-depth"));
        selected_render_system->setConfigOption("Video Mode", video_mode_str);

        RenderWindow* window = root->initialise(true, "FreeOrion " + FreeOrionVersionString());
        SceneManager* scene_manager = root->createSceneManager("OctreeSceneManager", "SceneMgr");

        Camera* camera = scene_manager->createCamera("Camera");
        // Position it at 500 in Z direction
        camera->setPosition(Vector3(0, 0, 500));
        // Look back along -Z
        camera->lookAt(Vector3(0, 0, -300));
        camera->setNearClipDistance(5);

        Viewport* viewport = window->addViewport(camera);
        viewport->setBackgroundColour(ColourValue(0, 0, 0));

        HumanClientApp app(root, window, scene_manager, camera, viewport);
#ifdef OGRE_STATIC_LIB
        ois_input_plugin = new OISInput;
        cg_plugin = new Ogre::CgPlugin;
        octree_plugin = new Ogre::OctreePlugin;
        particle_fx_plugin = new Ogre::ParticleFXPlugin;
        gl_plugin = new Ogre::GLPlugin;
        root->installPlugin(ois_input_plugin);
        root->installPlugin(cg_plugin);
        root->installPlugin(octree_plugin);
        root->installPlugin(particle_fx_plugin);
        root->installPlugin(gl_plugin);
#else
        root->loadPlugin(OGRE_INPUT_PLUGIN_NAME);
#endif
        app();
    } catch (const HumanClientApp::CleanQuit&) {
        // do nothing
    } catch (const std::invalid_argument& e) {
        Logger().errorStream() << "main() caught exception(std::invalid_arg): " << e.what();
        std::cerr << "main() caught exception(std::invalid_arg): " << e.what() << std::endl;
    } catch (const std::runtime_error& e) {
        Logger().errorStream() << "main() caught exception(std::runtime_error): " << e.what();
        std::cerr << "main() caught exception(std::runtime_error): " << e.what() << std::endl;
    } catch (const  boost::io::format_error& e) {
        Logger().errorStream() << "main() caught exception(boost::io::format_error): " << e.what();
        std::cerr << "main() caught exception(boost::io::format_error): " << e.what() << std::endl;
    } catch (const GG::ExceptionBase& e) {
        Logger().errorStream() << "main() caught exception(" << e.type() << "): " << e.what();
        std::cerr << "main() caught exception(" << e.type() << "): " << e.what() << std::endl;
    } catch (const std::exception& e) {
        Logger().errorStream() << "main() caught exception(std::exception): " << e.what();
        std::cerr << "main() caught exception(std::exception): " << e.what() << std::endl;
    }

    if (root) {
#ifdef OGRE_STATIC_LIB
        root->uninstallPlugin(ois_input_plugin);
        root->uninstallPlugin(cg_plugin);
        root->uninstallPlugin(octree_plugin);
        root->uninstallPlugin(particle_fx_plugin);
        root->uninstallPlugin(gl_plugin);
        delete ois_input_plugin;
        delete cg_plugin;
        delete octree_plugin;
        delete particle_fx_plugin;
        delete gl_plugin;
#else
        root->unloadPlugin(OGRE_INPUT_PLUGIN_NAME);
#endif
        delete root;
    }

    if (log_manager)
        delete log_manager;

    return 0;
}
