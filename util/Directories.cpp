#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

#include "Directories.h"

using namespace boost::filesystem;

#if defined(FREEORION_LINUX)
#include "binreloc.h"
#include <stdlib.h>
#include <iostream>

static bool Initialized = false;

void InitDirs()
{
    if (Initialized) return;
    // store working dir
    boost::filesystem::initial_path();

    std::cerr << __PRETTY_FUNCTION__ << std::endl;
    // initialise binreloc
    br_init(NULL);
    std::cerr << br_find_exe("UNDEFINED") << std::endl;
    path p = GetLocalDir();
    std::cerr << GetLocalDir().native_directory_string() << std::endl;
    if (!exists(p)) {
	create_directories(p);
    }
    
    p /= "save";
    if (!exists(p)) {
	create_directories(p);
    }
    Initialized = true;
}

const path GetLocalDir()
{
    static path p = path(getenv("HOME"),native) / path(".freeorion",native);
    return p;
}

const path GetGlobalDir()
{
    if (!Initialized) InitDirs();
    path p(br_find_data_dir("/usr/local/share"),native);
    p /= "freeorion";
    /* if the path does not exist, we fall back to the working directory */
    if (!exists(p)) {
	return initial_path();
    } else {
	return p;
    }
}

const path GetBinDir()
{
    if (!Initialized) InitDirs();
    path p(br_find_bin_dir("/usr/local/bin"),native);
    std::cerr << "bindir: " << p.native_directory_string() << std::endl;
    /* if the path does not exist, we fall back to the working directory */
    if (!exists(p)) {
	return initial_path();
    } else {
	return p;
    }
}

const path GetConfigPath()
{
    static const path p = GetLocalDir() / "config.xml";
    return p;
}

#elif defined(FREEORION_WINDOWS)

// FIXME: UNIMPLEMENTED
void InitDirs()
{
    initial_path();
}

// FIXME: UNIMPLEMENTED
const path GetLocalDir()
{
    return initial_path();
}

const path GetGlobalDir()
{
    return initial_path();
}

// FIXME: UNIMPLEMENTED
const path GetBinDir()
{
    return initial_path();
}

const path GetConfigPath()
{
    static const path p(GetLocalDir() / "default/config.xml",native);
    return p;
}

#else
#  error Neither FREEORION_LINUX nor FREEORION_WINDOWS set
#endif

