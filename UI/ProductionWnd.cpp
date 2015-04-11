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
#include "../universe/Building.h"
#include "../universe/ShipDesign.h"

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>

#include <boost/cast.hpp>

#include <cmath>


namespace {
    const float OUTER_LINE_THICKNESS = 2.0f;

    void AddOptions(OptionsDB& db) {
        // queue width used also on research screen. prevent double-adding...
        if (!db.OptionExists("UI.queue-width"))
            db.Add("UI.queue-width",                    UserStringNop("OPTIONS_DB_UI_QUEUE_WIDTH"),         300,    RangedValidator<int>(200, 500));
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
            CUILabel* text = new CUILabel(nameText, GG::FORMAT_TOP | GG::FORMAT_LEFT | GG::FORMAT_NOWRAP);
            text->SetTextColor(txtClr);
            text->OffsetMove(GG::Pt(GG::X0, GG::Y(-3))); //
            AttachChild(text);
            Resize(GG::Pt(nwidth, text->Height()));
        }

        void Render() {}
    };

    //////////////
    // QuantRow //
    //////////////
    class QuantRow : public GG::ListBox::Row {
    public:
        QuantRow(int quantity, int designID, GG::X nwidth, GG::Y h,
                 bool inProgress, bool amBlockType) :
            GG::ListBox::Row(),
            width(0),
            m_quant(quantity)
        {
            QuantLabel* newLabel = new QuantLabel(m_quant, designID, nwidth, h, inProgress, amBlockType);
            width = newLabel->Width();
            height = newLabel->Height();
            push_back(newLabel);
            Resize(GG::Pt(nwidth, height-GG::Y0));//might subtract more; assessing aesthetics
            //OffsetMove(GG::Pt(GG::X0, GG::Y(-2))); // didn't appear to have an effect
        }

        GG::X width;
        GG::Y height;

        int Quant() const { return m_quant; }

    private:
        int m_quant;
    };

    //////////////////////
    // QuantitySelector //
    //////////////////////
    class QuantitySelector : public CUIDropDownList {
    public:
        mutable boost::signals2::signal<void (int,int)> QuantChangedSignal;

        /** \name Structors */
        QuantitySelector(const ProductionQueue::Element &build, GG::X xoffset, GG::Y yoffset,
                         GG::Y h, bool inProgress, GG::X nwidth, bool amBlockType) :
            CUIDropDownList(6),
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
            GG::X width = GG::X0;
            DisableDropArrow();
            SetStyle(GG::LIST_LEFT | GG::LIST_NOSORT);
            SetColor(
                inProgress
                    ? GG::LightColor(ClientUI::ResearchableTechTextAndBorderColor())
                    : ClientUI::ResearchableTechTextAndBorderColor()
            );
            SetInteriorColor(
                inProgress
                    ? GG::LightColor(ClientUI::ResearchableTechFillColor())
                    : ClientUI::ResearchableTechFillColor()
            );
            SetNumCols(1);
            //m_quantity_selector->SetColWidth(0, GG::X(14));
            //m_quantity_selector->LockColWidths();

            int quantInts[] = {1, 2, 3, 4, 5, 10, 20, 50, 99};
            std::set<int> myQuantSet(quantInts, quantInts+9);
            if (amBlockType)
                myQuantSet.insert(blocksize); //as currently implemented this one not actually necessary since blocksize has no other way to change
            else
                myQuantSet.insert(quantity);
            GG::Y height;
            for (std::set<int>::iterator it=myQuantSet.begin(); it != myQuantSet.end(); it++ ) {
                QuantRow* newRow =  new QuantRow(*it, build.item.design_id, nwidth, h, inProgress, amBlockType);
                if (newRow->width)
                    width = newRow->width;
                GG::DropDownList::iterator latest_it = Insert(newRow);
                if (amBlockType)
                    if (build.blocksize == *it)
                        Select(latest_it);
                    else {}
                else
                    if (build.remaining == *it)
                        Select(latest_it);
                height = newRow->height;
                //Resize(GG::Pt(width, height)); //doesn't work on DropDownList itself, goes by Row
            }
            // set dropheight.  shrink to fit a small number, but cap at a reasonable max
            SetDropHeight(GG::Y(std::max( 8, int(myQuantSet.size() ) )*height + 4));
            //QuantLabel ref1 = QuantLabel(quantity, ClientUI::Pts());
            //QuantLabel ref2 = QuantLabel(100, ClientUI::Pts());
            //OffsetMove(GG::Pt(ref1.Width()-GG::X(50) +GG::X(8), GG::Y(-4)));
        }

        void SelectionChanged(GG::DropDownList::iterator it) {
            int quant;
            //SetInteriorColor(GG::Clr(0, 0, 0, 0));
            amOn = false;
            //Hide();
            //Render();
            DebugLogger() << "QuantSelector:  selection made ";
            if (it != end()) {
                quant = boost::polymorphic_downcast<const QuantRow*>(*it)->Quant();
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
        const ProductionQueue::Element m_build;
        int     quantity;
        int     prevQuant;
        int     blocksize;
        int     prevBlocksize;
        bool    amBlockType;
        bool    amOn;
        GG::Y   h;

        void LosingFocus() {
            amOn = false;
            DropDownList::LosingFocus();
        }

        void LButtonDown(const GG::Pt&  pt, GG::Flags<GG::ModKey> mod_keys) {
            amOn = !amOn;
            DropDownList::LButtonDown(pt, mod_keys);
        }

        void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
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
        QueueProductionItemPanel(GG::X w, const ProductionQueue::Element& build, double turn_cost,
                                 int turns, int number, int turns_completed, double partially_complete_turn);

        virtual void    Render();
        void            UpdateQueue();
        void            ItemQuantityChanged(int quant, int blocksize);
        void            ItemBlocksizeChanged(int quant, int blocksize);

        mutable boost::signals2::signal<void(int,int)>    PanelUpdateQuantSignal;

    private:
        void Draw(GG::Clr clr, bool fill);

        const ProductionQueue::Element  m_build;
        CUILabel*                       m_name_text;
        GG::Control*                    m_location_text;
        CUILabel*                       m_PPs_and_turns_text;
        CUILabel*                       m_turns_remaining_until_next_complete_text;
        GG::StaticGraphic*              m_icon;
        MultiTurnProgressBar*           m_progress_bar;
        QuantitySelector*               m_quantity_selector;
        QuantitySelector*               m_block_size_selector;
        bool                            m_in_progress;
        int                             m_total_turns;
        int                             m_turns_completed;
        double                          m_partially_complete_turn;
        bool                            m_order_issuing_enabled;
    };

    /////////////////////////////
    // ProductionItemBrowseWnd //
    /////////////////////////////
    boost::shared_ptr<GG::BrowseInfoWnd> ProductionItemBrowseWnd(const ProductionQueue::Element& elem) {
        //const Empire* empire = GetEmpire(elem.empire_id);

        std::string main_text;
        std::string item_name;

        //int min_turns = 1;
        float total_cost = 0.0f;
        float max_allocation = 0.0f;
        boost::shared_ptr<GG::Texture> icon;
        //bool available = false;
        bool location_ok = false;

        if (elem.item.build_type == BT_BUILDING) {
            const BuildingType* building_type = GetBuildingType(elem.item.name);
            if (!building_type) {
                boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd;
                return browse_wnd;
            }
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
            if (!design) {
                boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd;
                return browse_wnd;
            }
            main_text += UserString("OBJ_SHIP") + "\n";

            item_name = design->Name(true);
            //available = empire->ShipDesignAvailable(elem.item.design_id);
            location_ok = design->ProductionLocation(elem.empire_id, elem.location);
            //min_turns = design->ProductionTime(elem.empire_id, elem.location);
            total_cost = design->ProductionCost(elem.empire_id, elem.location) * elem.blocksize;
            max_allocation = design->PerTurnCost(elem.empire_id, elem.location) * elem.blocksize;
            icon = ClientUI::ShipDesignIcon(elem.item.design_id);
        }

        if (TemporaryPtr<UniverseObject> location = GetUniverseObject(elem.location))
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
                        % DoubleToString(progress, 3, false)
                        % DoubleToString(total_cost, 3, false)
                        % DoubleToString(allocation, 3, false)
                        % DoubleToString(max_allocation, 3, false)) + "\n";

        int ETA = elem.turns_left_to_completion;
        if (ETA != -1)
            main_text += boost::io::str(FlexibleFormat(UserString("TECH_WND_ETA"))
                            % ETA);

        std::string title_text;
        if (elem.blocksize > 1)
            title_text = boost::lexical_cast<std::string>(elem.blocksize) + "x ";
        title_text += item_name;

        boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd(new IconTextBrowseWnd(icon, title_text, main_text));
        return browse_wnd;
    }

    //////////////////////////////////////////////////
    // QueueRow
    //////////////////////////////////////////////////
    struct QueueRow : GG::ListBox::Row {
        QueueRow(GG::X w, const ProductionQueue::Element& elem, int queue_index_) :
            GG::ListBox::Row(GG::X1, GG::Y1, "PRODUCTION_QUEUE_ROW"),
            queue_index(queue_index_),
            m_build(elem)
        {
            const Empire* empire = GetEmpire(HumanClientApp::GetApp()->EmpireID());
            double total_cost(1.0);
            int minimum_turns(1.0);
            if (empire)
                boost::tie(total_cost, minimum_turns) = empire->ProductionCostAndTime(elem);
            total_cost *= elem.blocksize;
            double per_turn_cost = total_cost / std::max(1, minimum_turns);
            double progress = empire->ProductionStatus(queue_index);
            if (progress == -1.0)
                progress = 0.0;

            QueueProductionItemPanel* panel =
                new QueueProductionItemPanel(w, elem, elem.allocated_pp, minimum_turns, elem.remaining,
                                             static_cast<int>(progress / std::max(1e-6,per_turn_cost)),
                                             std::fmod(progress, per_turn_cost) / std::max(1e-6,per_turn_cost));
            Resize(panel->Size());
            push_back(panel);

            SetDragDropDataType("PRODUCTION_QUEUE_ROW");

            SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
            SetBrowseInfoWnd(ProductionItemBrowseWnd(elem));

            GG::Connect(panel->PanelUpdateQuantSignal,  &QueueRow::RowQuantChanged, this);
        }

        virtual void Disable(bool b) {
            GG::ListBox::Row::Disable(b);

            for (std::vector<GG::Control*>::iterator it = this->m_cells.begin();
                 it != this->m_cells.end(); ++it)
            {
                GG::Control* ctrl = *it;
                if (ctrl)
                    ctrl->Disable(this->Disabled());
            }
        }

        void RowQuantChanged(int quantity, int blocksize) {
            if (this->Disabled())
                return;
            RowQuantChangedSignal(queue_index, quantity, blocksize);
        }

        const int                                           queue_index;
        const ProductionQueue::Element                      m_build;
        mutable boost::signals2::signal<void (int,int,int)> RowQuantChangedSignal;
    };

    //////////////////////////////////////////////////
    // QueueProductionItemPanel implementation
    //////////////////////////////////////////////////
    QueueProductionItemPanel::QueueProductionItemPanel(GG::X w, const ProductionQueue::Element& build,
                                                       double turn_spending, int turns, int number,
                                                       int turns_completed, double partially_complete_turn) :
        GG::Control(GG::X0, GG::Y0, w, GG::Y(10), GG::NO_WND_FLAGS),
        m_build(build),
        m_name_text(0),
        m_location_text(0),
        m_PPs_and_turns_text(0),
        m_turns_remaining_until_next_complete_text(0),
        m_icon(0),
        m_progress_bar(0),
        m_quantity_selector(0),
        m_block_size_selector(0),
        m_in_progress(build.allocated_pp || build.turns_left_to_next_item == 1),
        m_total_turns(turns),
        m_turns_completed(turns_completed),
        m_partially_complete_turn(partially_complete_turn)
    {
        const int MARGIN = 2;

        const int FONT_PTS = ClientUI::Pts();
        const GG::Y METER_HEIGHT(FONT_PTS);

        const GG::Y HEIGHT = MARGIN + FONT_PTS + MARGIN + METER_HEIGHT + MARGIN + FONT_PTS + MARGIN + 6;

        const int GRAPHIC_SIZE = Value(HEIGHT - 9);    // 9 pixels accounts for border thickness so the sharp-cornered icon doesn't with the rounded panel corner

        const GG::X METER_WIDTH = w - GRAPHIC_SIZE - 3*MARGIN - 3;

        Resize(GG::Pt(w, HEIGHT));

        GG::Clr clr = m_in_progress
            ? GG::LightColor(ClientUI::ResearchableTechTextAndBorderColor())
            : ClientUI::ResearchableTechTextAndBorderColor();

        // get graphic and player-visible name text for item
        boost::shared_ptr<GG::Texture> graphic;
        std::string name_text;
        if (build.item.build_type == BT_BUILDING) {
            graphic = ClientUI::BuildingIcon(build.item.name);
            name_text = UserString(build.item.name);
        } else if (build.item.build_type == BT_SHIP) {
            graphic = ClientUI::ShipDesignIcon(build.item.design_id);
            name_text = GetShipDesign(build.item.design_id)->Name();
        } else {
            graphic = ClientUI::GetTexture(""); // get "missing texture" texture by supply intentionally bad path
            name_text = UserString("FW_UNKNOWN_DESIGN_NAME");
        }

        // get location indicator text
        std::string location_text;
        bool system_selected = false;
        if (TemporaryPtr<const UniverseObject> location = GetUniverseObject(build.location)) {
            system_selected = (location->SystemID() != -1 && location ->SystemID() == SidePanel::SystemID());
            if (GetOptionsDB().Get<bool>("UI.show-production-location-on-queue"))
                location_text = boost::io::str(FlexibleFormat(UserString("PRODUCTION_QUEUE_ITEM_LOCATION")) % location->Name()) + " ";
        }

        // create and arrange widgets to display info
        GG::Y top(MARGIN);
        GG::X left(MARGIN);

        if (graphic) {
            m_icon = new GG::StaticGraphic(graphic, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
            m_icon->MoveTo(GG::Pt(left, top));
            m_icon->Resize(GG::Pt(GG::X(GRAPHIC_SIZE), GG::Y(GRAPHIC_SIZE)));
        }
        else
            m_icon = 0;

        left += GRAPHIC_SIZE + MARGIN;

        if (m_build.item.build_type == BT_SHIP) {
            m_quantity_selector = new QuantitySelector(m_build, left, GG::Y(MARGIN), GG::Y(FONT_PTS-2*MARGIN), m_in_progress, GG::X(FONT_PTS*2.5), false);
            GG::Connect(m_quantity_selector->SelChangedSignal,        &QuantitySelector::SelectionChanged, m_quantity_selector);
            left += m_quantity_selector->Width();
            m_block_size_selector = new QuantitySelector(m_build, left,    GG::Y(MARGIN), GG::Y(FONT_PTS-2*MARGIN), m_in_progress, GG::X(FONT_PTS*2.5), true);
            GG::Connect(m_block_size_selector->SelChangedSignal,           &QuantitySelector::SelectionChanged, m_block_size_selector);
            left += m_block_size_selector->Width();
        }

        const GG::X NAME_WIDTH = w - left - MARGIN;
        m_name_text = new CUILabel(name_text, GG::FORMAT_TOP | GG::FORMAT_LEFT);
        m_name_text->MoveTo(GG::Pt(left, top));
        m_name_text->Resize(GG::Pt(NAME_WIDTH, GG::Y(FONT_PTS + 2*MARGIN)));
        m_name_text->SetTextColor(clr);
        m_name_text->ClipText(true);

        GG::Clr location_clr = clr;
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();
        const Empire* this_client_empire = GetEmpire(client_empire_id);
        if (this_client_empire && system_selected) {
            m_location_text = new ShadowedTextControl(location_text, ClientUI::GetBoldFont(), this_client_empire->Color(), GG::FORMAT_TOP | GG::FORMAT_RIGHT);
        } else {
            CUILabel* l_location_text = new CUILabel(location_text, GG::FORMAT_TOP | GG::FORMAT_RIGHT);
            l_location_text->SetTextColor(location_clr);
            m_location_text = l_location_text;
        }
        m_location_text->MoveTo(GG::Pt(left, top));
        m_location_text->Resize(GG::Pt(NAME_WIDTH, GG::Y(FONT_PTS + 2*MARGIN)));

        top += m_name_text->Height();
        left = GG::X(GRAPHIC_SIZE + MARGIN*2);
        m_progress_bar = new MultiTurnProgressBar(turns, turns_completed + partially_complete_turn,
                                                  GG::LightColor(ClientUI::TechWndProgressBarBackgroundColor()),
                                                  ClientUI::TechWndProgressBarColor(),
                                                  m_in_progress
                                                    ? ClientUI::ResearchableTechFillColor()
                                                    : GG::LightColor(ClientUI::ResearchableTechFillColor()));
        m_progress_bar->MoveTo(GG::Pt(left, top));
        m_progress_bar->Resize(GG::Pt(METER_WIDTH, METER_HEIGHT));

        top += m_progress_bar->Height() + MARGIN;

        std::string turn_spending_text = boost::io::str(FlexibleFormat(UserString("PRODUCTION_TURN_COST_STR")) % DoubleToString(turn_spending, 3, false));
        GG::X TURNS_AND_COST_WIDTH = METER_WIDTH / 2;
        m_PPs_and_turns_text = new CUILabel(turn_spending_text, GG::FORMAT_LEFT);
        m_PPs_and_turns_text->MoveTo(GG::Pt(left, top));
        m_PPs_and_turns_text->Resize(GG::Pt(TURNS_AND_COST_WIDTH, GG::Y(FONT_PTS + MARGIN)));
        m_PPs_and_turns_text->SetTextColor(clr);

        left += TURNS_AND_COST_WIDTH;

        int turns_left = build.turns_left_to_next_item;
        std::string turns_left_text = turns_left < 0
            ? UserString("PRODUCTION_TURNS_LEFT_NEVER")
            : str(FlexibleFormat(UserString("PRODUCTION_TURNS_LEFT_STR")) % turns_left);
        m_turns_remaining_until_next_complete_text = new CUILabel(turns_left_text, GG::FORMAT_RIGHT);
        m_turns_remaining_until_next_complete_text->MoveTo(GG::Pt(left, top));
        m_turns_remaining_until_next_complete_text->Resize(GG::Pt(TURNS_AND_COST_WIDTH, GG::Y(FONT_PTS + MARGIN)));
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
    }

    void QueueProductionItemPanel::ItemQuantityChanged(int quant, int blocksize)
    { PanelUpdateQuantSignal(quant,m_build.blocksize); }

    void QueueProductionItemPanel::ItemBlocksizeChanged(int quant, int blocksize) // made separate funcion in case wna to do extra checking
    { PanelUpdateQuantSignal(m_build.remaining, blocksize); }

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
        PartlyRoundedRect(UpperLeft(), LowerRight(), CORNER_RADIUS, true, false, true, false, fill);
    }
}


