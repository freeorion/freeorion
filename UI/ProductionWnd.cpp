#include "ProductionWnd.h"

#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "BuildDesignatorWnd.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "QueueListBox.h"
#include "SidePanel.h"
#include "IconTextBrowseWnd.h"
#include "../Empire/Empire.h"
#include "../client/human/HumanClientApp.h"
#include "../util/i18n.h"
#include "../util/Order.h"
#include "../util/ScopedTimer.h"
#include "../universe/Building.h"
#include "../universe/ShipDesign.h"
#include "../universe/Enums.h"

#include <GG/DrawUtil.h>
#include <GG/Layout.h>
#include <GG/StaticGraphic.h>

#include <boost/cast.hpp>

#include <cmath>
#include <iterator>


namespace {
    const float OUTER_LINE_THICKNESS = 2.0f;

    void AddOptions(OptionsDB& db) {
        // queue width used also on research screen. prevent double-adding...
        if (!db.OptionExists("UI.queue-width"))
            db.Add("UI.queue-width",                    UserStringNop("OPTIONS_DB_UI_QUEUE_WIDTH"),         350,    RangedValidator<int>(200, 500));
        db.Add("UI.show-production-location-on-queue",  UserStringNop("OPTIONS_DB_UI_PROD_QUEUE_LOCATION"), true,   Validator<bool>());
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    ////////////////
    // QuantLabel //
    ////////////////
    class QuantLabel : public GG::Control {
    public:
        QuantLabel(int quantity, int designID, GG::X nwidth,
                   GG::Y h, bool inProgress, bool amBlockType) :
            Control(GG::X0, GG::Y0, nwidth, h, GG::NO_WND_FLAGS)
        {
            GG::Clr txtClr = inProgress ? GG::LightColor(ClientUI::ResearchableTechTextAndBorderColor()) : ClientUI::ResearchableTechTextAndBorderColor();
            std::string nameText;
            if (amBlockType)
                nameText = boost::io::str(FlexibleFormat(UserString("PRODUCTION_QUEUE_MULTIPLES")) % quantity);
            else
                nameText = boost::io::str(FlexibleFormat(UserString("PRODUCTION_QUEUE_REPETITIONS")) % quantity);
            //nameText += GetShipDesign(designID)->Name();
            GG::Label* text = new CUILabel(nameText, GG::FORMAT_TOP | GG::FORMAT_LEFT | GG::FORMAT_NOWRAP);
            text->SetTextColor(txtClr);
            text->OffsetMove(GG::Pt(GG::X0, GG::Y(-3)));
            AttachChild(text);
            Resize(GG::Pt(nwidth, text->Height()));
        }

        void Render() override
        {}
    };

    //////////////
    // QuantRow //
    //////////////
    class QuantRow : public GG::ListBox::Row {
    public:
        QuantRow(int quantity, int designID, GG::X nwidth, GG::Y h,
                 bool inProgress, bool amBlockType) :
            GG::ListBox::Row(),
            m_quant(quantity)
        {
            QuantLabel* newLabel = new QuantLabel(m_quant, designID, nwidth, h, inProgress, amBlockType);
            push_back(newLabel);
            Resize(GG::Pt(nwidth, newLabel->Height()-GG::Y0));//might subtract more; assessing aesthetics
        }

        int Quant() const { return m_quant; }

    private:
        int m_quant;
    };

    //////////////////////
    // QuantitySelector //
    //////////////////////
    class QuantitySelector : public CUIDropDownList {
    public:
        mutable boost::signals2::signal<void (int, int)> QuantChangedSignal;

        /** \name Structors */
        QuantitySelector(const ProductionQueue::Element &build, GG::X xoffset, GG::Y yoffset,
                         GG::Y h, bool inProgress, GG::X nwidth, bool amBlockType) :
            CUIDropDownList(12),
            quantity(build.remaining),
            prevQuant(build.remaining),
            blocksize(build.blocksize),
            prevBlocksize(build.blocksize),
            amBlockType(amBlockType),
            amOn(false),
            h(h)
        {
            MoveTo(GG::Pt(xoffset, yoffset));
            Resize(GG::Pt(nwidth, h - GG::Y(2)));

            DisableDropArrow();
            SetStyle(GG::LIST_LEFT | GG::LIST_NOSORT);
            SetColor(inProgress ? GG::LightColor(ClientUI::ResearchableTechTextAndBorderColor())
                                : ClientUI::ResearchableTechTextAndBorderColor());
            SetInteriorColor(inProgress ? GG::LightColor(ClientUI::ResearchableTechFillColor())
                                        : ClientUI::ResearchableTechFillColor());
            SetNumCols(1);

            int quantInts[] = {1, 2, 3, 4, 5, 10, 20, 30, 40, 50, 99};
            std::set<int> myQuantSet(quantInts, quantInts + 11);

            if (amBlockType)
                myQuantSet.insert(blocksize); //as currently implemented this one not actually necessary since blocksize has no other way to change
            else
                myQuantSet.insert(quantity);

            for (int quantity : myQuantSet) {
                QuantRow* row =  new QuantRow(quantity, build.item.design_id, nwidth, h, inProgress, amBlockType);
                GG::DropDownList::iterator latest_it = Insert(row);

                if (amBlockType) {
                    if (build.blocksize == quantity)
                        Select(latest_it);
                } else {
                    if (build.remaining == quantity)
                        Select(latest_it);
                }
            }

            GG::Connect(this->SelChangedSignal, &QuantitySelector::SelectionChanged, this);
        }

        void SelectionChanged(GG::DropDownList::iterator it) {
            amOn = false;

            DebugLogger() << "QuantSelector:  selection made ";
            if (it != end()) {
                int quant = boost::polymorphic_downcast<const QuantRow*>(*it)->Quant();
                if (amBlockType) {
                    DebugLogger() << "Blocksize Selector:  selection changed to " << quant;
                    blocksize = quant;
                } else {
                    DebugLogger() << "Quantity Selector:  selection changed to " << quant;
                    quantity = quant;
                }
            }
        }

    private:
        const ProductionQueue::Element elem;
        int     quantity;
        int     prevQuant;
        int     blocksize;
        int     prevBlocksize;
        bool    amBlockType;
        bool    amOn;
        GG::Y   h;

        void LosingFocus() override {
            amOn = false;
            DropDownList::LosingFocus();
        }

        void LButtonDown(const GG::Pt&  pt, GG::Flags<GG::ModKey> mod_keys) override {
            amOn = !amOn;
            DropDownList::LButtonDown(pt, mod_keys);
        }

        void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override {
            if (this->Disabled())
                return;

            DropDownList::LClick(pt, mod_keys);
            if ( (quantity != prevQuant) || (blocksize != prevBlocksize) )
                QuantChangedSignal(quantity, blocksize);
        }
    };

    //////////////////////////////////////////////////
    // QueueProductionItemPanel
    //////////////////////////////////////////////////
    class QueueProductionItemPanel : public GG::Control {
    public:
        QueueProductionItemPanel(GG::X x, GG::Y y, GG::X w,
                                 const ProductionQueue::Element& build, double turn_cost, double total_cost,
                                 int turns, int number, double completed_progress);

        void PreRender() override;
        void Render() override;
        void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

        void            UpdateQueue();
        void            ItemQuantityChanged(int quant, int blocksize);
        void            ItemBlocksizeChanged(int quant, int blocksize);

        static GG::Y    DefaultHeight();

        mutable boost::signals2::signal<void(int,int)>    PanelUpdateQuantSignal;

    private:
        void            Draw(GG::Clr clr, bool fill);
        void            DoLayout();
        void            Init();

        const ProductionQueue::Element  elem;
        GG::Label*                      m_name_text;
        GG::Label*                      m_location_text;
        GG::Label*                      m_PPs_and_turns_text;
        GG::Label*                      m_turns_remaining_until_next_complete_text;
        GG::StaticGraphic*              m_icon;
        MultiTurnProgressBar*           m_progress_bar;
        QuantitySelector*               m_quantity_selector;
        QuantitySelector*               m_block_size_selector;
        bool                            m_in_progress;
        int                             m_total_turns;
        double                          m_turn_spending;
        double                          m_total_cost;
        double                          m_completed_progress;
        bool                            m_order_issuing_enabled;
        bool                            m_initialized;
    };

    /////////////////////////////
    // ProductionItemBrowseWnd //
    /////////////////////////////
    std::shared_ptr<GG::BrowseInfoWnd> ProductionItemBrowseWnd(const ProductionQueue::Element& elem) {
        //const Empire* empire = GetEmpire(elem.empire_id);

        std::string main_text;
        std::string item_name;

        //int min_turns = 1;
        float total_cost = 0.0f;
        float max_allocation = 0.0f;
        std::shared_ptr<GG::Texture> icon;
        //bool available = false;
        bool location_ok = false;

        if (elem.item.build_type == BT_BUILDING) {
            const BuildingType* building_type = GetBuildingType(elem.item.name);
            if (!building_type)
                return nullptr;
            main_text += UserString("OBJ_BUILDING") + "\n";

            item_name = UserString(elem.item.name);
            //available = empire->BuildingTypeAvailable(elem.item.name);
            location_ok = building_type->ProductionLocation(elem.empire_id, elem.location);
            //min_turns = building_type->ProductionTime(elem.empire_id, elem.location);
            total_cost = building_type->ProductionCost(elem.empire_id, elem.location);
            max_allocation = building_type->PerTurnCost(elem.empire_id, elem.location);
            icon = ClientUI::BuildingIcon(elem.item.name);

        } else if (elem.item.build_type == BT_SHIP) {
            const ShipDesign* design = GetShipDesign(elem.item.design_id);
            if (!design)
                return nullptr;
            main_text += UserString("OBJ_SHIP") + "\n";

            item_name = design->Name(true);
            //available = empire->ShipDesignAvailable(elem.item.design_id);
            location_ok = design->ProductionLocation(elem.empire_id, elem.location);
            //min_turns = design->ProductionTime(elem.empire_id, elem.location);
            total_cost = design->ProductionCost(elem.empire_id, elem.location) * elem.blocksize;
            max_allocation = design->PerTurnCost(elem.empire_id, elem.location) * elem.blocksize;
            icon = ClientUI::ShipDesignIcon(elem.item.design_id);
        }

        if (std::shared_ptr<UniverseObject> rally_object = GetUniverseObject(elem.rally_point_id)) {
            main_text += boost::io::str(FlexibleFormat(UserString("PRODUCTION_QUEUE_RALLIED_TO"))
                                        % rally_object->Name()) + "\n";
        }

        if (std::shared_ptr<UniverseObject> location = GetUniverseObject(elem.location))
            main_text += boost::io::str(FlexibleFormat(UserString("PRODUCTION_QUEUE_ENQUEUED_ITEM_LOCATION"))
                                        % location->Name()) + "\n";

        if (location_ok)
            main_text += UserString("PRODUCTION_LOCATION_OK") + "\n";
        else
            main_text += UserString("PRODUCTION_LOCATION_INVALID") + "\n";

        float progress = elem.progress;
        float allocation = elem.allocated_pp;

        // %1% / %2%  +  %3% / %4% PP/turn
        main_text += boost::io::str(FlexibleFormat(UserString("PRODUCTION_WND_PROGRESS"))
                        % DoubleToString(progress*100.0f, 3, false)
                        % DoubleToString(total_cost, 3, false)
                        % DoubleToString(allocation, 3, false)
                        % DoubleToString(max_allocation, 3, false)) + "\n";

        int ETA = elem.turns_left_to_completion;
        if (ETA != -1)
            main_text += boost::io::str(FlexibleFormat(UserString("TECH_WND_ETA"))
                                        % ETA);

        std::string title_text;
        if (elem.blocksize > 1)
            title_text = std::to_string(elem.blocksize) + "x ";
        title_text += item_name;

        return std::make_shared<IconTextBrowseWnd>(icon, title_text, main_text);
    }

    //////////////////////////////////////////////////
    // QueueRow
    //////////////////////////////////////////////////
    const int MARGIN = 2;
    const GG::X X_MARGIN = GG::X(MARGIN);
    const GG::Y Y_MARGIN = GG::Y(MARGIN);

    struct QueueRow : GG::ListBox::Row {
        QueueRow(GG::X w, const ProductionQueue::Element& elem, int queue_index_) :
            GG::ListBox::Row(w, QueueProductionItemPanel::DefaultHeight(),
                             BuildDesignatorWnd::PRODUCTION_ITEM_DROP_TYPE),
            panel(nullptr),
            queue_index(queue_index_),
            elem(elem)
        { RequirePreRender(); }

        void Init() {
            const Empire* empire = GetEmpire(HumanClientApp::GetApp()->EmpireID());
            float total_cost(1.0f);
            int minimum_turns(1);
            if (empire)
                std::tie(total_cost, minimum_turns) = empire->ProductionCostAndTime(elem);
            total_cost *= elem.blocksize;

            float pp_accumulated = empire ? empire->ProductionStatus(queue_index) : 0.0f; // returns as PP
            if (pp_accumulated == -1.0f)
                pp_accumulated = 0.0f;

            panel = new QueueProductionItemPanel(GG::X0, GG::Y0, ClientWidth() - MARGIN - MARGIN,
                                                 elem, elem.allocated_pp, total_cost, minimum_turns, elem.remaining,
                                                 pp_accumulated);
            push_back(panel);

            // Since this is only called during PreRender force panel to PreRender()
            GG::GUI::PreRenderWindow(panel);
            GetLayout()->PreRender();

            SetDragDropDataType(BuildDesignatorWnd::PRODUCTION_ITEM_DROP_TYPE);

            SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
            SetBrowseInfoWnd(ProductionItemBrowseWnd(elem));

            GG::Connect(panel->PanelUpdateQuantSignal,  &QueueRow::RowQuantChanged, this);

        }

        void PreRender() override {
            GG::ListBox::Row::PreRender();

            if (!panel)
                Init();
        }

        void Disable(bool b) override {
            GG::ListBox::Row::Disable(b);

            for (GG::Control* ctrl : m_cells) {
                if (ctrl)
                    ctrl->Disable(this->Disabled());
            }
        }

        void RowQuantChanged(int quantity, int blocksize) {
            if (this->Disabled())
                return;
            RowQuantChangedSignal(queue_index, quantity, blocksize);
        }

        QueueProductionItemPanel*                           panel;
        const int                                           queue_index;
        const ProductionQueue::Element                      elem;
        mutable boost::signals2::signal<void (int,int,int)> RowQuantChangedSignal;
    };

    //////////////////////////////////////////////////
    // QueueProductionItemPanel implementation
    //////////////////////////////////////////////////
    QueueProductionItemPanel::QueueProductionItemPanel(GG::X x, GG::Y y, GG::X w, const ProductionQueue::Element& build,
                                                       double turn_spending, double total_cost, int turns, int number,
                                                       double completed_progress) :
        GG::Control(x, y, w, DefaultHeight(), GG::NO_WND_FLAGS),
        elem(build),
        m_name_text(nullptr),
        m_location_text(nullptr),
        m_PPs_and_turns_text(nullptr),
        m_turns_remaining_until_next_complete_text(nullptr),
        m_icon(nullptr),
        m_progress_bar(nullptr),
        m_quantity_selector(nullptr),
        m_block_size_selector(nullptr),
        m_in_progress(build.allocated_pp || build.turns_left_to_next_item == 1),
        m_total_turns(turns),
        m_turn_spending(turn_spending),
        m_total_cost(total_cost),
        m_completed_progress(completed_progress),
        m_initialized(false)
    {
        SetChildClippingMode(ClipToClient);
        RequirePreRender();
    }

    GG::Y QueueProductionItemPanel::DefaultHeight() {
        const int FONT_PTS = ClientUI::Pts();
        const GG::Y HEIGHT = Y_MARGIN + FONT_PTS + Y_MARGIN + FONT_PTS + Y_MARGIN + FONT_PTS + Y_MARGIN + 6;
        return HEIGHT;
    }

    void QueueProductionItemPanel::Init() {
        m_initialized = true;

        GG::Clr clr = m_in_progress
            ? GG::LightColor(ClientUI::ResearchableTechTextAndBorderColor())
            : ClientUI::ResearchableTechTextAndBorderColor();

        // get graphic and player-visible name text for item
        std::shared_ptr<GG::Texture> graphic;
        std::string name_text;
        if (elem.item.build_type == BT_BUILDING) {
            graphic = ClientUI::BuildingIcon(elem.item.name);
            name_text = UserString(elem.item.name);
        } else if (elem.item.build_type == BT_SHIP) {
            graphic = ClientUI::ShipDesignIcon(elem.item.design_id);
            const ShipDesign* design = GetShipDesign(elem.item.design_id);
            if (design)
                name_text = design->Name();
            else
                ErrorLogger() << "QueueProductionItemPanel unable to get design with id: " << elem.item.design_id;
        } else {
            graphic = ClientUI::GetTexture(""); // get "missing texture" texture by supply intentionally bad path
            name_text = UserString("FW_UNKNOWN_DESIGN_NAME");
        }

        // get location indicator text
        std::string location_text;
        bool system_selected = false;
        if (std::shared_ptr<const UniverseObject> location = GetUniverseObject(elem.location)) {
            system_selected = (location->SystemID() != -1 && location ->SystemID() == SidePanel::SystemID());
            if (GetOptionsDB().Get<bool>("UI.show-production-location-on-queue"))
                location_text = boost::io::str(FlexibleFormat(UserString("PRODUCTION_QUEUE_ITEM_LOCATION")) % location->Name()) + " ";
        }

        const int FONT_PTS = ClientUI::Pts();

        m_icon = nullptr;
        if (graphic)
            m_icon = new GG::StaticGraphic(graphic, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);

        if (elem.item.build_type == BT_SHIP) {
            m_quantity_selector = new QuantitySelector(elem, GG::X1, GG::Y(MARGIN), GG::Y(FONT_PTS-2*MARGIN),
                                                       m_in_progress, GG::X(FONT_PTS*2.5), false);
            m_block_size_selector = new QuantitySelector(elem, GG::X1, GG::Y(MARGIN), GG::Y(FONT_PTS-2*MARGIN),
                                                         m_in_progress, GG::X(FONT_PTS*2.5), true);
        }

        m_name_text = new CUILabel(name_text, GG::FORMAT_TOP | GG::FORMAT_LEFT);
        m_name_text->SetTextColor(clr);
        m_name_text->ClipText(true);

        GG::Clr location_clr = clr;
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();
        const Empire* this_client_empire = GetEmpire(client_empire_id);
        if (this_client_empire && system_selected) {
            m_location_text = new GG::TextControl(GG::X0, GG::Y0, GG::X1, GG::Y1, "<s>" + location_text + "</s>",
                                                  ClientUI::GetBoldFont(), this_client_empire->Color(), GG::FORMAT_TOP | GG::FORMAT_RIGHT);
        } else {
            m_location_text = new CUILabel(location_text, GG::FORMAT_TOP | GG::FORMAT_RIGHT);
            m_location_text->SetTextColor(location_clr);
        }

        double perc_complete = 1.0;
        double next_progress = 0.0;
        if (m_total_cost > 0.0) {
            perc_complete = m_completed_progress / m_total_cost;
            next_progress = m_turn_spending / std::max(m_turn_spending, m_total_cost);
        }

        GG::Clr outline_color = ClientUI::ResearchableTechFillColor();
        if (m_in_progress)
            outline_color = GG::LightColor(outline_color);

        m_progress_bar = new MultiTurnProgressBar(m_total_turns,
                                                  perc_complete,
                                                  next_progress,
                                                  GG::LightColor(ClientUI::TechWndProgressBarBackgroundColor()),
                                                  ClientUI::TechWndProgressBarColor(),
                                                  outline_color);

        double max_spending_per_turn = m_total_cost / m_total_turns;
        std::string turn_spending_text = boost::io::str(FlexibleFormat(UserString("PRODUCTION_TURN_COST_STR"))
            % DoubleToString(m_turn_spending, 3, false)
            % DoubleToString(max_spending_per_turn, 3, false));
        m_PPs_and_turns_text = new CUILabel(turn_spending_text, GG::FORMAT_LEFT);
        m_PPs_and_turns_text->SetTextColor(clr);


        int turns_left = elem.turns_left_to_next_item;
        std::string turns_left_text = turns_left < 0
            ? UserString("PRODUCTION_TURNS_LEFT_NEVER")
            : str(FlexibleFormat(UserString("PRODUCTION_TURNS_LEFT_STR")) % turns_left);
        m_turns_remaining_until_next_complete_text = new CUILabel(turns_left_text, GG::FORMAT_RIGHT);
        m_turns_remaining_until_next_complete_text->SetTextColor(clr);
        m_turns_remaining_until_next_complete_text->ClipText(true);

        if (m_icon)
            AttachChild(m_icon);
        if (m_quantity_selector)
            AttachChild(m_quantity_selector);
        if (m_block_size_selector)
            AttachChild(m_block_size_selector);
        AttachChild(m_name_text);
        AttachChild(m_location_text);
        AttachChild(m_PPs_and_turns_text);
        AttachChild(m_turns_remaining_until_next_complete_text);
        AttachChild(m_progress_bar);
        if (m_quantity_selector)
            GG::Connect(m_quantity_selector->QuantChangedSignal,    &QueueProductionItemPanel::ItemQuantityChanged, this);
        if (m_block_size_selector)
            GG::Connect(m_block_size_selector->QuantChangedSignal,  &QueueProductionItemPanel::ItemBlocksizeChanged, this);

        // Since this is only called during PreRender force quantity indicators to PreRender()
        GG::GUI::PreRenderWindow(m_quantity_selector);
        GG::GUI::PreRenderWindow(m_block_size_selector);
    }

    void QueueProductionItemPanel::PreRender() {
        if (!m_initialized)
            Init();

        GG::Wnd::PreRender();
        DoLayout();
    }

    void QueueProductionItemPanel::DoLayout() {
        const int FONT_PTS = ClientUI::Pts();
        const GG::Y METER_HEIGHT(FONT_PTS);
        const GG::Y HEIGHT = GG::Y(MARGIN) + FONT_PTS + MARGIN + METER_HEIGHT + MARGIN + FONT_PTS + MARGIN + 6;
        const int GRAPHIC_SIZE = Value(HEIGHT - 9);    // 9 pixels accounts for border thickness so the sharp-cornered icon doesn't with the rounded panel corner
        const GG::X METER_WIDTH = Width() - GRAPHIC_SIZE - 3*MARGIN - 3;

        // create and arrange widgets to display info
        GG::Y top(MARGIN);
        GG::X left(MARGIN);

        if (m_icon) {
            m_icon->MoveTo(GG::Pt(left, top));
            m_icon->Resize(GG::Pt(GG::X(GRAPHIC_SIZE), GG::Y(GRAPHIC_SIZE)));
        }

        left += GRAPHIC_SIZE + MARGIN;
        if (m_quantity_selector) {
            m_quantity_selector->MoveTo(GG::Pt(left, GG::Y(MARGIN)));
            left += m_quantity_selector->Width();
        }
        if (m_block_size_selector) {
            m_block_size_selector->MoveTo(GG::Pt(left, GG::Y(MARGIN)));
            left += m_block_size_selector->Width();
        }

        const GG::X NAME_WIDTH = Width() - left - MARGIN - 3;
        m_name_text->MoveTo(GG::Pt(left, top));
        m_name_text->Resize(GG::Pt(NAME_WIDTH, GG::Y(FONT_PTS + 2*MARGIN)));
        m_name_text->SetChildClippingMode(ClipToClient);

        m_location_text->MoveTo(GG::Pt(left, top));
        m_location_text->Resize(GG::Pt(NAME_WIDTH, GG::Y(FONT_PTS + 2*MARGIN)));

        top += m_name_text->Height();
        left = GG::X(GRAPHIC_SIZE + MARGIN*2);
        m_progress_bar->MoveTo(GG::Pt(left, top));
        m_progress_bar->Resize(GG::Pt(METER_WIDTH, METER_HEIGHT));

        top += m_progress_bar->Height() + MARGIN;

        GG::X TURNS_AND_COST_WIDTH = METER_WIDTH / 2;
        m_PPs_and_turns_text->MoveTo(GG::Pt(left, top));
        m_PPs_and_turns_text->Resize(GG::Pt(TURNS_AND_COST_WIDTH, GG::Y(FONT_PTS + MARGIN)));

        left += TURNS_AND_COST_WIDTH;

        m_turns_remaining_until_next_complete_text->MoveTo(GG::Pt(left, top));
        m_turns_remaining_until_next_complete_text->Resize(GG::Pt(TURNS_AND_COST_WIDTH, GG::Y(FONT_PTS + MARGIN)));
    }

    void QueueProductionItemPanel::ItemQuantityChanged(int quant, int blocksize)
    { PanelUpdateQuantSignal(quant,elem.blocksize); }

    void QueueProductionItemPanel::ItemBlocksizeChanged(int quant, int blocksize) // made separate funcion in case wna to do extra checking
    { PanelUpdateQuantSignal(elem.remaining, blocksize); }

    void QueueProductionItemPanel::Render() {
        GG::Clr fill = m_in_progress
            ? GG::LightColor(ClientUI::ResearchableTechFillColor())
            : ClientUI::ResearchableTechFillColor();
        GG::Clr text_and_border = m_in_progress
            ? GG::LightColor(ClientUI::ResearchableTechTextAndBorderColor())
            : ClientUI::ResearchableTechTextAndBorderColor();

        glDisable(GL_TEXTURE_2D);
        Draw(fill, true);
        glEnable(GL_LINE_SMOOTH);
        glLineWidth(static_cast<GLfloat>(OUTER_LINE_THICKNESS));
        Draw(GG::Clr(text_and_border.r, text_and_border.g, text_and_border.b, 127), false);
        glLineWidth(1.0);
        glDisable(GL_LINE_SMOOTH);
        Draw(GG::Clr(text_and_border.r, text_and_border.g, text_and_border.b, 255), false);
        glEnable(GL_TEXTURE_2D);
    }

    void QueueProductionItemPanel::Draw(GG::Clr clr, bool fill) {
        const int CORNER_RADIUS = 7;
        glColor(clr);
        GG::Pt LINE_WIDTH(GG::X(3), GG::Y0);
        PartlyRoundedRect(UpperLeft(), LowerRight() - LINE_WIDTH, CORNER_RADIUS, true, false, true, false, fill);
    }

    void QueueProductionItemPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
        const GG::Pt old_size = Size();
        GG::Control::SizeMove(ul, lr);
        if (Size() != old_size)
            RequirePreRender();
    }

