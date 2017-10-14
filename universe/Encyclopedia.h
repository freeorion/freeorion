#ifndef _Encyclopedia_h_
#define _Encyclopedia_h_

#include "../util/Pending.h"

#include <boost/optional/optional.hpp>

#include <string>
#include <map>
#include <vector>

#include "../util/Export.h"

struct FO_COMMON_API EncyclopediaArticle {
    EncyclopediaArticle() :
        name(""),
        category(""),
        short_description(""),
        description(""),
        icon("")
    {}
    EncyclopediaArticle(const std::string& name_, const std::string& category_,
                        const std::string& short_description_, const std::string& description_,
                        const std::string& icon_) :
        name(name_),
        category(category_),
        short_description(short_description_),
        description(description_),
        icon(icon_)
    {}
    std::string name;
    std::string category;
    std::string short_description;
    std::string description;
    std::string icon;
};

class FO_COMMON_API Encyclopedia {
public:
    using ArticleMap = std::map<std::string, std::vector<EncyclopediaArticle>>;

    Encyclopedia();
    unsigned int GetCheckSum() const;

    /** Sets articles to the value of \p future. */
    FO_COMMON_API void SetArticles(Pending::Pending<ArticleMap>&& future);

    FO_COMMON_API const ArticleMap& Articles() const;

    const EncyclopediaArticle                               empty_article;
private:
    mutable ArticleMap m_articles;

    /** Future articles.  mutable so that it can be assigned to m_articles when completed.*/
    mutable boost::optional<Pending::Pending<ArticleMap>> m_pending_articles = boost::none;
};

FO_COMMON_API Encyclopedia& GetEncyclopedia();

#endif
