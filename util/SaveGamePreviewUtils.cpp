/*
 *
 */

#include "OptionsDB.h"
#include "SaveGamePreviewUtils.h"

#include "i18n.h"
#include "Directories.h"
#include "Logger.h"
#include "MultiplayerCommon.h"
#include "EnumText.h"
#include "Serialize.h"
#include "Serialize.ipp"

#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>

#include <fstream>

namespace fs = boost::filesystem;

namespace {
    const std::string UNABLE_TO_OPEN_FILE("Unable to open file");

    /// Splits time and date on separate lines for an ISO datetime string
    std::string split_time(const std::string& time) {
        std::string result = time;
        std::string::size_type pos = result.find("T");
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

        full.filename = PathString(path.filename());

        if (!ifs)
            throw std::runtime_error(UNABLE_TO_OPEN_FILE);


        DebugLogger() << "LoadSaveGamePreviewData: Loading preview from:" << path.string();
        try {
            try {
                // first attempt binary deserialziation
                freeorion_bin_iarchive ia(ifs);

                ia >> BOOST_SERIALIZATION_NVP(full.preview);
                ia >> BOOST_SERIALIZATION_NVP(full.galaxy);

            } catch (...) {
                // if binary deserialization failed, try more-portable XML deserialization

                // reset to start of stream (attempted binary serialization will have consumed some input...)
                boost::iostreams::seek(ifs, 0, std::ios_base::beg);


                // set up filter to decompress data
                boost::iostreams::filtering_istreambuf i;
                i.push(boost::iostreams::zlib_decompressor());
                i.push(ifs);

                // pass decompressed xml into stringstream storage that the iarchve requires...
                std::stringstream ss;
                boost::iostreams::copy(i, ss);

                // extract xml data from stringstream
                freeorion_xml_iarchive ia(ss);

                ia >> BOOST_SERIALIZATION_NVP(full.preview);
                ia >> BOOST_SERIALIZATION_NVP(full.galaxy);
            }

        } catch (const std::exception& e) {
            ErrorLogger() << "LoadSaveGamePreviewData: Failed to read preview of " << path.string() << " because: " << e.what();
            return false;
        }

        if (full.preview.Valid()) {
            DebugLogger() << "LoadSaveGamePreviewData: Successfully loaded preview from:" << path.string();
            return true;
        } else {
            DebugLogger() << "LoadSaveGamePreviewData: Passing save file with no preview: " << path.string();
            return false;
        }
    }
}


SaveGamePreviewData::SaveGamePreviewData() :
    magic_number(PREVIEW_PRESENT_MARKER),
    main_player_name(UserString("UNKNOWN_VALUE_SYMBOL_2")),
    main_player_empire_name(UserString("UNKNOWN_VALUE_SYMBOL_2")),
    current_turn(-1),
    number_of_empires(-1),
    number_of_human_players(-1)
{}

bool SaveGamePreviewData::Valid() const {
    return magic_number == SaveGamePreviewData::PREVIEW_PRESENT_MARKER && current_turn >= -1 ;
}

template<class Archive>
void SaveGamePreviewData::serialize(Archive& ar, unsigned int version)
{
    ar & BOOST_SERIALIZATION_NVP(magic_number)
       & BOOST_SERIALIZATION_NVP(main_player_name)
       & BOOST_SERIALIZATION_NVP(main_player_empire_name)
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
void PreviewInformation::serialize(Archive& ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_NVP(subdirectories)
       & BOOST_SERIALIZATION_NVP(folder)
       & BOOST_SERIALIZATION_NVP(previews);
}

template void PreviewInformation::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
template void PreviewInformation::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
template void PreviewInformation::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
template void PreviewInformation::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

std::string ColumnInPreview(const FullPreview& full, const std::string& name, bool thin) {
    if (name == "player") {
        return full.preview.main_player_name;
    } else if (name == "empire") {
        return full.preview.main_player_empire_name;
    } else if (name == "turn") {
        return boost::lexical_cast<std::string>(full.preview.current_turn);
    } else if (name == "time") {
        if (thin) {
            return split_time(full.preview.save_time);
        } else {
            return full.preview.save_time;
        }
    } else if (name == "file") {
        return full.filename;
    } else if (name == "galaxy_size") {
        return boost::lexical_cast<std::string>(full.galaxy.m_size);
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
        return boost::lexical_cast<std::string>(full.preview.number_of_empires);
    } else if (name == "number_of_humans") {
        return boost::lexical_cast<std::string>(full.preview.number_of_human_players);
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
        path = GetSaveDir() / path;
    }

    if (!fs::exists(path)) {
        ErrorLogger() << "SaveFileListBox::LoadSaveGamePreviews: Save Game directory \"" << path << "\" not found";
        return;
    }
    if (!fs::is_directory(path)) {
        ErrorLogger() << "SaveFileListBox::LoadSaveGamePreviews: Save Game directory \"" << path << "\" was not a directory";
        return;
    }

    for (fs::directory_iterator it(path); it != end_it; ++it) {
        try {
            std::string filename = PathString(it->path().filename());
            if (it->path().filename().extension() == extension & !fs::is_directory(it->path())) {
                if (LoadSaveGamePreviewData(*it, data)) {
                    // Add preview entry to list
                    previews.push_back(data);
                }
            }
        } catch (const std::exception& e) {
            ErrorLogger() << "SaveFileListBox::LoadSaveGamePreviews: Failed loading preview from " << it->path() << " because: " << e.what();
        }
    }
}

bool IsInside(const fs::path& path, const fs::path& directory) {
    const fs::path target = fs::canonical(directory);

    if (!path.has_parent_path()) {
        return false;
    }

    fs::path cur = path.parent_path();
    while (cur.has_parent_path()) {
        if (cur == target) {
            return true;
        } else {
            cur = cur.parent_path();
        }
    }
    return false;
}
