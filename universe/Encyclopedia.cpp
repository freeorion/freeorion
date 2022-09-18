#include "Encyclopedia.h"

#include "../util/CheckSums.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/i18n.h"


namespace {
    const EncyclopediaArticle empty_article;
}

Encyclopedia& GetEncyclopedia() {
    static Encyclopedia encyclopedia;
    return encyclopedia;
}

uint32_t Encyclopedia::GetCheckSum() const {
    uint32_t retval{0};

    for (const auto& n : Articles()) {
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
    CheckSums::CheckSumCombine(retval, Articles().size());

    return retval;
}

void Encyclopedia::SetArticles(Pending::Pending<ArticleMap>&& future)
{ m_pending_articles = std::move(future); }

const Encyclopedia::ArticleMap& Encyclopedia::Articles() const {
    if (auto parsed = WaitForPending(m_pending_articles)) {
        std::swap(m_articles, *parsed);

        TraceLogger() << "(Category) Encyclopedia Articles:";
        for (const auto& [category, articles] : m_articles) {
            for (const EncyclopediaArticle& article : articles)
                TraceLogger() << "(" << category << ") : " << article.name;
        }
    }

    return m_articles;
}

const EncyclopediaArticle& Encyclopedia::GetArticleByKey(const std::string& key) const {
    const auto& articles = Articles();
    for (auto& category_articles : articles) {
        for (auto& article : category_articles.second) {
            if (article.name == key)
                return article;
        }
    }
    return empty_article;
}

const EncyclopediaArticle& Encyclopedia::GetArticleByCategoryAndKey(
    std::string_view category, std::string_view key) const
{
    const auto& articles = Articles();
    auto it = articles.find(category);
    if (it == articles.end())
        return empty_article;
    const auto& category_articles = it->second;
    for (auto& article : category_articles) {
        if (article.name == key)
            return article;
    }
    return empty_article;
}

const EncyclopediaArticle& Encyclopedia::GetArticleByName(const std::string& name) const {
    const auto& articles = Articles();
    for (auto& category_articles : articles) {
        for (auto& article : category_articles.second) {
            if (UserString(article.name) == name)
                return article;
        }
    }
    return empty_article;
}
