#include "Directories.h"

#include "OptionsDB.h"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>

#include <cstdlib>

#ifdef __FreeBSD__
#include <sys/sysctl.h>
#endif

namespace fs = boost::filesystem;

namespace {
    bool g_initialized = false;
    fs::path bin_dir = fs::initial_path();
}

#if defined(FREEORION_MACOSX)

#include <iostream>
#include <sys/param.h>
#include <CoreFoundation/CoreFoundation.h>
#include <mach-o/dyld.h>

/* sets up the directories in the following way:
   localdir: ~/Library/FreeOrion
   globaldir: FreeOrion.app/Contents/Resources
   bindir:  FreeOrion.app/Contents/Executables
   configpath: ~/Library/FreeOrion/config.xml
   pythonhome: FreeOrion.app/Contents/Frameworks/Python.framework/Versions/Current
*/
namespace {
    fs::path   s_user_dir;
    fs::path   s_root_data_dir;
    fs::path   s_bin_dir;
    fs::path   s_config_path;
    fs::path   s_python_home;
}

void InitBinDir(const std::string& argv0);

void InitDirs(const std::string& argv0) {
    if (g_initialized)
        return;

    // store working dir
    fs::initial_path();
    fs::path bundle_path;
    fs::path app_path;

    CFBundleRef bundle = CFBundleGetMainBundle();
    char bundle_dir[MAXPATHLEN];
    
    if (bundle) {
        CFURLRef bundleurl = CFBundleCopyBundleURL(bundle);
        CFURLGetFileSystemRepresentation(bundleurl, true, reinterpret_cast<UInt8*>(bundle_dir), MAXPATHLEN);
    } else {
        // executable is not the main binary in application bundle (i.e. Server or AI)
        uint32_t size = sizeof(bundle_dir);
        if (_NSGetExecutablePath(bundle_dir, &size) != 0) {
            std::cerr << "_NSGetExecutablePath() failed: buffer too small; need size " << size << std::endl;
            exit(-1);
        }
    }
    
    bundle_path = fs::path(bundle_dir);

    // search bundle_path for a directory named "FreeOrion.app", exiting if not found, else constructing a path to application bundle contents
    fs::path::iterator appiter =   std::find(bundle_path.begin(), bundle_path.end(), "FreeOrion.app");
    if (appiter == bundle_path.end()) {
        std::cerr << "Error: Application bundle must be named 'FreeOrion.app' and executables must not be called from outside of it." << std::endl;
        exit(-1);
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
    s_python_home   =   app_path / "Frameworks" / "Python.framework" / "Versions" / "Current";

    fs::path p = s_user_dir;
    if (!exists(p))
        fs::create_directories(p);

    p /= "save";
    if (!exists(p))
        fs::create_directories(p);

    g_initialized = true;
}

const fs::path GetUserDir() {
    if (!g_initialized)
        InitDirs("");
    return s_user_dir;
}

const fs::path GetRootDataDir() {
    if (!g_initialized)
        InitDirs("");
    return s_root_data_dir;
}

const fs::path GetBinDir() {
    if (!g_initialized)
        InitDirs("");
    return s_bin_dir;
}

const fs::path GetPythonHome() {
    if (!g_initialized)
        InitDirs("");
    return s_python_home;
}

#elif defined(FREEORION_LINUX)
#include "binreloc.h"
#include <unistd.h>

void InitBinDir(const std::string& argv0);

void InitDirs(const std::string& argv0) {
    if (g_initialized)
        return;

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
        // get this executable's path by following link
        const size_t BUF_SIZE = 2048;
        char buf[BUF_SIZE] = {0};

#ifdef __FreeBSD__
        int mib[4];
        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC;
        mib[2] = KERN_PROC_PATHNAME;
        mib[3] = -1;
        size_t buf_size = sizeof(buf);
        sysctl(mib, 4, buf, &buf_size, 0, 0);
#else
        size_t exe_path_size = readlink("/proc/self/exe", buf, BUF_SIZE);
        if (exe_path_size == static_cast<size_t>(-1)) {
            problem = true;

        } else {
            if (exe_path_size >= BUF_SIZE || exe_path_size < 0)
                exe_path_size = BUF_SIZE - 1;   // ensure buffer isn't accessed out of range
            buf[exe_path_size] = 0;             // null terminate c-string
        }
#endif

        if (!problem) {
            buf[BUF_SIZE - 1] = 0;              // to be safe, else initializing an std::string with a non-null-terminated string could read invalid data outside the buffer range
            std::string path_text(buf);

            fs::path binary_file = fs::system_complete(fs::path(path_text));
            bin_dir = binary_file.branch_path();

            // check that a "freeorion" file (hopefully the freeorion binary) exists in the found directory
            fs::path p(bin_dir);
            p /= "freeorion";
            if (!exists(p))
                problem = true;
        }

    } catch (fs::filesystem_error err) {
        problem = true;
    }

    if (problem) {
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

    fs::path local_dir = GetUserDir();
    if (!exists(local_dir))
        fs::create_directories(local_dir);

    fs::path p(GetSaveDir());
    if (!exists(p))
        fs::create_directories(p);

    InitBinDir(argv0);

    g_initialized = true;
}

const fs::path GetUserDir() {
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

const fs::path GetResourceDir() {
    // if resource dir option has been set, use specified location.  otherwise,
    // use default location
    std::string options_resource_dir = GetOptionsDB().Get<std::string>("resource-dir");
    if (!options_resource_dir.empty() && fs::is_directory(fs::path(options_resource_dir)) && fs::exists(fs::path(options_resource_dir)))
        return fs::path(options_resource_dir);

    return fs::path(GetOptionsDB().GetDefault<std::string>("resource-dir"));
}

const fs::path GetConfigPath() {
    static const fs::path p = GetUserDir() / "config.xml";
    return p;
}

const fs::path GetSaveDir() {
    // if save dir option has been set, use specified location.  otherwise,
    // use default location
    std::string options_save_dir = GetOptionsDB().Get<std::string>("save-dir");
    if (!options_save_dir.empty())
        return fs::path(options_save_dir);

    return fs::path(GetOptionsDB().GetDefault<std::string>("save-dir"));
}

fs::path RelativePath(const fs::path& from, const fs::path& to) {
    fs::path retval;
#if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
    fs::path from_abs = fs::absolute(from);
    fs::path to_abs = fs::absolute(to);
#else
    fs::path from_abs = fs::complete(from);
    fs::path to_abs = fs::complete(to);
#endif
    fs::path::iterator from_it = from_abs.begin();
    fs::path::iterator end_from_it = from_abs.end();
    fs::path::iterator to_it = to_abs.begin();
    fs::path::iterator end_to_it = to_abs.end();
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