    class ProdQueueListBox : public QueueListBox {
    public:
        ProdQueueListBox() :
            QueueListBox(BuildDesignatorWnd::PRODUCTION_ITEM_DROP_TYPE,  UserString("PRODUCTION_QUEUE_PROMPT"))
        {}

        boost::signals2::signal<void (GG::ListBox::iterator, int)>  QueueItemRalliedToSignal;
        boost::signals2::signal<void ()>                            ShowPediaSignal;
        boost::signals2::signal<void (GG::ListBox::iterator, bool)> QueueItemPausedSignal;

    protected:
        void ItemRightClickedImpl(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) override {
            // mostly duplicated equivalent in QueueListBox, but with extra commands...

            GG::MenuItem menu_contents;
            menu_contents.next_level.push_back(GG::MenuItem(UserString("MOVE_UP_QUEUE_ITEM"),   1, false, false));
            menu_contents.next_level.push_back(GG::MenuItem(UserString("MOVE_DOWN_QUEUE_ITEM"), 2, false, false));
            menu_contents.next_level.push_back(GG::MenuItem(UserString("DELETE_QUEUE_ITEM"),    3, false, false));

            // inspect clicked item: was it a ship?
            GG::ListBox::Row* row = *it;
            QueueRow* queue_row = row ? dynamic_cast<QueueRow*>(row) : nullptr;
            BuildType build_type = queue_row ? queue_row->elem.item.build_type : INVALID_BUILD_TYPE;
            if (build_type == BT_SHIP) {
                // for ships, add a set rally point command
                if (std::shared_ptr<const System> system = GetSystem(SidePanel::SystemID())) {
                    std::string rally_prompt = boost::io::str(FlexibleFormat(UserString("RALLY_QUEUE_ITEM")) % system->PublicName(HumanClientApp::GetApp()->EmpireID()));
                    menu_contents.next_level.push_back(GG::MenuItem(rally_prompt,               4, false, false));
                }
            }

            // pause / resume commands
            if (queue_row && queue_row->elem.paused) {
                menu_contents.next_level.push_back(GG::MenuItem(UserString("RESUME"),           7, false, false));
            } else {
                menu_contents.next_level.push_back(GG::MenuItem(UserString("PAUSE"),            8, false, false));
            }

            // pedia lookup
            std::string item_name = "";
            if (build_type == BT_BUILDING) {
                item_name = queue_row->elem.item.name;
            } else if (build_type == BT_SHIP) {
                item_name = GetShipDesign(queue_row->elem.item.design_id)->Name(false);
            } else {
                ErrorLogger() << "Invalid build type (" << build_type << ") for row item";
                return;
            }
            if (UserStringExists(item_name))
                item_name = UserString(item_name);
            std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % item_name);
            menu_contents.next_level.push_back(GG::MenuItem(popup_label, 5, false, false));


