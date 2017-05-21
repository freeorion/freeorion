#include "Encyclopedia.h"

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/CheckSums.h"
#include "../parse/Parse.h"

namespace CheckSums {
    void CheckSumCombine(unsigned int& sum, const Encyclopedia& c) {
        TraceLogger() << "CheckSumCombine(Encyclopedia)";
        for (const auto& n : c.articles) {
            CheckSumCombine(sum, n.first);
            for (const auto& a : n.second) {
                CheckSumCombine(sum, a.name);
                CheckSumCombine(sum, a.category);
                CheckSumCombine(sum, a.short_description);
                CheckSumCombine(sum, a.description);
                CheckSumCombine(sum, a.icon);
            }
            CheckSumCombine(sum, n.second.size());
        }
        CheckSumCombine(sum, c.articles.size());
    }
}


const Encyclopedia& GetEncyclopedia() {
    static Encyclopedia encyclopedia;
    return encyclopedia;
}

Encyclopedia::Encyclopedia() :
    articles(),
    empty_article()
{
    try {
        parse::encyclopedia_articles(*this);

        unsigned int checksum{0};
        CheckSums::CheckSumCombine(checksum, this);
        DebugLogger() << "Encyclopedia Articles checksum: " << checksum;

    } catch (const std::exception& e) {
        ErrorLogger() << "Failed parsing encyclopedia articles: error: " << e.what();
        throw e;
    }

    TraceLogger() << "(Category) Encyclopedia Articles:";
    for (const auto& entry : articles) {
        const std::string& category = entry.first;
        for (const EncyclopediaArticle& article : entry.second)
        { TraceLogger() << "(" << category << ") : " << article.name; }
    }
}
