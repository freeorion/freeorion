#include "Directories.h"

#include "OptionsDB.h"

#include <GG/utf8/checked.h>
#include "../universe/Enums.h"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

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
    auto appiter = std::find(bundle_path.begin(), bundle_path.end(), "FreeOrion.app");
    if (appiter == bundle_path.end()) {
        std::cerr << "Error: Application bundle must be named 'FreeOrion.app' and executables must not be called from outside of it." << std::endl;
        exit(-1);
    } else {
        for (auto piter = bundle_path.begin(); piter != appiter; ++piter) {
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

    // Intentionally do not create the server save dir.
    // The server save dir is publically accessible and should not be
    // automatically created for the user.

    g_initialized = true;
}

const fs::path GetUserConfigDir() {
    if (!g_initialized)
        InitDirs("");
    return s_user_dir;
}

const fs::path GetUserDataDir() {
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

#elif defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD)
#include "binreloc.h"
#include <unistd.h>
#include <boost/filesystem/fstream.hpp>

namespace {
    /// Copy directory from to directory to only to a depth of safe_depth
    void copy_directory_safe(fs::path from, fs::path to, int safe_depth) {
        if (safe_depth < 0)
            return;

        fs::copy(from, to);
        fs::directory_iterator it(from);
        while (it != fs::directory_iterator()) {
            const fs::path p = *it++;
            if (fs::is_directory(p)) {
                copy_directory_safe(p, to / p.filename(), safe_depth - 1);
            } else {
                fs::copy(p, to / p.filename());
            }
        }
    }

    /** If the old configuration directory exists, but neither
        the XDG_CONFIG_DIR nor the XDG_DATA_DIR exist then
        copy the config and data files and inform the user.

        It also updates the data dir in the config.xml and persisten_config.xml files.
     */
    void MigrateOldConfigDirsToXDGLocation() {
        const fs::path old_path = fs::path(getenv("HOME")) / ".freeorion";
        const fs::path config_path = GetUserConfigDir();
        const fs::path data_path = GetUserDataDir();

        bool dont_migrate = !exists(old_path) || exists(config_path) || exists(data_path);
        if (dont_migrate)
            return;

        std::stringstream msg;
        msg << "Freeorion added support for the XDG Base Directory Specification." << std::endl << std::endl
            << "Configuration files and data were migrated from:" << std::endl
            << old_path << std::endl << std::endl
            << "Configuration were files copied to:" << std::endl << config_path << std::endl << std::endl
            << "Data Files were copied to:" << std::endl << data_path << std::endl << std::endl
            << "If your save.path option in persistent_config.xml was ~/.config, then you need to update it."
            << std::endl;

        try {
            fs::create_directories(config_path);
            fs::create_directories(data_path);

            const fs::path old_config_file = old_path / "config.xml";
            const fs::path old_persistent_file = old_path / "persistent_config.xml";

            if (exists(old_config_file))
                fs::copy(old_config_file, config_path / old_config_file.filename());
            if (exists(old_persistent_file))
                fs::copy(old_persistent_file, config_path / old_persistent_file.filename());

            fs::directory_iterator it(old_path);
            while (it != fs::directory_iterator()) {
                const fs::path p = *it++;
                if (p == old_config_file || p == old_persistent_file)
                    continue;

                if (fs::is_directory(p)) {
                    int arbitrary_safe_depth = 6;
                    copy_directory_safe(p, data_path / p.filename(), arbitrary_safe_depth);
                } else {
                    fs::copy(p, data_path / p.filename());
                }
            }

            //Start update of save.path in config file and complete it in CompleteXDGMigration()
            fs::path sentinel = GetUserDataDir() / "MIGRATION_TO_XDG_IN_PROGRESS";
            if (!exists(sentinel)) {
                fs::ofstream touchfile(sentinel);
                touchfile << " ";
            }

            fs::ofstream msg_file(old_path / "MIGRATION.README");
            msg_file << msg.str() << std::endl
                     << "You can delete this file it is a one time message." << std::endl << std::endl;

        } catch(fs::filesystem_error const & e) {
            std::cerr << "Error: Unable to migrate files from old config dir" << std::endl
                      << old_path << std::endl
                      << " to new XDG specified config dir" << std::endl << config_path << std::endl
                      << " and data dir" << std::endl << data_path << std::endl
                      << " because " << e.what()  << std::endl;
            throw;
        }

        std::cout << msg.str();
    }

}

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

    br_init(nullptr);

    MigrateOldConfigDirsToXDGLocation();

    fs::path cp = GetUserConfigDir();
    if (!exists(cp)) {
        fs::create_directories(cp);
    }

    fs::path p = GetUserDataDir();
    if (!exists(p)) {
        fs::create_directories(p);
    }

    p /= "save";
    if (!exists(p)) {
        fs::create_directories(p);
    }

    // Intentionally do not create the server save dir.
    // The server save dir is publically accessible and should not be
    // automatically created for the user.

    InitBinDir(argv0);

    g_initialized = true;
}

const fs::path GetUserConfigDir() {
    static fs::path p = getenv("XDG_CONFIG_HOME")
        ? fs::path(getenv("XDG_CONFIG_HOME")) / "freeorion"
        : fs::path(getenv("HOME")) / ".config" / "freeorion";
    return p;
}

const fs::path GetUserDataDir() {
    static fs::path p = getenv("XDG_DATA_HOME")
        ? fs::path(getenv("XDG_DATA_HOME")) / "freeorion"
        : fs::path(getenv("HOME")) / ".local" / "share" / "freeorion";
    return p;
}

const fs::path GetRootDataDir() {
    if (!g_initialized) InitDirs("");
    char* dir_name = br_find_data_dir(SHAREPATH);
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
        char buf[2048] = {'\0'};

#ifdef __FreeBSD__
        int mib[4];
        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC;
        mib[2] = KERN_PROC_PATHNAME;
        mib[3] = -1;
        size_t buf_size = sizeof(buf);
        sysctl(mib, 4, buf, &buf_size, 0, 0);
#elif defined(__OpenBSD__)
        // OpenBSD does not have executable path's retrieval feature
        std::string argpath(argv0);
        boost::erase_all(argpath, "\"");
        if (argpath[0] != '/')
            problem = (nullptr == realpath(argpath.c_str(), buf));
        else
            strncpy(buf, argpath.c_str(), sizeof(buf));
#else
        problem = (-1 == readlink("/proc/self/exe", buf, sizeof(buf) - 1));
#endif

        if (!problem) {
            buf[sizeof(buf) - 1] = '\0';              // to be safe, else initializing an std::string with a non-null-terminated string could read invalid data outside the buffer range
            std::string path_text(buf);

            fs::path binary_file = fs::system_complete(fs::path(path_text));
            bin_dir = binary_file.branch_path();

            // check that a "freeoriond" file (hopefully the freeorion server binary) exists in the found directory
            fs::path p(bin_dir);
            p /= "freeoriond";
            if (!exists(p))
                problem = true;
        }

    } catch (fs::filesystem_error err) {
        problem = true;
    }

    if (problem) {
        // failed trying to parse the call path, so try hard-coded standard location...
        char* dir_name = br_find_bin_dir(BINPATH);
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

    fs::path local_dir = GetUserConfigDir();
    if (!exists(local_dir))
        fs::create_directories(local_dir);

    fs::path p(GetSaveDir());
    if (!exists(p))
        fs::create_directories(p);

    // Intentionally do not create the server save dir.
    // The server save dir is publically accessible and should not be
    // automatically created for the user.

    InitBinDir(argv0);

    g_initialized = true;
}

const fs::path GetUserConfigDir() {
    static fs::path p = fs::path(std::wstring(_wgetenv(L"APPDATA"))) / "FreeOrion";
    return p;
}

const fs::path GetUserDataDir() {
    static fs::path p = fs::path(std::wstring(_wgetenv(L"APPDATA"))) / "FreeOrion";
    return p;
}

const fs::path GetRootDataDir()
{ return fs::initial_path(); }

const fs::path GetBinDir() {
    if (!g_initialized) InitDirs("");
    return bin_dir;
}

const fs::path GetPythonHome()
{ return GetBinDir(); }

void InitBinDir(const std::string& argv0) {
    try {
        fs::path binary_file = fs::system_complete(FilenameToPath(argv0));
        bin_dir = binary_file.branch_path();
    } catch (const fs::filesystem_error &) {
        bin_dir = fs::initial_path();
    }
}

#else
#  error Neither FREEORION_LINUX, FREEORION_FREEBSD, FREEORION_OPENBSD nor FREEORION_WIN32 set
#endif

void CompleteXDGMigration() {
    fs::path sentinel = GetUserDataDir() / "MIGRATION_TO_XDG_IN_PROGRESS";
    if (exists(sentinel)) {
        fs::remove(sentinel);
        // Update data dir in config file
        const std::string options_save_dir = GetOptionsDB().Get<std::string>("save.path");
        const fs::path old_path = fs::path(getenv("HOME")) / ".freeorion";
        if (fs::path(options_save_dir) == old_path)
            GetOptionsDB().Set<std::string>("save.path", GetUserDataDir().string());
    }
}

const fs::path GetResourceDir() {
    // if resource dir option has been set, use specified location. otherwise,
    // use default location
    std::string options_resource_dir = GetOptionsDB().Get<std::string>("resource.path");
    fs::path dir = FilenameToPath(options_resource_dir);
    if (fs::exists(dir) && fs::is_directory(dir))
        return dir;

    dir = GetOptionsDB().GetDefault<std::string>("resource.path");
    if (!fs::is_directory(dir) || !fs::exists(dir))
        dir = FilenameToPath(GetOptionsDB().GetDefault<std::string>("resource.path"));

    return dir;
}

const fs::path GetConfigPath() {
    static const fs::path p = GetUserConfigDir() / "config.xml";
    return p;
}

const fs::path GetPersistentConfigPath() {
    static const fs::path p = GetUserConfigDir() / "persistent_config.xml";
    return p;
}

const fs::path GetSaveDir() {
    // if save dir option has been set, use specified location.  otherwise,
    // use default location
    std::string options_save_dir = GetOptionsDB().Get<std::string>("save.path");
    if (options_save_dir.empty())
        options_save_dir = GetOptionsDB().GetDefault<std::string>("save.path");
    return FilenameToPath(options_save_dir);
}

const fs::path GetServerSaveDir() {
    // if server save dir option has been set, use specified location.  otherwise,
    // use default location
    std::string options_save_dir = GetOptionsDB().Get<std::string>("save.server.path");
    if (options_save_dir.empty())
        options_save_dir = GetOptionsDB().GetDefault<std::string>("save.server.path");
    return FilenameToPath(options_save_dir);
}

fs::path RelativePath(const fs::path& from, const fs::path& to) {
    fs::path retval;
    fs::path from_abs = fs::absolute(from);
    fs::path to_abs = fs::absolute(to);
    auto from_it = from_abs.begin();
    auto end_from_it = from_abs.end();
    auto to_it = to_abs.begin();
    auto end_to_it = to_abs.end();
    while (from_it != end_from_it && to_it != end_to_it && *from_it == *to_it) {
        ++from_it;
        ++to_it;
    }
    for (; from_it != end_from_it; ++from_it)
    { retval /= ".."; }
    for (; to_it != end_to_it; ++to_it)
    { retval /= *to_it; }
    return retval;
}

#if defined(FREEORION_WIN32)

std::string PathToString(const fs::path& path) {
    fs::path::string_type native_string = path.generic_wstring();
    std::string retval;
    utf8::utf16to8(native_string.begin(), native_string.end(), std::back_inserter(retval));
    return retval;
}

fs::path FilenameToPath(const std::string& path_str) {
    // convert UTF-8 directory string to UTF-16
    boost::filesystem::path::string_type directory_native;
    utf8::utf8to16(path_str.begin(), path_str.end(), std::back_inserter(directory_native));
#if (BOOST_VERSION >= 106300)
    return fs::path(directory_native).generic_path();
#else
    return fs::path(directory_native);
#endif
}

#else // defined(FREEORION_WIN32)

std::string PathToString(const fs::path& path)
{ return path.string(); }

fs::path FilenameToPath(const std::string& path_str)
{ return fs::path(path_str); }

#endif // defined(FREEORION_WIN32)

std::string FilenameTimestamp() {
    boost::posix_time::time_facet* facet = new boost::posix_time::time_facet("%Y%m%d_%H%M%S");
    std::stringstream date_stream;
    date_stream.imbue(std::locale(date_stream.getloc(), facet));
    date_stream << boost::posix_time::microsec_clock::local_time();
    return date_stream.str();
}

/**  \brief Return a vector of absolute paths to files in the given path
 *
 * @param[in] path relative or absolute directory (searched recursively)
 * @return Any regular files in
 * @return  if absolute directory: path
 * @return  if relative directory: GetResourceDir() / path
*/
std::vector<fs::path> ListDir(const fs::path& path) {
    std::vector<fs::path> retval;
    bool is_rel = path.is_relative();
    if (!is_rel && (fs::is_empty(path) || !fs::is_directory(path))) {
        DebugLogger() << "ListDir: File " << PathToString(path) << " was not included as it is empty or not a directoy";
    } else {
        const fs::path& default_path = is_rel ? GetResourceDir() / path : path;

        for (fs::recursive_directory_iterator dir_it(default_path);
             dir_it != fs::recursive_directory_iterator(); ++dir_it)
        {
            if (fs::is_regular_file(dir_it->status())) {
                retval.push_back(dir_it->path());
            } else if (!fs::is_directory(dir_it->status())) {
                TraceLogger() << "Parse: Unknown file not included: " << PathToString(dir_it->path());
            }
        }
    }

    if (retval.empty()) {
        DebugLogger() << "ListDir: No files found for " << path.string();
    }

    return retval;
}

bool IsValidUTF8(const std::string& in)
{ return utf8::is_valid(in.begin(), in.end()); }

bool IsInDir(const fs::path& dir, const fs::path& test_dir) {
    if (!fs::exists(dir) || !fs::is_directory(dir))
        return false;

    if (fs::exists(test_dir) && !fs::is_directory(test_dir))
        return false;

    // Resolve any symbolic links, dots or dot-dots
    auto canon_dir = fs::canonical(dir);
    // Don't resolve path if directory doesn't exist
    // TODO: Change to fs::weakly_canonical after bump boost version above 1.60
    auto canon_path = test_dir;
    if (fs::exists(test_dir))
        canon_path = fs::canonical(test_dir);

    // Paths shorter than dir are not in dir
    auto dir_length = std::distance(canon_dir.begin(), canon_dir.end());
    auto path_length = std::distance(canon_path.begin(), canon_path.end());
    if (path_length < dir_length)
        return false;

    // Check that the whole dir path matches the test path
    // Extra portions of path are contained in dir
    return std::equal(canon_dir.begin(), canon_dir.end(), canon_path.begin());
}

fs::path GetPath(PathType path_type) {
    switch (path_type) {
    case PATH_BINARY:
        return GetBinDir();
    case PATH_RESOURCE:
        return GetResourceDir();
    case PATH_DATA_ROOT:
        return GetRootDataDir();
    case PATH_DATA_USER:
        return GetUserDataDir();
    case PATH_CONFIG:
        return GetUserConfigDir();
    case PATH_SAVE:
        return GetSaveDir();
    case PATH_TEMP:
        return fs::temp_directory_path();
    case PATH_PYTHON:
#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32)
        return GetPythonHome();
#endif
    case PATH_INVALID:
    default:
        ErrorLogger() << "Invalid path type " << path_type;
        return fs::temp_directory_path();
    }
}

fs::path GetPath(const std::string& path_string) {
    if (path_string.empty()) {
        ErrorLogger() << "GetPath called with empty argument";
        return fs::temp_directory_path();
    }

    PathType path_type;
    try {
        path_type = boost::lexical_cast<PathType>(path_string);
    } catch (const boost::bad_lexical_cast& ec) {
        // try partial match
        std::string retval = path_string;
        for (const auto& path_type_str : PathTypeStrings()) {
            std::string path_type_string = PathToString(GetPath(path_type_str));
            boost::replace_all(retval, path_type_str, path_type_string);
        }
        if (path_string != retval) {
            return FilenameToPath(retval);
        } else {
            ErrorLogger() << "Invalid cast for PathType from string " << path_string;
            return fs::temp_directory_path();
        }
    }
    return GetPath(path_type);
}

bool IsExistingFile(const fs::path& path) {
    try {
        auto stat = fs::status(path);
        return fs::exists(stat) && fs::is_regular_file(stat);
    } catch(boost::filesystem::filesystem_error& ec) {
        ErrorLogger() << "Filesystem error during stat of " << PathToString(path) << " : " << ec.what();
    }

    return false;
}

std::vector<fs::path> PathsInDir(const boost::filesystem::path& abs_dir_path,
                                 std::function<bool (const fs::path&)> pred,
                                 bool recursive_search)
{
    std::vector<fs::path> retval;
    if (abs_dir_path.is_relative()) {
        ErrorLogger() << "Passed relative path for fileysstem operation " << PathToString(abs_dir_path);
        return retval;
    }

    try {
        auto dir_stat = fs::status(abs_dir_path);
        if (!fs::exists(dir_stat) || !fs::is_directory(dir_stat)) {
            ErrorLogger() << "Path is not an existing directory " << PathToString(abs_dir_path);
            return retval;
        }

        if (recursive_search) {
            using dir_it_type = boost::filesystem::recursive_directory_iterator;
            for (dir_it_type node_it(abs_dir_path); node_it != dir_it_type(); ++node_it) {
                auto obj_path = node_it->path();
                if (pred(obj_path))
                    retval.push_back(obj_path);
            }
        } else {
            using dir_it_type = boost::filesystem::directory_iterator;
            for (dir_it_type node_it(abs_dir_path); node_it != dir_it_type(); ++node_it) {
                auto obj_path = node_it->path();
                if (pred(obj_path))
                    retval.push_back(obj_path);
            }
        }
    } catch(const fs::filesystem_error& ec) {
        ErrorLogger() << "Filesystem error during directory traversal " << PathToString(abs_dir_path)
                      << " : " << ec.what();
        return {};
    }

    return retval;
}
