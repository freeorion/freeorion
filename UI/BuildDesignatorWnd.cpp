#include "BuildDesignatorWnd.h"

#include "CUIControls.h"
#include "ClientUI.h"
#include "CUIWnd.h"
#include "SidePanel.h"
#include "TechTreeWnd.h"
#include "MapWnd.h"
#include "EncyclopediaDetailPanel.h"
#include "IconTextBrowseWnd.h"
#include "DesignWnd.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/ScopedTimer.h"
#include "../universe/UniverseObject.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../universe/BuildingType.h"
#include "../universe/ShipDesign.h"
#include "../universe/ShipHull.h"
#include "../universe/ShipPart.h"
#include "../universe/Conditions.h"
#include "../universe/ValueRef.h"
#include "../client/human/GGHumanClientApp.h"

#include <GG/Layout.h>
#include <GG/StaticGraphic.h>

#include <iterator>

namespace {
    const std::string PROD_PEDIA_WND_NAME = "production.pedia";
    const std::string PROD_SELECTOR_WND_NAME = "production.selector";
    const std::string PROD_SIDEPANEL_WND_NAME = "production.sidepanel";

    void AddOptions(OptionsDB& db)
    { db.Add("ui." + PROD_PEDIA_WND_NAME + ".hidden.enabled", UserStringNop("OPTIONS_DB_PRODUCTION_PEDIA_HIDDEN"), false); }
    bool temp_bool = RegisterOptions(&AddOptions);

    constexpr int MAX_PRODUCTION_TURNS = 200;
    constexpr float EPSILON = 0.001f;
    int IconTextBrowseWndRowHeight() { return ClientUI::Pts()*3/2; }
    constexpr int   EDGE_PAD(3);
    constexpr GG::X ICON_BROWSE_TEXT_WIDTH{400};
    constexpr GG::X ICON_BROWSE_ICON_WIDTH{64};
    constexpr GG::Y ICON_BROWSE_ICON_HEIGHT{64};


#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
    constexpr std::string EMPTY_STRING;
#else
    const std::string EMPTY_STRING;
#endif

    constexpr int ProductionTurns(const float total_cost, const int minimum_production_time,
                                  const float local_pp_output, float stockpile, const float stockpile_limit_per_turn)
        noexcept(noexcept(std::min(1.0f, -1.0f)) && noexcept(std::max(1, 0)))
    {
        const float max_allocation_per_turn = total_cost / std::max(1, minimum_production_time);
        //std::cout << "\nProductionTurnsprod max per turn: " << max_allocation_per_turn << "  total cost: " << total_cost
        //          << "  min time: " << minimum_production_time << "  local pp: " << local_pp_output << "  stockpile: " << stockpile << std::endl;

        // allocate production each turn, limited by total stockpile, stockpile per turn limit, and connected industry output
        int prod_time_here = 0;
        float total_allocated = 0.0f;
        for (; prod_time_here < MAX_PRODUCTION_TURNS && total_allocated < total_cost - EPSILON;) {
            const float avail_stockpile = std::min(stockpile, stockpile_limit_per_turn);
            float industry_output_used = local_pp_output;
            float stockpile_used = 0.0f;

            if (local_pp_output >= max_allocation_per_turn) {
                industry_output_used = max_allocation_per_turn;

            } else if (local_pp_output + avail_stockpile >= max_allocation_per_turn) {
                industry_output_used = local_pp_output;
                stockpile_used = max_allocation_per_turn - local_pp_output;

            } else if (local_pp_output + avail_stockpile > 0.0f) {
                industry_output_used = local_pp_output;
                stockpile_used = avail_stockpile;

            } else {
                // no progress possible
                break;
            }

            stockpile -= stockpile_used;
            total_allocated += (stockpile_used + industry_output_used);
            prod_time_here++;
            //std::cout << "prod time here: " << prod_time_here << ": stockpile used: " << stockpile_used
            //          << "   industry used: " << industry_output_used << "  total cost: " << total_cost << std::endl;
        }

        return std::max(minimum_production_time, prod_time_here);
    }


    //////////////////////////////////
    // ProductionItemPanel
    //////////////////////////////////
    class ProductionItemPanel : public GG::Control {
    public:
        ProductionItemPanel(GG::X w, GG::Y h, const ProductionQueue::ProductionItem& item,
                            int empire_id, int location_id) :
            Control(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS),
            m_item(item),
            m_empire_id(empire_id),
            m_location_id(location_id)
        {
            SetChildClippingMode(ChildClippingMode::ClipToClient);
        }

        /** Renders panel background and border. */
        void Render() override {
            if (!m_initialized)
                Init();
            GG::Clr background_clr = this->Disabled() ? ClientUI::WndColor() : ClientUI::CtrlColor();
            GG::FlatRectangle(UpperLeft(), LowerRight(), background_clr, ClientUI::WndOuterBorderColor(), 1u);
        }

        void SizeMove(GG::Pt ul, GG::Pt lr) override {
            const auto old_size = Size();
            GG::Control::SizeMove(ul, lr);
            if (old_size != Size())
                DoLayout();
        }

    private:
        void DoLayout() {
            if (!m_initialized)
                return;

            const GG::X ICON_WIDTH{Value(ClientHeight())};
            const GG::X ITEM_NAME_WIDTH{ClientUI::Pts() * 16};
            //const GG::X COST_WIDTH{ClientUI::Pts() * 4};
            const GG::X TIME_WIDTH{ClientUI::Pts() * 3};
            const GG::X DESC_WIDTH{ClientUI::Pts() * 18};

            GG::X left(GG::X0);
            GG::Y bottom(ClientHeight());

            m_icon->SizeMove(GG::Pt(left, GG::Y0), GG::Pt(left + ICON_WIDTH, bottom));
            left += ICON_WIDTH + GG::X(3);
            m_name->SizeMove(GG::Pt(left, GG::Y0), GG::Pt(left + ITEM_NAME_WIDTH, bottom));
            left += ITEM_NAME_WIDTH;
            //m_cost->SizeMove(GG::Pt(left, GG::Y0), GG::Pt(left + COST_WIDTH, bottom));
            //left += COST_WIDTH;
            m_time->SizeMove(GG::Pt(left, GG::Y0), GG::Pt(left + TIME_WIDTH, bottom));
            left += TIME_WIDTH;
            m_desc->SizeMove(GG::Pt(left, GG::Y0), GG::Pt(left + DESC_WIDTH, bottom));
        }

        void Init() {
            if (m_initialized)
                return;
            m_initialized = true;

            const ScriptingContext& context = IApp::GetApp()->GetContext();
            auto empire = context.GetEmpire(m_empire_id);

            std::shared_ptr<GG::Texture>                texture;
            std::string                                 name_text;
            std::string                                 time_text;
            std::string                                 desc_text;
            std::vector<const Condition::Condition*>    location_conditions;

            switch (m_item.build_type) {
            case BuildType::BT_BUILDING: {
                texture = ClientUI::BuildingIcon(m_item.name);
                desc_text = UserString("BT_BUILDING");
                name_text = UserString(m_item.name);
                break;
            }
            case BuildType::BT_SHIP: {
                texture = ClientUI::ShipDesignIcon(m_item.design_id);
                desc_text = UserString("BT_SHIP");
                if (const ShipDesign* design = context.ContextUniverse().GetShipDesign(m_item.design_id))
                    name_text = design->Name(true);
                break;
            }
            case BuildType::BT_STOCKPILE: {
                texture = ClientUI::MeterIcon(MeterType::METER_STOCKPILE);
                desc_text = UserString("BT_STOCKPILE");
                name_text = UserString(m_item.name);
                break;
            }
            default:
                ErrorLogger() << "ProductionItemPanel::Init got invalid item type";
                texture = ClientUI::GetTexture("");
            }

            float local_pp_output = 0.0f;
            float stockpile = 0.0f;
            float stockpile_limit_per_turn = 0.0f;

            // cost / turn, and minimum production turns
            if (empire) {
                // from industry output
                local_pp_output = empire->GetIndustryPool().GroupAvailable(m_location_id);

                // from stockpile
                stockpile = empire->GetIndustryPool().Stockpile();
                stockpile_limit_per_turn = empire->GetProductionQueue().StockpileCapacity(context.ContextObjects());

                auto [total_cost, minimum_production_time] = m_item.ProductionCostAndTime(m_empire_id, m_location_id, context);

                int production_time = ProductionTurns(total_cost, minimum_production_time,
                                                      local_pp_output, stockpile,
                                                      stockpile_limit_per_turn);

                if (production_time >= MAX_PRODUCTION_TURNS)
                    time_text = std::to_string(MAX_PRODUCTION_TURNS) + "+";
                else
                    time_text = std::to_string(production_time);
            }

            m_icon = GG::Wnd::Create<GG::StaticGraphic>(
                std::move(texture), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
            m_name = GG::Wnd::Create<CUILabel>(std::move(name_text), GG::FORMAT_LEFT);
            //m_cost = GG::Wnd::Create<CUILabel>(cost_text);
            m_time = GG::Wnd::Create<CUILabel>(std::move(time_text));
            m_desc = GG::Wnd::Create<CUILabel>(std::move(desc_text), GG::FORMAT_LEFT);

            AttachChild(m_icon);
            AttachChild(m_name);
            //AttachChild(m_cost);
            AttachChild(m_time);
            AttachChild(m_desc);

            DoLayout();
        }

        bool                                    m_initialized = false;
        const ProductionQueue::ProductionItem   m_item;
        int                                     m_empire_id = ALL_EMPIRES;
        int                                     m_location_id = INVALID_OBJECT_ID;
        std::shared_ptr<GG::StaticGraphic>      m_icon;
        std::shared_ptr<GG::Label>              m_name;
        //std::shared_ptr<GG::Label>              m_cost;
        std::shared_ptr<GG::Label>              m_time;
        std::shared_ptr<GG::Label>              m_desc;
    };

