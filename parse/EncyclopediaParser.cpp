#include "Parse.h"

#include <boost/filesystem/fstream.hpp>
#include <yaml-cpp/yaml.h>
#include "../universe/Encyclopedia.h"
#include "../util/Directories.h"
#include "../util/ScopedTimer.h"


namespace parse {
    Encyclopedia::ArticleMap encyclopedia_articles(const boost::filesystem::path& path) {
        Encyclopedia::ArticleMap articles;

        ScopedTimer timer("Encyclopedia Parsing", true);

        for (const auto& file : ListDir(path, IsYAML)) {
            YAML::Node doc;
            try {
                boost::filesystem::ifstream ifs(file);
                doc = YAML::Load(ifs);
                ifs.close();
            }
            catch(YAML::Exception& e) {
                ErrorLogger() << file.filename() << ":" << e.mark.line << ":" << e.mark.column << ": " << e.what();
                continue;
            }

            if (!doc["articles"]) {
                ErrorLogger() << file.filename() << ": articles key not found, skipping.";
                continue;
            }

            for (const auto& article_node : doc["articles"]) {
                articles[article_node["category"].as<std::string>()].emplace_back(
                    article_node["name"].as<std::string>(),
                    article_node["category"].as<std::string>(),
                    article_node["short_description"].as<std::string>(""),
                    article_node["description"].as<std::string>(),
                    article_node["icon"].as<std::string>());
            }
        }

        return articles;
    }
}