//////////////////////////////////////////////////
// ProductionWnd                                //
//////////////////////////////////////////////////
ProductionWnd::ProductionWnd(GG::X w, GG::Y h) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::INTERACTIVE | GG::ONTOP),
    m_production_info_panel(0),
    m_queue_lb(0),
    m_build_designator_wnd(0),
    m_order_issuing_enabled(false)
{
    DebugLogger() << "ProductionWindow:  app-width: "<< GetOptionsDB().Get<int>("app-width")
                           << " ; windowed width: " << GetOptionsDB().Get<int>("app-width-windowed");

    m_production_info_panel = new ProductionInfoPanel(UserString("PRODUCTION_INFO_PANEL_TITLE"),
                                                      UserString("PRODUCTION_INFO_PP"),
                                                      static_cast<GLfloat>(OUTER_LINE_THICKNESS),
                                                      ClientUI::KnownTechFillColor(),
                                                      ClientUI::KnownTechTextAndBorderColor());

    m_queue_lb = new QueueListBox("PRODUCTION_QUEUE_ROW", UserString("PRODUCTION_QUEUE_PROMPT"));
    m_queue_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_NOSEL | GG::LIST_USERDELETE);
    m_queue_lb->SetName("ProductionQueue ListBox");

    GG::Pt buid_designator_wnd_size = ClientSize() - GG::Pt(GG::X(GetOptionsDB().Get<int>("UI.queue-width")), GG::Y0);
    m_build_designator_wnd = new BuildDesignatorWnd(buid_designator_wnd_size.x, buid_designator_wnd_size.y);
    m_build_designator_wnd->MoveTo(GG::Pt(GG::X(GetOptionsDB().Get<int>("UI.queue-width")), GG::Y0));

    SetChildClippingMode(ClipToClient);

    GG::Connect(m_build_designator_wnd->AddNamedBuildToQueueSignal,     static_cast<void (ProductionWnd::*)(BuildType, const std::string&, int, int)>(&ProductionWnd::AddBuildToQueueSlot), this);
    GG::Connect(m_build_designator_wnd->AddIDedBuildToQueueSignal,      static_cast<void (ProductionWnd::*)(BuildType, int, int, int)>(&ProductionWnd::AddBuildToQueueSlot), this);
    GG::Connect(m_build_designator_wnd->BuildQuantityChangedSignal,     &ProductionWnd::ChangeBuildQuantitySlot, this);
    GG::Connect(m_build_designator_wnd->SystemSelectedSignal,           SystemSelectedSignal);
    GG::Connect(m_queue_lb->QueueItemMovedSignal,                       &ProductionWnd::QueueItemMoved, this);
    GG::Connect(m_queue_lb->LeftClickedSignal,                          &ProductionWnd::QueueItemClickedSlot, this);
    GG::Connect(m_queue_lb->DoubleClickedSignal,                        &ProductionWnd::QueueItemDoubleClickedSlot, this);

    AttachChild(m_production_info_panel);
    AttachChild(m_queue_lb);
    AttachChild(m_build_designator_wnd);

    DoLayout();
}

