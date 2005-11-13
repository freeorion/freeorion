#include "Directories.h"

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
    static fs::path p = fs::path(getenv("HOME"), fs::native) / path(".freeorion", fs::native);
    return p;
}

const fs::path GetGlobalDir()
{
    if (!g_initialized) InitDirs();
    fs::path p(br_find_data_dir("/usr/local/share"), fs::native);
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
    fs::path p(br_find_bin_dir("/usr/local/bin"), fs::native);
    // if the path does not exist, we fall back to the working directory
    if (!exists(p)) {
        return fs::initial_path();
    } else {
        return p;
    }
}

const fs::path GetConfigPath()
{
    static const fs::path p = GetLocalDir() / "config.xml";
    return p;
}

#elif defined(FREEORION_WIN32)

void InitDirs()
{
    if (g_initialized)
        return;

    fs::path p = fs::initial_path() / "save";
    if (!exists(p))
        fs::create_directories(p);

    g_initialized = true;
}

const fs::path GetLocalDir()
{
    return fs::initial_path();
}

const fs::path GetGlobalDir()
{
    return fs::initial_path();
}

const fs::path GetBinDir()
{
    return fs::initial_path();
}

const fs::path GetConfigPath()
{
    static const fs::path p = GetLocalDir() / "config.xml";
    return p;
}

#else
#  error Neither FREEORION_LINUX nor FREEORION_WIN32 set
#endif
