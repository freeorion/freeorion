#include "Directories.h"

#include "OptionsDB.h"
#include "i18n.h"

#include <boost/algorithm/string/trim.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>

#include <cstdlib>
#include <mutex>

#if defined(FREEORION_MACOSX)
#  include <iostream>
#  include <sys/param.h>
#  include <mach-o/dyld.h>
#  include <CoreFoundation/CoreFoundation.h>
#  include <patchlevel.h>
#  include <boost/preprocessor/cat.hpp>
#  include <boost/preprocessor/stringize.hpp>
#endif

#if defined(FREEORION_ANDROID)
#  include <thread>
#  include <forward_list>
#  include <android/asset_manager.h>
#  include <android/asset_manager_jni.h>
#  include <android/log.h>
#endif

#if defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD) || defined(FREEORION_NETBSD) || defined(FREEORION_DRAGONFLY) || defined(FREEORION_HAIKU) || defined(FREEORION_ANDROID)
#  include "binreloc.h"
#  include <unistd.h>
#  include <boost/filesystem/fstream.hpp>

#  if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
#    include <sys/sysctl.h>
#  endif
#  ifdef __HAIKU__
#    include <FindDirectory.h>
#  endif
#endif

#if defined(FREEORION_WIN32) || defined(FREEORION_MACOSX) || defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD) || defined(FREEORION_NETBSD) || defined(FREEORION_DRAGONFLY) || defined(FREEORION_HAIKU) || defined(FREEORION_ANDROID)
#else
#  error Neither FREEORION_LINUX, FREEORION_MACOSX, FREEORION_FREEBSD, FREEORION_OPENBSD, FREEORION_NETBSD, FREEORION_DRAGONFLY, FREEORION_WIN32, FREEORION_HAIKU nor FREEORION_ANDROID set
#endif

#if defined(FREEORION_WIN32)
#  include <windows.h>
#endif

namespace fs = ::boost::filesystem;

namespace {
    bool g_initialized = false;
    fs::path bin_dir = fs::initial_path();

#if defined(FREEORION_MACOSX)
    fs::path       s_user_dir;
    fs::path       s_root_data_dir;
    fs::path       s_bin_dir;
    //! pythonhome: FreeOrion.app/Contents/Frameworks/Python.framework/Versions/{PythonMajor}.{PythonMinor}
    fs::path       s_python_home;
#endif

#if defined(FREEORION_ANDROID)
    thread_local JNIEnv* s_jni_env = nullptr;
    fs::path       s_user_dir;
    fs::path       s_cache_dir;
    fs::path       s_python_home;
    jweak          s_activity;
    AAssetManager* s_asset_manager;
    jobject        s_jni_asset_manager;
    JavaVM*        s_java_vm;

void RedirectOutputLogAndroid(int priority, const char* tag, int fd)
{
    std::thread background([priority, tag, fd]() {
        int pipes[2];
        pipe(pipes);
        dup2(pipes[1], fd);
        FILE *inputFile = fdopen(pipes[0], "r");
        char readBuffer[256];
        while (true) {
            fgets(readBuffer, sizeof(readBuffer), inputFile);
            __android_log_write(priority, tag, readBuffer);
        }
    });
    background.detach();
}

void CopyInitialResourceAndroid(const std::string& rel_path)
{
    AAsset* asset = AAssetManager_open(s_asset_manager, ("default/python/" + rel_path).c_str(), AASSET_MODE_STREAMING);
    if (!asset)
        return;

    off64_t asset_length = AAsset_getLength64(asset);
    if (asset_length <= 0) {
        AAsset_close(asset);
        return;
    }

    char buf[4096];
    int nb_read = 0;
    fs::ofstream ofs(s_python_home / rel_path, std::ios::binary);
    while ((nb_read = AAsset_read(asset, buf, 4096)) > 0) {
        ofs.write(buf, nb_read);
    }
    ofs.close();
}
#endif

#if defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD) || defined(FREEORION_NETBSD) || defined(FREEORION_DRAGONFLY) || defined(FREEORION_HAIKU)
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


namespace {
    constexpr auto PathTypeToStringImpl(PathType path_type) -> std::string_view
    {
        switch (path_type) {
            case PathType::PATH_BINARY:    return "PATH_BINARY";
            case PathType::PATH_RESOURCE:  return "PATH_RESOURCE";
            case PathType::PATH_DATA_ROOT: return "PATH_DATA_ROOT";
            case PathType::PATH_DATA_USER: return "PATH_DATA_USER";
            case PathType::PATH_CONFIG:    return "PATH_CONFIG";
            case PathType::PATH_CACHE:     return "PATH_CACHE";
            case PathType::PATH_SAVE:      return "PATH_SAVE";
            case PathType::PATH_TEMP:      return "PATH_TEMP";
#if !defined(FREEORION_LINUX)
            case PathType::PATH_PYTHON:    return "PATH_PYTHON";
#endif
            case PathType::PATH_INVALID:   return "PATH_INVALID";
            default:                       return "";
        }
    }