    std::string EnqueueAndLocationConditionDescription(const std::string& building_name, int candidate_object_id,
                                                       int empire_id, bool only_failed_conditions)
    {
        std::vector<const Condition::Condition*> enqueue_conditions;
        Condition::EmpireHasBuildingTypeAvailable bld_avail_cond(building_name);
        enqueue_conditions.push_back(&bld_avail_cond);
        if (const BuildingType* building_type = GetBuildingType(building_name)) {
            enqueue_conditions.push_back(building_type->EnqueueLocation());
            enqueue_conditions.push_back(building_type->Location());
        }

        const ScriptingContext& context = IApp::GetApp()->GetContext();
        const auto& objects = context.ContextObjects();
        const auto empire = context.GetEmpire(empire_id);
        const ScriptingContext source_context(context, ScriptingContext::Source{},
                                              empire ? empire->Source(objects).get() : nullptr);
        const UniverseObject* candidate = objects.getRaw(candidate_object_id);

        if (only_failed_conditions)
            return ConditionFailedDescription(enqueue_conditions, source_context, candidate);
        else
            return ConditionDescription(enqueue_conditions, source_context, candidate);
    }

    std::string LocationConditionDescription(int ship_design_id, int candidate_object_id,
                                             int empire_id, bool only_failed_conditions)
    {
#if defined(__GNUC__) && (__GNUC__ < 13)
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
        static constinit const Condition::CanProduceShips can_prod_ship_cond;
#else
        static constexpr Condition::CanProduceShips can_prod_ship_cond;
#endif
        const Condition::EmpireHasShipDesignAvailable ship_avail_cond{ship_design_id};

        std::vector<const Condition::Condition*> location_conditions;
        location_conditions.reserve(4);
        location_conditions.push_back(&can_prod_ship_cond);
        location_conditions.push_back(&ship_avail_cond);

        const ScriptingContext& context = IApp::GetApp()->GetContext();
        const Universe& universe = context.ContextUniverse();

        if (const ShipDesign* ship_design = universe.GetShipDesign(ship_design_id)) {
            if (const ShipHull* ship_hull = GetShipHull(ship_design->Hull()))
                location_conditions.push_back(ship_hull->Location());
            for (const std::string& part_name : ship_design->Parts()) {
                if (const ShipPart* part = GetShipPart(part_name))
                    location_conditions.push_back(part->Location());
            }
        }

        const ObjectMap& objects = context.ContextObjects();
        auto empire = context.GetEmpire(empire_id);
        const ScriptingContext source_context{context, ScriptingContext::Source{}, empire->Source(objects).get()};
        const auto* candidate = objects.getRaw(candidate_object_id);

        if (only_failed_conditions)
            return ConditionFailedDescription(location_conditions, source_context, candidate);
        else
            return ConditionDescription(location_conditions, source_context, candidate);
    }

    class ProductionItemRowBrowseWnd : public GG::BrowseInfoWnd {
    public:
        ProductionItemRowBrowseWnd(const ProductionQueue::ProductionItem& item,
                                   int candidate_object_id, int empire_id) :
            GG::BrowseInfoWnd(GG::X0, GG::Y0, ICON_BROWSE_TEXT_WIDTH + ICON_BROWSE_ICON_WIDTH, GG::Y1),
            m_item(std::move(item)),
            m_candidate_object_id(candidate_object_id),
            m_empire_id(empire_id)
        {
            RequirePreRender();
            //std::cout << "ProductionItemRowBrowseWnd construct " << item.name << std::endl;
        }

        bool WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const override {
            assert(mode <= wnd->BrowseModes().size());
            return true;
        }

        void Render() override {
            const auto ul = UpperLeft();
            const auto lr = LowerRight();
            const GG::Y ROW_HEIGHT{IconTextBrowseWndRowHeight()};
            GG::FlatRectangle(ul, lr, ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);    // main background
            GG::FlatRectangle(GG::Pt(ul.x + ICON_BROWSE_ICON_WIDTH, ul.y), GG::Pt(lr.x, ul.y + ROW_HEIGHT),
                              ClientUI::WndOuterBorderColor(), ClientUI::WndOuterBorderColor(), 0); // top title filled background
        }

        void PreRender() override {
            GG::Wnd::PreRender();

            SetChildClippingMode(ChildClippingMode::ClipToClient);
            auto [icon, main_text] = [this]() -> std::pair<std::shared_ptr<GG::Texture>, std::string> {
                switch (m_item.build_type) {
                case BuildType::BT_BUILDING:  return PreRenderBuilding();  break;
                case BuildType::BT_SHIP:      return PreRenderDesign();    break;
                case BuildType::BT_STOCKPILE: return PreRenderStockpile(); break;
                default: return {nullptr, EMPTY_STRING};
                }
            }();
            auto& title = [this]() -> const auto& {
                switch (m_item.build_type) {
                case BuildType::BT_STOCKPILE: [[fallthrough]];
                case BuildType::BT_BUILDING:  return UserString(m_item.name);  break;
                case BuildType::BT_SHIP:      return m_item.name;    break;
                default: return EMPTY_STRING;
                }
            }();

            //std::cout << "ProductionItemRowBrowseWnd::PreRender: " << m_item.name  << " : " << title << std::endl;


            m_icon = GG::Wnd::Create<GG::StaticGraphic>(icon, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE, GG::INTERACTIVE);
            m_icon->Resize(GG::Pt(ICON_BROWSE_ICON_WIDTH, ICON_BROWSE_ICON_HEIGHT));
            AttachChild(m_icon);

            const GG::Y ROW_HEIGHT{IconTextBrowseWndRowHeight()};

            m_title_text_label = GG::Wnd::Create<CUILabel>(title, GG::FORMAT_LEFT);
            m_title_text_label->MoveTo(GG::Pt(m_icon->Width() + GG::X(EDGE_PAD), GG::Y0));
            m_title_text_label->Resize(GG::Pt(ICON_BROWSE_TEXT_WIDTH, ROW_HEIGHT));
            m_title_text_label->SetFont(ClientUI::GetBoldFont());


            m_main_text_label = GG::Wnd::Create<CUILabel>(ValueRefLinkText(std::move(main_text), false),
                                                          GG::FORMAT_LEFT | GG::FORMAT_TOP | GG::FORMAT_WORDBREAK);
            m_main_text_label->MoveTo(GG::Pt(m_icon->Width() + GG::X(EDGE_PAD), ROW_HEIGHT));
            m_main_text_label->Resize(GG::Pt(ICON_BROWSE_TEXT_WIDTH, ICON_BROWSE_ICON_HEIGHT));
            m_main_text_label->SetResetMinSize(true);
            m_main_text_label->Resize(m_main_text_label->MinSize());

            AttachChild(m_title_text_label);
            AttachChild(m_main_text_label);

            Resize(GG::Pt(ICON_BROWSE_TEXT_WIDTH + ICON_BROWSE_ICON_WIDTH,
                          std::max(m_icon->Height(), ROW_HEIGHT + m_main_text_label->Height())));
        }

    private:
        std::array<float, 3> GetOutputStockpile(const ScriptingContext& context) const {
            // get available PP for empire at candidate location
            if (auto empire = context.GetEmpire(m_empire_id)) {
                auto local_pp_output = empire->GetIndustryPool().GroupAvailable(m_candidate_object_id);
                auto stockpile = empire->GetIndustryPool().Stockpile();
                auto stockpile_limit_per_turn = empire->GetProductionQueue().StockpileCapacity(context.ContextObjects());
                return std::array{local_pp_output, stockpile, stockpile_limit_per_turn};
            }
            return std::array{0.0f, 0.0f, 0.0f};
        }

        auto GetObjName(const ScriptingContext& context) const {
            auto obj = context.ContextObjects().getRaw(m_candidate_object_id);
            std::string candidate_name = obj ? obj->Name() : "";
            if (GetOptionsDB().Get<bool>("ui.name.id.shown"))
                candidate_name += " (" + std::to_string(m_candidate_object_id) + ")";
            return std::pair{obj, std::move(candidate_name)};
        }

