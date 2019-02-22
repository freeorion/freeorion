#include "OptionsDB.h"
#include "SaveGamePreviewUtils.h"

#include "i18n.h"
#include "Directories.h"
#include "Logger.h"
#include "EnumText.h"
#include "Serialize.h"
#include "Serialize.ipp"
#include "ScopedTimer.h"
#include "Version.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>


#include <fstream>

namespace fs = boost::filesystem;

namespace {
    const std::string UNABLE_TO_OPEN_FILE("Unable to open file");
    const std::string XML_SAVE_FILE_DESCRIPTION("This is an XML archive FreeOrion saved game. Initial header information is uncompressed. The main gamestate information follows, possibly stored as zlib-comprssed XML archive in the last entry in the main archive.");
    const std::string BIN_SAVE_FILE_DESCRIPTION("This is binary archive FreeOrion saved game.");

    const std::string XML_COMPRESSED_MARKER("zlib-xml");

    /// Splits time and date on separate lines for an ISO datetime string
    std::string split_time(const std::string& time) {
        std::string result = time;
        std::string::size_type pos = result.find('T');
        if (pos != std::string::npos) {
            result.replace(pos, 1, "\n");
        }
        return result;
    }

    /// Populates a SaveGamePreviewData from a given file
    /// returns true on success, false if preview data could not be found
    bool LoadSaveGamePreviewData(const fs::path& path, FullPreview& full) {
        if (!fs::exists(path)) {
            DebugLogger() << "LoadSaveGamePreviewData: Save file note found: " << path.string();
            return false;
        }

        fs::ifstream ifs(path, std::ios_base::binary);

        full.filename = PathToString(path.filename());

        if (!ifs)
            throw std::runtime_error(UNABLE_TO_OPEN_FILE);

        // alias structs so variable passed into NVP deserialization macro has the
        // same name as that passed into serialization macro in SaveGame function.
        SaveGamePreviewData& save_preview_data = full.preview;
        GalaxySetupData& galaxy_setup_data = full.galaxy;

        DebugLogger() << "LoadSaveGamePreviewData: Loading preview from: " << path.string();
        try {
            // read the first five letters of the stream and check if it is opening an xml file
            std::string xxx5(5, ' ');
            ifs.read(&xxx5[0], 5);
            const std::string xml5{"<?xml"};
            // reset to start of stream
            boost::iostreams::seek(ifs, 0, std::ios_base::beg);
            // binary deserialization iff document is not xml
            if (xml5 != xxx5) {
                ScopedTimer timer("LoadSaveGamePreviewData (binary): " + path.string(), true);

                // first attempt binary deserialziation
                freeorion_bin_iarchive ia(ifs);

                ia >> BOOST_SERIALIZATION_NVP(save_preview_data);
                ia >> BOOST_SERIALIZATION_NVP(galaxy_setup_data);

            } else {
                DebugLogger() << "Deserializing XML data";
                freeorion_xml_iarchive ia(ifs);
                ia >> BOOST_SERIALIZATION_NVP(save_preview_data);

                if (BOOST_VERSION >= 106600 && save_preview_data.save_format_marker == XML_COMPRESSED_MARKER)
                    throw std::invalid_argument("Save Format Not Compatible with Boost Version " BOOST_LIB_VERSION);

                ia >> BOOST_SERIALIZATION_NVP(galaxy_setup_data);
            }

            DebugLogger() << "Loaded preview with: " << save_preview_data.number_of_human_players << " human players";

        } catch (const std::exception& e) {
            ErrorLogger() << "LoadSaveGamePreviewData: Failed to read preview of " << path.string() << " because: " << e.what();
            return false;
        }

        if (full.preview.Valid()) {
            DebugLogger() << "LoadSaveGamePreviewData: Successfully loaded preview from: " << path.string();
            return true;
        } else {
            DebugLogger() << "LoadSaveGamePreviewData: Passing save file with no preview: " << path.string();
            return false;
        }
    }
}


SaveGamePreviewData::SaveGamePreviewData() :
    magic_number(PREVIEW_PRESENT_MARKER),
    description(),
    freeorion_version(UserString("UNKNOWN_VALUE_SYMBOL_2")),
    main_player_name(UserString("UNKNOWN_VALUE_SYMBOL_2")),
    main_player_empire_name(UserString("UNKNOWN_VALUE_SYMBOL_2")),
    current_turn(-1),
    number_of_empires(-1),
    number_of_human_players(-1),
    save_format_marker("")
{}

