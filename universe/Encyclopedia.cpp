#include "Encyclopedia.h"

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/CheckSums.h"
#include "../parse/Parse.h"


const Encyclopedia& GetEncyclopedia() {
    static Encyclopedia encyclopedia;
    return encyclopedia;
}

Encyclopedia::Encyclopedia() :
    articles(),
    empty_article()
{
    try {
        articles = parse::encyclopedia_articles();
        DebugLogger() << "Encyclopedia Articles checksum: " << GetCheckSum();

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

unsigned int Encyclopedia::GetCheckSum() const {
    unsigned int retval{0};

    for (const auto& n : articles) {
        CheckSums::CheckSumCombine(retval, n.first);
        for (const auto& a : n.second) {
            CheckSums::CheckSumCombine(retval, a.name);
            CheckSums::CheckSumCombine(retval, a.category);
            CheckSums::CheckSumCombine(retval, a.short_description);
            CheckSums::CheckSumCombine(retval, a.description);
            CheckSums::CheckSumCombine(retval, a.icon);
        }
        CheckSums::CheckSumCombine(retval, n.second.size());
    }
    CheckSums::CheckSumCombine(retval, articles.size());

    return retval;
}
