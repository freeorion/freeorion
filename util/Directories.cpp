#include "Directories.h"

#include "OptionsDB.h"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>

#include <cstdlib>


namespace fs = boost::filesystem;

namespace {
    bool g_initialized = false;
    fs::path bin_dir = fs::initial_path();
}

#if defined(FREEORION_MACOSX)

#include <iostream>
#include <sys/param.h>
#include <CoreFoundation/CoreFoundation.h>

/* sets up the directories in the following way:
   localdir: ~/Library/FreeOrion
   globaldir: FreeOrion.app/Contents/Resources
   bindir:  FreeOrion.app/Contents/Executables
   configpath: ~/Library/FreeOrion/config.xml */
namespace {
    fs::path   s_user_dir;
    fs::path   s_root_data_dir;
    fs::path   s_bin_dir;
    fs::path   s_config_path;
}

void InitBinDir(const std::string& argv0);

void InitDirs(const std::string& argv0)
{
    if (g_initialized)
        return;

    if (fs::path::default_name_check_writable())
        fs::path::default_name_check(fs::native);

    // store working dir
    fs::initial_path();
    fs::path bundle_path;
    fs::path app_path;

    CFBundleRef bundle =    CFBundleGetMainBundle();
    char bundle_dir[MAXPATHLEN];
    CFURLRef    bundleurl   =   CFBundleCopyBundleURL(bundle);
    CFURLGetFileSystemRepresentation(bundleurl, true, reinterpret_cast<UInt8*>(bundle_dir), MAXPATHLEN);

    bundle_path                =   fs::path(bundle_dir);
    fs::path::iterator appiter =   std::find(bundle_path.begin(), bundle_path.end(), "FreeOrion.app");
    if (appiter == bundle_path.end()) { // uncomfortable case. search
#ifndef FREEORION_BUILD_RELEASE
        fs::directory_iterator dir_end;
        // search for Resource directories && executables
        bool foundexec = false,
             foundresources = false;
        for (fs::directory_iterator diter(bundle_path); diter != dir_end; ++diter) {
            if ((*diter).filename() == "Executables")
                foundexec = true;
            else if ((*diter).filename() == "Resources")
                foundresources = true;
        }
        // if nothing found. assume that we are in the Executables directory which may be the case during debugging
        if (!(foundexec && foundresources)) {
            // in this case we are either the server or the ai client
            fs::path::iterator  appiter =   std::find(bundle_path.begin(), bundle_path.end(), "Executables");
            if (appiter != bundle_path.end()) {
                for (fs::path::iterator piter  = bundle_path.begin(); piter != appiter; ++piter)
                    app_path /= *piter;

                foundresources = foundexec = true;
            }
        }

        if (!(foundexec && foundresources))
#endif
        {
            std::cerr << "Error: Do not call executable from outside of the applications bundle!" << std::endl;
            exit(-1);
        }
        app_path = bundle_path;
    } else {
        for (fs::path::iterator piter = bundle_path.begin(); piter != appiter; ++piter) {
            app_path /= *piter;
        }
        app_path /= "FreeOrion.app/Contents";
    }

    s_root_data_dir =   app_path / "Resources";
    s_user_dir      =   fs::path(getenv("HOME")) / "Library" / "Application Support" / "FreeOrion";
    s_bin_dir       =   app_path / "Executables";
    s_config_path   =   s_user_dir / "config.xml";

    fs::path p = s_user_dir;
    if (!exists(p))
        fs::create_directories(p);

    p /= "save";
    if (!exists(p))
        fs::create_directories(p);

    g_initialized = true;
}

const fs::path GetUserDir()
{
    if (!g_initialized)
        InitDirs();
    return s_user_dir;
}

const fs::path GetRootDataDir()
{
    if (!g_initialized)
        InitDirs();
    return s_root_data_dir;
}

const fs::path GetBinDir()
{
    if (!g_initialized)
        InitDirs();
    return s_bin_dir;
}

#elif defined(FREEORION_LINUX)
#include "binreloc.h"

void InitBinDir(const std::string& argv0);

