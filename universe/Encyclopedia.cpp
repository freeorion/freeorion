#include "Encyclopedia.h"

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
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
        parse::encyclopedia_articles(*this);
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