        std::pair<std::shared_ptr<GG::Texture>, std::string> PreRenderBuilding() {
            const ScriptingContext& context = IApp::GetApp()->GetContext();
            auto [obj, candidate_name] = GetObjName(context);
            auto [local_pp_output, stockpile, stockpile_limit_per_turn] = GetOutputStockpile(context);

            std::string main_text;
            main_text.reserve(1000); // guesstimate

            const auto* building_type = GetBuildingType(m_item.name);


            if (building_type && (obj || building_type->ProductionCostTimeLocationInvariant())) {
                // if location object is available, or cost and time are invariation to location, can safely evaluate cost and time
                const float total_cost = building_type->ProductionCost(m_empire_id, m_candidate_object_id, context);
                const int minimum_production_time =
                    std::max(1, building_type->ProductionTime( m_empire_id, m_candidate_object_id, context));

                if (obj) {
                    // if location object is available, can evaluate production time at that location
                    int prod_time_here = ProductionTurns(total_cost, minimum_production_time, local_pp_output,
                                                         stockpile, stockpile_limit_per_turn);

                    if (prod_time_here < MAX_PRODUCTION_TURNS) {
                        main_text += boost::io::str(FlexibleFormat(UserString("PRODUCTION_WND_TOOLTIP_PROD_TIME")) %
                                                    std::to_string(prod_time_here) % candidate_name);
                    } else {
                        main_text += boost::io::str(FlexibleFormat(UserString("NO_PRODUCTION_HERE_CANT_PRODUCE")) %
                                                    candidate_name);
                    }
                }

                main_text += "\n" + boost::io::str(FlexibleFormat(UserString("PRODUCTION_WND_TOOLTIP_PROD_TIME_MINIMUM")) %
                                                   std::to_string(minimum_production_time));
                main_text += "\n" + boost::io::str(FlexibleFormat(UserString("PRODUCTION_WND_TOOLTIP_PROD_COST")) %
                                                   DoubleToString(total_cost, 3, false));

            } else if (building_type) {
                // no location object, but have location-dependent cost or time

                const int minimum_production_time =
                    std::max(1, building_type->ProductionTime(m_empire_id, m_candidate_object_id, context));
                // 9999 is arbitrary large time returned for evaluation failure due to lack of location object but object-dependent time
                if (minimum_production_time >= 9999) {
                    main_text += "\n" + boost::io::str(FlexibleFormat(UserString("PRODUCTION_WND_TOOLTIP_PROD_TIME_MINIMUM")) %
                                                       UserString("PRODUCTION_WND_TOOLTIP_LOCATION_DEPENDENT"));
                } else {
                    main_text += "\n" + boost::io::str(FlexibleFormat(UserString("PRODUCTION_WND_TOOLTIP_PROD_TIME_MINIMUM")) %
                                                       std::to_string(minimum_production_time));
                }

                const float total_cost = building_type->ProductionCost(m_empire_id, m_candidate_object_id, context);
                // 999999.9f is arbitrary large cost returned for evaluation failure due to lack of location object but object-dependnet cost
                if (total_cost >= 999999.9f) {
                    main_text += "\n" + boost::io::str(FlexibleFormat(UserString("PRODUCTION_WND_TOOLTIP_PROD_COST")) %
                                                       UserString("PRODUCTION_WND_TOOLTIP_LOCATION_DEPENDENT"));
                } else {
                    main_text += "\n" + boost::io::str(FlexibleFormat(UserString("PRODUCTION_WND_TOOLTIP_PROD_COST")) %
                                                       DoubleToString(total_cost, 3, false));
                }
            }

            if (building_type)
                main_text += "\n\n" + UserString(building_type->Description());

            // show build conditions
            auto enqueue_and_location_condition_failed_text =
                EnqueueAndLocationConditionDescription(m_item.name, m_candidate_object_id, m_empire_id, true);
            if (!enqueue_and_location_condition_failed_text.empty()) {
                if (auto location = context.ContextObjects().get(m_candidate_object_id)) {
                    std::string failed_cond_loc = boost::io::str(
                        FlexibleFormat(UserString("PRODUCTION_WND_TOOLTIP_FAILED_COND")) % location->Name());
                    main_text += "\n\n" + failed_cond_loc + ":\n" + enqueue_and_location_condition_failed_text;
                }
            }

            return std::pair{ClientUI::BuildingIcon(m_item.name), std::move(main_text)};
        }

        std::pair<std::shared_ptr<GG::Texture>, std::string> PreRenderDesign() {
            const ScriptingContext& context = IApp::GetApp()->GetContext();
            auto [obj, candidate_name] = GetObjName(context);
            auto [local_pp_output, stockpile, stockpile_limit_per_turn] = GetOutputStockpile(context);

            const ShipDesign* design = context.ContextUniverse().GetShipDesign(m_item.design_id);
            std::string main_text;
            main_text.reserve(1000); // guesstimate

            if (design && (obj || design->ProductionCostTimeLocationInvariant())) {
                // if location object is available, or cost and time are invariation to location, can safely evaluate cost and time
                float total_cost = design->ProductionCost(m_empire_id, m_candidate_object_id, context);
                int minimum_production_time = std::max(1, design->ProductionTime(m_empire_id, m_candidate_object_id, context));

                if (obj) {
                    // if location object is available, can evaluate production time at that location
                    int prod_time_here = ProductionTurns(total_cost, minimum_production_time, local_pp_output,
                                                            stockpile, stockpile_limit_per_turn);

                    if (prod_time_here < MAX_PRODUCTION_TURNS) {
                        main_text += boost::io::str(FlexibleFormat(UserString("PRODUCTION_WND_TOOLTIP_PROD_TIME")) %
                                                    std::to_string(prod_time_here) % candidate_name);
                    } else {
                        main_text += boost::io::str(FlexibleFormat(UserString("NO_PRODUCTION_HERE_CANT_PRODUCE")) %
                                                    candidate_name);
                    }
                }

                main_text += "\n" + boost::io::str(FlexibleFormat(UserString("PRODUCTION_WND_TOOLTIP_PROD_TIME_MINIMUM")) %
                                                    std::to_string(minimum_production_time));
                main_text += "\n" + boost::io::str(FlexibleFormat(UserString("PRODUCTION_WND_TOOLTIP_PROD_COST")) %
                                                    DoubleToString(total_cost, 3, false));

            } else if (design) {
                // no location object, but have location-dependent cost or time

                int minimum_production_time = std::max(1, design->ProductionTime(m_empire_id, m_candidate_object_id, context));
                // 9999 is arbitrary large time returned for evaluation failure due to lack of location object but object-dependent time
                if (minimum_production_time >= 9999) {
                    main_text += "\n" + boost::io::str(FlexibleFormat(UserString("PRODUCTION_WND_TOOLTIP_PROD_TIME_MINIMUM")) %
                                                        UserString("PRODUCTION_WND_TOOLTIP_LOCATION_DEPENDENT"));
                } else {
                    main_text += "\n" + boost::io::str(FlexibleFormat(UserString("PRODUCTION_WND_TOOLTIP_PROD_TIME_MINIMUM")) %
                                                        std::to_string(minimum_production_time));
                }

                float total_cost = design->ProductionCost(m_empire_id, m_candidate_object_id, context);
                // 999999.9f is arbitrary large cost returned for evaluation failure due to lack of location object but object-dependnet cost
                if (total_cost >= 999999.9f) {
                    main_text += "\n" + boost::io::str(FlexibleFormat(UserString("PRODUCTION_WND_TOOLTIP_PROD_COST")) %
                                                        UserString("PRODUCTION_WND_TOOLTIP_LOCATION_DEPENDENT"));
                } else {
                    main_text += "\n" + boost::io::str(FlexibleFormat(UserString("PRODUCTION_WND_TOOLTIP_PROD_COST")) %
                                                        DoubleToString(total_cost, 3, false));
                }
            }

            main_text += "\n\n" + design->Description(true);
            main_text += "\n\n" + UserString("ENC_SHIP_HULL") + ": " + UserString(design->Hull());

            // load ship parts, stack ship parts that are used multiple times
            std::map<std::string_view, int> ship_part_names;
            if (design) {
                for (const auto& part_name : design->Parts()) {
                    if (ship_part_names.contains(part_name))
                        ship_part_names[part_name]++;
                    else
                        ship_part_names[part_name] = 1;
                }
            }

            std::string ship_parts_formatted;
            for (const auto& [part_name, count] : ship_part_names) {
                if (!UserStringExists(part_name)) continue;
                if (ship_part_names[part_name] == 1)
                    ship_parts_formatted += (UserString(part_name) + ", ");
                else
                    ship_parts_formatted += (UserString(part_name) + " x" + std::to_string(count) + ", ");
            }

            main_text += "\n" + UserString("PRODUCTION_WND_TOOLTIP_PARTS") + ": " +
                ship_parts_formatted.substr(0, ship_parts_formatted.length() - 2);

            // show build conditions
            const auto location_condition_failed_text =
                LocationConditionDescription(m_item.design_id, m_candidate_object_id, m_empire_id, true);
            if (!location_condition_failed_text.empty())
                if (auto location = context.ContextObjects().getRaw(m_candidate_object_id)) {
                    std::string failed_cond_loc = boost::io::str(FlexibleFormat(
                        UserString("PRODUCTION_WND_TOOLTIP_FAILED_COND")) % location->Name());
                    main_text += ("\n\n" + failed_cond_loc + ":\n" + location_condition_failed_text);
                }

            return std::pair{ClientUI::ShipDesignIcon(m_item.design_id), std::move(main_text)};
        }

