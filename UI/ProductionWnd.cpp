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

#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>

#include <boost/cast.hpp>

#include <cmath>


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

            for (std::set<int>::iterator it = myQuantSet.begin(); it != myQuantSet.end(); ++it)
            {
                QuantRow* row =  new QuantRow(*it, build.item.design_id, nwidth, h, inProgress, amBlockType);
                GG::DropDownList::iterator latest_it = Insert(row);

                if (amBlockType) {
                    if (build.blocksize == *it)
                        Select(latest_it);
                } else {
                    if (build.remaining == *it)
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
        QueueProductionItemPanel(GG::X w, const ProductionQueue::Element& build, double turn_cost, double total_cost,
                                 int turns, int number, int turns_completed, double partially_complete_turn);

        virtual void    Render();
        void            UpdateQueue();
        void            ItemQuantityChanged(int quant, int blocksize);
        void            ItemBlocksizeChanged(int quant, int blocksize);
        virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);

        mutable boost::signals2::signal<void(int,int)>    PanelUpdateQuantSignal;

    private:
        void            Draw(GG::Clr clr, bool fill);
        void            DoLayout();

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

        if (TemporaryPtr<UniverseObject> rally_object = GetUniverseObject(elem.rally_point_id)) {
            main_text += boost::io::str(FlexibleFormat(UserString("PRODUCTION_QUEUE_RALLIED_TO"))
                                        % rally_object->Name()) + "\n";
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
            GG::ListBox::Row(w, GG::Y1, BuildDesignatorWnd::PRODUCTION_ITEM_DROP_TYPE),
            queue_index(queue_index_),
            elem(elem)
        {
            //std::cout << "QueueRow(" << w << ", ...)" << std::endl;

            const Empire* empire = GetEmpire(HumanClientApp::GetApp()->EmpireID());
            float total_cost(1.0f);
            int minimum_turns(1);
            if (empire)
                boost::tie(total_cost, minimum_turns) = empire->ProductionCostAndTime(elem);
            total_cost *= elem.blocksize;
            float per_turn_cost = total_cost / std::max(1, minimum_turns);
            float progress = empire ? empire->ProductionStatus(queue_index) : 0.0f;
            if (progress == -1.0f)
                progress = 0.0f;

            panel = new QueueProductionItemPanel(w, elem, elem.allocated_pp, total_cost, minimum_turns, elem.remaining,
                                                 static_cast<int>(progress / std::max(1e-6f, per_turn_cost)),
                                                 std::fmod(progress, per_turn_cost) / std::max(1e-6f, per_turn_cost));
            Resize(panel->Size());
            push_back(panel);

            SetDragDropDataType(BuildDesignatorWnd::PRODUCTION_ITEM_DROP_TYPE);

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

        virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            const GG::Pt old_size = Size();
            GG::ListBox::Row::SizeMove(ul, lr);
            if (!empty() && old_size != Size() && panel) {
                //std::cout << "QueueRow resized to: " << Size() << std::endl;
                panel->Resize(Size());
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
    const int MARGIN = 2;

    QueueProductionItemPanel::QueueProductionItemPanel(GG::X w, const ProductionQueue::Element& build,
                                                       double turn_spending, double total_cost, int turns, int number,
                                                       int turns_completed, double partially_complete_turn) :
        GG::Control(GG::X0, GG::Y0, w, GG::Y(10), GG::NO_WND_FLAGS),
        elem(build),
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
        GG::Clr clr = m_in_progress
            ? GG::LightColor(ClientUI::ResearchableTechTextAndBorderColor())
            : ClientUI::ResearchableTechTextAndBorderColor();

        // get graphic and player-visible name text for item
        boost::shared_ptr<GG::Texture> graphic;
        std::string name_text;
        if (build.paused) {
            name_text = UserString("PAUSED");
        } else if (build.item.build_type == BT_BUILDING) {
            graphic = ClientUI::BuildingIcon(build.item.name);
            name_text = UserString(build.item.name);
        } else if (build.item.build_type == BT_SHIP) {
            graphic = ClientUI::ShipDesignIcon(build.item.design_id);
            const ShipDesign* design = GetShipDesign(build.item.design_id);
            if (design)
                name_text = design->Name();
            else
                ErrorLogger() << "QueueProductionItemPanel unable to get design with id: " << build.item.design_id;
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


        const int FONT_PTS = ClientUI::Pts();


        m_icon = 0;
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

        m_progress_bar = new MultiTurnProgressBar(turns, turns_completed + partially_complete_turn, total_cost, turn_spending,
                                                  GG::LightColor(ClientUI::TechWndProgressBarBackgroundColor()),
                                                  ClientUI::TechWndProgressBarColor(),
                                                  m_in_progress
                                                    ? ClientUI::ResearchableTechFillColor()
                                                    : GG::LightColor(ClientUI::ResearchableTechFillColor()));

        double max_spending_per_turn = total_cost / m_total_turns;
        std::string turn_spending_text = boost::io::str(FlexibleFormat(UserString("PRODUCTION_TURN_COST_STR"))
            % DoubleToString(turn_spending, 3, false)
            % DoubleToString(max_spending_per_turn, 3, false));
        m_PPs_and_turns_text = new CUILabel(turn_spending_text, GG::FORMAT_LEFT);
        m_PPs_and_turns_text->SetTextColor(clr);


        int turns_left = build.turns_left_to_next_item;
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

        const GG::Y HEIGHT = GG::Y(MARGIN) + FONT_PTS + MARGIN + FONT_PTS + MARGIN + FONT_PTS + MARGIN + 6;
        Resize(GG::Pt(w, HEIGHT));
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
            if (m_quantity_selector) {
                left += m_quantity_selector->Width();
            }
        }

        const GG::X NAME_WIDTH = Width() - left - MARGIN;
        m_name_text->MoveTo(GG::Pt(left, top));
        m_name_text->Resize(GG::Pt(NAME_WIDTH, GG::Y(FONT_PTS + 2*MARGIN)));

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
        PartlyRoundedRect(UpperLeft(), LowerRight(), CORNER_RADIUS, true, false, true, false, fill);
    }

    void QueueProductionItemPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            const GG::Pt old_size = Size();
            GG::Control::SizeMove(ul, lr);
            if (Size() != old_size)
                DoLayout();
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
        void ItemRightClickedImpl(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) {
            // mostly duplicated equivalent in QueueListBox, but with extra commands...

            GG::MenuItem menu_contents;
            menu_contents.next_level.push_back(GG::MenuItem(UserString("MOVE_UP_QUEUE_ITEM"),   1, false, false));
            menu_contents.next_level.push_back(GG::MenuItem(UserString("MOVE_DOWN_QUEUE_ITEM"), 2, false, false));
            menu_contents.next_level.push_back(GG::MenuItem(UserString("DELETE_QUEUE_ITEM"),    3, false, false));

            // inspect clicked item: was it a ship?
            GG::ListBox::Row* row = *it;
            QueueRow* queue_row = row ? dynamic_cast<QueueRow*>(row) : 0;
            BuildType build_type = queue_row ? queue_row->elem.item.build_type : INVALID_BUILD_TYPE;
            if (build_type == BT_SHIP) {
                // for ships, add a set rally point command
                if (TemporaryPtr<const System> system = GetSystem(SidePanel::SystemID())) {
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


            GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents, ClientUI::TextColor(),
                                ClientUI::WndOuterBorderColor(), ClientUI::WndColor(), ClientUI::EditHiliteColor());

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
        m_queue_lb(0)
    {
        Init(HumanClientApp::GetApp()->EmpireID());
    }
    //@}

    /** \name Mutators */ //@{
    virtual void        SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
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
    m_production_info_panel(0),
    m_queue_wnd(0),
    m_build_designator_wnd(0),
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
    ScopedTimer timer("ProductionWnd::UpdateQueue");

    m_queue_wnd->SetEmpire(m_empire_shown_id);

    QueueListBox* queue_lb = m_queue_wnd->GetQueueListBox();
    std::size_t first_visible_queue_row = std::distance(queue_lb->begin(), queue_lb->FirstRowShown());
    queue_lb->Clear();

    const Empire* empire = GetEmpire(m_empire_shown_id);
    if (!empire)
        return;

    const ProductionQueue& queue = empire->GetProductionQueue();

    int i = 0;
    for (ProductionQueue::const_iterator it = queue.begin(); it != queue.end(); ++it, ++i) {
        QueueRow* row = new QueueRow(queue_lb->RowWidth(), *it, i);
        GG::Connect(row->RowQuantChangedSignal,     &ProductionWnd::ChangeBuildQuantityBlockSlot, this);
        queue_lb->Insert(row);
    }

    if (!queue_lb->Empty())
        queue_lb->BringRowIntoView(--queue_lb->end());
    if (first_visible_queue_row < queue_lb->NumRows())
        queue_lb->BringRowIntoView(boost::next(queue_lb->begin(), first_visible_queue_row));
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
    TemporaryPtr<UniverseObject> loc_obj = GetUniverseObject(prod_loc_id);
    if (loc_obj) {
        // extract available and allocated PP at production location
        std::map<std::set<int>, float> available_pp = queue.AvailablePP(empire->GetResourcePool(RE_INDUSTRY));
        const std::map<std::set<int>, float>& allocated_pp = queue.AllocatedPP();

        float available_pp_at_loc = 0.0f, allocated_pp_at_loc = 0.0f;   // for the resource sharing group containing the selected production location

        for (std::map<std::set<int>, float>::const_iterator map_it = available_pp.begin();
             map_it != available_pp.end(); ++map_it)
        {
            if (map_it->first.find(prod_loc_id) != map_it->first.end()) {
                available_pp_at_loc = map_it->second;
                break;
            }
        }
                for (std::map<std::set<int>, float>::const_iterator map_it = allocated_pp.begin();
             map_it != allocated_pp.end(); ++map_it)
        {
            if (map_it->first.find(prod_loc_id) != map_it->first.end()) {
                allocated_pp_at_loc = map_it->second;
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