            CUIPopupMenu popup(pt.x, pt.y, menu_contents);

            if (popup.Run()) {
                switch (popup.MenuID()) {
                case 1: { // move item to top
                    if (GG::ListBox::Row* row = *it)
                        this->QueueItemMovedSignal(row, 0);
                    break;
                }
                case 2: { // move item to bottom
                    if (GG::ListBox::Row* row = *it)
                        this->QueueItemMovedSignal(row, NumRows());
                    break;
                }
                case 3: { // delete item
                    this->QueueItemDeletedSignal(it);
                    break;
                }

                case 4: { // rally to
                    this->QueueItemRalliedToSignal(it, SidePanel::SystemID());
                    break;
                }
                case 5: { // pedia lookup
                    ShowPediaSignal();
                    this->LeftClickedSignal(it, pt, modkeys);
                    break;
                }

                case 7: { // resume
                    this->QueueItemPausedSignal(it, false);
                    break;
                }
                case 8: { // pause
                    this->QueueItemPausedSignal(it, true);
                    break;
                }

                default:
                    break;
                }
            }
        }
    };
}

//////////////////////////////////////////////////
// ProductionQueueWnd                           //
//////////////////////////////////////////////////
class ProductionQueueWnd : public CUIWnd {
public:
    /** \name Structors */ //@{
    ProductionQueueWnd(GG::X x, GG::Y y, GG::X w, GG::Y h) :
        CUIWnd("", x, y, w, h, GG::INTERACTIVE | GG::RESIZABLE | GG::DRAGABLE | GG::ONTOP | PINABLE,
               "production.ProductionQueueWnd"),
        m_queue_lb(nullptr)
    {
        Init(HumanClientApp::GetApp()->EmpireID());
    }
    //@}