    constexpr auto PathTypesImpl() {
        static_assert(std::is_same_v<std::underlying_type_t<PathType>, int8_t>);
        static_assert(int8_t(PathType::PATH_INVALID) > 0);
        std::array<PathType, NUM_PATH_TYPES> retval{};

        for (PathType pt = PathType(0); pt < PathType::PATH_INVALID; pt = PathType(int8_t(pt) + 1))
            retval[std::size_t(pt)] = pt;
        return retval;
    }

    constexpr auto PathTypesViewsImpl() {
        std::array<std::string_view, NUM_PATH_TYPES> retval{};
        std::size_t idx = 0;
        for (const auto& pt : PathTypesImpl())
            retval[idx++]= PathTypeToStringImpl(pt);
        return retval;
    };
    constexpr auto path_type_views = PathTypesViewsImpl();
}

auto PathTypeToString(PathType path_type) -> std::string_view
{ return PathTypeToStringImpl(path_type); }

std::array<PathType, NUM_PATH_TYPES> PathTypes()
{ return PathTypesImpl(); }

auto PathTypeStrings() noexcept -> const std::array<std::string_view, NUM_PATH_TYPES>&
{ return path_type_views; }

void InitBinDir(std::string const& argv0)
{
#if defined(FREEORION_WIN32)
    try {
        fs::path binary_file = fs::system_complete(FilenameToPath(argv0));
        bin_dir = binary_file.parent_path();
    } catch (const fs::filesystem_error &) {
        bin_dir = fs::initial_path();
    }
#elif defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD) || defined(FREEORION_NETBSD) || defined(FREEORION_DRAGONFLY) || defined(FREEORION_HAIKU)
    bool problem = false;
    try {
        // get this executable's path by following link
        char buf[2048] = {'\0'};

#if defined(FREEORION_LINUX)
        problem = (-1 == readlink("/proc/self/exe", buf, sizeof(buf) - 1));
#endif
#if defined(FREEORION_FREEBSD) || defined(FREEORION_DRAGONFLY)
        int mib[4];
        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC;
        mib[2] = KERN_PROC_PATHNAME;
        mib[3] = -1;
        std::size_t buf_size = sizeof(buf);
        sysctl(mib, 4, buf, &buf_size, 0, 0);
#endif
#if defined(FREEORION_NETBSD)
        int mib[4];
        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC_ARGS;
        mib[2] = -1;
        mib[3] = KERN_PROC_PATHNAME;
        std::size_t buf_size = sizeof(buf);
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
            bin_dir = binary_file.parent_path();

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
#elif defined(FREEORION_MACOSX) || defined(FREEORION_ANDROID)
    // no binary directory setup required.
#endif
}

void InitDirs(std::string const& argv0, bool test)
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

    std::cout << "Bundle dir " << bundle_dir;

    bundle_path = fs::path(bundle_dir);

    if (!test) {
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
        s_bin_dir       =   app_path / "Executables";
        s_python_home   =   app_path / "SharedSupport";
    } else {
        s_root_data_dir = bundle_path.parent_path().parent_path();
        s_bin_dir = bundle_path;
        s_python_home = bundle_path.parent_path() / "dep";
    }

    s_user_dir      =   fs::path(getenv("HOME")) / "Library" / "Application Support" / "FreeOrion";

    fs::path p = s_user_dir;
    if (!exists(p))
        fs::create_directories(p);

    p /= "save";
    if (!exists(p))
        fs::create_directories(p);

    // Intentionally do not create the server save dir.
    // The server save dir is publically accessible and should not be
    // automatically created for the user.
#elif defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD) || defined(FREEORION_NETBSD) || defined(FREEORION_DRAGONFLY) || defined(FREEORION_HAIKU)
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

    fs::path ca = GetUserCacheDir();
    if (!exists(ca)) {
        fs::create_directories(ca);
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
    JNIEnv *env;
    if (s_jni_env) {
        env = s_jni_env;
    } else {
        s_java_vm->AttachCurrentThreadAsDaemon(&env, nullptr);
    }

    jobject activity = env->NewLocalRef(s_activity);

    jclass activity_cls = env->GetObjectClass(activity);

    jmethodID get_files_dir_mid = env->GetMethodID(activity_cls, "getFilesDir", "()Ljava/io/File;");
    jobject files_dir = env->CallObjectMethod(activity, get_files_dir_mid);

    jclass file_cls = env->GetObjectClass(files_dir);
    jmethodID get_absolute_path_mid = env->GetMethodID(file_cls, "getAbsolutePath", "()Ljava/lang/String;");
    jstring files_dir_path = reinterpret_cast<jstring>(env->CallObjectMethod(files_dir, get_absolute_path_mid));

    const char *files_dir_chars = env->GetStringUTFChars(files_dir_path, NULL);
    s_user_dir = fs::path(files_dir_chars);
    env->ReleaseStringUTFChars(files_dir_path, files_dir_chars);

    jmethodID get_cache_dir_mid = env->GetMethodID(activity_cls, "getCacheDir", "()Ljava/io/File;");
    jobject cache_dir = env->CallObjectMethod(activity, get_cache_dir_mid);

    file_cls = env->GetObjectClass(cache_dir);
    get_absolute_path_mid = env->GetMethodID(file_cls, "getAbsolutePath", "()Ljava/lang/String;");
    jstring cache_dir_path = reinterpret_cast<jstring>(env->CallObjectMethod(cache_dir, get_absolute_path_mid));

    const char *cache_dir_chars = env->GetStringUTFChars(cache_dir_path, NULL);
    s_cache_dir = fs::path(cache_dir_chars);
    env->ReleaseStringUTFChars(cache_dir_path, cache_dir_chars);

    jmethodID get_assets_mid = env->GetMethodID(activity_cls, "getAssets", "()Landroid/content/res/AssetManager;");
    jobject asset_manager = env->CallObjectMethod(activity, get_assets_mid);
    s_jni_asset_manager = env->NewGlobalRef(asset_manager);
    s_asset_manager = AAssetManager_fromJava(env, s_jni_asset_manager);

    if (!s_jni_env) {
        s_java_vm->DetachCurrentThread();
    }

    RedirectOutputLogAndroid(ANDROID_LOG_ERROR, "stderr", 2);
    RedirectOutputLogAndroid(ANDROID_LOG_INFO, "stdout", 1);

    s_python_home = s_cache_dir / "python";
    fs::create_directories(s_python_home / "lib");
    CopyInitialResourceAndroid("lib/python39.zip");
#endif

    g_initialized = true;
}

auto GetUserConfigDir() -> fs::path const
{
#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32) || defined(FREEORION_ANDROID)
    return GetUserDataDir();
#elif defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD) || defined(FREEORION_NETBSD) || defined(FREEORION_DRAGONFLY) || defined(FREEORION_HAIKU)
    static fs::path p = getenv("XDG_CONFIG_HOME")
        ? fs::path(getenv("XDG_CONFIG_HOME")) / "freeorion"
        : fs::path(getenv("HOME")) / ".config" / "freeorion";
    return p;
#endif
}

auto GetUserCacheDir() -> fs::path const
{
#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32)
    return GetUserDataDir();
#elif defined(FREEORION_ANDROID)
    if (!g_initialized)
        InitDirs("");
    return s_cache_dir;
#elif defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD) || defined(FREEORION_HAIKU)
    static fs::path p = getenv("XDG_CACHE_HOME")
        ? fs::path(getenv("XDG_CACHE_HOME")) / "freeorion"
        : fs::path(getenv("HOME")) / ".cache" / "freeorion";
    return p;
#endif
}

auto GetUserDataDir() -> fs::path const
{
#if defined(FREEORION_MACOSX) || defined(FREEORION_ANDROID)
    if (!g_initialized)
        InitDirs("");
    return s_user_dir;
#elif defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD) || defined(FREEORION_NETBSD) || defined(FREEORION_DRAGONFLY) || defined(FREEORION_HAIKU) || defined(FREEORION_ANDROID)
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
#elif defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD) || defined(FREEORION_NETBSD) || defined(FREEORION_DRAGONFLY) || defined(FREEORION_HAIKU)
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
#elif defined(FREEORION_WIN32)
    return fs::initial_path();
#elif defined(FREEORION_ANDROID)
    return fs::path(".");
#endif
}

auto GetBinDir() -> fs::path const
{
#if defined(FREEORION_MACOSX)
    if (!g_initialized)
        InitDirs("");
    return s_bin_dir;
#elif defined(FREEORION_LINUX) || defined(FREEORION_FREEBSD) || defined(FREEORION_OPENBSD) || defined(FREEORION_NETBSD) || defined(FREEORION_DRAGONFLY) || defined(FREEORION_HAIKU) || defined(FREEORION_ANDROID)
    if (!g_initialized)
        InitDirs("");
    return bin_dir;
#elif defined(FREEORION_WIN32)
    if (!g_initialized)
        InitDirs("");
    return bin_dir;
#endif
}

#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32) || defined(FREEORION_ANDROID)
auto GetPythonHome() -> fs::path const
{
#if defined(FREEORION_MACOSX) || defined(FREEORION_ANDROID)
    if (!g_initialized)
        InitDirs("");
    return s_python_home;
#elif defined(FREEORION_WIN32)
    return GetBinDir();
#endif
}
#endif

#if defined(FREEORION_ANDROID)
void SetAndroidEnvironment(JNIEnv* env, jobject activity)
{
    s_jni_env = env;
    s_jni_env->GetJavaVM(&s_java_vm);
    s_activity = env->NewWeakGlobalRef(activity);
}

std::string GetAndroidLang()
{
    std::string retval;

    JNIEnv *env;
    if (s_jni_env != nullptr) {
        env = s_jni_env;
    } else {
        s_java_vm->AttachCurrentThreadAsDaemon(&env, nullptr);
    }

    jclass locale_class = env->FindClass("java/util/Locale");
    jmethodID get_default_mid = env->GetStaticMethodID(locale_class, "getDefault", "()Ljava/util/Locale;");
    jobject locale = env->CallStaticObjectMethod(locale_class, get_default_mid);

    jmethodID get_language_mid = env->GetMethodID(locale_class, "getLanguage", "()Ljava/lang/String;");
    jstring language = reinterpret_cast<jstring>(env->CallObjectMethod(locale, get_language_mid));

    const char *language_chars = env->GetStringUTFChars(language, nullptr);
    retval = std::string(language_chars);
    env->ReleaseStringUTFChars(language, language_chars);

    if (!s_jni_env)
        s_java_vm->DetachCurrentThread();

    return retval;
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
            GetOptionsDB().Set("save.path", GetUserDataDir().string());
    }
}

namespace {
    std::mutex res_dir_mutex;
    bool init = true;
    fs::path res_dir;

