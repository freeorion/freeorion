#include "Directories.h"

#include "OptionsDB.h"
#include "i18n.h"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <cstdlib>
#include <mutex>

#if defined(FREEORION_WIN32)
#  include <codecvt>
#  include <locale>
#endif

#if defined(FREEORION_MACOSX)
#  include <iostream>
#  include <sys/param.h>
#  include <mach-o/dyld.h>
#  include <CoreFoundation/CoreFoundation.h>
#endif

#if defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD) || defined(FREEORION_HAIKU) || defined(FREEORION_ANDROID)
#include "binreloc.h"
#include <unistd.h>
#include <boost/filesystem/fstream.hpp>

#  ifdef __FreeBSD__
#    include <sys/sysctl.h>
#  endif
#  ifdef __HAIKU__
#    include <FindDirectory.h>
#  endif
#endif

# if defined(FREEORION_WIN32) || defined(FREEORION_MACOSX) || defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD) || defined(FREEORION_HAIKU) || defined(FREEORION_ANDROID)
# else
#  error Neither FREEORION_LINUX, FREEORION_MACOSX, FREEORION_FREEBSD, FREEORION_OPENBSD, FREEORION_WIN32, FREEORION_HAIKU nor FREEORION_ANDROID set
#endif


namespace fs = ::boost::filesystem;

namespace {
    bool g_initialized = false;
    fs::path bin_dir = fs::initial_path();

    const std::string EMPTY_STRING = "";
    const std::string PATH_BINARY_STR = "PATH_BINARY";
    const std::string PATH_RESOURCE_STR = "PATH_RESOURCE";
    const std::string PATH_DATA_ROOT_STR = "PATH_DATA_ROOT";
    const std::string PATH_DATA_USER_STR = "PATH_DATA_USER";
    const std::string PATH_CONFIG_STR = "PATH_CONFIG";
    const std::string PATH_SAVE_STR = "PATH_SAVE";
    const std::string PATH_TEMP_STR = "PATH_TEMP";
    const std::string PATH_PYTHON_STR = "PATH_PYTHON";
    const std::string PATH_INVALID_STR = "PATH_INVALID";

#if defined(FREEORION_MACOSX)
    fs::path   s_user_dir;
    fs::path   s_root_data_dir;
    fs::path   s_bin_dir;
    //! pythonhome: FreeOrion.app/Contents/Frameworks/Python.framework/Versions/{PythonMajor}.{PythonMinor}
    fs::path   s_python_home;
#endif

#if defined(FREEORION_ANDROID)
    JNIEnv*    s_jni_env;
    fs::path   s_user_dir;
#endif

#if defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD) || defined(FREEORION_HAIKU)
    //! Copy directory from to directory to only to a depth of safe_depth
    void copy_directory_safe(fs::path from, fs::path to, int safe_depth)
    {
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

    //! Migrate existing old FreeOrion configuration directory to XDG Base Dir
    //!
    //! If the old configuration directory exists, but neither
    //! the XDG_CONFIG_DIR nor the XDG_DATA_DIR exist then copy the config and
    //! data files and inform the user.
    //!
    //! It also updates the data dir in the config.xml and persisten_config.xml files.
    void MigrateOldConfigDirsToXDGLocation()
    {
        const fs::path old_path = fs::path(getenv("HOME")) / ".freeorion";
        const fs::path config_path = GetUserConfigDir();
        const fs::path data_path = GetUserDataDir();

        bool dont_migrate = !exists(old_path) || exists(config_path) || exists(data_path);
        if (dont_migrate)
            return;

        std::stringstream msg;
        msg << "Freeorion added support for the XDG Base Directory Specification.\n\n"
            << "Configuration files and data were migrated from:\n"
            << old_path << "\n\n"
            << "Configuration were files copied to:\n"
            << config_path << "\n\n"
            << "Data Files were copied to:\n" << data_path << "\n\n"
            << "If your save.path option in persistent_config.xml was ~/.config, then you need to update it.\n";

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
            msg_file << msg.str() << "\n"
                     << "You can delete this file it is a one time message.\n\n";

        } catch(fs::filesystem_error const & e) {
            std::cerr << "Error: Unable to migrate files from old config dir\n"
                      << old_path << "\n"
                      << " to new XDG specified config dir\n"
                      << config_path << "\n"
                      << " and data dir\n"
                      << data_path << "\n"
                      << " because " << e.what() << "\n";
            throw;
        }

        std::cout << msg.str();
    }
#endif
}

