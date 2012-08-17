/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <GG/adobe/implementation/expression_filter.hpp>

#include <iterator>
#include <cctype>

#include <boost/bind.hpp>

#include <GG/adobe/algorithm/copy.hpp>
#include <GG/adobe/algorithm/find.hpp>
#include <GG/adobe/algorithm/lower_bound.hpp>
#include <GG/adobe/functional.hpp>
#include <GG/adobe/table_index.hpp>
#if 0 // TZL
#include <GG/adobe/xml_parser.hpp>
#endif

/*************************************************************************************************/

namespace {

/*************************************************************************************************/

template <typename I,
          typename O,
          typename P>
std::pair<I, O> copy_until(I first,
                           I last,
                           O output,
                           P pred = adobe::always_true<typename std::iterator_traits<I>::value_type>())
{
    for (; first != last && pred(*first); ++first)
        *output++ = *first;

    return std::make_pair(first, output);
}

/*************************************************************************************************/

#define ADOBE_CODE_POINT(x) boost::uint32_t(x##UL)

/*************************************************************************************************/

typedef std::pair<adobe::string_t, boost::uint32_t> code_point_set_value_type;
typedef adobe::vector<code_point_set_value_type>    code_point_set_t;

/*************************************************************************************************/

const code_point_set_t& code_point_set()
{
    typedef code_point_set_value_type value_type;

    static code_point_set_t code_point_set_s;
    static bool             inited(false);

    if (!inited)
    {
        inited = true;

        value_type default_entry_set[] =
        {
            value_type("Aacute", ADOBE_CODE_POINT(0x00C1)), value_type("aacute", ADOBE_CODE_POINT(0x00E1)), value_type("Acirc", ADOBE_CODE_POINT(0x00C2)), value_type("acirc", ADOBE_CODE_POINT(0x00E2)),
            value_type("acute", ADOBE_CODE_POINT(0x00B4)), value_type("AElig", ADOBE_CODE_POINT(0x00C6)), value_type("aelig", ADOBE_CODE_POINT(0x00E6)), value_type("Agrave", ADOBE_CODE_POINT(0x00C0)),
            value_type("agrave", ADOBE_CODE_POINT(0x00E0)), value_type("alefsym", ADOBE_CODE_POINT(0x2135)), value_type("Alpha", ADOBE_CODE_POINT(0x0391)), value_type("alpha", ADOBE_CODE_POINT(0x03B1)),
            value_type("amp", ADOBE_CODE_POINT(0x0026)), value_type("and", ADOBE_CODE_POINT(0x2227)), value_type("ang", ADOBE_CODE_POINT(0x2220)), value_type("Aring", ADOBE_CODE_POINT(0x00C5)),
            value_type("aring", ADOBE_CODE_POINT(0x00E5)), value_type("asymp", ADOBE_CODE_POINT(0x2248)), value_type("Atilde", ADOBE_CODE_POINT(0x00C3)), value_type("atilde", ADOBE_CODE_POINT(0x00E3)),
            value_type("Auml", ADOBE_CODE_POINT(0x00C4)), value_type("auml", ADOBE_CODE_POINT(0x00E4)), value_type("bdquo", ADOBE_CODE_POINT(0x201E)), value_type("Beta", ADOBE_CODE_POINT(0x0392)),
            value_type("beta", ADOBE_CODE_POINT(0x03B2)), value_type("brvbar", ADOBE_CODE_POINT(0x00A6)), value_type("bull", ADOBE_CODE_POINT(0x2022)), value_type("cap", ADOBE_CODE_POINT(0x2229)),
            value_type("Ccedil", ADOBE_CODE_POINT(0x00C7)), value_type("ccedil", ADOBE_CODE_POINT(0x00E7)), value_type("cedil", ADOBE_CODE_POINT(0x00B8)), value_type("cent", ADOBE_CODE_POINT(0x00A2)),
            value_type("Chi", ADOBE_CODE_POINT(0x03A7)), value_type("chi", ADOBE_CODE_POINT(0x03C7)), value_type("circ", ADOBE_CODE_POINT(0x02C6)), value_type("clubs", ADOBE_CODE_POINT(0x2663)),
            value_type("cong", ADOBE_CODE_POINT(0x2245)), value_type("copy", ADOBE_CODE_POINT(0x00A9)), value_type("cr", ADOBE_CODE_POINT(0x000D)), value_type("crarr", ADOBE_CODE_POINT(0x21B5)),
            value_type("cup", ADOBE_CODE_POINT(0x222A)), value_type("curren", ADOBE_CODE_POINT(0x00A4)), value_type("dagger", ADOBE_CODE_POINT(0x2020)), value_type("Dagger", ADOBE_CODE_POINT(0x2021)),
            value_type("darr", ADOBE_CODE_POINT(0x2193)), value_type("dArr", ADOBE_CODE_POINT(0x21D3)), value_type("deg", ADOBE_CODE_POINT(0x00B0)), value_type("Delta", ADOBE_CODE_POINT(0x0394)),
            value_type("delta", ADOBE_CODE_POINT(0x03B4)), value_type("diams", ADOBE_CODE_POINT(0x2666)), value_type("divide", ADOBE_CODE_POINT(0x00F7)), value_type("Eacute", ADOBE_CODE_POINT(0x00C9)),
            value_type("eacute", ADOBE_CODE_POINT(0x00E9)), value_type("Ecirc", ADOBE_CODE_POINT(0x00CA)), value_type("ecirc", ADOBE_CODE_POINT(0x00EA)), value_type("Egrave", ADOBE_CODE_POINT(0x00C8)),
            value_type("egrave", ADOBE_CODE_POINT(0x00E8)), value_type("empty", ADOBE_CODE_POINT(0x2205)), value_type("emsp", ADOBE_CODE_POINT(0x2003)), value_type("ensp", ADOBE_CODE_POINT(0x2002)),
            value_type("Epsilon", ADOBE_CODE_POINT(0x0395)), value_type("epsilon", ADOBE_CODE_POINT(0x03B5)), value_type("equiv", ADOBE_CODE_POINT(0x2261)), value_type("Eta", ADOBE_CODE_POINT(0x0397)),
            value_type("eta", ADOBE_CODE_POINT(0x03B7)), value_type("ETH", ADOBE_CODE_POINT(0x00D0)), value_type("eth", ADOBE_CODE_POINT(0x00F0)), value_type("Euml", ADOBE_CODE_POINT(0x00CB)),
            value_type("euml", ADOBE_CODE_POINT(0x00EB)), value_type("euro", ADOBE_CODE_POINT(0x20AC)), value_type("exist", ADOBE_CODE_POINT(0x2203)), value_type("fnof", ADOBE_CODE_POINT(0x0192)),
            value_type("forall", ADOBE_CODE_POINT(0x2200)), value_type("frac12", ADOBE_CODE_POINT(0x00BD)), value_type("frac14", ADOBE_CODE_POINT(0x00BC)), value_type("frac34", ADOBE_CODE_POINT(0x00BE)),
            value_type("frasl", ADOBE_CODE_POINT(0x2044)), value_type("Gamma", ADOBE_CODE_POINT(0x0393)), value_type("gamma", ADOBE_CODE_POINT(0x03B3)), value_type("ge", ADOBE_CODE_POINT(0x2265)),
            value_type("gt", ADOBE_CODE_POINT(0x003E)), value_type("harr", ADOBE_CODE_POINT(0x2194)), value_type("hArr", ADOBE_CODE_POINT(0x21D4)), value_type("hearts", ADOBE_CODE_POINT(0x2665)),
            value_type("hellip", ADOBE_CODE_POINT(0x2026)), value_type("Iacute", ADOBE_CODE_POINT(0x00CD)), value_type("iacute", ADOBE_CODE_POINT(0x00ED)), value_type("Icirc", ADOBE_CODE_POINT(0x00CE)),
            value_type("icirc", ADOBE_CODE_POINT(0x00EE)), value_type("iexcl", ADOBE_CODE_POINT(0x00A1)), value_type("Igrave", ADOBE_CODE_POINT(0x00CC)), value_type("igrave", ADOBE_CODE_POINT(0x00EC)),
            value_type("image", ADOBE_CODE_POINT(0x2111)), value_type("infin", ADOBE_CODE_POINT(0x221E)), value_type("int", ADOBE_CODE_POINT(0x222B)), value_type("Iota", ADOBE_CODE_POINT(0x0399)),
            value_type("iota", ADOBE_CODE_POINT(0x03B9)), value_type("iquest", ADOBE_CODE_POINT(0x00BF)), value_type("isin", ADOBE_CODE_POINT(0x2208)), value_type("Iuml", ADOBE_CODE_POINT(0x00CF)),
            value_type("iuml", ADOBE_CODE_POINT(0x00EF)), value_type("Kappa", ADOBE_CODE_POINT(0x039A)), value_type("kappa", ADOBE_CODE_POINT(0x03BA)), value_type("Lambda", ADOBE_CODE_POINT(0x039B)),
            value_type("lambda", ADOBE_CODE_POINT(0x03BB)), value_type("lang", ADOBE_CODE_POINT(0x2329)), value_type("laquo", ADOBE_CODE_POINT(0x00AB)), value_type("larr", ADOBE_CODE_POINT(0x2190)),
            value_type("lArr", ADOBE_CODE_POINT(0x21D0)), value_type("lceil", ADOBE_CODE_POINT(0x2308)), value_type("ldquo", ADOBE_CODE_POINT(0x201C)), value_type("le", ADOBE_CODE_POINT(0x2264)),
            value_type("lf", ADOBE_CODE_POINT(0x000A)), value_type("lfloor", ADOBE_CODE_POINT(0x230A)), value_type("lowast", ADOBE_CODE_POINT(0x2217)), value_type("loz", ADOBE_CODE_POINT(0x25CA)),
            value_type("lrm", ADOBE_CODE_POINT(0x200E)), value_type("lsaquo", ADOBE_CODE_POINT(0x2039)), value_type("lsquo", ADOBE_CODE_POINT(0x2018)), value_type("lt", ADOBE_CODE_POINT(0x003C)),
            value_type("macr", ADOBE_CODE_POINT(0x00AF)), value_type("mdash", ADOBE_CODE_POINT(0x2014)), value_type("micro", ADOBE_CODE_POINT(0x00B5)), value_type("middot", ADOBE_CODE_POINT(0x00B7)),
            value_type("minus", ADOBE_CODE_POINT(0x2212)), value_type("Mu", ADOBE_CODE_POINT(0x039C)), value_type("mu", ADOBE_CODE_POINT(0x03BC)), value_type("nabla", ADOBE_CODE_POINT(0x2207)),
            value_type("nbsp", ADOBE_CODE_POINT(0x00A0)), value_type("ndash", ADOBE_CODE_POINT(0x2013)), value_type("ne", ADOBE_CODE_POINT(0x2260)), value_type("ni", ADOBE_CODE_POINT(0x220B)),
            value_type("not", ADOBE_CODE_POINT(0x00AC)), value_type("notin", ADOBE_CODE_POINT(0x2209)), value_type("nsub", ADOBE_CODE_POINT(0x2284)), value_type("Ntilde", ADOBE_CODE_POINT(0x00D1)),
            value_type("ntilde", ADOBE_CODE_POINT(0x00F1)), value_type("Nu", ADOBE_CODE_POINT(0x039D)), value_type("nu", ADOBE_CODE_POINT(0x03BD)), value_type("Oacute", ADOBE_CODE_POINT(0x00D3)),
            value_type("oacute", ADOBE_CODE_POINT(0x00F3)), value_type("Ocirc", ADOBE_CODE_POINT(0x00D4)), value_type("ocirc", ADOBE_CODE_POINT(0x00F4)), value_type("OElig", ADOBE_CODE_POINT(0x0152)),
            value_type("oelig", ADOBE_CODE_POINT(0x0153)), value_type("Ograve", ADOBE_CODE_POINT(0x00D2)), value_type("ograve", ADOBE_CODE_POINT(0x00F2)), value_type("oline", ADOBE_CODE_POINT(0x203E)),
            value_type("Omega", ADOBE_CODE_POINT(0x03A9)), value_type("omega", ADOBE_CODE_POINT(0x03C9)), value_type("Omicron", ADOBE_CODE_POINT(0x039F)), value_type("omicron", ADOBE_CODE_POINT(0x03BF)),
            value_type("oplus", ADOBE_CODE_POINT(0x2295)), value_type("or", ADOBE_CODE_POINT(0x2228)), value_type("ordf", ADOBE_CODE_POINT(0x00AA)), value_type("ordm", ADOBE_CODE_POINT(0x00BA)),
            value_type("Oslash", ADOBE_CODE_POINT(0x00D8)), value_type("oslash", ADOBE_CODE_POINT(0x00F8)), value_type("Otilde", ADOBE_CODE_POINT(0x00D5)), value_type("otilde", ADOBE_CODE_POINT(0x00F5)),
            value_type("otimes", ADOBE_CODE_POINT(0x2297)), value_type("Ouml", ADOBE_CODE_POINT(0x00D6)), value_type("ouml", ADOBE_CODE_POINT(0x00F6)), value_type("para", ADOBE_CODE_POINT(0x00B6)),
            value_type("part", ADOBE_CODE_POINT(0x2202)), value_type("permil", ADOBE_CODE_POINT(0x2030)), value_type("perp", ADOBE_CODE_POINT(0x22A5)), value_type("Phi", ADOBE_CODE_POINT(0x03A6)),
            value_type("phi", ADOBE_CODE_POINT(0x03C6)), value_type("Pi", ADOBE_CODE_POINT(0x03A0)), value_type("pi", ADOBE_CODE_POINT(0x03C0)), value_type("piv", ADOBE_CODE_POINT(0x03D6)),
            value_type("plusmn", ADOBE_CODE_POINT(0x00B1)), value_type("pound", ADOBE_CODE_POINT(0x00A3)), value_type("prime", ADOBE_CODE_POINT(0x2032)), value_type("Prime", ADOBE_CODE_POINT(0x2033)),
            value_type("prod", ADOBE_CODE_POINT(0x220F)), value_type("prop", ADOBE_CODE_POINT(0x221D)), value_type("Psi", ADOBE_CODE_POINT(0x03A8)), value_type("psi", ADOBE_CODE_POINT(0x03C8)),
            value_type("quot", ADOBE_CODE_POINT(0x0022)), value_type("radic", ADOBE_CODE_POINT(0x221A)), value_type("rang", ADOBE_CODE_POINT(0x232A)), value_type("raquo", ADOBE_CODE_POINT(0x00BB)),
            value_type("rarr", ADOBE_CODE_POINT(0x2192)), value_type("rArr", ADOBE_CODE_POINT(0x21D2)), value_type("rceil", ADOBE_CODE_POINT(0x2309)), value_type("rdquo", ADOBE_CODE_POINT(0x201D)),
            value_type("real", ADOBE_CODE_POINT(0x211C)), value_type("reg", ADOBE_CODE_POINT(0x00AE)), value_type("rfloor", ADOBE_CODE_POINT(0x230B)), value_type("Rho", ADOBE_CODE_POINT(0x03A1)),
            value_type("rho", ADOBE_CODE_POINT(0x03C1)), value_type("rlm", ADOBE_CODE_POINT(0x200F)), value_type("rsaquo", ADOBE_CODE_POINT(0x203A)), value_type("rsquo", ADOBE_CODE_POINT(0x2019)),
            value_type("sbquo", ADOBE_CODE_POINT(0x201A)), value_type("Scaron", ADOBE_CODE_POINT(0x0160)), value_type("scaron", ADOBE_CODE_POINT(0x0161)), value_type("sdot", ADOBE_CODE_POINT(0x22C5)),
            value_type("sect", ADOBE_CODE_POINT(0x00A7)), value_type("shy", ADOBE_CODE_POINT(0x00AD)), value_type("Sigma", ADOBE_CODE_POINT(0x03A3)), value_type("sigma", ADOBE_CODE_POINT(0x03C3)),
            value_type("sigmaf", ADOBE_CODE_POINT(0x03C2)), value_type("sim", ADOBE_CODE_POINT(0x223C)), value_type("spades", ADOBE_CODE_POINT(0x2660)), value_type("sub", ADOBE_CODE_POINT(0x2282)),
            value_type("sube", ADOBE_CODE_POINT(0x2286)), value_type("sum", ADOBE_CODE_POINT(0x2211)), value_type("sup", ADOBE_CODE_POINT(0x2283)), value_type("sup1", ADOBE_CODE_POINT(0x00B9)),
            value_type("sup2", ADOBE_CODE_POINT(0x00B2)), value_type("sup3", ADOBE_CODE_POINT(0x00B3)), value_type("supe", ADOBE_CODE_POINT(0x2287)), value_type("szlig", ADOBE_CODE_POINT(0x00DF)),
            value_type("tab", ADOBE_CODE_POINT(0x0009)), value_type("Tau", ADOBE_CODE_POINT(0x03A4)), value_type("tau", ADOBE_CODE_POINT(0x03C4)), value_type("there4", ADOBE_CODE_POINT(0x2234)),
            value_type("Theta", ADOBE_CODE_POINT(0x0398)), value_type("theta", ADOBE_CODE_POINT(0x03B8)), value_type("thetasym", ADOBE_CODE_POINT(0x03D1)), value_type("thinsp", ADOBE_CODE_POINT(0x2009)),
            value_type("THORN", ADOBE_CODE_POINT(0x00DE)), value_type("thorn", ADOBE_CODE_POINT(0x00FE)), value_type("tilde", ADOBE_CODE_POINT(0x02DC)), value_type("times", ADOBE_CODE_POINT(0x00D7)),
            value_type("trade", ADOBE_CODE_POINT(0x2122)), value_type("Uacute", ADOBE_CODE_POINT(0x00DA)), value_type("uacute", ADOBE_CODE_POINT(0x00FA)), value_type("uarr", ADOBE_CODE_POINT(0x2191)),
            value_type("uArr", ADOBE_CODE_POINT(0x21D1)), value_type("Ucirc", ADOBE_CODE_POINT(0x00DB)), value_type("ucirc", ADOBE_CODE_POINT(0x00FB)), value_type("Ugrave", ADOBE_CODE_POINT(0x00D9)),
            value_type("ugrave", ADOBE_CODE_POINT(0x00F9)), value_type("uml", ADOBE_CODE_POINT(0x00A8)), value_type("upsih", ADOBE_CODE_POINT(0x03D2)), value_type("Upsilon", ADOBE_CODE_POINT(0x03A5)),
            value_type("upsilon", ADOBE_CODE_POINT(0x03C5)), value_type("Uuml", ADOBE_CODE_POINT(0x00DC)), value_type("uuml", ADOBE_CODE_POINT(0x00FC)), value_type("weierp", ADOBE_CODE_POINT(0x2118)),
            value_type("Xi", ADOBE_CODE_POINT(0x039E)), value_type("xi", ADOBE_CODE_POINT(0x03BE)), value_type("Yacute", ADOBE_CODE_POINT(0x00DD)), value_type("yacute", ADOBE_CODE_POINT(0x00FD)),
            value_type("yen", ADOBE_CODE_POINT(0x00A5)), value_type("yuml", ADOBE_CODE_POINT(0x00FF)), value_type("Yuml", ADOBE_CODE_POINT(0x0178)), value_type("Zeta", ADOBE_CODE_POINT(0x0396)),
            value_type("zeta", ADOBE_CODE_POINT(0x03B6)), value_type("zwj", ADOBE_CODE_POINT(0x200D)), value_type("zwnj", ADOBE_CODE_POINT(0x200C))
    #if 1
            // NOTE (fbrereto) : These are not a part of the HTML default entity list, but are still useful
            , value_type("apos", ADOBE_CODE_POINT(0x0027))
    #endif
        };

        std::size_t entry_set_size(sizeof(default_entry_set) / sizeof(default_entry_set[0]));

        code_point_set_s.reserve(entry_set_size);

        adobe::copy(default_entry_set, std::back_inserter(code_point_set_s));

        assert(code_point_set_s.size() == entry_set_size);
    }

    return code_point_set_s;
}

/*************************************************************************************************/

#undef ADOBE_CODE_POINT

/*************************************************************************************************/

typedef adobe::table_index<const boost::uint32_t, const code_point_set_t::value_type> code_point_index_type;

/*************************************************************************************************/

const code_point_index_type& code_point_index()
{
    static const code_point_index_type index(code_point_set().begin(),
                                             code_point_set().end(),
                                             &code_point_set_t::value_type::second);
    static bool                        inited(false);

    if (!inited)
    {
        inited = true;

        const_cast<code_point_index_type&>(index).sort();
        // const_cast<code_point_index_type&>(index).unique();

        assert(index.size() == code_point_set().size());
    }

    return index;
}

/*************************************************************************************************/

code_point_index_type::const_iterator code_point_index_find(boost::uint32_t code_point)
{
    const code_point_index_type&          index(code_point_index());
    code_point_index_type::const_iterator found(adobe::lower_bound(index,
                                                                   code_point,
                                                                   adobe::compare_members(&code_point_set_t::value_type::second)));

    return found == index.end() || found->second != code_point ?
           index.end() :
           found;
}

/*************************************************************************************************/

typedef adobe::table_index<const adobe::string_t, const code_point_set_t::value_type> entity_name_index_type;

/*************************************************************************************************/

const entity_name_index_type& entity_name_index()
{
    static const entity_name_index_type index(code_point_set().begin(),
                                              code_point_set().end(),
                                              &code_point_set_t::value_type::first);
    static bool                         inited(false);

    if (!inited)
    {
        inited = true;

        const_cast<entity_name_index_type&>(index).sort();
        // const_cast<entity_name_index_type&>(index).unique();

        assert(index.size() == code_point_set().size());
    }

    return index;
}

/*************************************************************************************************/

entity_name_index_type::const_iterator entity_name_index_find(const adobe::string_t& entity_name)
{
    const entity_name_index_type&           index(entity_name_index());
    entity_name_index_type::const_iterator  found(adobe::lower_bound(index,
                                                                     entity_name,
                                                                     adobe::compare_members(&code_point_set_t::value_type::first)));

    return found == index.end() || found->first != entity_name ?
           index.end() :
           found;
}

/*************************************************************************************************/

} // namespace

