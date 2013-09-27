#include "ProductionWnd.h"

#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "BuildDesignatorWnd.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "QueueListBox.h"
#include "SidePanel.h"
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
    const GG::X PRODUCTION_INFO_AND_QUEUE_WIDTH(250); //standard width 250
    const GG::X WIDE_PRODUCTION_INFO_AND_QUEUE_WIDTH(350); //standard width 250
    const double PI = 3.141594;
    const double OUTER_LINE_THICKNESS = 2.0;

    //////////////////////////////////////////////////
    // QueueRow
    //////////////////////////////////////////////////
    struct QueueRow : GG::ListBox::Row {
        QueueRow(GG::X w, const ProductionQueue::Element& build, int queue_index_);
        const int queue_index;
        const ProductionQueue::Element m_build;
        mutable boost::signal<void (int,int,int)>RowQuantChangedSignal;
        void RowQuantChanged(int quantity, int blocksize) {
            RowQuantChangedSignal(queue_index, quantity, blocksize);
        }
    };

    class QuantLabel : public GG::Control {
    public:
        QuantLabel(int quantity, int designID, boost::shared_ptr<GG::Font> font, GG::X nwidth, GG::Y h, bool inProgress, bool amBlockType) :
            Control(GG::X0, GG::Y0, nwidth, h, GG::Flags<GG::WndFlag>())
        {
            GG::Clr txtClr = inProgress ? GG::LightColor(ClientUI::ResearchableTechTextAndBorderColor()) : ClientUI::ResearchableTechTextAndBorderColor();
            std::string nameText;
            if (amBlockType)
                nameText = boost::io::str(FlexibleFormat(UserString("PRODUCTION_QUEUE_MULTIPLES")) % quantity);
            else
                nameText = boost::io::str(FlexibleFormat(UserString("PRODUCTION_QUEUE_REPETITIONS")) % quantity);
            //nameText += GetShipDesign(designID)->Name();
            GG::TextControl* text = new GG::TextControl(GG::X0, GG::Y0, nameText, font, txtClr, GG::FORMAT_TOP | GG::FORMAT_LEFT);
            text->OffsetMove(GG::Pt(GG::X0, GG::Y(-3))); //
            AttachChild(text);
            Resize(GG::Pt(nwidth, text->Height()));
        }
        void Render()
        {}
    };

    class QuantRow : public GG::ListBox::Row {
    public:
        QuantRow(int quantity, int designID, boost::shared_ptr<GG::Font> font, GG::X nwidth, GG::Y h, bool inProgress, bool amBlockType) :
            GG::ListBox::Row(),
            width(0),
            m_quant(quantity)
        {
            QuantLabel* newLabel = new QuantLabel(m_quant, designID, font, nwidth, h, inProgress,amBlockType);
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

    class QuantitySelector : public CUIDropDownList {
    public:
        mutable boost::signal<void (int,int)> QuantChangedSignal;

        /** \name Structors */
        QuantitySelector(const ProductionQueue::Element &build, GG::X xoffset, GG::Y yoffset, GG::Y h, boost::shared_ptr<GG::Font> font, bool inProgress, GG::X nwidth, bool amBlockType) :
            CUIDropDownList(xoffset, yoffset, nwidth,h-GG::Y(2), h, inProgress ? GG::LightColor(ClientUI::ResearchableTechTextAndBorderColor()) : ClientUI::ResearchableTechTextAndBorderColor(), 
                           ( inProgress ? GG::LightColor(ClientUI::ResearchableTechFillColor()) : ClientUI::ResearchableTechFillColor() ) ),
            quantity(build.remaining),
            prevQuant(build.remaining),
            blocksize(build.blocksize),
            prevBlocksize(build.blocksize),
            amBlockType(amBlockType),
            amOn(false),
            h(h)
        {
            GG::X width = GG::X0;
            DisableDropArrow();
            SetStyle(GG::LIST_LEFT | GG::LIST_NOSORT);
            //SetInteriorColor(GG::Clr(0, 0, 0, 200));
            SetNumCols(1);
            //m_quantityBox->SetColWidth(0, GG::X(14));
            //m_quantityBox->LockColWidths();

            int quantInts[] = {1, 2, 3, 4, 5, 10, 20, 50, 99};
            std::set<int> myQuantSet(quantInts,quantInts+9);
            if (amBlockType)
                myQuantSet.insert(blocksize); //as currently implemented this one not actually necessary since blocksize has no other way to change
            else
                myQuantSet.insert(quantity);
            GG::Y height;
            for (std::set<int>::iterator it=myQuantSet.begin(); it != myQuantSet.end(); it++ ) {
                QuantRow* newRow =  new QuantRow(*it, build.item.design_id, font, nwidth, h, inProgress,amBlockType);
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

        QuantitySelector(const ProductionQueue::Element &build, GG::Y h) :
            //CUIDropDownList(GG::X0, GG::Y0, GG::X(36),h, h, GG::CLR_ZERO, GG::FloatClr(0.0, 0.0, 0.0, 0.5)),
            CUIDropDownList(GG::X0, GG::Y0, GG::X(36),h, h, ClientUI::KnownTechTextAndBorderColor(), ClientUI::KnownTechFillColor()),
            quantity(build.remaining),
            prevQuant(build.remaining),
            blocksize(build.blocksize),
            prevBlocksize(build.blocksize),
            amBlockType(false),
            amOn(false)
        {
            DisableDropArrow();
            SetStyle(GG::LIST_CENTER);
            SetInteriorColor(GG::Clr(0, 0, 0, 200));
            SetNumCols(1);
            //m_quantityBox->SetColWidth(0, GG::X(14));
            //m_quantityBox->LockColWidths();

            int quantInts[] = {1, 5, 10, 20, 50, 99};
            std::set<int> myQuantSet(quantInts,quantInts+6);
            myQuantSet.insert(build.remaining);
            for (std::set<int>::iterator it=myQuantSet.begin(); it != myQuantSet.end(); it++ ) {
                GG::DropDownList::iterator latest_it = Insert(new QuantRow(*it, 0, ClientUI::GetFont(), GG::X1, h, false, amBlockType));
                if (build.remaining == *it)
                    Select(latest_it);
            }
            // set dropheight.  shrink to fit a small number, but cap at a reasonable max
            SetDropHeight(std::min(8, int(myQuantSet.size()))* CUISimpleDropDownListRow::DEFAULT_ROW_HEIGHT +4);
        }

        QuantitySelector(GG::X x, GG::Y y, GG::X w, GG::Y h, GG::Y drop_ht, GG::Clr border_color/* = ClientUI::CtrlBorderColor()*/,
            GG::Clr interior/* = ClientUI::WndColor()*/, GG::Flags<GG::WndFlag> flags/* = INTERACTIVE*/) : 
            CUIDropDownList(GG::X0, GG::Y0, GG::X0, GG::Y0, GG::Y0, GG::Clr(0, 0, 0, 0), GG::FloatClr(0.0, 0.0, 0.0, 0.5),GG::Flags<GG::WndFlag>()),
            quantity(0),
            prevQuant(0),
            blocksize(1),
            prevBlocksize(1),
            amBlockType(false),
            amOn(false)
        {}

        void SelectionChanged(GG::DropDownList::iterator it) {
            int quant;
            //SetInteriorColor(GG::Clr(0, 0, 0, 0));
            amOn=false;
            //Hide();
            //Render();
            Logger().debugStream() << "QuantSelector:  selection made ";
            if (it != end()) {
                quant = boost::polymorphic_downcast<const QuantRow*>(*it)->Quant();
                if (amBlockType) {
                    Logger().debugStream() << "Blocksize Selector:  selection changed to " << quant;
                    blocksize=quant;
                } else {
                    Logger().debugStream() << "Quantity Selector:  selection changed to " << quant;
                    quantity=quant;
                }
            }
        }

    private:
        const ProductionQueue::Element m_build;
        int quantity;
        int prevQuant;
        int blocksize;
        int prevBlocksize;
        bool amBlockType;
        bool amOn;
        GG::Y h;

        void GainingFocus() {
            //amOn = !amOn;
            //if (amOn)
            //Logger().debugStream() << "QuantSelector:  gained focus";
            DropDownList::GainingFocus();
        }

        void LosingFocus() {
            //amOn = !amOn;
            //if (amOn)
            //Logger().debugStream() << "QuantSelector:  lost focus";
            //SetInteriorColor(GG::Clr(0, 0, 0, 0));
            amOn=false;
            DropDownList::LosingFocus();
        }

        void LButtonDown ( const GG::Pt&  pt, GG::Flags< GG::ModKey >  mod_keys  )   {
            //Logger().debugStream() << "QuantSelector:  got a left button down msg";
            if (!amOn ) {
                //SetInteriorColor(ClientUI::KnownTechFillColor());
                amOn=true;
                //Logger().debugStream() << "QuantSelector:  just came on";
                Render();
            } else {
                //SetInteriorColor(GG::Clr(0, 0, 0, 0));
                amOn=false;
                //Logger().debugStream() << "QuantSelector:  just went off";
                Render();
            }
            DropDownList::LButtonDown(pt, mod_keys);
        }

        void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
            Logger().debugStream() << "QuantSelector:  got a left click";

            DropDownList::LClick(pt, mod_keys);
            if ( (quantity != prevQuant) || (blocksize != prevBlocksize) )
                QuantChangedSignal(quantity, blocksize);
        }
    };

    //////////////////////////////////////////////////
    // QueueBuildPanel
    //////////////////////////////////////////////////
    class QueueBuildPanel : public GG::Control {
    public:
        QueueBuildPanel(GG::X w, const ProductionQueue::Element& build, double turn_cost,
                        int turns, int number, int turns_completed, double partially_complete_turn);
        virtual void Render();
        void UpdateQueue();
        void ItemQuantityChanged(int quant, int blocksize) ;
        void ItemBlocksizeChanged(int quant, int blocksize) ;
        mutable boost::signal<void(int,int)>    PanelUpdateQuantSignal;

    private:
        void Draw(GG::Clr clr, bool fill);

        const ProductionQueue::Element  m_build;
        GG::TextControl*                m_name_text;
        GG::TextControl*                m_location_text;
        GG::TextControl*                m_PPs_and_turns_text;
        GG::TextControl*                m_turns_remaining_until_next_complete_text;
        GG::StaticGraphic*              m_icon;
        MultiTurnProgressBar*           m_progress_bar;
        QuantitySelector*               m_quantityBox;
        QuantitySelector*               m_blockBox;
        bool                            m_in_progress;
        int                             m_total_turns;
        int                             m_turns_completed;
        double                          m_partially_complete_turn;
    };

    //////////////////////////////////////////////////
    // QueueRow implementation
    //////////////////////////////////////////////////
    QueueRow::QueueRow(GG::X w, const ProductionQueue::Element& elem, int queue_index_) :
        GG::ListBox::Row(GG::X1, GG::Y1, "PRODUCTION_QUEUE_ROW"),
        queue_index(queue_index_),
        m_build(elem)
    {
        const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
        double total_cost(1.0);
        int minimum_turns(1.0);
        if (empire)
            boost::tie(total_cost, minimum_turns) = empire->ProductionCostAndTime(elem);
        total_cost *= elem.blocksize;
        double per_turn_cost = total_cost / std::max(1, minimum_turns);
        double progress = empire->ProductionStatus(queue_index);
        if (progress == -1.0)
            progress = 0.0;

        QueueBuildPanel* panel = new QueueBuildPanel(w, elem,
                                                     elem.allocated_pp, minimum_turns, elem.remaining,
                                                     static_cast<int>(progress / std::max(1e-6,per_turn_cost)),
                                                     std::fmod(progress, per_turn_cost) / std::max(1e-6,per_turn_cost));
        Resize(panel->Size());
        push_back(panel);

        GG::Connect(panel->PanelUpdateQuantSignal,  &QueueRow::RowQuantChanged, this);
    }

    //////////////////////////////////////////////////
    // QueueBuildPanel implementation
    //////////////////////////////////////////////////
    QueueBuildPanel::QueueBuildPanel(GG::X w, const ProductionQueue::Element& build, double turn_spending, int turns, int number, int turns_completed, double partially_complete_turn) :
        GG::Control(GG::X0, GG::Y0, w, GG::Y(10), GG::Flags<GG::WndFlag>()),
        m_build(build),
        m_name_text(0),
        m_location_text(0),
        m_PPs_and_turns_text(0),
        m_turns_remaining_until_next_complete_text(0),
        m_icon(0),
        m_progress_bar(0),
        m_quantityBox(0),
        m_blockBox(0),
        m_in_progress(build.allocated_pp || build.turns_left_to_next_item==1),
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

        GG::Clr clr = m_in_progress ? GG::LightColor(ClientUI::ResearchableTechTextAndBorderColor()) : ClientUI::ResearchableTechTextAndBorderColor();
        boost::shared_ptr<GG::Font> font = ClientUI::GetFont();

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
            if (w > GG::X(300))
                location_text = boost::io::str(FlexibleFormat(UserString("PRODUCTION_QUEUE_ITEM_LOCATION")) % location->Name())+" ";
        }

        // create and arrange widgets to display info
        GG::Y top(MARGIN);
        GG::X left(MARGIN);

        if (graphic)
            m_icon = new GG::StaticGraphic(left, top, GG::X(GRAPHIC_SIZE), GG::Y(GRAPHIC_SIZE), graphic, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
        else
            m_icon = 0;

        left += GRAPHIC_SIZE + MARGIN;

        if (m_build.item.build_type == BT_SHIP) {
            m_quantityBox = new QuantitySelector(m_build, left, GG::Y(MARGIN), GG::Y(FONT_PTS-2*MARGIN), font, m_in_progress, GG::X(FONT_PTS*2.5), false);
            GG::Connect(m_quantityBox->SelChangedSignal,        &QuantitySelector::SelectionChanged, m_quantityBox);
            left += m_quantityBox->Width();
            m_blockBox = new QuantitySelector(m_build, left,    GG::Y(MARGIN), GG::Y(FONT_PTS-2*MARGIN), font, m_in_progress, GG::X(FONT_PTS*2.5), true);
            GG::Connect(m_blockBox->SelChangedSignal,           &QuantitySelector::SelectionChanged, m_blockBox);
            left += m_blockBox->Width();
        }

        const GG::X NAME_WIDTH = w - left - MARGIN;
        m_name_text = new GG::TextControl(left, top, NAME_WIDTH, GG::Y(FONT_PTS + 2*MARGIN), name_text, font, clr, GG::FORMAT_TOP | GG::FORMAT_LEFT);
        m_name_text->ClipText(true);

        GG::Clr location_clr = clr;
        boost::shared_ptr<GG::Font> location_font = font;
        int client_empire_id = HumanClientApp::GetApp()->EmpireID();
        const Empire* this_client_empire = Empires().Lookup(client_empire_id);
        if (this_client_empire && system_selected) {
            location_font = ClientUI::GetBoldFont(); //
            //location_clr = m_in_progress ? GG::LightColor(this_client_empire->Color()) : this_client_empire->Color();
            location_clr = this_client_empire->Color();
            m_location_text = new ShadowedTextControl(left, top, NAME_WIDTH, GG::Y(FONT_PTS + 2*MARGIN), location_text, location_font, location_clr, GG::FORMAT_TOP | GG::FORMAT_RIGHT);
        } else {
            m_location_text = new GG::TextControl(left, top, NAME_WIDTH, GG::Y(FONT_PTS + 2*MARGIN), location_text, location_font, location_clr, GG::FORMAT_TOP | GG::FORMAT_RIGHT);
        }

        top += m_name_text->Height();
        left = GG::X(GRAPHIC_SIZE + MARGIN*2);
        m_progress_bar = new MultiTurnProgressBar(METER_WIDTH, METER_HEIGHT, turns,
                                                  turns_completed + partially_complete_turn,
                                                  GG::LightColor(ClientUI::TechWndProgressBarBackgroundColor()),
                                                  ClientUI::TechWndProgressBarColor(),
                                                  m_in_progress ? ClientUI::ResearchableTechFillColor() : GG::LightColor(ClientUI::ResearchableTechFillColor()) );
        m_progress_bar->MoveTo(GG::Pt(left, top));

        top += m_progress_bar->Height() + MARGIN;

        std::string turn_spending_text = boost::io::str(FlexibleFormat(UserString("PRODUCTION_TURN_COST_STR")) % DoubleToString(turn_spending, 3, false));
        GG::X TURNS_AND_COST_WIDTH = METER_WIDTH / 2;
        m_PPs_and_turns_text = new GG::TextControl(left, top, TURNS_AND_COST_WIDTH, GG::Y(FONT_PTS + MARGIN),
                                                   turn_spending_text, font, clr, GG::FORMAT_LEFT);

        left += TURNS_AND_COST_WIDTH;

        int turns_left = build.turns_left_to_next_item;
        std::string turns_left_text = turns_left < 0 ? UserString("PRODUCTION_TURNS_LEFT_NEVER") : str(FlexibleFormat(UserString("PRODUCTION_TURNS_LEFT_STR")) % turns_left);
        m_turns_remaining_until_next_complete_text = new GG::TextControl(left, top, TURNS_AND_COST_WIDTH, GG::Y(FONT_PTS + MARGIN),
                                                                         turns_left_text, font, clr, GG::FORMAT_RIGHT);
        m_turns_remaining_until_next_complete_text->ClipText(true);

        if (m_icon)
            AttachChild(m_icon);
        if (m_quantityBox)
            AttachChild(m_quantityBox);
        if (m_blockBox)
            AttachChild(m_blockBox);
        AttachChild(m_name_text);
        AttachChild(m_location_text);
        AttachChild(m_PPs_and_turns_text);
        AttachChild(m_turns_remaining_until_next_complete_text);
        AttachChild(m_progress_bar);
        if (m_quantityBox)
            GG::Connect(m_quantityBox->QuantChangedSignal,          &QueueBuildPanel::ItemQuantityChanged, this);
        if (m_blockBox)
            GG::Connect(m_blockBox->QuantChangedSignal,             &QueueBuildPanel::ItemBlocksizeChanged, this);
    }

    void QueueBuildPanel::ItemQuantityChanged(int quant, int blocksize)
    { PanelUpdateQuantSignal(quant,m_build.blocksize); }

    void QueueBuildPanel::ItemBlocksizeChanged(int quant, int blocksize) // made separate funcion in case wna to do extra checking
    { PanelUpdateQuantSignal(m_build.remaining, blocksize); }

    void QueueBuildPanel::Render() {
        GG::Clr fill = m_in_progress ? GG::LightColor(ClientUI::ResearchableTechFillColor()) : ClientUI::ResearchableTechFillColor();
        GG::Clr text_and_border = m_in_progress ? GG::LightColor(ClientUI::ResearchableTechTextAndBorderColor()) : ClientUI::ResearchableTechTextAndBorderColor();

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

    void QueueBuildPanel::Draw(GG::Clr clr, bool fill) {
        const int CORNER_RADIUS = 7;
        glColor(clr);
        PartlyRoundedRect(UpperLeft(), LowerRight(), CORNER_RADIUS, true, false, true, false, fill);
    }
}
//////////////////////////////////////////////////
// ProdQueueListBox                                  //
//////////////////////////////////////////////////
/** List of starting points for designs, such as empty hulls, existing designs
  * kept by this empire or seen elsewhere in the universe, design template
  * scripts or saved (on disk) designs from previous games. */
class ProdQueueListBox : public QueueListBox {
public:
    ProdQueueListBox(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& drop_type_str, const std::string& prompt_str);

private: 
    void ItemRightClicked(GG::ListBox::iterator it, const GG::Pt& pt);
};

ProdQueueListBox::ProdQueueListBox(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& drop_type_str, const std::string& prompt_str):
    QueueListBox(x, y, w, h, drop_type_str, prompt_str)
{
    GG::Connect(GG::ListBox::RightClickedSignal,     &ProdQueueListBox::ItemRightClicked,    this);
}

/** create popup menu with a Delete Item command to provide same functionality as
 * DoubleClick since under laggy conditions it DoubleClick can have trouble
 * being interpreted correctly (can instead be treated as simply two unrelated left clicks) */
void ProdQueueListBox::ItemRightClicked(GG::ListBox::iterator it, const GG::Pt& pt) {
    GG::MenuItem menu_contents;
    menu_contents.next_level.push_back(GG::MenuItem(UserString("DELETE_QUEUE_ITEM"), 1, false, false));
    GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents, GG::CLR_RED,
                        ClientUI::WndOuterBorderColor(), ClientUI::WndColor());
    if (popup.Run()) {
        switch (popup.MenuID()) {
        case 1: { // delete item
            // emit a signal so that the ProductionWnd can take necessary steps
            DoubleClickedSignal(it);
            break;
        }

        default:
            break;
        }
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
    m_enabled(false)
{
    Logger().debugStream() << "ProductionWindow:  app-width: "<< GetOptionsDB().Get<int>("app-width") << " ; windowed width: " << GetOptionsDB().Get<int>("app-width-windowed");

    if (GetOptionsDB().Get<int>("app-width-windowed") <= 1280) {
        m_production_info_panel = new ProductionInfoPanel(PRODUCTION_INFO_AND_QUEUE_WIDTH, GG::Y(200), UserString("PRODUCTION_INFO_PANEL_TITLE"), UserString("PRODUCTION_INFO_PP"),
                                                          static_cast<GLfloat>(OUTER_LINE_THICKNESS), ClientUI::KnownTechFillColor(), ClientUI::KnownTechTextAndBorderColor());
    } else {
        m_production_info_panel = new ProductionInfoPanel(WIDE_PRODUCTION_INFO_AND_QUEUE_WIDTH, GG::Y(200), UserString("PRODUCTION_INFO_PANEL_TITLE"), UserString("PRODUCTION_INFO_PP"),
                                                          static_cast<GLfloat>(OUTER_LINE_THICKNESS), ClientUI::KnownTechFillColor(), ClientUI::KnownTechTextAndBorderColor());
    }

    m_queue_lb = new ProdQueueListBox(GG::X(2), m_production_info_panel->LowerRight().y, m_production_info_panel->Width() - 4,
                                  ClientSize().y - 4 - m_production_info_panel->Height(), "PRODUCTION_QUEUE_ROW", UserString("PRODUCTION_QUEUE_PROMPT"));
    m_queue_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_NOSEL | GG::LIST_USERDELETE);

    GG::Pt buid_designator_wnd_size = ClientSize() - GG::Pt(m_production_info_panel->Width(), GG::Y0);
    m_build_designator_wnd = new BuildDesignatorWnd(buid_designator_wnd_size.x, buid_designator_wnd_size.y);
    m_build_designator_wnd->MoveTo(GG::Pt(m_production_info_panel->Width(), GG::Y0));

    SetChildClippingMode(ClipToClient);

    GG::Connect(m_build_designator_wnd->AddNamedBuildToQueueSignal,     &ProductionWnd::AddBuildToQueueSlot, this);
    GG::Connect(m_build_designator_wnd->AddIDedBuildToQueueSignal,      &ProductionWnd::AddBuildToQueueSlot, this);
    GG::Connect(m_build_designator_wnd->BuildQuantityChangedSignal,     &ProductionWnd::ChangeBuildQuantitySlot, this);
    GG::Connect(m_build_designator_wnd->SystemSelectedSignal,           SystemSelectedSignal);
    GG::Connect(m_queue_lb->QueueItemMoved,                             &ProductionWnd::QueueItemMoved, this);
    GG::Connect(m_queue_lb->LeftClickedSignal,                          &ProductionWnd::QueueItemClickedSlot, this);
    GG::Connect(m_queue_lb->DoubleClickedSignal,                        &ProductionWnd::QueueItemDoubleClickedSlot, this);

    AttachChild(m_production_info_panel);
    AttachChild(m_queue_lb);
    AttachChild(m_build_designator_wnd);
}

ProductionWnd::~ProductionWnd()
{ m_empire_connection.disconnect(); }

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
    EmpireManager& manager = HumanClientApp::GetApp()->Empires();
    if (Empire* empire = manager.Lookup(HumanClientApp::GetApp()->EmpireID()))
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
    HumanClientApp::GetApp()->Orders().IssueOrder(
        OrderPtr(new ProductionQueueOrder(HumanClientApp::GetApp()->EmpireID(),
                                          boost::polymorphic_downcast<QueueRow*>(row)->queue_index,
                                          position)));
    Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (empire)
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
    Logger().debugStream() << "ProductionWnd::UpdateQueue()";
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
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
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
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
    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new ProductionQueueOrder(HumanClientApp::GetApp()->EmpireID(), build_type, name, number, location)));
    Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (empire)
        empire->UpdateProductionQueue();
    m_build_designator_wnd->CenterOnBuild(m_queue_lb->NumRows() - 1);
}

