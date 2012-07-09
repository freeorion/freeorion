// -*- C++ -*-
#ifndef _Encyclopedia_h_
#define _Encyclopedia_h_

#include <string>
#include <map>
#include <vector>

struct EncyclopediaArticle {
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

struct Encyclopedia {
    Encyclopedia();
    std::map<std::string, std::vector<EncyclopediaArticle> >   articles;
};

#endif
