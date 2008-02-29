#include "Parser.h"

#include "ParserUtil.h"
#include "ValueRefParser.h"

using namespace boost::spirit;
using namespace phoenix;

const Skip skip_p;

const function<push_back_impl> push_back_ = push_back_impl();
const function<insert_impl> insert_ = insert_impl();

rule<Scanner, NameClosure::context_t>   name_p;
rule<Scanner, NameClosure::context_t>   file_name_p;
rule<Scanner, ColourClosure::context_t> colour_p;

symbols<PlanetSize>             planet_size_p;
symbols<PlanetType>             planet_type_p;
symbols<PlanetEnvironment>      planet_environment_type_p;
symbols<UniverseObjectType>     universe_object_type_p;
symbols<StarType>               star_type_p;
symbols<FocusType>              focus_type_p;
symbols<EmpireAffiliationType>  affiliation_type_p;
symbols<UnlockableItemType>     unlockable_item_type_p;
symbols<TechType>               tech_type_p;
symbols<ShipPartClass>          part_class_p;
symbols<ShipSlotType>           slot_type_p;

namespace {
    bool Init()
    {
        name_p =
            lexeme_d['"' >> (*(alnum_p | '_'))[name_p.this_ = construct_<std::string>(arg1, arg2)] >> '"'];

        file_name_p =
            lexeme_d['"' >> (*(alnum_p | '_' | '-' | '/' | '.'))[file_name_p.this_ = construct_<std::string>(arg1, arg2)] >> '"'];

        colour_p =
            ('(' >> limit_d(0u, 255u)[uint_p[colour_p.r = arg1]] >> ','
                 >> limit_d(0u, 255u)[uint_p[colour_p.g = arg1]] >> ','
                 >> limit_d(0u, 255u)[uint_p[colour_p.b = arg1]] >> ','
                 >> limit_d(0u, 255u)[uint_p[colour_p.a = arg1]] >> ')')
            [colour_p.this_ = construct_<GG::Clr>(colour_p.r, colour_p.g, colour_p.b, colour_p.a)];

        planet_size_p.add
            ("tiny", SZ_TINY)
            ("small", SZ_SMALL)
            ("medium", SZ_MEDIUM)
            ("large", SZ_LARGE)
            ("huge", SZ_HUGE)
            ("asteroids", SZ_ASTEROIDS)
            ("gasgiant", SZ_GASGIANT);

        planet_type_p.add
            ("swamp", PT_SWAMP)
            ("toxic", PT_TOXIC)
            ("inferno", PT_INFERNO)
            ("radiated", PT_RADIATED)
            ("barren", PT_BARREN)
            ("tundra", PT_TUNDRA)
            ("desert", PT_DESERT)
            ("terran", PT_TERRAN)
            ("ocean", PT_OCEAN)
            ("asteroids", PT_ASTEROIDS)
            ("gasgiant", PT_GASGIANT);

        planet_environment_type_p.add
            ("uninhabitable", PE_UNINHABITABLE)
            ("hostile", PE_HOSTILE)
            ("poor", PE_POOR)
            ("adequate", PE_ADEQUATE)
            ("good", PE_GOOD);

        universe_object_type_p.add
            ("building", OBJ_BUILDING)
            ("ship", OBJ_SHIP)
            ("fleet", OBJ_FLEET) 
            ("planet", OBJ_PLANET)
            ("populationcenter", OBJ_POP_CENTER)
            ("productioncenter", OBJ_PROD_CENTER)
            ("system", OBJ_SYSTEM);

        star_type_p.add
            ("blue", STAR_BLUE)
            ("white", STAR_WHITE)
            ("yellow", STAR_YELLOW)
            ("orange", STAR_ORANGE)
            ("red", STAR_RED)
            ("neutron", STAR_NEUTRON)
            ("blackhole", STAR_BLACK);

        focus_type_p.add
            ("unknown",     FOCUS_UNKNOWN)
            ("balanced",    FOCUS_BALANCED)
            ("farming",     FOCUS_FARMING)
            ("industry",    FOCUS_INDUSTRY)
            ("mining",      FOCUS_MINING)
            ("research",    FOCUS_RESEARCH)
            ("trade",       FOCUS_TRADE);

        affiliation_type_p.add
            ("theempire",   AFFIL_SELF)
            ("enemyof",     AFFIL_ENEMY)
            ("allyof",      AFFIL_ALLY);

        unlockable_item_type_p.add
            ("building",    UIT_BUILDING)
            ("shippart",    UIT_SHIP_PART)
            ("shiphull",    UIT_SHIP_HULL);

        tech_type_p.add
            ("theory",      TT_THEORY)
            ("application", TT_APPLICATION)
            ("refinement",  TT_REFINEMENT);

        part_class_p.add
            ("shortrange",      PC_SHORT_RANGE)
            ("missiles",        PC_MISSILES)
            ("fighters",        PC_FIGHTERS)
            ("pointdefense",    PC_POINT_DEFENSE)
            ("shield",          PC_SHIELD)
            ("armour",          PC_ARMOUR)
            ("detection",       PC_DETECTION)
            ("stealth",         PC_STEALTH)
            ("fuel",            PC_FUEL);

        slot_type_p.add
            ("external",    SL_EXTERNAL)
            ("internal",    SL_INTERNAL);

        return true;
    }
    bool dummy = Init();
}

void ReportError(std::ostream& os, const char* input, const parse_info<const char*>& result)
{
    int line = 1;
    const char* line_first = result.stop;
    const char* line_last = input;
    while (input < line_first && *(line_first - 1) != '\n') {
        --line_first;
    }
    while (line_last < result.stop) {
        if (*line_last == '\n')
            ++line;
        ++line_last;
    }
    int input_length = std::strlen(input);
    while (line_last < input + input_length && *line_last != '\n') {
        ++line_last;
    }
    os << "error at or after the indicated point on line " << line << ":\n"
       << std::string(line_first, line_last) << "\n"
       << std::string(result.stop - line_first, ' ') << "^\n"
       << std::endl;
}