ProductionWnd::~ProductionWnd()
{ m_empire_connection.disconnect(); }

int ProductionWnd::SelectedPlanetID() const
{ return m_build_designator_wnd->SelectedPlanetID(); }

bool ProductionWnd::InWindow(const GG::Pt& pt) const
{ return m_production_info_panel->InWindow(pt) || m_queue_lb->InWindow(pt) || m_build_designator_wnd->InWindow(pt); }

bool ProductionWnd::InClient(const GG::Pt& pt) const
{ return m_production_info_panel->InClient(pt) || m_queue_lb->InClient(pt) || m_build_designator_wnd->InClient(pt); }

void ProductionWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    GG::Wnd::SizeMove(ul, lr);
    if (old_size != Size())
        DoLayout();
}

void ProductionWnd::DoLayout() {
    m_production_info_panel->Resize(GG::Pt(GG::X(GetOptionsDB().Get<int>("UI.queue-width")), m_production_info_panel->MinUsableSize().y));
    GG::Pt queue_ul = GG::Pt(GG::X(2), m_production_info_panel->Height());
    GG::Pt queue_size = GG::Pt(m_production_info_panel->Width() - 4,
                               ClientSize().y - 4 - m_production_info_panel->Height());
    m_queue_lb->SizeMove(queue_ul, queue_ul + queue_size);

    GG::Pt build_wnd_size = ClientSize() - GG::Pt(m_production_info_panel->Width(), GG::Y0);
    GG::Pt build_wnd_ul = GG::Pt(m_production_info_panel->Width(), GG::Y0);
    m_build_designator_wnd->SizeMove(build_wnd_ul, build_wnd_ul + build_wnd_size);
}

