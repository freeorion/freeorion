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
    empty_article(),
    m_articles()
{
    m_pending_articles = std::async(std::launch::async, []{
            return parse::encyclopedia_articles();});
}

unsigned int Encyclopedia::GetCheckSum() const {
    unsigned int retval{0};

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

void Encyclopedia::SetArticles(std::future<ArticleMap>&& future)
{ m_pending_articles = std::move(future); }

const Encyclopedia::ArticleMap& Encyclopedia::Articles() const {
    if (m_pending_articles != boost::none) {
        // Only print waiting message if not immediately ready
        while (m_pending_articles->wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
            DebugLogger() << "Waiting for Articles to parse.";
        }

        try {
            auto x = std::move(m_pending_articles->get());
            std::swap(m_articles, x);

            TraceLogger() << "(Category) Encyclopedia Articles:";
            for (const auto& entry : m_articles) {
                const std::string& category = entry.first;
                for (const EncyclopediaArticle& article : entry.second)
                { TraceLogger() << "(" << category << ") : " << article.name; }
            }

        } catch (const std::exception& e) {
            ErrorLogger() << "Failed parsing articles: error: " << e.what();
            throw e;
        }

        m_pending_articles = boost::none;
    }

    return m_articles;
}