auto PathTypeToString(PathType path_type) -> std::string const&
{
    switch (path_type) {
        case PathType::PATH_BINARY:    return PATH_BINARY_STR;
        case PathType::PATH_RESOURCE:  return PATH_RESOURCE_STR;
        case PathType::PATH_PYTHON:    return PATH_PYTHON_STR;
        case PathType::PATH_DATA_ROOT: return PATH_DATA_ROOT_STR;
        case PathType::PATH_DATA_USER: return PATH_DATA_USER_STR;
        case PathType::PATH_CONFIG:    return PATH_CONFIG_STR;
        case PathType::PATH_SAVE:      return PATH_SAVE_STR;
        case PathType::PATH_TEMP:      return PATH_TEMP_STR;
        case PathType::PATH_INVALID:   return PATH_INVALID_STR;
        default:                       return EMPTY_STRING;
    }
}

auto PathTypeStrings() -> std::vector<std::string> const&
{
    static std::vector<std::string> path_type_list;
    if (path_type_list.empty()) {
        path_type_list.reserve(10); // should be enough
        for (auto path_type = PathType(0); path_type < PathType::PATH_INVALID;
             path_type = PathType(int(path_type) + 1))
        {
            // PATH_PYTHON is only valid for FREEORION_WIN32 or FREEORION_MACOSX
#if defined(FREEORION_LINUX)
            if (path_type == PathType::PATH_PYTHON)
                continue;
#endif
            path_type_list.emplace_back(PathTypeToString(path_type));
        }
    }
    return path_type_list;
}