        std::pair<std::shared_ptr<GG::Texture>, std::string> PreRenderStockpile()
        { return std::pair{ClientUI::MeterIcon(MeterType::METER_STOCKPILE), UserString("PROJECT_BT_STOCKPILE_DESC")}; }

        std::shared_ptr<GG::StaticGraphic>     m_icon;
        std::shared_ptr<GG::Label>             m_title_text_label;
        std::shared_ptr<GG::Label>             m_main_text_label;
        const ProductionQueue::ProductionItem& m_item;
        const int                              m_candidate_object_id = INVALID_OBJECT_ID;
        const int                              m_empire_id = ALL_EMPIRES;
    };


    ////////////////////////////////////////////////
    // ProductionItemRow
    ////////////////////////////////////////////////
    class ProductionItemRow : public GG::ListBox::Row {
    public:
        ProductionItemRow(GG::X w, GG::Y h, const ProductionQueue::ProductionItem& item,
                          int empire_id, int location_id) :
            GG::ListBox::Row(w, h),
            m_item(item)
        {
            SetName("ProductionItemRow");
            SetMargin(0);
            SetRowAlignment(GG::ALIGN_NONE);
            SetChildClippingMode(ChildClippingMode::ClipToClient);

            ScopedTimer timer("ProductionItemRow: " + item.name);

            if (m_item.build_type == BuildType::BT_SHIP) {
                SetDragDropDataType(std::to_string(m_item.design_id));
            } else {
                SetDragDropDataType(m_item.name);
            }

            m_panel = GG::Wnd::Create<ProductionItemPanel>(w, h, m_item, empire_id, location_id);

            const ScriptingContext& context = IApp::GetApp()->GetContext();
            if (auto empire = context.GetEmpire(empire_id)) {
                if (!empire->ProducibleItem(m_item, location_id, context)) {
                    this->Disable(true);
                    m_panel->Disable(true);
                }
            }

            SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
            SetBrowseInfoWnd(GG::Wnd::Create<ProductionItemRowBrowseWnd>(m_item, location_id, empire_id));
        };

        void CompleteConstruction() override {
            GG::ListBox::Row::CompleteConstruction();
            push_back(m_panel);
        }

        const ProductionQueue::ProductionItem& Item() const noexcept { return m_item; }

        void SizeMove(GG::Pt ul, GG::Pt lr) override {
            const auto old_size = Size();
            GG::ListBox::Row::SizeMove(ul, lr);
            if (!empty() && old_size != Size() && m_panel)
                m_panel->Resize(Size());
        }

    private:
        ProductionQueue::ProductionItem         m_item;
        std::shared_ptr<ProductionItemPanel>    m_panel;
    };

    //////////////////////////////////
    // BuildableItemsListBox
    //////////////////////////////////
    class BuildableItemsListBox : public CUIListBox {
    public:
        BuildableItemsListBox(void) :
            CUIListBox()
        {
            SetStyle(GG::LIST_NOSORT | GG::LIST_NOSEL);
            // preinitialize listbox/row column widths, because what
            // ListBox::Insert does on default is not suitable for this case
            SetNumCols(1);
            SetColWidth(0, GG::X0);
            LockColWidths();

            SetVScrollWheelIncrement(Value(ListRowHeight())*3);
        }

        void SizeMove(GG::Pt ul, GG::Pt lr) override {
            const auto old_size = Size();
            CUIListBox::SizeMove(ul, lr);
            if (old_size != Size()) {
                const GG::Pt row_size = ListRowSize();
                for (auto& row : *this)
                    row->Resize(row_size);
            }
        }

        GG::Pt ListRowSize() const
        { return GG::Pt(Width() - ClientUI::ScrollWidth() - 5, ListRowHeight()); }

        static GG::Y ListRowHeight()
        { return GG::Y(ClientUI::Pts() * 3/2); }
    };
}

//////////////////////////////////////////////////
// BuildDesignatorWnd::BuildSelector
//////////////////////////////////////////////////
class BuildDesignatorWnd::BuildSelector : public CUIWnd {
public:
    explicit BuildSelector(std::string_view config_name = "");
    void CompleteConstruction() override;

    /** returns set of BulldType shown in this selector */
    const auto& GetBuildTypesShown() const noexcept { return m_build_types_shown; }

    /** .first -> available items; .second -> unavailable items */
    auto GetAvailabilitiesShown() const noexcept { return m_availabilities_shown; }

    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    /** Sets build location for this selector, which may be used to filter
      * items in the list or enable / disable them at some point in the
      * future. */
    void SetBuildLocation(int location_id, bool refresh_list = true);

    /** Sets id of empire (or ALL_EMPIRES) for which to show items in this
      * BuildSelector. */
    void SetEmpireID(int empire_id = ALL_EMPIRES, bool refresh_list = true);

    /** Clear and refill list of buildable items, according to current
      * filter settings. */
    void Refresh();

    /** Show or hide indicated types of buildable items */
    void ShowType(BuildType type, bool refresh_list = true);
    void ShowAllTypes(bool refresh_list = true);
    void HideType(BuildType type, bool refresh_list = true);
    void HideAllTypes(bool refresh_list = true);

    /** Show or hide indicated availabilities of buildable items.  Available
      * items are those which have been unlocked for this selector's emipre. */
    void ShowAvailability(bool available, bool refresh_list = true);
    void HideAvailability(bool available, bool refresh_list = true);

    mutable boost::signals2::signal<void (const BuildingType*)> DisplayBuildingTypeSignal;
    mutable boost::signals2::signal<void (const ShipDesign*)>   DisplayShipDesignSignal;
    mutable boost::signals2::signal<void ()>                    DisplayStockpileProjectSignal;
    mutable boost::signals2::signal<void (ProductionQueue::ProductionItem, int, int)>
                                                                RequestBuildItemSignal;
    mutable boost::signals2::signal<void ()>                    ShowPediaSignal;

private:
    static constexpr GG::X TEXT_MARGIN_X{3};
    static constexpr GG::Y TEXT_MARGIN_Y{3};

    void DoLayout();

    bool BuildableItemVisible(BuildType build_type);
    bool BuildableItemVisible(BuildType build_type, const std::string& name);
    bool BuildableItemVisible(BuildType build_type, int design_id);

    /** Clear and refill list of buildable items, according to current
      * filter settings. */
    void PopulateList();

    void AddBuildItemToQueue(GG::ListBox::iterator it, bool top);

    /** respond to the user single-clicking a producible item in the build selector */
    void BuildItemLeftClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys);

    /** respond to the user right-clicking a producible item in the build selector */
    void BuildItemRightClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys);

    std::map<BuildType, std::shared_ptr<CUIStateButton>>    m_build_type_buttons;
    std::vector<std::shared_ptr<CUIStateButton>>            m_availability_buttons;
    std::set<BuildType>                                     m_build_types_shown;
    std::pair<bool, bool>                                   m_availabilities_shown; //!< .first -> available items; .second -> unavailable items
    std::shared_ptr<BuildableItemsListBox>                  m_buildable_items;
    GG::Pt                                                  m_original_ul;
    int                                                     m_production_location;
    int                                                     m_empire_id;
    mutable boost::signals2::scoped_connection              m_empire_ship_designs_changed_connection;

    friend class BuildDesignatorWnd;        // so BuildDesignatorWnd can access buttons
};

BuildDesignatorWnd::BuildSelector::BuildSelector(std::string_view config_name) :
    CUIWnd(UserString("PRODUCTION_WND_BUILD_ITEMS_TITLE"),
           GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | GG::ONTOP | PINABLE,
           config_name),
    m_buildable_items(GG::Wnd::Create<BuildableItemsListBox>()),
    m_production_location(INVALID_OBJECT_ID),
    m_empire_id(ALL_EMPIRES)
{}