/*************************************************************************************************/

namespace adobe {

#if 0 // TZL
/*************************************************************************************************/

const adobe::string_t& entity_map_find(boost::uint32_t code_point)
{
    code_point_index_type::const_iterator found(code_point_index_find(code_point));

    if (found == code_point_index().end())
        throw std::range_error("code point not found");

    return found->first;
}

/*************************************************************************************************/

boost::uint32_t entity_map_find(const adobe::string_t& entity)
{
    adobe::string_t::const_iterator first(entity.begin());

    if (entity.size() > 1 && *first == '#')
    {
        adobe::string_t::const_iterator last(entity.end());
        boost::uint32_t                 result;

        ++first;

        if (*first == 'x')
            adobe::xatoi(++first, last, result);
        else
            adobe::datoi(first, last, result);

        return result;
    }

    entity_name_index_type::const_iterator found(entity_name_index_find(entity));

    if (found == entity_name_index().end())
        throw std::range_error("entity name not found");

    return found->second;
}
#endif

/*************************************************************************************************/

bool needs_entity_escape(const string_t& value)
{
#if 0 // TZL
    for (string_t::const_iterator iter(value.begin()); iter != value.end(); ++iter)
    {
        char c(*iter);

        code_point_index_type::const_iterator found(code_point_index_find(static_cast<boost::uint32_t>(c)));

        if (found != code_point_index().end() ||
            std::isprint(c) == false ||
            (c & 0x80) == 1)
            return true;
    }
#endif

    return false;
}

/*************************************************************************************************/

string_t entity_escape(const string_t& value)
{
    string_t result;

    result.reserve(value.size());

    for (string_t::const_iterator iter(value.begin()); iter != value.end(); ++iter)
    {
        char c(*iter);

        code_point_index_type::const_iterator found(code_point_index_find(static_cast<boost::uint32_t>(c)));

        if (found != code_point_index().end())
        {
            result.push_back('&');
            result.append(found->first);
            result.push_back(';');
        }
        else if (std::isprint(c) == false || (c & 0x80) == 1)
        {
            char nybble1((c >> 4) & 0x0f);
            char nybble2(c & 0x0f);

            result.append(adobe::string_t("&#x"));

            result.push_back(nybble1 >= 0x0a ?
                             'A' + (nybble1 - 0x0a) :
                             '0' + nybble1);

            result.push_back(nybble2 >= 0x0a ?
                             'A' + (nybble2 - 0x0a) :
                             '0' + nybble2);

            result.push_back(';');
        }
        else
            result.push_back(c);
    }

    return result;
}

/*************************************************************************************************/

bool needs_entity_unescape(const string_t& value)
{
#if 0 // TZL
    /*
        This isn't ideal; it's possible for any string to have an ampersand
        followed by a semicolon, which will return true in this case but the
        string does not need unescaping.
    */
    string_t::const_iterator first(find(value, '&'));
    string_t::const_iterator next(std::find(first, value.end(), ';'));

    return next != value.end();
#else
    return false;
#endif
}

/*************************************************************************************************/

string_t entity_unescape(const string_t& value)
{
    string_t                 result;
    string_t::const_iterator iter(value.begin());

    result.reserve(value.size());

    while (iter != value.end())
    {
        string_t::const_iterator next(copy_until(iter,
                                                 value.end(),
                                                 std::back_inserter(result),
                                                 boost::bind(std::not_equal_to<char>(), _1, '&')).first);

        if (next == value.end())
            return result;

        ++next;

        string_t::const_iterator next_end(std::find(next, value.end(), ';'));

        if (next_end == value.end())
            return result;

        // snip out the entity and look it up in the map

#if 0 // TZL
        boost::uint32_t code_point(entity_map_find(string_t(next, next_end)));
#else
        boost::uint32_t code_point(0);
#endif

        if (code_point > 255)
            /*throw std::runtime_error("code point size overflow")*/;
        else
            result.push_back(static_cast<char>(code_point));

        iter = ++next_end;
    }

    return result;
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