void ProductionWnd::AddBuildToQueueSlot(BuildType build_type, int design_id, int number, int location) {
    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new ProductionQueueOrder(HumanClientApp::GetApp()->EmpireID(), build_type, design_id, number, location)));
    Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (empire)
        empire->UpdateProductionQueue();
    m_build_designator_wnd->CenterOnBuild(m_queue_lb->NumRows() - 1);
}

void ProductionWnd::ChangeBuildQuantitySlot(int queue_idx, int quantity) {
    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new ProductionQueueOrder(HumanClientApp::GetApp()->EmpireID(), queue_idx, quantity, true)));
    Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (empire)
        empire->UpdateProductionQueue();
}

void ProductionWnd::ChangeBuildQuantityBlockSlot(int queue_idx, int quantity, int blocksize) {
    HumanClientApp::GetApp()->Orders().IssueOrder(OrderPtr(new ProductionQueueOrder(HumanClientApp::GetApp()->EmpireID(), queue_idx, quantity, blocksize)));
    Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (empire)
        empire->UpdateProductionQueue();
}

void ProductionWnd::DeleteQueueItem(GG::ListBox::iterator it) {
    HumanClientApp::GetApp()->Orders().IssueOrder(
        OrderPtr(new ProductionQueueOrder(HumanClientApp::GetApp()->EmpireID(),
                                          std::distance(m_queue_lb->begin(), it))));
    Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (empire)
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
    m_enabled = enable;
    m_queue_lb->EnableOrderIssuing(m_enabled);
}
