#include "Encyclopedia.h"

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "../parse/Parse.h"

const Encyclopedia& GetEncyclopedia() {
    static Encyclopedia encyclopedia;
    return encyclopedia;
}

Encyclopedia::Encyclopedia() :
    articles()
{
    parse::encyclopedia_articles(GetResourceDir() / "encyclopedia.txt", *this);
    if (GetOptionsDB().Get<bool>("verbose-logging")) {
        DebugLogger() << "(Category) Encyclopedia Articles:";
        for (std::map<std::string, std::vector<EncyclopediaArticle> >::const_iterator
             category_it = articles.begin(); category_it != articles.end(); ++category_it)
        {
            const std::string& category = category_it->first;
            const std::vector<EncyclopediaArticle>& article_vec = category_it->second;
            for (std::vector<EncyclopediaArticle>::const_iterator article_it = article_vec.begin();
                 article_it != article_vec.end(); ++article_it)
            { DebugLogger() << "(" << UserString(category) << ") : " << UserString(article_it->name); }
        }
    }
}