bool SaveGamePreviewData::Valid() const
{ return magic_number == SaveGamePreviewData::PREVIEW_PRESENT_MARKER && current_turn >= -1; }

void SaveGamePreviewData::SetBinary(bool bin)
{ description = bin ? BIN_SAVE_FILE_DESCRIPTION : XML_SAVE_FILE_DESCRIPTION; }

template<class Archive>
void SaveGamePreviewData::serialize(Archive& ar, unsigned int version)
{
    if (version >= 2) {
        if (Archive::is_saving::value) {
            freeorion_version = FreeOrionVersionString();
        }
        ar & BOOST_SERIALIZATION_NVP(description)
           & BOOST_SERIALIZATION_NVP(freeorion_version);
        if (version >= 3) {
            ar & BOOST_SERIALIZATION_NVP(save_format_marker);
            if (version >= 4) {
                ar & BOOST_SERIALIZATION_NVP(uncompressed_text_size)
                   & BOOST_SERIALIZATION_NVP(compressed_text_size);
            }
        }
    }
    ar & BOOST_SERIALIZATION_NVP(magic_number)
       & BOOST_SERIALIZATION_NVP(main_player_name);
    ar & BOOST_SERIALIZATION_NVP(main_player_empire_name)
       & BOOST_SERIALIZATION_NVP(main_player_empire_colour)
       & BOOST_SERIALIZATION_NVP(save_time)
       & BOOST_SERIALIZATION_NVP(current_turn);
    if (version > 0) {
        ar & BOOST_SERIALIZATION_NVP(number_of_empires)
           & BOOST_SERIALIZATION_NVP(number_of_human_players);
    }
}

template void SaveGamePreviewData::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, unsigned int);
template void SaveGamePreviewData::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, unsigned int);
template void SaveGamePreviewData::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, unsigned int);
template void SaveGamePreviewData::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, unsigned int);


template<class Archive>
void FullPreview::serialize(Archive& ar, unsigned int version)
{
    ar & BOOST_SERIALIZATION_NVP(filename)
       & BOOST_SERIALIZATION_NVP(preview)
       & BOOST_SERIALIZATION_NVP(galaxy);
}

template void FullPreview::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, unsigned int);
template void FullPreview::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, unsigned int);
template void FullPreview::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, unsigned int);
template void FullPreview::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, unsigned int);


template<typename Archive>
void PreviewInformation::serialize(Archive& ar, const unsigned int version)
{
    ar & BOOST_SERIALIZATION_NVP(subdirectories)
       & BOOST_SERIALIZATION_NVP(folder)
       & BOOST_SERIALIZATION_NVP(previews);
}

template void PreviewInformation::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void PreviewInformation::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void PreviewInformation::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void PreviewInformation::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);


bool SaveFileWithValidHeader(const boost::filesystem::path& path) {
    if (!fs::exists(path))
        return false;

    fs::ifstream ifs(path, std::ios_base::binary);
    if (!ifs)
        return false;

    // dummy holders for deserialized data
    SaveGamePreviewData                 ignored_save_preview_data;
    GalaxySetupData                     ignored_galaxy_setup_data;
    ServerSaveGameData                  ignored_server_save_game_data;
    std::vector<PlayerSaveHeaderData>   ignored_player_save_header_data;
    std::map<int, SaveGameEmpireData>   ignored_empire_save_game_data;

    DebugLogger() << "SaveFileWithValidHeader: Loading headers from: " << path.string();
    try {
        // read the first five letters of the stream and check if it is opening an xml file
        std::string xxx5(5, ' ');
        ifs.read(&xxx5[0], 5);
        const std::string xml5{"<?xml"};
        // reset to start of stream 
        boost::iostreams::seek(ifs, 0, std::ios_base::beg);
        // binary deserialization iff document is not xml
        if (xml5 != xxx5) {
            ScopedTimer timer("SaveFileWithValidHeader (binary): " + path.string(), true);

            freeorion_bin_iarchive ia(ifs);

            ia >> BOOST_SERIALIZATION_NVP(ignored_save_preview_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_galaxy_setup_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_server_save_game_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_player_save_header_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_empire_save_game_data);
        } else {
            DebugLogger() << "Deserializing XML data";
            freeorion_xml_iarchive ia(ifs);

            ia >> BOOST_SERIALIZATION_NVP(ignored_save_preview_data);

            if (BOOST_VERSION >= 106600 && ignored_save_preview_data.save_format_marker == XML_COMPRESSED_MARKER)
                throw std::invalid_argument("Save Format Not Compatible with Boost Version " BOOST_LIB_VERSION);

            ia >> BOOST_SERIALIZATION_NVP(ignored_galaxy_setup_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_server_save_game_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_player_save_header_data);
            ia >> BOOST_SERIALIZATION_NVP(ignored_empire_save_game_data);
        }

    } catch (const std::exception& e) {
        ErrorLogger() << "SaveFileWithValidHeader: Failed to read headers of " << path.string() << " because: " << e.what();
        return false;
    }
    return true;
}