    /** \name Mutators */ //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
        GG::Pt sz = Size();
        CUIWnd::SizeMove(ul, lr);
        if (Size() != sz)
            DoLayout();
    }

    ProdQueueListBox*   GetQueueListBox() { return m_queue_lb; }

    void                SetEmpire(int id) {
        if (const Empire* empire = GetEmpire(id))
            SetName(boost::io::str(FlexibleFormat(UserString("PRODUCTION_QUEUE_EMPIRE")) % empire->Name()));
        else
            SetName("");
    }
    //@}

private:
    void DoLayout() {
        m_queue_lb->SizeMove(GG::Pt(GG::X0, GG::Y0),
                             GG::Pt(ClientWidth(), ClientHeight() - GG::Y(CUIWnd::INNER_BORDER_ANGLE_OFFSET)));
    }

    void Init(int empire_id) {
        m_queue_lb = new ProdQueueListBox();
        m_queue_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_NOSEL | GG::LIST_USERDELETE);
        m_queue_lb->SetName("ProductionQueue ListBox");

        SetEmpire(empire_id);

        AttachChild(m_queue_lb);
        DoLayout();
    }

    ProdQueueListBox*   m_queue_lb;
};

//////////////////////////////////////////////////
// ProductionWnd                                //
//////////////////////////////////////////////////
ProductionWnd::ProductionWnd(GG::X w, GG::Y h) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::INTERACTIVE | GG::ONTOP),
    m_production_info_panel(nullptr),
    m_queue_wnd(nullptr),
    m_build_designator_wnd(nullptr),
    m_order_issuing_enabled(false),
    m_empire_shown_id(ALL_EMPIRES)
{
    //DebugLogger() << "ProductionWindow:  app-width: "<< GetOptionsDB().Get<int>("app-width")
    //              << " ; windowed width: " << GetOptionsDB().Get<int>("app-width-windowed");

    GG::X queue_width(GetOptionsDB().Get<int>("UI.queue-width"));
    GG::Y info_height(ClientUI::Pts()*8);

    m_production_info_panel = new ProductionInfoPanel(UserString("PRODUCTION_WND_TITLE"), UserString("PRODUCTION_INFO_PP"),
                                                      GG::X0, GG::Y0, queue_width, info_height,
                                                      "production.InfoPanel");
    m_queue_wnd = new ProductionQueueWnd(GG::X0, info_height, queue_width, ClientSize().y - info_height);
    m_build_designator_wnd = new BuildDesignatorWnd(ClientSize().x, ClientSize().y);

    SetChildClippingMode(ClipToClient);

    GG::Connect(m_build_designator_wnd->AddBuildToQueueSignal,              &ProductionWnd::AddBuildToQueueSlot, this);
    GG::Connect(m_build_designator_wnd->BuildQuantityChangedSignal,         &ProductionWnd::ChangeBuildQuantitySlot, this);
    GG::Connect(m_build_designator_wnd->SystemSelectedSignal,               SystemSelectedSignal);
    GG::Connect(m_queue_wnd->GetQueueListBox()->QueueItemMovedSignal,       &ProductionWnd::QueueItemMoved, this);
    GG::Connect(m_queue_wnd->GetQueueListBox()->QueueItemDeletedSignal,     &ProductionWnd::DeleteQueueItem, this);
    GG::Connect(m_queue_wnd->GetQueueListBox()->LeftClickedSignal,          &ProductionWnd::QueueItemClickedSlot, this);
    GG::Connect(m_queue_wnd->GetQueueListBox()->DoubleClickedSignal,        &ProductionWnd::QueueItemDoubleClickedSlot, this);
    GG::Connect(m_queue_wnd->GetQueueListBox()->QueueItemRalliedToSignal,   &ProductionWnd::QueueItemRallied, this);
    GG::Connect(m_queue_wnd->GetQueueListBox()->ShowPediaSignal,            &ProductionWnd::ShowPedia, this);
    GG::Connect(m_queue_wnd->GetQueueListBox()->QueueItemPausedSignal,      &ProductionWnd::QueueItemPaused, this);

    AttachChild(m_production_info_panel);
    AttachChild(m_queue_wnd);
    AttachChild(m_build_designator_wnd);
}