void BuildDesignatorWnd::BuildSelector::CompleteConstruction() {
    // create build type toggle buttons (ship, building, all)
    m_build_type_buttons[BuildType::BT_BUILDING] = GG::Wnd::Create<CUIStateButton>(
        UserString("PRODUCTION_WND_CATEGORY_BT_BUILDING"),
        GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    AttachChild(m_build_type_buttons[BuildType::BT_BUILDING]);
    m_build_type_buttons[BuildType::BT_SHIP] = GG::Wnd::Create<CUIStateButton>(
        UserString("PRODUCTION_WND_CATEGORY_BT_SHIP"),
        GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>());
    AttachChild(m_build_type_buttons[BuildType::BT_SHIP]);

    // create availability toggle buttons (available, not available)
    m_availability_buttons.push_back(GG::Wnd::Create<CUIStateButton>(
        UserString("PRODUCTION_WND_AVAILABILITY_AVAILABLE"),
        GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>()));
    AttachChild(m_availability_buttons.back());
    m_availability_buttons.push_back(GG::Wnd::Create<CUIStateButton>(
        UserString("PRODUCTION_WND_AVAILABILITY_UNAVAILABLE"),
        GG::FORMAT_CENTER, std::make_shared<CUILabelButtonRepresenter>()));
    AttachChild(m_availability_buttons.back());

    // selectable list of buildable items
    AttachChild(m_buildable_items);

    m_buildable_items->LeftClickedRowSignal.connect(
        [this](GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys)
        { BuildItemLeftClicked(it, pt, modkeys); });

    m_buildable_items->DoubleClickedRowSignal.connect(
        [this](GG::ListBox::iterator it, GG::Pt, GG::Flags<GG::ModKey> modkeys)
        { AddBuildItemToQueue(it, modkeys & GG::MOD_KEY_CTRL); });

    m_buildable_items->RightClickedRowSignal.connect(
        [this](GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys)
        { BuildItemRightClicked(it, pt, modkeys); });

    //auto header = GG::Wnd::Create<GG::ListBox::Row>();
    //std::shared_ptr<GG::Font> font = ClientUI::GetFont();
    //GG::Clr clr = ClientUI::TextColor();
    //header->push_back("item", font, clr);
    //header->push_back("PP/turn", font, clr);
    //header->push_back("turns", font, clr);
    //header->push_back("description", font, clr);
    //header->SetColWidths(col_widths);
    //m_buildable_items->SetColHeaders(header);


    //m_buildable_items->SetNumCols(static_cast<int>(col_widths.size()));
    //m_buildable_items->LockColWidths();

    //for (unsigned int i = 0; i < col_widths.size(); ++i) {
    //    m_buildable_items->SetColWidth(i, col_widths[i]);
    //    m_buildable_items->SetColAlignment(i, GG::ALIGN_LEFT);
    //}

    CUIWnd::CompleteConstruction();

    DoLayout();
    SaveDefaultedOptions();
}

void BuildDesignatorWnd::BuildSelector::DoLayout() {
    int num_buttons = 4;
    GG::X x(GG::X0);
    GG::X button_width = ClientWidth() / num_buttons;
    GG::Y button_height{ClientUI::Pts()*4/3};

    m_build_type_buttons[BuildType::BT_BUILDING]->SizeMove(GG::Pt(x, GG::Y0), GG::Pt(x + button_width, button_height));
    x += button_width;
    m_build_type_buttons[BuildType::BT_SHIP]->SizeMove(    GG::Pt(x, GG::Y0), GG::Pt(x + button_width, button_height));
    x += button_width;
    m_availability_buttons[0]->SizeMove(                   GG::Pt(x, GG::Y0), GG::Pt(x + button_width, button_height));
    x += button_width;
    m_availability_buttons[1]->SizeMove(                   GG::Pt(x, GG::Y0), GG::Pt(x + button_width, button_height));
    x += button_width;

    m_buildable_items->SizeMove(GG::Pt(GG::X0, button_height),
                                ClientSize() - GG::Pt(GG::X0, GG::Y(INNER_BORDER_ANGLE_OFFSET)));
}

void BuildDesignatorWnd::BuildSelector::SizeMove(GG::Pt ul, GG::Pt lr) {
    GG::Pt old_size = GG::Wnd::Size();
    CUIWnd::SizeMove(ul, lr);
    if (old_size != GG::Wnd::Size())
        DoLayout();
}

void BuildDesignatorWnd::BuildSelector::SetBuildLocation(int location_id, bool refresh_list) {
    if (m_production_location != location_id) {
        m_production_location = location_id;
        if (refresh_list)
            Refresh();
    }
}

void BuildDesignatorWnd::BuildSelector::SetEmpireID(int empire_id, bool refresh_list) {
    if (empire_id == m_empire_id)
        return;

    m_empire_id = empire_id;
    if (refresh_list) {
         Refresh();
    } else {
        // ensure signal connection set up properly, without actually
        // repopulating the list, as would be dine in Refresh()
        m_empire_ship_designs_changed_connection.disconnect();
        const ScriptingContext& context = GGHumanClientApp::GetApp()->GetContext();
        if (auto empire = context.GetEmpire(m_empire_id))
            m_empire_ship_designs_changed_connection = empire->ShipDesignsChangedSignal.connect(
                [this]() { Refresh(); }, boost::signals2::at_front);
    }
}

void BuildDesignatorWnd::BuildSelector::Refresh() {
    ScopedTimer timer("BuildDesignatorWnd::BuildSelector::Refresh()");
    const ScriptingContext& context = GGHumanClientApp::GetApp()->GetContext();

    if (auto prod_loc = context.ContextObjects().get(this->m_production_location))
        this->SetName(boost::io::str(FlexibleFormat(UserString("PRODUCTION_WND_BUILD_ITEMS_TITLE_LOCATION")) % prod_loc->Name()));
    else
        this->SetName(UserString("PRODUCTION_WND_BUILD_ITEMS_TITLE"));

    m_empire_ship_designs_changed_connection.disconnect();
    if (auto empire = context.GetEmpire(m_empire_id))
        m_empire_ship_designs_changed_connection = empire->ShipDesignsChangedSignal.connect(
            [this]() { Refresh(); }, boost::signals2::at_front);
    PopulateList();
}

void BuildDesignatorWnd::BuildSelector::ShowType(BuildType type, bool refresh_list) {
    if (!m_build_types_shown.contains(type)) {
        m_build_types_shown.insert(type);
        if (refresh_list)
            Refresh();
    }
}

void BuildDesignatorWnd::BuildSelector::HideType(BuildType type, bool refresh_list) {
    auto it = m_build_types_shown.find(type);
    if (it != m_build_types_shown.end()) {
        m_build_types_shown.erase(it);
        if (refresh_list)
            Refresh();
    }
}

void BuildDesignatorWnd::BuildSelector::ShowAllTypes(bool refresh_list) {
    m_build_types_shown.insert(BuildType::BT_BUILDING);
    m_build_types_shown.insert(BuildType::BT_SHIP);
    m_build_types_shown.insert(BuildType::BT_STOCKPILE);
    if (refresh_list)
        Refresh();
}

void BuildDesignatorWnd::BuildSelector::HideAllTypes(bool refresh_list) {
    m_build_types_shown.clear();
    if (refresh_list)
        Refresh();
}

void BuildDesignatorWnd::BuildSelector::ShowAvailability(bool available, bool refresh_list) {
    if (available) {
        if (!m_availabilities_shown.first) {
            m_availabilities_shown.first = true;
            if (refresh_list)
                Refresh();
        }
    } else {
        if (!m_availabilities_shown.second) {
            m_availabilities_shown.second = true;
            if (refresh_list)
                Refresh();
        }
    }
}

void BuildDesignatorWnd::BuildSelector::HideAvailability(bool available, bool refresh_list) {
    if (available) {
        if (m_availabilities_shown.first) {
            m_availabilities_shown.first = false;
            if (refresh_list)
                Refresh();
        }
    } else {
        if (m_availabilities_shown.second) {
            m_availabilities_shown.second = false;
            if (refresh_list)
                Refresh();
        }
    }
}

bool BuildDesignatorWnd::BuildSelector::BuildableItemVisible(BuildType build_type) {
    if (build_type != BuildType::BT_STOCKPILE)
        throw std::invalid_argument("BuildableItemVisible was passed an invalid build type without id");

    const ScriptingContext& context = IApp::GetApp()->GetContext();
    if (auto empire = context.GetEmpire(m_empire_id))
        return empire->ProducibleItem(build_type, m_production_location, context);
    return true;
}

bool BuildDesignatorWnd::BuildSelector::BuildableItemVisible(BuildType build_type, const std::string& name) {
    if (build_type != BuildType::BT_BUILDING)
        throw std::invalid_argument("BuildableItemVisible was passed an invalid build type with a name");

    if (!m_build_types_shown.contains(build_type))
        return false;

    const BuildingType* building_type = GetBuildingType(name);
    if (!building_type || !building_type->Producible())
        return false;

    const ScriptingContext& context = IApp::GetApp()->GetContext();
    auto empire = context.GetEmpire(m_empire_id);
    if (!empire)
        return true;

    // check that item is both enqueuable and producible, since most buildings currently have
    // nonselective EnqueueLocation conditions
    bool enqueuable_here = empire->EnqueuableItem(BuildType::BT_BUILDING, name, m_production_location,
                                                  context) &&
                           empire->ProducibleItem(BuildType::BT_BUILDING, name, m_production_location,
                                                  context);

    if (enqueuable_here)
        return m_availabilities_shown.first;
    else
        return m_availabilities_shown.second;
}

bool BuildDesignatorWnd::BuildSelector::BuildableItemVisible(BuildType build_type, int design_id) {
    if (build_type != BuildType::BT_SHIP)
        throw std::invalid_argument("BuildableItemVisible was passed an invalid build type with an id");

    if (!m_build_types_shown.contains(build_type))
        return false;

    const ScriptingContext& context = IApp::GetApp()->GetContext();

    const ShipDesign* design = context.ContextUniverse().GetShipDesign(design_id);
    if (!design || !design->Producible())
        return false;

    const auto& empire = context.GetEmpire(m_empire_id);
    if (!empire)
        return true;

    bool producible_here = empire->ProducibleItem(BuildType::BT_SHIP, design_id,
                                                  m_production_location, context);

    if (producible_here)
        return m_availabilities_shown.first;
    else
        return m_availabilities_shown.second;
}

void BuildDesignatorWnd::BuildSelector::PopulateList() {
    const ScriptingContext& context = IApp::GetApp()->GetContext();
    const Universe& universe{context.ContextUniverse()};
    auto empire = context.GetEmpire(m_empire_id);
    if (!empire)
        return;

    SectionedScopedTimer timer("BuildDesignatorWnd::BuildSelector::PopulateList");

    // Capture the list scroll state
    // Try to preserve the same queue context with completely new queue items
    std::size_t initial_offset_from_begin = std::distance(m_buildable_items->begin(), m_buildable_items->FirstRowShown());
    std::size_t initial_offset_to_end = std::distance(m_buildable_items->FirstRowShown(), m_buildable_items->end());
    bool initial_last_visible_row_is_end(m_buildable_items->LastVisibleRow() == m_buildable_items->end());

    m_buildable_items->Clear(); // the list of items to be populated

    auto default_font = ClientUI::GetFont();
    const GG::Pt row_size = m_buildable_items->ListRowSize();

    timer.EnterSection("fixed projects");
    // populate list with fixed projects
    if (BuildableItemVisible(BuildType::BT_STOCKPILE)) {
        auto stockpile_row = GG::Wnd::Create<ProductionItemRow>(
            row_size.x, row_size.y, ProductionQueue::ProductionItem(BuildType::BT_STOCKPILE),
            m_empire_id, m_production_location);
        m_buildable_items->Insert(std::move(stockpile_row));
    }

    // populate list with building types
    //DebugLogger() << "BuildDesignatorWnd::BuildSelector::PopulateList() : Adding Buildings ";
    if (m_build_types_shown.contains(BuildType::BT_BUILDING)) {
        // create and insert rows...
        const auto is_visible = [this](const auto& name) { return BuildableItemVisible(BuildType::BT_BUILDING, name); };
        const auto create_row = [this, row_size](const auto& name) {
            return GG::Wnd::Create<ProductionItemRow>(
                row_size.x, row_size.y, ProductionQueue::ProductionItem(BuildType::BT_BUILDING, name),
                m_empire_id, m_production_location);
        };
        auto buildable_rows_rng = GetBuildingTypeManager() | range_keys
            | range_filter(is_visible) | range_transform(create_row);
        m_buildable_items->Insert({buildable_rows_rng.begin(), buildable_rows_rng.end()});
    }

    // populate with ship designs
    //DebugLogger() << "BuildDesignatorWnd::BuildSelector::PopulateList() : Adding ship designs";
    if (m_build_types_shown.contains(BuildType::BT_SHIP)) {
        // get ids of designs to show... for specific empire, or for all empires
        std::vector<int> design_ids;
        if (empire) {
            design_ids = ClientUI::GetClientUI()->GetShipDesignManager()->DisplayedDesigns()->OrderedIDs();
        } else {
            design_ids.reserve(universe.ShipDesigns().size());
            std::transform(universe.ShipDesigns().begin(), universe.ShipDesigns().end(),
                           std::back_inserter(design_ids),
                           [](const auto id_design) { return id_design.first; });
        }

        // create and insert rows...
        std::vector<std::shared_ptr<GG::ListBox::Row>> rows;
        rows.reserve(design_ids.size());
        for (int ship_design_id : design_ids) {
            if (!BuildableItemVisible(BuildType::BT_SHIP, ship_design_id))
                continue;
            const ShipDesign* ship_design = universe.GetShipDesign(ship_design_id);
            if (!ship_design)
                continue;
            timer.EnterSection(ship_design->Name());
            auto item_row = GG::Wnd::Create<ProductionItemRow>(
                row_size.x, row_size.y, 
                ProductionQueue::ProductionItem(BuildType::BT_SHIP, ship_design_id, universe),
                m_empire_id, m_production_location);
            rows.push_back(std::move(item_row));
        }
        m_buildable_items->Insert(std::move(rows));
    }

    timer.EnterSection("end bits");
    // resize inserted rows and record first row to show
    for (auto& row : *m_buildable_items)
        row->Resize(row_size);

    // Restore the list scroll state
    // If we were at the top stay at the top
    if (initial_offset_from_begin == 0)
        m_buildable_items->SetFirstRowShown(m_buildable_items->begin());

    // If we were not at the bottom then keep the same first row position
    else if (!initial_last_visible_row_is_end && initial_offset_from_begin < m_buildable_items->NumRows())
        m_buildable_items->SetFirstRowShown(
            std::next(m_buildable_items->begin(), initial_offset_from_begin));

    // otherwise keep the same relative position from the bottom to
    // preserve the end of list dead space
    else if (initial_offset_to_end < m_buildable_items->NumRows())
        m_buildable_items->SetFirstRowShown(
            std::next(m_buildable_items->begin(), m_buildable_items->NumRows() -  initial_offset_to_end));
    else
        m_buildable_items->SetFirstRowShown(m_buildable_items->begin());
}

void BuildDesignatorWnd::BuildSelector::BuildItemLeftClicked(GG::ListBox::iterator it,
                                                             GG::Pt pt, GG::Flags<GG::ModKey> modkeys)
{
    ProductionItemRow* item_row = dynamic_cast<ProductionItemRow*>(it->get());
    if (!item_row)
        return;
    const ProductionQueue::ProductionItem& item = item_row->Item();

    const BuildType build_type = item.build_type;

    if (build_type == BuildType::BT_BUILDING) {
        const BuildingType* building_type = GetBuildingType(item.name);
        if (!building_type) {
            ErrorLogger() << "BuildDesignatorWnd::BuildSelector::BuildItemSelected unable to get building type: " << item.name;
            return;
        }
        DisplayBuildingTypeSignal(building_type);

    } else if (build_type == BuildType::BT_SHIP) {
        const ShipDesign* design = IApp::GetApp()->GetContext().ContextUniverse().GetShipDesign(item.design_id);
        if (!design) {
            ErrorLogger() << "BuildDesignatorWnd::BuildSelector::BuildItemSelected unable to find design with id " << item.design_id;
            return;
        }
        DisplayShipDesignSignal(design);

    } else if (build_type == BuildType::BT_STOCKPILE) {
        DisplayStockpileProjectSignal();
    }
}

void BuildDesignatorWnd::BuildSelector::AddBuildItemToQueue(GG::ListBox::iterator it, bool top) {
    if ((*it)->Disabled())
        return;
    if (ProductionItemRow* item_row = dynamic_cast<ProductionItemRow*>(it->get()))
        RequestBuildItemSignal(item_row->Item(), 1, top ? 0 : -1);
}

void BuildDesignatorWnd::BuildSelector::BuildItemRightClicked(GG::ListBox::iterator it,
                                                              GG::Pt pt, GG::Flags<GG::ModKey> modkeys)
{
    ProductionItemRow* item_row = dynamic_cast<ProductionItemRow*>(it->get());
    if (!item_row)
        return;
    const ProductionQueue::ProductionItem& item = item_row->Item();

    std::string_view item_name;
    if (item.build_type == BuildType::BT_BUILDING) {
        item_name = item.name;
    } else if (item.build_type == BuildType::BT_SHIP) {
        item_name = IApp::GetApp()->GetContext().ContextUniverse().GetShipDesign(item.design_id)->Name(false);
    } else if (item.build_type == BuildType::BT_STOCKPILE) {
        item_name = UserStringNop("PROJECT_BT_STOCKPILE");
    } else {
        ErrorLogger() << "Invalid build type (" << item.build_type << ") for item";
        return;
    }

    auto add_bottom_queue_action = [this, it]() { AddBuildItemToQueue(it, false); };
    auto add_top_queue_action = [this, it]() { AddBuildItemToQueue(it, true); };
    auto pedia_lookup_action = [this, it, pt, &modkeys]() {
        ShowPediaSignal();
        BuildItemLeftClicked(it, pt, modkeys);
    };

    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);

    if (!((*it)->Disabled())) {
        popup->AddMenuItem(GG::MenuItem(UserString("PRODUCTION_DETAIL_ADD_TO_QUEUE"),   false, false, add_bottom_queue_action));
        popup->AddMenuItem(GG::MenuItem(UserString("PRODUCTION_DETAIL_ADD_TO_TOP_OF_QUEUE"),  false, false, add_top_queue_action));
    }

    if (UserStringExists(item_name))
        item_name = UserString(item_name);

    std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % item_name);
    popup->AddMenuItem(GG::MenuItem(std::move(popup_label), false, false, pedia_lookup_action));
    popup->Run();
}