void ProductionWnd::Render()
{}

void ProductionWnd::Refresh() {
    // useful at start of turn or when loading empire from save.
    // since empire object is recreated based on turn update from server, 
    // connections of signals emitted from the empire must be remade
    m_empire_connection.disconnect();

    if (Empire* empire = GetEmpire(HumanClientApp::GetApp()->EmpireID()))
        m_empire_connection = GG::Connect(empire->GetProductionQueue().ProductionQueueChangedSignal,
                                          &ProductionWnd::ProductionQueueChangedSlot, this);

    UpdateInfoPanel();
    UpdateQueue();

    m_build_designator_wnd->Refresh();
}

void ProductionWnd::Reset() {
    //std::cout << "ProductionWnd::Reset()" << std::endl;
    UpdateInfoPanel();
    UpdateQueue();
    m_queue_lb->BringRowIntoView(m_queue_lb->begin());
    m_build_designator_wnd->Reset();
}

void ProductionWnd::Update() {
    //std::cout << "ProductionWnd::Update()" << this << std::endl;
    UpdateInfoPanel();
    UpdateQueue();

    m_build_designator_wnd->Update();
}

void ProductionWnd::ShowBuildingTypeInEncyclopedia(const std::string& building_type)
{ m_build_designator_wnd->ShowBuildingTypeInEncyclopedia(building_type); }