    void RefreshResDir() {
        std::scoped_lock res_dir_lock(res_dir_mutex);
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
    std::scoped_lock res_dir_lock(res_dir_mutex);
    if (init) {
        [[unlikely]]
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
        retval /= "..";
    for (; to_it != end_to_it; ++to_it)
        retval /= *to_it;
    return retval;
}

auto FilenameToPath(std::string const& path_str) -> fs::path
{
#if defined(FREEORION_WIN32)
    // convert UTF-8 directory string to UTF-16
    int utf16_sz = MultiByteToWideChar(CP_UTF8, 0, path_str.data(), path_str.length(), NULL, 0);
    std::wstring utf16_string(utf16_sz, 0);
    if (utf16_sz > 0)
        MultiByteToWideChar(CP_UTF8, 0, path_str.data(), path_str.size(), utf16_string.data(), utf16_sz);
    static_assert(std::is_same_v<fs::path::string_type, std::wstring>);
    return fs::path(utf16_string).generic_path();
#else
    return fs::path(path_str);
#endif
}

auto PathToString(fs::path const& path) -> std::string
{
#if defined(FREEORION_WIN32)
    fs::path::string_type native_string = path.generic_wstring();
    // convert UTF-16 native path to UTF-8
    int utf8_sz = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                                      native_string.data(), native_string.size(),
                                      NULL, 0, NULL, NULL);
    std::string utf8_string(utf8_sz, 0);
    if (utf8_sz > 0)
        WideCharToMultiByte(CP_UTF8, 0, native_string.data(), native_string.size(),
                            utf8_string.data(), utf8_sz, NULL, NULL);
    return utf8_string;
#else
    return path.string();
#endif
}

#if !defined(FREEORION_ANDROID)
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