void InitDirs(const std::string& argv0) {
    if (g_initialized)
        return;

    if (fs::path::default_name_check_writable())
        fs::path::default_name_check(fs::native);

    /* store working dir.  some implimentations get the value of initial_path
     * from the value of current_path the first time initial_path is called,
     * so it is necessary to call initial_path as soon as possible after
     * starting the program, so that current_path doesn't have a chance to
     * change before initial_path is initialized. */
    fs::initial_path();

    br_init(0);

    fs::path p = GetUserDir();
    if (!exists(p)) {
        fs::create_directories(p);
    }
    
    p /= "save";
    if (!exists(p)) {
        fs::create_directories(p);
    }

    InitBinDir(argv0);

    g_initialized = true;
}

const fs::path GetUserDir() {
    if (fs::path::default_name_check_writable())
        fs::path::default_name_check(fs::native);
    static fs::path p = fs::path(getenv("HOME")) / ".freeorion";
    return p;
}

const fs::path GetRootDataDir() {
    if (!g_initialized) InitDirs("");
    char* dir_name = br_find_data_dir("/usr/local/share");
    fs::path p(dir_name);
    std::free(dir_name);
    p /= "freeorion";
    // if the path does not exist, we fall back to the working directory
    if (!exists(p)) {
        return fs::initial_path();
    } else {
        return p;
    }
}

const fs::path GetBinDir() {
    if (!g_initialized) InitDirs("");
    return bin_dir;
}

void InitBinDir(const std::string& argv0) {
    bool problem = false;
    try {
        fs::path binary_file = fs::system_complete(fs::path(argv0));
        bin_dir = binary_file.branch_path();
    } catch (fs::filesystem_error err) {
        problem = true;
    }

    if (problem || !exists(bin_dir)) {
        // failed trying to parse the call path, so try hard-coded standard location...
        char* dir_name = br_find_bin_dir("/usr/local/bin");
        fs::path p(dir_name);
        std::free(dir_name);

        // if the path does not exist, fall back to the working directory
        if (!exists(p)) {
            bin_dir = fs::initial_path();
        } else {
            bin_dir = p;
        }
    }
}

#elif defined(FREEORION_WIN32)

void InitBinDir(const std::string& argv0);

void InitDirs(const std::string& argv0) {
    if (g_initialized)
        return;

    fs::path::default_name_check(fs::native);

    fs::path local_dir = GetUserDir();
    if (!exists(local_dir))
        fs::create_directories(local_dir);

    fs::path p = GetUserDir() / "save";
    if (!exists(p))
        fs::create_directories(p);

    InitBinDir(argv0);

    g_initialized = true;
}

const fs::path GetUserDir() {
    if (fs::path::default_name_check_writable())
        fs::path::default_name_check(fs::native);
    static fs::path p = fs::path(std::string(getenv("APPDATA"))) / "FreeOrion";
    return p;
}

const fs::path GetRootDataDir() {
    return fs::initial_path();
}

const fs::path GetBinDir() {
    if (!g_initialized) InitDirs("");
    return bin_dir;
}

void InitBinDir(const std::string& argv0) {
    try {
        fs::path binary_file = fs::system_complete(fs::path(argv0));
        bin_dir = binary_file.branch_path();
    } catch (fs::filesystem_error err) {
        bin_dir = fs::initial_path();
    }
}

#else
#  error Neither FREEORION_LINUX nor FREEORION_WIN32 set
#endif

const boost::filesystem::path GetResourceDir() {
    return fs::path(GetOptionsDB().Get<std::string>("resource-dir"));
}

const fs::path GetConfigPath() {
    static const fs::path p = GetUserDir() / "config.xml";
    return p;
}

boost::filesystem::path RelativePath(const boost::filesystem::path& from, const boost::filesystem::path& to) {
    boost::filesystem::path retval;
    boost::filesystem::path from_abs = boost::filesystem::complete(from);
    boost::filesystem::path to_abs = boost::filesystem::complete(to);
    boost::filesystem::path::iterator from_it = from_abs.begin();
    boost::filesystem::path::iterator end_from_it = from_abs.end();
    boost::filesystem::path::iterator to_it = to_abs.begin();
    boost::filesystem::path::iterator end_to_it = to_abs.end();
    while (from_it != end_from_it && to_it != end_to_it && *from_it == *to_it) {
        ++from_it;
        ++to_it;
    }
    for (; from_it != end_from_it; ++from_it) {
        retval /= "..";
    }
    for (; to_it != end_to_it; ++to_it) {
        retval /= *to_it;
    }
    return retval;
}
