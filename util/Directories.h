#ifndef _Directories_h_
#define _Directories_h_

#include <boost/filesystem/path.hpp>
#include <string>

#include "Export.h"
#include "../universe/EnumsFwd.h"

/** This function must be called before any Get*Dir function is called. It
  * stores the current working directory as well as creating local
  * directories if they do not yet exist. */
FO_COMMON_API void CompleteXDGMigration();

/** This function completes the migration of directories to the XDG
  * specified location by updating the save.path option to the new location
  * after the option is loaded from XML files.  It only updates the option
  * if it is set to the old default option. */
FO_COMMON_API void InitDirs(const std::string& argv0);

/** Returns the directory where FreeOrion should store user specific
  * configuration data , like the configuration file.  Under Unix, this
  * would be <tt>$XDG_CONFIG_HOME/freeorion</tt>, under Windows, this
  * might be something along the lines of
  * <tt>C:\\Documents and Settings\\Username\\FreeOrion</tt>
  * or even <tt>\\\\Gandalf\\Users\\Frodo\\Settings\\FreeOrion</tt>.
  * \note <ul><li> If the directory does not exist, it will be created.
  * Under Windows and OSX it is the same as the GetUserDataDir()
  * <li>This directory can be considered writable!</ul> */
FO_COMMON_API const boost::filesystem::path GetUserConfigDir();

/** Returns the directory where FreeOrion should store user specific data,
  * like the savegames.  Under Unix, this would be
  * <tt>$XDG_DATA_HOME/freeorion</tt>, under Windows, this might be something
  * along the lines of <tt>C:\\Documents and Settings\\Username\\FreeOrion</tt>
  * or even <tt>\\\\Gandalf\\Users\\Frodo\\Settings\\FreeOrion</tt>.
  * \note <ul><li> If the directory does not exist, it will be created.
  * Under Windows and OSX it is the same as the GetUserConfigDir()
  * <li>This directory can be considered writable!</ul> */
FO_COMMON_API const boost::filesystem::path GetUserDataDir();

/** Converts UTF-8 string into a path, doing any required wide-character
  * conversions as determined by the operating system / filesystem. */
FO_COMMON_API boost::filesystem::path FilenameToPath(const std::string& path_str);

/** Returns the directory that contains all game content files, such as string
  * table files, in-game tech, building, special, etc. definition files, and
  * graphics files. */
FO_COMMON_API const boost::filesystem::path GetResourceDir();

/** Returns the root data directory of FreeOrion. Under Windows, it is the
  * directory where FreeOrion is installed, under Linux, this can be
  * <tt>/usr/local/share/freeorion</tt>, <tt>/opt/share/freorion</tt>, or even
  * (when FreeOrion was installed locally with autopackage)
  * <tt>~/.local/share/freeorion</tt>.  \note This directory and everything in
  * it should be assumed read-only! */
FO_COMMON_API const boost::filesystem::path GetRootDataDir();

/** Returns the directory where the binaries are located. Under
  * Unix, it will be something along the lines of <tt>/usr/local/bin</tt>,
  * under Windows, it will probably be the installation directory. \note This
  * directory and everything in it should be assumed read-only! */
FO_COMMON_API const boost::filesystem::path GetBinDir();

#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32)
/** This function returns the Python home directory from where it is embedded
  * On OSX: within the application bundle
  * On Windows: same directory where the binaries are located */
FO_COMMON_API const boost::filesystem::path GetPythonHome();
#endif

/** Returns the full path to the configfile. */
FO_COMMON_API const boost::filesystem::path GetConfigPath();

/** Returns the full path to the configfile. */
FO_COMMON_API const boost::filesystem::path GetPersistentConfigPath();

/** Returns the directory where save files are located.  This is typically
  * the directory "save" within the user directory. */
FO_COMMON_API const boost::filesystem::path GetSaveDir();

/** Returns the directory where server save files are located.  This is typically
  * the directory "save" within the user directory. */
FO_COMMON_API const boost::filesystem::path GetServerSaveDir();

/** Returns a utf-8 string from the given filesystem path. */
FO_COMMON_API std::string PathToString(const boost::filesystem::path& path);

/** Returns current timestamp in a form that can be used in file names */
FO_COMMON_API std::string FilenameTimestamp();

/** Returns the path to \a to, as it appears from \a from. */
FO_COMMON_API boost::filesystem::path RelativePath(const boost::filesystem::path& from, const boost::filesystem::path& to);

/** Returns a vector of files within \a path including a recursive search though sub-dirs */
FO_COMMON_API std::vector<boost::filesystem::path> ListDir(const boost::filesystem::path& path);

/** Returns true iff the string \a in is valid UTF-8. */
FO_COMMON_API bool IsValidUTF8(const std::string& in);

/** Returns true iff the \p test_dir is in \p dir and \p dir
    is existing directory. */
FO_COMMON_API bool IsInDir(const boost::filesystem::path& dir, const boost::filesystem::path& test_dir);

/** Returns path currently defined for @p path_type */
FO_COMMON_API boost::filesystem::path GetPath(PathType path_type);

/** Returns path for path type cast from @p path_string */
FO_COMMON_API boost::filesystem::path GetPath(const std::string& path_string);

/** Returns if path exists and is a regular file */
FO_COMMON_API bool IsExistingFile(const boost::filesystem::path& path);

/** All paths contained in a directory filtered by a functor
 *
 * @param[in] abs_dir_path Absolute path to directory
 * @param[in] pred Predicate functor accepting a boost::filesystem::path constant reference
 * @param[in] recursive_search If true, recurses into sub-directories
 *
 * @return vector List of filesytem objects found in @p abs_dir_path which satisfy @p pred */
FO_COMMON_API std::vector<boost::filesystem::path> PathsInDir(const boost::filesystem::path& abs_dir_path,
                                                              std::function<bool (const boost::filesystem::path&)> pred,
                                                              bool recursive_search);

#endif