ProductionWnd::~ProductionWnd()
{ m_empire_connection.disconnect(); }

int ProductionWnd::SelectedPlanetID() const
{ return m_build_designator_wnd->SelectedPlanetID(); }

bool ProductionWnd::InWindow(const GG::Pt& pt) const
{ return m_production_info_panel->InWindow(pt) || m_queue_wnd->InWindow(pt) || m_build_designator_wnd->InWindow(pt); }

bool ProductionWnd::InClient(const GG::Pt& pt) const
{ return m_production_info_panel->InClient(pt) || m_queue_wnd->InClient(pt) || m_build_designator_wnd->InClient(pt); }

void ProductionWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    GG::Wnd::SizeMove(ul, lr);
    if (old_size != Size())
        DoLayout();
}

void ProductionWnd::DoLayout() {
    GG::X queue_width(GetOptionsDB().Get<int>("UI.queue-width"));
    GG::Y info_height(ClientUI::Pts()*6 + 34);

    m_production_info_panel->MoveTo(GG::Pt(GG::X0, GG::Y0));
    m_production_info_panel->Resize(GG::Pt(queue_width, info_height));

    m_queue_wnd->MoveTo(GG::Pt(GG::X0, info_height));
    m_queue_wnd->Resize(GG::Pt(queue_width, ClientSize().y - info_height));

    m_build_designator_wnd->Resize(ClientSize());
}

