#include "Parser.h"

#include "ParserUtil.h"
#include "ValueRefParser.h"
#include "Condition.h"

using namespace boost::spirit::classic;
using namespace phoenix;

extern ParamLabel name_label;
extern ParamLabel low_label;
extern ParamLabel high_label;
extern ParamLabel class_label;
extern ParamLabel empire_label;
extern ParamLabel design_label;

rule<Scanner, ConditionClosure::context_t> condition2_p;

namespace {
    class ConditionParser2Definition
    {
    public:
        typedef rule<Scanner, ConditionClosure::context_t> Rule;

        ConditionParser2Definition();

    private:
        struct StringClosure : boost::spirit::classic::closure<StringClosure, Condition::ConditionBase*, std::string>
        {
            member1 this_;
            member2 name;
        };

        struct DesignHasPartClosure : boost::spirit::classic::closure<DesignHasPartClosure, Condition::ConditionBase*,
                                                                      ValueRef::ValueRefBase<int>*,
                                                                      ValueRef::ValueRefBase<int>*,
                                                                      std::string>
        {
            member1 this_;
            member2 int_ref_1;
            member3 int_ref_2;
            member4 name;
        };

        struct DesignHasPartClassClosure : boost::spirit::classic::closure<DesignHasPartClassClosure,
                                                                           Condition::ConditionBase*,
                                                                           ValueRef::ValueRefBase<int>*,
                                                                           ValueRef::ValueRefBase<int>*,
                                                                           ShipPartClass>
        {
            member1 this_;
            member2 int_ref_1;
            member3 int_ref_2;
            member4 part_class;
        };

        struct IntRefClosure : boost::spirit::classic::closure<IntRefClosure, Condition::ConditionBase*,
                                                               const ValueRef::ValueRefBase<int>*>
        {
            member1 this_;
            member2 int_ref;
        };

        typedef rule<Scanner, StringClosure::context_t>                 StringRule;
        typedef rule<Scanner, DesignHasPartClosure::context_t>          DesignHasPartRule;
        typedef rule<Scanner, DesignHasPartClassClosure::context_t>     DesignHasPartClassRule;
        typedef rule<Scanner, IntRefClosure::context_t>                 IntRefRule;

        StringRule                  owner_has_tech;
        StringRule                  has_special;
        StringRule                  design_has_hull;
        DesignHasPartRule           design_has_part;
        DesignHasPartClassRule      design_has_part_class;
        StringRule                  predefined_design;
        IntRefRule                  design_number;
        IntRefRule                  produced_by_empire;
        IntRefRule                  visible_to_empire;
        IntRefRule                  explored_by_empire;
        IntRefRule                  fleet_supplyable;
    };

    ConditionParser2Definition::ConditionParser2Definition()
    {
        owner_has_tech =
            (str_p("ownerhastech")
             >> name_label >> name_p[owner_has_tech.name = arg1])
            [owner_has_tech.this_ = new_<Condition::OwnerHasTech>(owner_has_tech.name)];

        has_special =
            (str_p("hasspecial")
             >> name_label >> name_p[has_special.name = arg1])
            [has_special.this_ = new_<Condition::HasSpecial>(has_special.name)];

        design_has_hull =
            (str_p("designhashull")
             >> name_label >> name_p[design_has_hull.name = arg1])
            [design_has_hull.this_ = new_<Condition::DesignHasHull>(design_has_hull.name)];

        design_has_part =
            (str_p("designhaspart")
             >> low_label >>    int_expr_p[ design_has_part.int_ref_2 = arg1]
             >> high_label >>   int_expr_p[ design_has_part.int_ref_1 = arg1]
             >> name_label >>   name_p[     design_has_part.name = arg1])
            [design_has_part.this_ = new_<Condition::DesignHasPart>(design_has_part.int_ref_2,
                                                                    design_has_part.int_ref_1,
                                                                    design_has_part.name)];

        design_has_part_class =
            (str_p("designhaspartclass")
             >> low_label >>    int_expr_p[     design_has_part_class.int_ref_2 = arg1]
             >> high_label >>   int_expr_p[     design_has_part_class.int_ref_1 = arg1]
             >> class_label >>  part_class_p[   design_has_part_class.part_class = arg1])
            [design_has_part_class.this_ = new_<Condition::DesignHasPartClass>(design_has_part_class.int_ref_2,
                                                                               design_has_part_class.int_ref_1,
                                                                               design_has_part_class.part_class)];

        predefined_design =
            (str_p("qwerqwer")
             >> name_label >> name_p[predefined_design.name = arg1])
             [predefined_design.this_ = new_<Condition::PredefinedShipDesign>(predefined_design.name)];

        design_number =
            (str_p("shipdqhwrehesignq")
             >> design_label >> int_expr_p[design_number.int_ref = arg1])
            [design_number.this_ = new_<Condition::NumberedShipDesign>(design_number.int_ref)];

        produced_by_empire =
            (str_p("producedbyempire")
             >> empire_label >> int_expr_p[produced_by_empire.int_ref = arg1])
            [produced_by_empire.this_ = new_<Condition::ProducedByEmpire>(produced_by_empire.int_ref)];

        visible_to_empire =
            (str_p("visibletoempire")
             >> empire_label >> int_expr_p[visible_to_empire.int_ref = arg1])
            [visible_to_empire.this_ = new_<Condition::VisibleToEmpire>(visible_to_empire.int_ref)];

        explored_by_empire =
            (str_p("exploredbyempire")
             >> empire_label >> int_expr_p[explored_by_empire.int_ref = arg1])
            [explored_by_empire.this_ = new_<Condition::ExploredByEmpire>(explored_by_empire.int_ref)];

        fleet_supplyable =
            (str_p("fleetsupplyablebyempire")
             >> empire_label >> int_expr_p[fleet_supplyable.int_ref = arg1])
            [fleet_supplyable.this_ = new_<Condition::FleetSupplyableByEmpire>(fleet_supplyable.int_ref)];

        condition2_p =
            owner_has_tech[condition2_p.this_ = arg1]
            | has_special[condition2_p.this_ = arg1]
            | design_has_hull[condition2_p.this_ = arg1]
            | design_has_part[condition2_p.this_ = arg1]
            | design_has_part_class[condition2_p.this_ = arg1]
            | predefined_design[condition2_p.this_ = arg1]
            | design_number[condition2_p.this_ = arg1]
            | produced_by_empire[condition2_p.this_ = arg1]
            | explored_by_empire[condition2_p.this_ = arg1]
            | visible_to_empire[condition2_p.this_ = arg1]
            | fleet_supplyable[condition2_p.this_ = arg1];
    }
    ConditionParser2Definition condition2_def;
}
