#ifndef _QueueListBox_h_
#define _QueueListBox_h_

#include <boost/optional/optional.hpp>

#include "CUIControls.h"

/** A simple ListBox row containing only a static label.*/
struct PromptRow : GG::ListBox::Row {
    PromptRow(GG::X w, const std::string& prompt_str);
    void CompleteConstruction() override;
    void SizeMove(GG::Pt ul, GG::Pt lr) override;

private:
     std::shared_ptr<GG::Label> m_prompt;
};

/** A list box type for representing queues (eg the research and production queues). */
class QueueListBox :
    public CUIListBox
{
public:
    QueueListBox(boost::optional<std::string_view> drop_type_str, std::string prompt_str);

    void CompleteConstruction() override;

    void Render() override;
    void SizeMove(GG::Pt ul, GG::Pt lr) override;
    void AcceptDrops(GG::Pt pt, std::vector<std::shared_ptr<GG::Wnd>> wnds, GG::Flags<GG::ModKey> mod_keys) override;
    void DragDropHere(GG::Pt pt, std::map<const Wnd*, bool>& drop_wnds_acceptable, GG::Flags<GG::ModKey> mod_keys) override;
    void DragDropLeave() override;

    GG::X           RowWidth() const noexcept;

    //! scans this ListBox for the input iterator \a and returns its distance
    //! from begin(), or -1 if not present
    int             IteraterIndex(const const_iterator it);

    virtual void    EnableOrderIssuing(bool enable = true);
    bool            OrderIssuingEnabled() const noexcept { return m_order_issuing_enabled; }
    bool            DisplayingValidQueueItems() const noexcept; ///< whether or not this QueueListBox is displaying valid queue items, as opposed to, for example, a prompt for the user to enter an item

    void            Clear();

    /** Change the empty list prompt text. */
    void            SetEmptyPromptText(std::string prompt);

    boost::signals2::signal<void (GG::ListBox::iterator)> QueueItemDeletedSignal;

protected:
    void KeyPress(GG::Key key, uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;

    virtual void ItemRightClickedImpl(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys);

    /** Return a functor that will signal that \p it should be moved to the top of the list.*/
    virtual std::function<void()> MoveToTopAction(GG::ListBox::iterator it);

    /** Return a functor that will signal that \p it should be moved to the bottom of the list.*/
    virtual std::function<void()> MoveToBottomAction(GG::ListBox::iterator it);

    /** Return a functor that will signal that \p it should be deleted.*/
    virtual std::function<void()> DeleteAction(GG::ListBox::iterator it) const;

private:
    void ItemRightClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys);
    void EnsurePromptHiddenSlot(iterator it);
    void ShowPromptSlot();
    void ShowPromptConditionallySlot(iterator it);

    iterator    m_drop_point;
    bool        m_show_drop_point = false;
    bool        m_order_issuing_enabled = true;
    bool        m_showing_prompt = false;
    std::string m_prompt_str;
};

#endif