void ProductionWnd::Render()
{}

void ProductionWnd::SetEmpireShown(int empire_id) {
    if (empire_id != m_empire_shown_id) {
        m_empire_shown_id = empire_id;
        Refresh();
    }
}

void ProductionWnd::Refresh() {
    // useful at start of turn or when loading empire from save, or when
    // the selected empire shown has changed.
    // because empire object is recreated based on turn update from server,
    // connections of signals emitted from the empire must be remade after
    // getting a turn update
    m_empire_connection.disconnect();
    if (Empire* empire = GetEmpire(m_empire_shown_id))
        m_empire_connection = GG::Connect(empire->GetProductionQueue().ProductionQueueChangedSignal,
                                          &ProductionWnd::ProductionQueueChangedSlot, this);

    UpdateInfoPanel();
    UpdateQueue();

    m_build_designator_wnd->Refresh();
}

void ProductionWnd::Reset() {
    m_empire_shown_id = ALL_EMPIRES;
    Refresh();
    m_queue_wnd->GetQueueListBox()->BringRowIntoView(m_queue_wnd->GetQueueListBox()->begin());
}

void ProductionWnd::Update() {
    // useful when empire hasn't changed, but production status of it might have
    UpdateInfoPanel();
    UpdateQueue();

    m_build_designator_wnd->Update();
}