std::string ColumnInPreview(const FullPreview& full, const std::string& name, bool thin) {
    if (name == "player") {
        return full.preview.main_player_name;
    } else if (name == "empire") {
        return full.preview.main_player_empire_name;
    } else if (name == "turn") {
        return std::to_string(full.preview.current_turn);
    } else if (name == "time") {
        if (thin) {
            return split_time(full.preview.save_time);
        } else {
            return full.preview.save_time;
        }
    } else if (name == "file") {
        return full.filename;
    } else if (name == "galaxy_size") {
        return std::to_string(full.galaxy.m_size);
    } else if (name == "seed") {
        return full.galaxy.m_seed;
    } else if (name == "galaxy_age") {
        return TextForGalaxySetupSetting(full.galaxy.m_age);
    } else if (name == "monster_freq") {
        return TextForGalaxySetupSetting(full.galaxy.m_monster_freq);
    } else if (name == "native_freq") {
        return TextForGalaxySetupSetting(full.galaxy.m_native_freq);
    } else if (name == "planet_freq") {
        return TextForGalaxySetupSetting(full.galaxy.m_planet_density);
    } else if (name == "specials_freq") {
        return TextForGalaxySetupSetting(full.galaxy.m_specials_freq);
    } else if (name == "starlane_freq") {
        return TextForGalaxySetupSetting(full.galaxy.m_starlane_freq);
    } else if (name == "galaxy_shape") {
        return TextForGalaxyShape(full.galaxy.m_shape);
    } else if (name == "ai_aggression") {
        return TextForAIAggression(full.galaxy.m_ai_aggr);
    } else if (name == "number_of_empires") {
        return std::to_string(full.preview.number_of_empires);
    } else if (name == "number_of_humans") {
        return std::to_string(full.preview.number_of_human_players);
    } else {
        ErrorLogger() << "FullPreview::Value Error: no such preview field: " << name;
        return "??";
    }
}

void LoadSaveGamePreviews(const fs::path& orig_path, const std::string& extension, std::vector<FullPreview>& previews) {
    FullPreview data;
    fs::directory_iterator end_it;

    fs::path path = orig_path;
    // Relative path relative to the save directory
    if (path.is_relative()) {
        ErrorLogger() << "LoadSaveGamePreviews: supplied path must not be relative, \"" << path << "\" ";
        return;
    }

    if (!fs::exists(path)) {
        ErrorLogger() << "LoadSaveGamePreviews: Save Game directory \"" << path << "\" not found";
        return;
    }
    if (!fs::is_directory(path)) {
        ErrorLogger() << "LoadSaveGamePreviews: Save Game directory \"" << path << "\" was not a directory";
        return;
    }

    for (fs::directory_iterator it(path); it != end_it; ++it) {
        try {
            if ((it->path().filename().extension() == extension) && !fs::is_directory(it->path())) {
                if (LoadSaveGamePreviewData(*it, data)) {
                    // Add preview entry to list
                    previews.push_back(data);
                }
            }
        } catch (const std::exception& e) {
            ErrorLogger() << "LoadSaveGamePreviews: Failed loading preview from " << it->path() << " because: " << e.what();
        }
    }
}