void ProductionWnd::ShowShipDesignInEncyclopedia(int design_id)
{ m_build_designator_wnd->ShowShipDesignInEncyclopedia(design_id); }

void ProductionWnd::CenterOnBuild(int queue_idx)
{ m_build_designator_wnd->CenterOnBuild(queue_idx); }

void ProductionWnd::SelectPlanet(int planet_id)
{ m_build_designator_wnd->SelectPlanet(planet_id); }

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
    const Empire* empire = GetEmpire(HumanClientApp::GetApp()->EmpireID());
    if (!empire)
        return;

    const ProductionQueue& queue = empire->GetProductionQueue();
    std::size_t first_visible_queue_row = std::distance(m_queue_lb->begin(), m_queue_lb->FirstRowShown());
    m_queue_lb->Clear();
    const GG::X QUEUE_WIDTH = m_queue_lb->Width() - 8 - 14;

    int i = 0;
    for (ProductionQueue::const_iterator it = queue.begin(); it != queue.end(); ++it, ++i) {
        QueueRow* newRow = new QueueRow(QUEUE_WIDTH, *it, i);
        GG::Connect(newRow->RowQuantChangedSignal,     &ProductionWnd::ChangeBuildQuantityBlockSlot, this);
        m_queue_lb->Insert(newRow);
    }

    if (!m_queue_lb->Empty())
        m_queue_lb->BringRowIntoView(--m_queue_lb->end());
    if (first_visible_queue_row < m_queue_lb->NumRows())
        m_queue_lb->BringRowIntoView(boost::next(m_queue_lb->begin(), first_visible_queue_row));
}