void ProductionWnd::ShowBuildingTypeInEncyclopedia(const std::string& building_type)
{ m_build_designator_wnd->ShowBuildingTypeInEncyclopedia(building_type); }

void ProductionWnd::ShowShipDesignInEncyclopedia(int design_id)
{ m_build_designator_wnd->ShowShipDesignInEncyclopedia(design_id); }

void ProductionWnd::ShowPlanetInEncyclopedia(int planet_id)
{ m_build_designator_wnd->ShowPlanetInEncyclopedia(planet_id); }

void ProductionWnd::ShowTechInEncyclopedia(const std::string& tech_name)
{ m_build_designator_wnd->ShowTechInEncyclopedia(tech_name); }

void ProductionWnd::ShowPartTypeInEncyclopedia(const std::string& part_type_name)
{ m_build_designator_wnd->ShowPartTypeInEncyclopedia(part_type_name); }

void ProductionWnd::ShowSpeciesInEncyclopedia(const std::string& species_name)
{ m_build_designator_wnd->ShowSpeciesInEncyclopedia(species_name); }

void ProductionWnd::ShowEmpireInEncyclopedia(int empire_id)
{ m_build_designator_wnd->ShowEmpireInEncyclopedia(empire_id); }

void ProductionWnd::ShowSpecialInEncyclopedia(const std::string& special_name)
{ m_build_designator_wnd->ShowSpecialInEncyclopedia(special_name); }

void ProductionWnd::ShowFieldTypeInEncyclopedia(const std::string& field_type_name)
{ m_build_designator_wnd->ShowFieldTypeInEncyclopedia(field_type_name); }

void ProductionWnd::ShowPedia()
{ m_build_designator_wnd->ShowPedia(); }

void ProductionWnd::HidePedia()
{ m_build_designator_wnd->HidePedia(); }

void ProductionWnd::TogglePedia()
{ m_build_designator_wnd->TogglePedia(); }

bool ProductionWnd::PediaVisible()
{ return m_build_designator_wnd->PediaVisible(); }

void ProductionWnd::CenterOnBuild(int queue_idx, bool open)
{ m_build_designator_wnd->CenterOnBuild(queue_idx, open); }

void ProductionWnd::SelectPlanet(int planet_id) {
    m_build_designator_wnd->SelectPlanet(planet_id);
    UpdateInfoPanel();
}

void ProductionWnd::SelectDefaultPlanet()
{ m_build_designator_wnd->SelectDefaultPlanet(); }

void ProductionWnd::SelectSystem(int system_id) { 
    if (system_id != SidePanel::SystemID()) {
        m_build_designator_wnd->SelectSystem(system_id); 
        // refresh so as to correctly highlight builds for selected system
        Update();
    }
}

void ProductionWnd::QueueItemMoved(GG::ListBox::Row* row, std::size_t position) {
    if (!m_order_issuing_enabled)
        return;
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    Empire* empire = GetEmpire(client_empire_id);
    if (!empire)
        return;

    HumanClientApp::GetApp()->Orders().IssueOrder(
        OrderPtr(new ProductionQueueOrder(client_empire_id,
                                          boost::polymorphic_downcast<QueueRow*>(row)->queue_index,
                                          position)));

    empire->UpdateProductionQueue();
}

void ProductionWnd::Sanitize()
{ m_build_designator_wnd->Clear(); }

void ProductionWnd::ProductionQueueChangedSlot() {
    UpdateInfoPanel();
    UpdateQueue();
    m_build_designator_wnd->Update();
}

void ProductionWnd::UpdateQueue() {
    DebugLogger() << "ProductionWnd::UpdateQueue()";
    ScopedTimer timer("ProductionWnd::UpdateQueue");

    m_queue_wnd->SetEmpire(m_empire_shown_id);
    QueueListBox* queue_lb = m_queue_wnd->GetQueueListBox();

    // Capture the list scroll state
    // Try to preserve the same queue context with completely new queue items
    std::size_t initial_offset_from_begin = std::distance(queue_lb->begin(), queue_lb->FirstRowShown());
    std::size_t initial_offset_to_end = std::distance(queue_lb->FirstRowShown(), queue_lb->end());
    bool initial_last_visible_row_is_end(queue_lb->LastVisibleRow() == queue_lb->end());

    queue_lb->Clear();

    const Empire* empire = GetEmpire(m_empire_shown_id);
    if (!empire)
        return;

    int i = 0;
    for (const ProductionQueue::Element& elem : empire->GetProductionQueue()) {
        QueueRow* row = new QueueRow(queue_lb->RowWidth(), elem, i);
        GG::Connect(row->RowQuantChangedSignal,     &ProductionWnd::ChangeBuildQuantityBlockSlot, this);
        queue_lb->Insert(row);
        ++i;
    }

    // Restore the list scroll state
    // If we were at the top stay at the top
    if (initial_offset_from_begin == 0)
        queue_lb->SetFirstRowShown(queue_lb->begin());

    // If we were not at the bottom then keep the same first row position
    else if (!initial_last_visible_row_is_end && initial_offset_from_begin < queue_lb->NumRows())
        queue_lb->SetFirstRowShown(std::next(queue_lb->begin(), initial_offset_from_begin));

    // otherwise keep the same relative position from the bottom to
    // preserve the end of list dead space
    else if (initial_offset_to_end < queue_lb->NumRows())
        queue_lb->SetFirstRowShown(std::next(queue_lb->begin(), queue_lb->NumRows() - initial_offset_to_end));
    else
        queue_lb->SetFirstRowShown(queue_lb->begin());
}