    // filter non-filename-safe characters that are valid single-byte UTF-8 characters,
    // in case the default locale for this system has weird chars in the time-date format
    auto do_remove = [](char c) -> bool { return !std::isalnum(c) && c != '_' && c != '-'; };
    retval.erase(std::remove_if(retval.begin(), retval.end(), do_remove), retval.end());
    TraceLogger() << "Filename filtered timestamp: " << retval;

    return retval;
}
#endif

auto IsFOCScript(const fs::path& path) -> bool
{ return IsExistingFile(path) && ".txt" == path.extension() && path.stem().extension() == ".focs"; }

auto IsFOCPyScript(const fs::path& path) -> bool
{ return IsExistingFile(path) && ".py" == path.extension() && path.stem().extension() == ".focs"; }

auto ListDir(const fs::path& path, std::function<bool (const fs::path&)> predicate) -> std::vector<fs::path>
{
    if (!predicate)
        predicate = static_cast<bool (*)(const fs::path&)>(IsExistingFile);

    std::vector<fs::path> retval;
#if defined(FREEORION_ANDROID)
    DebugLogger() << "ListDir: list directory " << path.string();
    std::forward_list<fs::path> directories;
    directories.push_front(path);

    // ToDo: Register thread once after moving to single backgroung parsing thread for Python
    JNIEnv *env;
    if (s_jni_env != nullptr) {
        env = s_jni_env;
    } else {
        s_java_vm->AttachCurrentThreadAsDaemon(&env, nullptr);
    }

    jmethodID list_mid = env->GetMethodID(env->GetObjectClass(s_jni_asset_manager), "list", "(Ljava/lang/String;)[Ljava/lang/String;");
    while (!directories.empty()) {
        fs::path dir = directories.front();
        directories.pop_front();
        DebugLogger() << "ListDir: recursively list directory " << dir.string();
        // Check assets with JNI to get subdirectories
        jstring path_object = env->NewStringUTF(PathToString(dir).c_str());
        jobjectArray list_object = reinterpret_cast<jobjectArray>(env->CallObjectMethod(s_jni_asset_manager, list_mid, path_object));
        env->DeleteLocalRef(path_object);
        if (!list_object) {
            DebugLogger() << "ListDir: no directory " << dir.string();
            continue;
        }
        const auto length = env->GetArrayLength(list_object);
        if (length == 0) {
            DebugLogger() << "ListDir: empty directory " << dir.string();
            continue;
        }
        for (int i = 0; i < length; ++i) {
            auto jstr = reinterpret_cast<jstring>(env->GetObjectArrayElement(list_object, i));
            if (!jstr)
                continue;
            const char* filename = env->GetStringUTFChars(jstr, nullptr);
            if (filename != nullptr) {
                fs::path file = dir / filename;
                DebugLogger() << "ListDir: found file " << file.string();
                env->ReleaseStringUTFChars(jstr, filename);
                if (predicate(file))
                    retval.push_back(file);

                directories.push_front(std::move(file));
            }
            env->DeleteLocalRef(jstr);
        }
    }
    if (!s_jni_env)
        s_java_vm->DetachCurrentThread();

#else
    bool is_rel = path.is_relative();
    if (!is_rel && (fs::is_empty(path) || !fs::is_directory(path))) {
        DebugLogger() << "ListDir: File " << PathToString(path) << " was not included as it is empty or not a directory";
    } else {
        const fs::path& default_path = is_rel ? GetResourceDir() / path : path;

        for (fs::recursive_directory_iterator dir_it(default_path);
             dir_it != fs::recursive_directory_iterator(); ++dir_it)
        {
            if (predicate(dir_it->path()))
                retval.push_back(dir_it->path());
            else
                TraceLogger() << "ListDir: Discarding non-matching path: " << PathToString(dir_it->path());
        }
    }
#endif

    if (retval.empty())
        DebugLogger() << "ListDir: No paths found for " << path.string();

    return retval;
}

auto IsInDir(fs::path const& dir, fs::path const& test_dir) -> bool
{
    if (!fs::exists(dir) || !fs::is_directory(dir))
        return false;

    if (fs::exists(test_dir) && !fs::is_directory(test_dir))
        return false;

    // Resolve any symbolic links, dots or dot-dots
    const auto canon_dir = fs::canonical(dir);
    // Don't resolve path if directory doesn't exist
    // TODO: Change to fs::weakly_canonical after bump boost version above 1.60
    const auto canon_path = fs::exists(test_dir) ? fs::canonical(test_dir) : test_dir;

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
    case PathType::PATH_CACHE:
        return GetUserCacheDir();
    case PathType::PATH_SAVE:
        return GetSaveDir();
    case PathType::PATH_TEMP:
        return fs::temp_directory_path();
#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32)
    case PathType::PATH_PYTHON:
        return GetPythonHome();
#endif
    case PathType::PATH_INVALID:
    default:
        ErrorLogger() << "Invalid path type " << path_type;
        return fs::temp_directory_path();
    }
}