void ProductionWnd::UpdateInfoPanel() {
    const Empire* empire = GetEmpire(HumanClientApp::GetApp()->EmpireID());
    if (!empire)
        return;
    const ProductionQueue& queue = empire->GetProductionQueue();
    double PPs = empire->ProductionPoints();
    double total_queue_cost = queue.TotalPPsSpent();
    ProductionQueue::const_iterator underfunded_it = queue.UnderfundedProject();
    double PPs_to_underfunded_projects = underfunded_it == queue.end() ? 0.0 : underfunded_it->allocated_pp;
    m_production_info_panel->Reset(PPs, total_queue_cost, queue.ProjectsInProgress(), PPs_to_underfunded_projects, queue.size());
}

void ProductionWnd::AddBuildToQueueSlot(BuildType build_type, const std::string& name, int number, int location) {
    if (!m_order_issuing_enabled)
        return;
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    Empire* empire = GetEmpire(client_empire_id);
    if (!empire)
        return;

    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(
        new ProductionQueueOrder(client_empire_id, build_type, name, number, location)));

    empire->UpdateProductionQueue();
    m_build_designator_wnd->CenterOnBuild(m_queue_lb->NumRows() - 1);
}

void ProductionWnd::AddBuildToQueueSlot(BuildType build_type, int design_id, int number, int location) {
    if (!m_order_issuing_enabled)
        return;
    int client_empire_id = HumanClientApp::GetApp()->EmpireID();
    Empire* empire = GetEmpire(client_empire_id);
    if (!empire)
        return;

    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(
        new ProductionQueueOrder(client_empire_id, build_type, design_id, number, location)));

    empire->UpdateProductionQueue();
    m_build_designator_wnd->CenterOnBuild(m_queue_lb->NumRows() - 1);
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
        OrderPtr(new ProductionQueueOrder(client_empire_id, std::distance(m_queue_lb->begin(), it))));

    empire->UpdateProductionQueue();
}

void ProductionWnd::QueueItemClickedSlot(GG::ListBox::iterator it, const GG::Pt& pt) {
    if (m_queue_lb->DisplayingValidQueueItems()) {
        m_build_designator_wnd->CenterOnBuild(std::distance(m_queue_lb->begin(), it));
    }
}

void ProductionWnd::QueueItemDoubleClickedSlot(GG::ListBox::iterator it) {
    if (m_queue_lb->DisplayingValidQueueItems()) {
        DeleteQueueItem(it);
    }
}

void ProductionWnd::EnableOrderIssuing(bool enable/* = true*/) {
    m_order_issuing_enabled = enable;
    m_queue_lb->EnableOrderIssuing(m_order_issuing_enabled);
}
