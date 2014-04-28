/*
 * Tools for dealing with save game previews.
 */

#ifndef SAVEGAMEPREVIEW_H
#define SAVEGAMEPREVIEW_H

#include <vector>
#include <string>

#include <boost/serialization/version.hpp>

#include <GG/Clr.h>

#include "../util/MultiplayerCommon.h"
#include "../util/Export.h"

namespace boost { namespace filesystem { class path; } }
namespace boost { namespace serialization { class access; } }

/** Contains preview information about a savegame.
 *  Stored the beginning of a savefile for quick access.
 */
struct FO_COMMON_API SaveGamePreviewData {
    /// Initialize with unknown markers.
    SaveGamePreviewData();
    
    /// Checks that this is a valid preview
    bool Valid() const;
    
    /// A marker for the presence of the header
    static const short PREVIEW_PRESENT_MARKER = 0xDA;
    /// This should always contain PREVIEW_PRESENT_MARKER
    short magic_number;
    
    /// The name of the hosting player, or the single human player in single player games
    std::string main_player_name;
    /// The name of the empire of the main player
    std::string main_player_empire_name;
    /// The colour of the empire of the main player
    GG::Clr main_player_empire_colour;
    /// The turn the game as saved one
    int current_turn;
    /// The time the game was saved as ISO 8601 YYYY-MM-DD"T"HH:MM:SS±HH:MM or Z
    std::string save_time;
    /// The number of empires in the game
    short number_of_empires;
    /// The number of human players in the game
    short number_of_human_players;
    
    template <class Archive>
    void serialize ( Archive& ar, unsigned int version );
};

BOOST_CLASS_VERSION(SaveGamePreviewData, 1);

/// Stores all aggregated information about a save file
struct FO_COMMON_API FullPreview{
    /// The name of the file
    std::string filename;
    /// The preview data from the file
    SaveGamePreviewData preview;
    /// The galaxy setup data from the file
    GalaxySetupData galaxy;
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/// The preview information the server sends to the client.
struct FO_COMMON_API PreviewInformation{
    /// A list of all subfolders of the save game directory, in the format /name1/child1/grandchild
    std::vector<std::string> subdirectories;
    /// The directory whose previews are being listed now
    std::string folder;
    /// The previews of the saves in this folder
    std::vector<FullPreview> previews;
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/// Get the value of column name in this preview
/// \param name The name of the column
/// \param thin If true, tries to make the value less wide
FO_COMMON_API std::string ColumnInPreview(const FullPreview& full, const std::string& name, bool thin = true);

/// Load previews from files
/// \param path Directory where to look for files
/// \param extension File name extension to filter by
/// \param [out] previews The previews will be put here
FO_COMMON_API void LoadSaveGamePreviews ( const boost::filesystem::path& path, const std::string& extension, std::vector<FullPreview>& previews );

/// If path is inside directory, returns true
FO_COMMON_API bool IsInside(const boost::filesystem::path& path, const boost::filesystem::path& directory);
#endif // SAVEGAMEPREVIEW_H
