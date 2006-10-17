#include "Directories.h"

#include "OptionsDB.h"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>

#include <cstdlib>


namespace fs = boost::filesystem;

namespace {
    bool g_initialized = false;
}

#if defined(FREEORION_LINUX)
#include "binreloc.h"

void InitDirs()
{
    if (g_initialized)
        return;

    if (fs::path::default_name_check_writable())
        fs::path::default_name_check(fs::native);

    // store working dir
    fs::initial_path();

    br_init(0);

    fs::path p = GetLocalDir();
    if (!exists(p)) {
        fs::create_directories(p);
    }
    
    p /= "save";
    if (!exists(p)) {
        fs::create_directories(p);
    }

    g_initialized = true;
}

const fs::path GetLocalDir()
{
    if (fs::path::default_name_check_writable())
        fs::path::default_name_check(fs::native);
    static fs::path p = fs::path(getenv("HOME")) / ".freeorion";
    return p;
}

const fs::path GetGlobalDir()
{
    if (!g_initialized) InitDirs();
    fs::path p(br_find_data_dir("/usr/local/share"));
    p /= "freeorion";
    // if the path does not exist, we fall back to the working directory
    if (!exists(p)) {
        return fs::initial_path();
    } else {
        return p;
    }
}

const fs::path GetBinDir()
{
    if (!g_initialized) InitDirs();
    fs::path p(br_find_bin_dir("/usr/local/bin"));
    // if the path does not exist, we fall back to the working directory
    if (!exists(p)) {
        return fs::initial_path();
    } else {
        return p;
    }
}

#elif defined(FREEORION_WIN32)

void InitDirs()
{
    if (g_initialized)
        return;

    fs::path::default_name_check(fs::native);

    fs::path local_dir = GetLocalDir();
    if (!exists(local_dir))
        fs::create_directories(local_dir);

    fs::path p = GetLocalDir() / "save";
    if (!exists(p))
        fs::create_directories(p);

    g_initialized = true;
}

const fs::path GetLocalDir()
{
    if (fs::path::default_name_check_writable())
        fs::path::default_name_check(fs::native);
    static fs::path p = fs::path(std::string(getenv("APPDATA"))) / "FreeOrion";
    return p;
}

const fs::path GetGlobalDir()
{
    return fs::initial_path();
}

const fs::path GetBinDir()
{
    return fs::initial_path();
}

#else
#  error Neither FREEORION_LINUX nor FREEORION_WIN32 set
#endif

const boost::filesystem::path GetSettingsDir()
{
    return fs::path(GetOptionsDB().Get<std::string>("settings-dir"));
}

const fs::path GetConfigPath()
{
    static const fs::path p = GetLocalDir() / "config.xml";
    return p;
}

boost::filesystem::path RelativePath (const boost::filesystem::path& from, const boost::filesystem::path& to)
{
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