void ProductionWnd::UpdateInfoPanel() {
    const Empire* empire = GetEmpire(m_empire_shown_id);
    if (!empire) {
        m_production_info_panel->SetName(UserString("PRODUCTION_WND_TITLE"));
        m_production_info_panel->ClearLocalInfo();
        return;
    } else {
        m_production_info_panel->SetEmpireID(m_empire_shown_id);
    }

    const ProductionQueue& queue = empire->GetProductionQueue();
    float PPs = empire->ProductionPoints();
    float total_queue_cost = queue.TotalPPsSpent();
    m_production_info_panel->SetTotalPointsCost(PPs, total_queue_cost);

    // find if there is a local location
    int prod_loc_id = this->SelectedPlanetID();
    std::shared_ptr<UniverseObject> loc_obj = GetUniverseObject(prod_loc_id);
    if (loc_obj) {
        // extract available and allocated PP at production location
        std::map<std::set<int>, float> available_pp = queue.AvailablePP(empire->GetResourcePool(RE_INDUSTRY));
        const std::map<std::set<int>, float>& allocated_pp = queue.AllocatedPP();

        float available_pp_at_loc = 0.0f, allocated_pp_at_loc = 0.0f;   // for the resource sharing group containing the selected production location

        for (const std::map<std::set<int>, float>::value_type& map : available_pp) {
            if (map.first.find(prod_loc_id) != map.first.end()) {
                available_pp_at_loc = map.second;
                break;
            }
        }

        for (const std::map<std::set<int>, float>::value_type& map : allocated_pp) {
            if (map.first.find(prod_loc_id) != map.first.end()) {
                allocated_pp_at_loc = map.second;
                break;
            }
        }

        m_production_info_panel->SetLocalPointsCost(available_pp_at_loc, allocated_pp_at_loc, loc_obj->Name());
    } else {
        // else clear local info...
        m_production_info_panel->ClearLocalInfo();
    }
}

void ProductionWnd::AddBuildToQueueSlot(const ProductionQueue::ProductionItem& item, int number, int location, int pos) {
    if (!m_order_issuing_enabled)
        return;
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    Empire* empire = GetEmpire(client_empire_id);
    if (!empire)
        return;

    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(
        new ProductionQueueOrder(client_empire_id, item, number, location, pos)));

    empire->UpdateProductionQueue();
    m_build_designator_wnd->CenterOnBuild(pos >= 0 ? pos : m_queue_wnd->GetQueueListBox()->NumRows() - 1);
}

void ProductionWnd::ChangeBuildQuantitySlot(int queue_idx, int quantity) {
    if (!m_order_issuing_enabled)
        return;
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    Empire* empire = GetEmpire(client_empire_id);
    if (!empire)
        return;

    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(
        new ProductionQueueOrder(client_empire_id, queue_idx, quantity, true)));

    empire->UpdateProductionQueue();
}

void ProductionWnd::ChangeBuildQuantityBlockSlot(int queue_idx, int quantity, int blocksize) {
    if (!m_order_issuing_enabled)
        return;
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    Empire* empire = GetEmpire(client_empire_id);
    if (!empire)
        return;

    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(
        new ProductionQueueOrder(client_empire_id, queue_idx, quantity, blocksize)));

    empire->UpdateProductionQueue();
}

void ProductionWnd::DeleteQueueItem(GG::ListBox::iterator it) {
    if (!m_order_issuing_enabled)
        return;
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    Empire* empire = GetEmpire(client_empire_id);
    if (!empire)
        return;

    HumanClientApp::GetApp()->Orders().IssueOrder(
        OrderPtr(new ProductionQueueOrder(client_empire_id, std::distance(m_queue_wnd->GetQueueListBox()->begin(), it))));

    empire->UpdateProductionQueue();
}

void ProductionWnd::QueueItemClickedSlot(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) {
    if (m_queue_wnd->GetQueueListBox()->DisplayingValidQueueItems()) {
        if (modkeys & GG::MOD_KEY_CTRL)
            DeleteQueueItem(it);
        else
            m_build_designator_wnd->CenterOnBuild(std::distance(m_queue_wnd->GetQueueListBox()->begin(), it));
    }
}

void ProductionWnd::QueueItemDoubleClickedSlot(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) {
    if (m_queue_wnd->GetQueueListBox()->DisplayingValidQueueItems()) {
        m_build_designator_wnd->CenterOnBuild(std::distance(m_queue_wnd->GetQueueListBox()->begin(), it), true);
    }
}

void ProductionWnd::QueueItemRallied(GG::ListBox::iterator it, int object_id) {
    if (!m_order_issuing_enabled)
        return;
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    Empire* empire = GetEmpire(client_empire_id);
    if (!empire)
        return;

    int rally_point_id = object_id;
    if (rally_point_id == INVALID_OBJECT_ID) {
        // get rally point from selected system
        rally_point_id = SidePanel::SystemID();
    }
    if (rally_point_id == INVALID_OBJECT_ID)
        return;

    HumanClientApp::GetApp()->Orders().IssueOrder(
        OrderPtr(new ProductionQueueOrder(client_empire_id, std::distance(m_queue_wnd->GetQueueListBox()->begin(), it),
                                          rally_point_id, false, false)));

    empire->UpdateProductionQueue();
}

void ProductionWnd::QueueItemPaused(GG::ListBox::iterator it, bool pause) {
    if (!m_order_issuing_enabled)
        return;
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    Empire* empire = GetEmpire(client_empire_id);
    if (!empire)
        return;

    HumanClientApp::GetApp()->Orders().IssueOrder(
        OrderPtr(new ProductionQueueOrder(client_empire_id, std::distance(m_queue_wnd->GetQueueListBox()->begin(), it),
                                          pause, -1.0f)));

    empire->UpdateProductionQueue();
}

void ProductionWnd::EnableOrderIssuing(bool enable/* = true*/) {
    m_order_issuing_enabled = enable;
    m_queue_wnd->GetQueueListBox()->EnableOrderIssuing(m_order_issuing_enabled);
}