auto IsExistingFile(const fs::path& path) -> bool
{
#if defined(FREEORION_ANDROID)
    DebugLogger() << "IsExistingFile: check file " << path.string();
    AAsset* asset = AAssetManager_open(s_asset_manager, PathToString(path).c_str(), AASSET_MODE_STREAMING);
    if (!asset) {
        DebugLogger() << "IsExistingFile: not found asset " << path.string();
        return false;
    }
    off64_t asset_length = AAsset_getLength64(asset);
    DebugLogger() << "IsExistingFile: asset length " << asset_length;
    AAsset_close(asset);
    return asset_length > 0;
#else
    try {
        auto stat = fs::status(path);
        return fs::exists(stat) && fs::is_regular_file(stat);
    } catch(fs::filesystem_error& ec) {
        ErrorLogger() << "Filesystem error during stat of " << PathToString(path) << " : " << ec.what();
    }

    return false;
#endif
}

auto IsExistingDir(boost::filesystem::path const& path) -> bool
{
#if defined(FREEORION_ANDROID)
    DebugLogger() << "IsExistingDir: check file " << path.string();
    AAssetDir* asset_dir = AAssetManager_openDir(s_asset_manager, PathToString(path).c_str());
    if (asset_dir != nullptr)  {
        const char* first_file_name = AAssetDir_getNextFileName(asset_dir);
        AAssetDir_close(asset_dir);
        if (first_file_name != nullptr) {
            return true;
        }
    }

    JNIEnv *env;
    if (s_jni_env != nullptr) {
        env = s_jni_env;
    } else {
        s_java_vm->AttachCurrentThreadAsDaemon(&env, nullptr);
    }

    // Check assets with JNI to get subdirectories
    jmethodID list_mid = env->GetMethodID(env->GetObjectClass(s_jni_asset_manager), "list", "(Ljava/lang/String;)[Ljava/lang/String;");
    jstring path_object = env->NewStringUTF(PathToString(path).c_str());
    jobjectArray list_object = reinterpret_cast<jobjectArray>(env->CallObjectMethod(s_jni_asset_manager, list_mid, path_object));
    env->DeleteLocalRef(path_object);
    if (!list_object) {
        if (!s_jni_env)
            s_java_vm->DetachCurrentThread();
        return false;
    }
    auto length = env->GetArrayLength(list_object);
    if (!s_jni_env)
        s_java_vm->DetachCurrentThread();

    return length > 0;
#else
    return fs::exists(path) && fs::is_directory(path);
#endif
}

auto ReadFile(boost::filesystem::path const& path, std::string& file_contents) -> bool
{
#if defined(FREEORION_ANDROID)
    DebugLogger() << "ReadFile: check file " << path.string();
    AAsset* asset = AAssetManager_open(s_asset_manager, PathToString(path).c_str(), AASSET_MODE_BUFFER);
    if (!asset)
        return false;
    off64_t asset_length = AAsset_getLength64(asset);
    if (asset_length <= 0) {
        AAsset_close(asset);
        return false;
    }
    file_contents = std::string(reinterpret_cast<const char*>(AAsset_getBuffer(asset)), asset_length);
    AAsset_close(asset);
#else
    boost::filesystem::ifstream ifs(path);
    if (!ifs)
        return false;

    // skip byte order mark (BOM)
    for (int BOM : {0xEF, 0xBB, 0xBF}) {
        if (BOM != ifs.get()) {
            // no header set stream back to start of file
            ifs.seekg(0, std::ios::beg);
            // and continue
            break;
        }
    }

    std::getline(ifs, file_contents, '\0');
#endif
    boost::trim(file_contents);

    // no problems?
    return !file_contents.empty();
}