//////////////////////////////////////////////////
// BuildDesignatorWnd
//////////////////////////////////////////////////
BuildDesignatorWnd::BuildDesignatorWnd(GG::X w, GG::Y h) :
    Wnd(GG::X0, GG::Y0, w, h, GG::INTERACTIVE | GG::ONTOP)
{}

void BuildDesignatorWnd::CompleteConstruction() {
    GG::Wnd::CompleteConstruction();

    m_enc_detail_panel = GG::Wnd::Create<EncyclopediaDetailPanel>(
        GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | CLOSABLE | PINABLE, PROD_PEDIA_WND_NAME);
    // Wnd is manually closed by user
    m_enc_detail_panel->ClosingSignal.connect([this]() { HidePedia(); });

    m_side_panel = GG::Wnd::Create<SidePanel>(PROD_SIDEPANEL_WND_NAME);
    m_build_selector = GG::Wnd::Create<BuildSelector>(PROD_SELECTOR_WND_NAME);
    InitializeWindows();
    GGHumanClientApp::GetApp()->RepositionWindowsSignal.connect([this]() { InitializeWindows(); });

    m_side_panel->EnableSelection();

    m_build_selector->DisplayBuildingTypeSignal.connect(
        [this](const BuildingType* bt) { m_enc_detail_panel->SetItem(bt); });

    m_build_selector->DisplayShipDesignSignal.connect(
        [this](const ShipDesign* design) { m_enc_detail_panel->SetItem(design); });

    m_build_selector->DisplayStockpileProjectSignal.connect(
        [this]() { m_enc_detail_panel->SetEncyclopediaArticle("PROJECT_BT_STOCKPILE"); });

    m_build_selector->ShowPediaSignal.connect([this]() { ShowPedia(); });

    m_build_selector->RequestBuildItemSignal.connect(
        [this](ProductionQueue::ProductionItem item, int num, int pos)
        { BuildItemRequested(std::move(item), num, pos); });

    SidePanel::PlanetSelectedSignal.connect(PlanetSelectedSignal);
    SidePanel::SystemSelectedSignal.connect(SystemSelectedSignal);

    // connect build type button clicks to update display
    m_build_selector->m_build_type_buttons[BuildType::BT_BUILDING]->CheckedSignal.connect(
        [this](bool) { ToggleType(BuildType::BT_BUILDING, true); });

    m_build_selector->m_build_type_buttons[BuildType::BT_SHIP]->CheckedSignal.connect(
        [this](bool) { ToggleType(BuildType::BT_SHIP, true); });

    // connect availability button clicks to update display
    // available items
    m_build_selector->m_availability_buttons.at(0)->CheckedSignal.connect(
        [this](bool) { ToggleAvailabilitly(true, true); });

    // UNavailable items
    m_build_selector->m_availability_buttons.at(1)->CheckedSignal.connect(
        [this](bool) { ToggleAvailabilitly(false, true); });

    AttachChild(m_enc_detail_panel);
    AttachChild(m_build_selector);
    AttachChild(m_side_panel);

    MoveChildUp(m_enc_detail_panel.get());
    MoveChildUp(m_build_selector.get());

    Clear(GGHumanClientApp::GetApp()->GetContext().ContextObjects());
}

