// -*- C++ -*-
#ifndef _Directories_h_
#define _Directories_h_

#include <boost/filesystem/path.hpp>
#include <string>

/** This function must be called before any Get*Dir function is called. It
  * stores the current working directory as well as creating local
  * directories if they do not yet exist. */
void InitDirs(const std::string& argv0);

/** Returns the directory where FreeOrion should store user specific data, like
  * the configuration file and savegames.  Under Unix, this would be
  * <tt>~/.freeorion</tt>, under Windows, this might be something along the
  * lines of <tt>C:\\Documents and Settings\\Username\\FreeOrion</tt>
  * or even <tt>\\\\Gandalf\\Users\\Frodo\\Settings\\FreeOrion</tt>.
  * \note <ul><li> If the directory does not exist, it will be created.
  * <li>This directory is the only one that can be considered writable!</ul> */
const boost::filesystem::path GetUserDir();

/** Returns the directory that contains all game content files, such as string
  * table files, in-game tech, building, special, etc. definition files, and
  * graphics files. */
const boost::filesystem::path GetResourceDir();

/** Returns the root data directory of FreeOrion. Under Windows, it is the
  * directory where FreeOrion is installed, under Linux, this can be
  * <tt>/usr/local/share/freeorion</tt>, <tt>/opt/share/freorion</tt>, or even
  * (when FreeOrion was installed locally with autopackage)
  * <tt>~/.local/share/freeorion</tt>.  \note This directory and everything in
  * it should be assumed read-only! */
const boost::filesystem::path GetRootDataDir();

/** Returns the directory where the binaries are located. Under
  * Unix, it will be something along the lines of <tt>/usr/local/bin</tt>,
  * under Windows, it will probably be the installation directory. \note This
  * directory and everything in it should be assumed read-only! */
const boost::filesystem::path GetBinDir();

/** Returns the full path to the configfile. */
const boost::filesystem::path GetConfigPath();

/** Returns the directory where save files are located.  This is typically
  * the directory "save" within the user directory. */
const boost::filesystem::path GetSaveDir();

#ifdef FREEORION_MACOSX
/** This function returns the Python home directory from where it is embedded
  * within the Mac OS X application bundle */
const boost::filesystem::path GetPythonHome();
#endif

/** Returns the path to \a to, as it appears from \a from. */
boost::filesystem::path RelativePath(const boost::filesystem::path& from, const boost::filesystem::path& to);

#endif