void InitBinDir(std::string const& argv0)
{
#if defined(FREEORION_WIN32)
    try {
        fs::path binary_file = fs::system_complete(FilenameToPath(argv0));
        bin_dir = binary_file.branch_path();
    } catch (const fs::filesystem_error &) {
        bin_dir = fs::initial_path();
    }
#elif defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD) || defined(FREEORION_HAIKU)
    bool problem = false;
    try {
        // get this executable's path by following link
        char buf[2048] = {'\0'};

#if defined(FREEORION_LINUX)
        problem = (-1 == readlink("/proc/self/exe", buf, sizeof(buf) - 1));
#endif
#if defined(FREEORION_FREEBSD)
        int mib[4];
        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC;
        mib[2] = KERN_PROC_PATHNAME;
        mib[3] = -1;
        size_t buf_size = sizeof(buf);
        sysctl(mib, 4, buf, &buf_size, 0, 0);
#endif
#if defined(FREEORION_OPENBSD)
        // OpenBSD does not have executable path's retrieval feature
        std::string argpath(argv0);
        boost::erase_all(argpath, "\"");
        if (argpath[0] != '/')
            problem = (nullptr == realpath(argpath.c_str(), buf));
        else
            strncpy(buf, argpath.c_str(), sizeof(buf));
#endif
#if defined(FREEORION_HAIKU)
        problem = (B_OK != find_path(B_APP_IMAGE_SYMBOL, B_FIND_PATH_IMAGE_PATH, NULL, buf, sizeof(buf)));
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

    } catch (...) {
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
#elif defined(FREEORION_MACOSX) || defined(FREEORION_ADROID)
    // no binary directory setup required.
#endif
}

void InitDirs(std::string const& argv0)
{
    if (g_initialized)
        return;

#if defined(FREEORION_MACOSX)
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
    s_python_home   =   app_path / "Frameworks" / "Python.framework" / "Versions" / FREEORION_PYTHON_VERSION;

    fs::path p = s_user_dir;
    if (!exists(p))
        fs::create_directories(p);

    p /= "save";
    if (!exists(p))
        fs::create_directories(p);

    // Intentionally do not create the server save dir.
    // The server save dir is publically accessible and should not be
    // automatically created for the user.
#elif defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD) || defined(FREEORION_HAIKU)
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
#elif defined(FREEORION_WIN32)
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
#elif defined(FREEORION_ANDROID)
    // ToDo: Move here directory initialization
#endif

    g_initialized = true;
}

auto GetUserConfigDir() -> fs::path const
{
#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32) || defined(FREEORION_ANDROID)
    return GetUserDataDir();
#elif defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD) || defined(FREEORION_HAIKU)
    static fs::path p = getenv("XDG_CONFIG_HOME")
        ? fs::path(getenv("XDG_CONFIG_HOME")) / "freeorion"
        : fs::path(getenv("HOME")) / ".config" / "freeorion";
    return p;
#endif
}

auto GetUserDataDir() -> fs::path const
{
#if defined(FREEORION_MACOSX) || defined(FREEORION_ANDROID)
    if (!g_initialized)
        InitDirs("");
    return s_user_dir;
#elif defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD) || defined(FREEORION_HAIKU) || defined(FREEORION_ANDROID)
    static fs::path p = getenv("XDG_DATA_HOME")
        ? fs::path(getenv("XDG_DATA_HOME")) / "freeorion"
        : fs::path(getenv("HOME")) / ".local" / "share" / "freeorion";
    return p;
#elif defined(FREEORION_WIN32)
    static fs::path p = fs::path(std::wstring(_wgetenv(L"APPDATA"))) / "FreeOrion";
    return p;
#endif
}

auto GetRootDataDir() -> fs::path const
{
#if defined(FREEORION_MACOSX)
    if (!g_initialized)
        InitDirs("");
    return s_root_data_dir;
#elif defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD) || defined(FREEORION_HAIKU)
    if (!g_initialized)
        InitDirs("");
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
#elif defined(FREEORION_WIN32) || defined(FREEORION_ANDROID)
    return fs::initial_path();
#endif
}

auto GetBinDir() -> fs::path const
{
#if defined(FREEORION_MACOSX)
    if (!g_initialized)
        InitDirs("");
    return s_bin_dir;
#elif defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD) || defined(FREEORION_HAIKU) || defined(FREEORION_ANDROID)
    if (!g_initialized)
        InitDirs("");
    return bin_dir;
#elif defined(FREEORION_WIN32)
    if (!g_initialized)
        InitDirs("");
    return bin_dir;
#endif
}

#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32)
auto GetPythonHome() -> fs::path const
{
#if defined(FREEORION_MACOSX)
    if (!g_initialized)
        InitDirs("");
    return s_python_home;
#elif defined(FREEORION_WIN32)
    return GetBinDir();
#endif
}
#endif

#if defined(FREEORION_ANDROID)
FO_COMMON_API void SetAndroidEnvironment(JNIEnv* env, jobject activity)
{
    s_jni_env = env;

    // ToDo: move actual initialization to InitDirs
    // ToDo: store activity in weak reference to use in InitDirs
    jclass activity_cls = env->GetObjectClass(activity);

    jmethodID get_files_dir_mid = env->GetMethodID(activity_cls, "getFilesDir", "()Ljava/io/File;");
    jobject files_dir = env->CallObjectMethod(activity, get_files_dir_mid);

    jclass file_cls = env->GetObjectClass(files_dir);
    jmethodID get_absolute_path_mid = env->GetMethodID(file_cls, "getAbsolutePath", "()Ljava/lang/String;");
    jstring files_dir_path = (jstring)env->CallObjectMethod(files_dir, get_absolute_path_mid);

    const char *files_dir_chars = env->GetStringUTFChars(files_dir_path, NULL);
    s_user_dir = fs::path(files_dir_chars);
    env->ReleaseStringUTFChars(files_dir_path, files_dir_chars);
}
#endif

void CompleteXDGMigration()
{
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

namespace {
    std::mutex res_dir_mutex;
    bool init = true;
    fs::path res_dir;

    void RefreshResDir() {
        std::lock_guard<std::mutex> res_dir_lock(res_dir_mutex);
        // if resource dir option has been set, use specified location. otherwise,
        // use default location
        res_dir = FilenameToPath(GetOptionsDB().Get<std::string>("resource.path"));
        if (!fs::exists(res_dir) || !fs::is_directory(res_dir))
            res_dir = FilenameToPath(GetOptionsDB().GetDefault<std::string>("resource.path"));
        DebugLogger() << "Refreshed ResDir";
    }
}

auto GetResourceDir() -> fs::path const
{
    std::lock_guard<std::mutex> res_dir_lock(res_dir_mutex);
    if (init) {
        init = false;
        res_dir = FilenameToPath(GetOptionsDB().Get<std::string>("resource.path"));
        if (!fs::exists(res_dir) || !fs::is_directory(res_dir))
            res_dir = FilenameToPath(GetOptionsDB().GetDefault<std::string>("resource.path"));
        GetOptionsDB().OptionChangedSignal("resource.path").connect(&RefreshResDir);
        TraceLogger() << "Initialized ResDir and connected change signal";
    }
    return res_dir;
}

auto GetConfigPath() -> fs::path const
{
    static const fs::path p = GetUserConfigDir() / "config.xml";
    return p;
}

auto GetPersistentConfigPath() -> fs::path const
{
    static const fs::path p = GetUserConfigDir() / "persistent_config.xml";
    return p;
}

auto GetSaveDir() -> fs::path const
{
    // if save dir option has been set, use specified location.  otherwise,
    // use default location
    std::string options_save_dir = GetOptionsDB().Get<std::string>("save.path");
    if (options_save_dir.empty())
        options_save_dir = GetOptionsDB().GetDefault<std::string>("save.path");
    return FilenameToPath(options_save_dir);
}

auto GetServerSaveDir() -> fs::path const
{
    // if server save dir option has been set, use specified location.  otherwise,
    // use default location
    std::string options_save_dir = GetOptionsDB().Get<std::string>("save.server.path");
    if (options_save_dir.empty())
        options_save_dir = GetOptionsDB().GetDefault<std::string>("save.server.path");
    return FilenameToPath(options_save_dir);
}

auto RelativePath(fs::path const& from, fs::path const& to) -> fs::path
{
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

auto FilenameToPath(std::string const& path_str) -> fs::path
{
#if defined(FREEORION_WIN32)
    // convert UTF-8 directory string to UTF-16
    fs::path::string_type directory_native = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes(path_str);
#if (BOOST_VERSION >= 106300)
    return fs::path(directory_native).generic_path();
#else
    return fs::path(directory_native);
#endif
#else // defined(FREEORION_WIN32)
    return fs::path(path_str);
#endif // defined(FREEORION_WIN32)
}

auto PathToString(fs::path const& path) -> std::string
{
#if defined(FREEORION_WIN32)
    fs::path::string_type native_string = path.generic_wstring();
    std::string retval = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.to_bytes(native_string);
    return retval;
#else // defined(FREEORION_WIN32)
    return path.string();
#endif // defined(FREEORION_WIN32)
}

auto FilenameTimestamp() -> std::string
{
    boost::posix_time::time_facet* facet = new boost::posix_time::time_facet("%Y%m%d_%H%M%S");
    std::stringstream date_stream;

    date_stream.imbue(std::locale(date_stream.getloc(), facet));// alternate locales: GetLocale("en_US.UTF-8") or GetLocale("ja_JA.UTF-8") or date_stream.getloc()
    date_stream << boost::posix_time::microsec_clock::local_time();
    std::string retval = date_stream.str();
    TraceLogger() << "Filename initial timestamp: " << retval;

    // replace spaces and colons with safer chars for filenames
    std::replace(retval.begin(), retval.end(), ' ', '_');
    std::replace(retval.begin(), retval.end(), ':', '-');

    // filter non-filename-safe characters that are valid single-byte UTF-8 characters, in case the default locale for this system has weird chars in the time-date format
    auto do_remove = [](char c) -> bool { return !std::isalnum(c) && c <= 127 && c != '_' && c != '-'; };
    retval.erase(std::remove_if(retval.begin(), retval.end(), do_remove), retval.end());
    TraceLogger() << "Filename filtered timestamp: " << retval;

    return retval;
}

auto IsFOCScript(const fs::path& path) -> bool
{ return fs::is_regular_file(path) && ".txt" == path.extension() && path.stem().extension() == ".focs"; }

auto ListDir(const fs::path& path, std::function<bool (const fs::path&)> predicate) -> std::vector<fs::path>
{
    std::vector<fs::path> retval;

    if (!predicate)
        predicate = static_cast<bool (*)(const fs::path&)>(fs::is_regular_file);

    bool is_rel = path.is_relative();
    if (!is_rel && (fs::is_empty(path) || !fs::is_directory(path))) {
        DebugLogger() << "ListDir: File " << PathToString(path) << " was not included as it is empty or not a directoy";
    } else {
        const fs::path& default_path = is_rel ? GetResourceDir() / path : path;

        for (fs::recursive_directory_iterator dir_it(default_path);
             dir_it != fs::recursive_directory_iterator(); ++dir_it)
        {
            if (predicate(dir_it->path()))
                retval.emplace_back(dir_it->path());
            else
                TraceLogger() << "ListDir: Discarding non-matching path: " << PathToString(dir_it->path());
        }
    }

    if (retval.empty()) {
        DebugLogger() << "ListDir: No paths found for " << path.string();
    }

    return retval;
}

auto IsInDir(fs::path const& dir, fs::path const& test_dir) -> bool
{
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

auto GetPath(PathType path_type) -> fs::path
{
    switch (path_type) {
    case PathType::PATH_BINARY:
        return GetBinDir();
    case PathType::PATH_RESOURCE:
        return GetResourceDir();
    case PathType::PATH_DATA_ROOT:
        return GetRootDataDir();
    case PathType::PATH_DATA_USER:
        return GetUserDataDir();
    case PathType::PATH_CONFIG:
        return GetUserConfigDir();
    case PathType::PATH_SAVE:
        return GetSaveDir();
    case PathType::PATH_TEMP:
        return fs::temp_directory_path();
    case PathType::PATH_PYTHON:
#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32)
        return GetPythonHome();
#endif
    case PathType::PATH_INVALID:
    default:
        ErrorLogger() << "Invalid path type " << path_type;
        return fs::temp_directory_path();
    }
}

auto GetPath(std::string const& path_string) -> fs::path
{
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

auto IsExistingFile(const fs::path& path) -> bool
{
    try {
        auto stat = fs::status(path);
        return fs::exists(stat) && fs::is_regular_file(stat);
    } catch(fs::filesystem_error& ec) {
        ErrorLogger() << "Filesystem error during stat of " << PathToString(path) << " : " << ec.what();
    }

    return false;
}