const std::set<BuildType>& BuildDesignatorWnd::GetBuildTypesShown() const noexcept
{ return m_build_selector->GetBuildTypesShown(); }

std::pair<bool, bool> BuildDesignatorWnd::GetAvailabilitiesShown() const noexcept
{ return m_build_selector->GetAvailabilitiesShown(); }

bool BuildDesignatorWnd::InWindow(GG::Pt pt) const noexcept
{ return (m_enc_detail_panel->InWindow(pt) && m_enc_detail_panel->Visible()) || m_build_selector->InWindow(pt) || m_side_panel->InWindow(pt); }

bool BuildDesignatorWnd::InClient(GG::Pt pt) const noexcept
{ return m_enc_detail_panel->InClient(pt) || m_build_selector->InClient(pt) || m_side_panel->InClient(pt); }

int BuildDesignatorWnd::SelectedPlanetID() const noexcept
{ return m_side_panel->SelectedPlanetID(); }

void BuildDesignatorWnd::SizeMove(GG::Pt ul, GG::Pt lr) {
    const auto old_size = Size();
    GG::Wnd::SizeMove(ul, lr);
    if (old_size != Size()) {
        m_enc_detail_panel->ValidatePosition();
        m_build_selector->ValidatePosition();
        m_side_panel->ValidatePosition();
    }
}

void BuildDesignatorWnd::CenterOnBuild(int queue_idx, bool open) {
    SetBuild(queue_idx);

    auto* app = GGHumanClientApp::GetApp();
    const ScriptingContext& context = app->GetContext();
    const ObjectMap& objects = context.ContextObjects();
    int empire_id = app->EmpireID();

    auto empire = context.GetEmpire(empire_id);
    if (!empire) {
        ErrorLogger() << "BuildDesignatorWnd::CenterOnBuild couldn't get empire with id " << empire_id;
        return;
    }

    const ProductionQueue& queue = empire->GetProductionQueue();
    if (0 <= queue_idx && queue_idx < static_cast<int>(queue.size())) {
        int location_id = queue[queue_idx].location;
        if (auto build_location = objects.get(location_id)) {
            // centre map on system of build location
            int system_id = build_location->SystemID();
            if (auto map = ClientUI::GetClientUI()->GetMapWnd(false)) {
                map->CenterOnObject(system_id);
                if (open) {
                    map->SelectSystem(system_id);
                    SelectPlanet(location_id, objects);
                }
            }
        }
    }
}

void BuildDesignatorWnd::SetBuild(int queue_idx) {
    const ScriptingContext& context = IApp::GetApp()->GetContext();

    int empire_id = GGHumanClientApp::GetApp()->EmpireID();
    auto empire = context.GetEmpire(empire_id);
    if (!empire) {
        ErrorLogger() << "BuildDesignatorWnd::SetBuild couldn't get empire with id " << empire_id;
        return;
    }

    const ProductionQueue& queue = empire->GetProductionQueue();
    if (0 <= queue_idx && queue_idx < static_cast<int>(queue.size())) {
        BuildType buildType = queue[queue_idx].item.build_type;
        if (buildType == BuildType::BT_BUILDING) {
            const BuildingType* building_type = GetBuildingType(queue[queue_idx].item.name);
            assert(building_type);
            m_build_selector->DisplayBuildingTypeSignal(building_type);
        } else if (buildType == BuildType::BT_SHIP) {
            const ShipDesign* design = context.ContextUniverse().GetShipDesign(queue[queue_idx].item.design_id);
            assert(design);
            m_build_selector->DisplayShipDesignSignal(design);
        } else if (buildType == BuildType::BT_STOCKPILE) {
            m_build_selector->DisplayStockpileProjectSignal();
        }
    } else {
        m_enc_detail_panel->OnIndex();
    }
    m_enc_detail_panel->Refresh();
}

void BuildDesignatorWnd::SelectSystem(int system_id, const ObjectMap& objects) {
    if (system_id == SidePanel::SystemID()) {
        // don't need to do anything.  already showing the requested system.
        return;
    }

    if (system_id != INVALID_OBJECT_ID) {
        // set sidepanel's system and autoselect a suitable planet
        SidePanel::SetSystem(system_id);
        SelectDefaultPlanet(objects);
    }
}

void BuildDesignatorWnd::SelectPlanet(int planet_id, const ObjectMap& objects) {
    SidePanel::SelectPlanet(planet_id, objects);
    if (planet_id != INVALID_OBJECT_ID)
        m_system_default_planets[SidePanel::SystemID()] = planet_id;
    m_build_selector->SetBuildLocation(this->BuildLocation());
}

void BuildDesignatorWnd::Refresh() {
    m_build_selector->SetEmpireID(GGHumanClientApp::GetApp()->EmpireID(), false);
    Update();
}

void BuildDesignatorWnd::Update() {
    SidePanel::Update();
    m_build_selector->Refresh();
    m_enc_detail_panel->Refresh();
}

void BuildDesignatorWnd::InitializeWindows() {
    GG::X queue_width(GetOptionsDB().Get<GG::X>("ui.queue.width"));

    const GG::X SIDEPANEL_WIDTH(GetOptionsDB().Get<GG::X>("ui.map.sidepanel.width"));
    static constexpr GG::Y PANEL_HEIGHT{240};

    const GG::Pt pedia_ul(queue_width, GG::Y0);
    const GG::Pt pedia_wh(Width() - SIDEPANEL_WIDTH - queue_width, PANEL_HEIGHT);

    const GG::Pt sidepanel_ul(Width() - SIDEPANEL_WIDTH, GG::Y0);
    const GG::Pt sidepanel_wh(SIDEPANEL_WIDTH, Height());

    const GG::Pt selector_ul(queue_width, Height() - PANEL_HEIGHT);
    const GG::Pt selector_wh(Width() - SIDEPANEL_WIDTH - queue_width, PANEL_HEIGHT);

    m_enc_detail_panel->InitSizeMove(pedia_ul,      pedia_ul + pedia_wh);
    m_side_panel->      InitSizeMove(sidepanel_ul,  sidepanel_ul + sidepanel_wh);
    m_build_selector->  InitSizeMove(selector_ul,   selector_ul + selector_wh);
}

void BuildDesignatorWnd::Reset(const ObjectMap& objects) {
    SelectSystem(INVALID_OBJECT_ID, objects);
    ShowAllTypes(false);            // show all types without populating the list
    HideAvailability(false, false); // hide unavailable items without populating the list
    ShowAvailability(true, false);  // show available items without populating the list
    m_build_selector->Refresh();
    m_enc_detail_panel->OnIndex();
}

void BuildDesignatorWnd::Clear(const ObjectMap& objects) {
    SidePanel::SetSystem(INVALID_OBJECT_ID);
    Reset(objects);
    m_system_default_planets.clear();
}

