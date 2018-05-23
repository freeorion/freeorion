/*
 * Tools for dealing with save game previews.
 */

#ifndef SAVEGAMEPREVIEW_H
#define SAVEGAMEPREVIEW_H

#include <vector>
#include <string>

#include <boost/serialization/version.hpp>

#include <GG/Clr.h>

#include "MultiplayerCommon.h"
#include "Export.h"
#include "Serialize.h"
#include "Directories.h"

/** Contains preview information about a savegame.
  * Stored the beginning of a savefile for quick access. */
struct FO_COMMON_API SaveGamePreviewData {
    SaveGamePreviewData();      /// Initialize with unknown markers.

    bool Valid() const;         /// Checks that this is a valid preview
    void SetBinary(bool bin = true);         /// Sets the description string appropriate for a binary or XML save file header

    static const short  PREVIEW_PRESENT_MARKER = 0xDA;  /// A marker for the presence of the header
    short               magic_number;                   /// This should always contain PREVIEW_PRESENT_MARKER

    std::string         description;                    /// Bit of text explaining what this file contains as human-readable text
    std::string         freeorion_version;              /// By what version of FreeOrion was this save generated

    std::string         main_player_name;               /// The name of the hosting player, or the single human player in single player games
    std::string         main_player_empire_name;        /// The name of the empire of the main player
    GG::Clr             main_player_empire_colour;      /// The colour of the empire of the main player
    int                 current_turn;                   /// The turn the game as saved one
    std::string         save_time;                      /// The time the game was saved as ISO 8601 YYYY-MM-DD"T"HH:MM:SS±HH:MM or Z
    short               number_of_empires;              /// The number of empires in the game
    short               number_of_human_players;        /// The number of human players in the game

    std::string         save_format_marker = "";        /// What format was used for this save?
    unsigned int        uncompressed_text_size = 0;     /// How many bytes capacity does the uncompressed save text take up? (ie. the part that was / will be compressed with zlib for compressed xml format saves)
    unsigned int        compressed_text_size = 0;       /// How many bytes capacity does the compressed save text take up?

    template <class Archive>
    void serialize(Archive& ar, unsigned int version);
};

BOOST_CLASS_VERSION(SaveGamePreviewData, 4);

extern template FO_COMMON_API void SaveGamePreviewData::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, unsigned int);
extern template FO_COMMON_API void SaveGamePreviewData::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, unsigned int);
extern template FO_COMMON_API void SaveGamePreviewData::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, unsigned int);
extern template FO_COMMON_API void SaveGamePreviewData::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, unsigned int);

/** Stores all aggregated information about a save file */
struct FO_COMMON_API FullPreview {
    std::string         filename;
    SaveGamePreviewData preview;
    GalaxySetupData     galaxy;
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** The preview information the server sends to the client. */
struct FO_COMMON_API PreviewInformation {
    std::vector<std::string>    subdirectories; /// A list of all subfolders of the save game directory, in the format /name1/child1/grandchild
    std::string                 folder;         /// The directory whose previews are being listed now
    std::vector<FullPreview>    previews;       /// The previews of the saves in this folder
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/// Attempts to load headers of a save file.
/// Returns true on success, false if header data could not be loaded
/// @param path Filename to load headers of
FO_COMMON_API bool SaveFileWithValidHeader(const boost::filesystem::path& path);

/// Get the value of column name in this preview
/// @param full FullPreview to match for column @p name
/// \param name The name of the column
/// \param thin If true, tries to make the value less wide
FO_COMMON_API std::string ColumnInPreview(const FullPreview& full, const std::string& name, bool thin = true);

/// Load previews from files
/// \param path Directory where to look for files
/// \param extension File name extension to filter by
/// \param [out] previews The previews will be put here
FO_COMMON_API void LoadSaveGamePreviews(const boost::filesystem::path& path, const std::string& extension,
                                        std::vector<FullPreview>& previews);
#endif // SAVEGAMEPREVIEW_H
