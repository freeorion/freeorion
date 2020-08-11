#ifndef _Encyclopedia_h_
#define _Encyclopedia_h_


#include <map>
#include <string>
#include <vector>
#include <boost/optional/optional.hpp>
#include "../util/Export.h"
#include "../util/Pending.h"


struct FO_COMMON_API EncyclopediaArticle {
    EncyclopediaArticle() = default;
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
    // map from category name to list of articles in that category
    using ArticleMap = std::map<std::string, std::vector<EncyclopediaArticle>>;

    Encyclopedia() {};
    unsigned int GetCheckSum() const;

    /** Sets articles to the value of \p future. */
    FO_COMMON_API void SetArticles(Pending::Pending<ArticleMap>&& future);

    FO_COMMON_API const ArticleMap& Articles() const;

    FO_COMMON_API const EncyclopediaArticle& GetArticleByKey(const std::string& key) const;
    FO_COMMON_API const EncyclopediaArticle& GetArticleByCategoryAndKey(const std::string& category, const std::string& key) const;
    FO_COMMON_API const EncyclopediaArticle& GetArticleByName(const std::string& name) const;

    const EncyclopediaArticle empty_article;
private:
    mutable ArticleMap m_articles;

    /** Future articles.  mutable so that it can be assigned to m_articles when completed.*/
    mutable boost::optional<Pending::Pending<ArticleMap>> m_pending_articles = boost::none;
};

FO_COMMON_API Encyclopedia& GetEncyclopedia();

#endif