void BuildDesignatorWnd::ShowType(BuildType type, bool refresh_list) {
    if (type == BuildType::BT_BUILDING || type == BuildType::BT_SHIP) {
        m_build_selector->ShowType(type, refresh_list);
        m_build_selector->m_build_type_buttons[type]->SetCheck();
    } else if (type == BuildType::BT_STOCKPILE) {
        m_build_selector->ShowType(type, refresh_list);
    } else {
        ErrorLogger() << "BuildDesignatorWnd::ShowType(" << type << ")";
        throw std::invalid_argument("BuildDesignatorWnd::ShowType was passed an invalid BuildType");
    }
}

void BuildDesignatorWnd::ShowAllTypes(bool refresh_list) {
    m_build_selector->ShowAllTypes(refresh_list);
    m_build_selector->m_build_type_buttons[BuildType::BT_BUILDING]->SetCheck();
    m_build_selector->m_build_type_buttons[BuildType::BT_SHIP]->SetCheck();
}

void BuildDesignatorWnd::HideType(BuildType type, bool refresh_list) {
    DebugLogger() << "BuildDesignatorWnd::HideType(" << type << ")";
    if (type == BuildType::BT_BUILDING || type == BuildType::BT_SHIP) {
        m_build_selector->HideType(type, refresh_list);
        m_build_selector->m_build_type_buttons[type]->SetCheck(false);
    } else if (type == BuildType::BT_STOCKPILE) {
        m_build_selector->HideType(type, refresh_list);
    } else {
        throw std::invalid_argument("BuildDesignatorWnd::HideType was passed an invalid BuildType");
    }
}

void BuildDesignatorWnd::HideAllTypes(bool refresh_list) {
    m_build_selector->HideAllTypes(refresh_list);
    m_build_selector->m_build_type_buttons[BuildType::BT_BUILDING]->SetCheck(false);
    m_build_selector->m_build_type_buttons[BuildType::BT_SHIP]->SetCheck(false);
}

void BuildDesignatorWnd::ToggleType(BuildType type, bool refresh_list) {
    if (type == BuildType::BT_BUILDING || type == BuildType::BT_SHIP) {
        const std::set<BuildType>& types_shown = m_build_selector->GetBuildTypesShown();
        if (!types_shown.contains(type))
            ShowType(type, refresh_list);
        else
            HideType(type, refresh_list);
    } else {
        throw std::invalid_argument("BuildDesignatorWnd::ShowType was passed an invalid BuildType");
    }
}

void BuildDesignatorWnd::ToggleAllTypes(bool refresh_list) {
    const std::set<BuildType>& types_shown = m_build_selector->GetBuildTypesShown();
    if (types_shown.size() == int(BuildType::NUM_BUILD_TYPES) - 1)  // -1 because there are no buttons for BuildType::BT_NOT_BUILDING
        HideAllTypes(refresh_list);
    else
        ShowAllTypes(refresh_list);
}

void BuildDesignatorWnd::ShowAvailability(bool available, bool refresh_list) {
    m_build_selector->ShowAvailability(available, refresh_list);
    if (available)
        m_build_selector->m_availability_buttons.at(0)->SetCheck();
    else
        m_build_selector->m_availability_buttons.at(1)->SetCheck();
}

void BuildDesignatorWnd::HideAvailability(bool available, bool refresh_list) {
    m_build_selector->HideAvailability(available, refresh_list);
    if (available)
        m_build_selector->m_availability_buttons.at(0)->SetCheck(false);
    else
        m_build_selector->m_availability_buttons.at(1)->SetCheck(false);
}

void BuildDesignatorWnd::ToggleAvailabilitly(bool available, bool refresh_list) {
    const auto avail_shown = m_build_selector->GetAvailabilitiesShown();
    if (available) {
        if (avail_shown.first)
            HideAvailability(true, refresh_list);
        else
            ShowAvailability(true, refresh_list);
    } else {
        if (avail_shown.second)
            HideAvailability(false, refresh_list);
        else
            ShowAvailability(false, refresh_list);
    }
}

void BuildDesignatorWnd::ShowBuildingTypeInEncyclopedia(std::string building_type)
{ m_enc_detail_panel->SetBuildingType(std::move(building_type)); }

void BuildDesignatorWnd::ShowShipDesignInEncyclopedia(int design_id)
{ m_enc_detail_panel->SetDesign(design_id); }

void BuildDesignatorWnd::ShowPlanetInEncyclopedia(int planet_id)
{ m_enc_detail_panel->SetPlanet(planet_id); }

void BuildDesignatorWnd::ShowTechInEncyclopedia(std::string tech_name)
{ m_enc_detail_panel->SetTech(std::move(tech_name)); }

void BuildDesignatorWnd::ShowPolicyInEncyclopedia(std::string policy_name)
{ m_enc_detail_panel->SetPolicy(std::move(policy_name)); }

void BuildDesignatorWnd::ShowShipPartInEncyclopedia(std::string part_name)
{ m_enc_detail_panel->SetShipPart(std::move(part_name)); }

void BuildDesignatorWnd::ShowSpeciesInEncyclopedia(std::string species_name)
{ m_enc_detail_panel->SetSpecies(std::move(species_name)); }

void BuildDesignatorWnd::ShowEmpireInEncyclopedia(int empire_id)
{ m_enc_detail_panel->SetEmpire(empire_id); }

void BuildDesignatorWnd::ShowSpecialInEncyclopedia(std::string special_name)
{ m_enc_detail_panel->SetSpecial(std::move(special_name)); }

void BuildDesignatorWnd::ShowFieldTypeInEncyclopedia(std::string field_type_name)
{ m_enc_detail_panel->SetFieldType(std::move(field_type_name)); }

void BuildDesignatorWnd::ShowPedia() {
    m_enc_detail_panel->Refresh();
    m_enc_detail_panel->Show();

    OptionsDB& db = GetOptionsDB();
    db.Set("ui." + PROD_PEDIA_WND_NAME + ".hidden.enabled", false);
}

void BuildDesignatorWnd::HidePedia() {
    m_enc_detail_panel->Hide();

    OptionsDB& db = GetOptionsDB();
    db.Set("ui." + PROD_PEDIA_WND_NAME + ".hidden.enabled", true);
}

void BuildDesignatorWnd::TogglePedia() {
    if (!m_enc_detail_panel->Visible())
        ShowPedia();
    else
        HidePedia();
}

bool BuildDesignatorWnd::PediaVisible()
{ return m_enc_detail_panel->Visible(); }

int BuildDesignatorWnd::BuildLocation() const
{ return m_side_panel->SelectedPlanetID(); }

void BuildDesignatorWnd::BuildItemRequested(ProductionQueue::ProductionItem item,
                                            int num_to_build, int pos)
{
    const ScriptingContext& context = IApp::GetApp()->GetContext();
    auto empire = context.GetEmpire(GGHumanClientApp::GetApp()->EmpireID());
    if (empire && empire->EnqueuableItem(item, BuildLocation(), context))
        AddBuildToQueueSignal(std::move(item), num_to_build, BuildLocation(), pos);
}

void BuildDesignatorWnd::BuildQuantityChanged(int queue_idx, int quantity)
{ BuildQuantityChangedSignal(queue_idx, quantity); }

void BuildDesignatorWnd::SelectDefaultPlanet(const ObjectMap& objects) {
    int system_id = SidePanel::SystemID();
    if (system_id == INVALID_OBJECT_ID) {
        this->SelectPlanet(INVALID_OBJECT_ID, objects);
        return;
    }


    // select recorded default planet for this system, if there is one recorded
    // unless that planet can't be selected or doesn't exist in this system
    auto it = m_system_default_planets.find(system_id);
    if (it != m_system_default_planets.end()) {
        int planet_id = it->second;
        if (m_side_panel->PlanetSelectable(planet_id, objects)) {
            this->SelectPlanet(it->second, objects);
            return;
        }
    }

    // couldn't reselect stored default, so need to find a reasonable other
    // planet to select.  attempt to find one owned by this client's player

    // only checking visible objects for this clients empire (and not the
    // latest known objects) as an empire shouldn't be able to use a planet or
    // system it can't currently see as a production location.
    auto sys = objects.get<System>(system_id);
    if (!sys) {
        ErrorLogger() << "BuildDesignatorWnd::SelectDefaultPlanet couldn't get system with id " << system_id;
        return;
    }

    auto planets = objects.find<const Planet>(sys->PlanetIDs());

    if (planets.empty()) {
        this->SelectPlanet(INVALID_OBJECT_ID, objects);
        return;
    }


    //bool found_planet = false;                              // was a suitable planet found?
    int best_planet_id = INVALID_OBJECT_ID; // id of selected planet
    double best_planet_pop = -99999.9;                      // arbitrary negative number, so any planet's pop will be better

    for (auto& planet : planets) {
        int planet_id = planet->ID();
        if (!m_side_panel->PlanetSelectable(planet_id, objects))
            continue;

        double planet_pop = planet->GetMeter(MeterType::METER_POPULATION)->Initial();
        if (planet_pop > best_planet_pop) {
            // found new planet to pick
            //found_planet = true;
            best_planet_pop = planet_pop;
            best_planet_id = planet_id;
        }
    }

    // select top pop planet or invalid planet if no suitable planet found
    this->SelectPlanet(best_planet_id, objects);
}
