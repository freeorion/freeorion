#ifndef _QueueListBox_h_
#define _QueueListBox_h_

#include "CUIControls.h"

/** A list box type for representing queues (eg the research and production queues). */
class QueueListBox :
    public CUIListBox
{
public:
    QueueListBox(const std::string& drop_type_str, const std::string& prompt_str);

    void Render() override;

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void AcceptDrops(const GG::Pt& pt, const std::vector<GG::Wnd*>& wnds, GG::Flags<GG::ModKey> mod_keys) override;

    void DragDropHere(const GG::Pt& pt, std::map<const GG::Wnd*, bool>& drop_wnds_acceptable,
                      GG::Flags<GG::ModKey> mod_keys) override;

    void DragDropLeave() override;

    GG::X           RowWidth() const;

    virtual void    KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys);

    virtual void    EnableOrderIssuing(bool enable = true);
    bool            OrderIssuingEnabled() const { return m_order_issuing_enabled; }
    bool            DisplayingValidQueueItems(); ///< whether or not this QueueListBox is displaying valid queue items, as opposed to, for example, a prompt for the user to enter an item

    void            Clear();

    boost::signals2::signal<void (GG::ListBox::Row*, std::size_t)>  QueueItemMovedSignal;
    boost::signals2::signal<void (GG::ListBox::iterator)>           QueueItemDeletedSignal;

protected:
    void KeyPress(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;

    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) const override;

    virtual void    ItemRightClickedImpl(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys);

    /** Return a functor that will signal that \p it should be moved to the top of the list.*/
    virtual std::function<void()> MoveToTopAction(GG::ListBox::iterator it) const;

    /** Return a functor that will signal that \p it should be moved to the bottom of the list.*/
    virtual std::function<void()> MoveToBottomAction(GG::ListBox::iterator it) const;

    /** Return a functor that will signal that \p it should be deleted.*/
    virtual std::function<void()> DeleteAction(GG::ListBox::iterator it) const;

private:
    void            ItemRightClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys);
    void            EnsurePromptHiddenSlot(iterator it);
    void            ShowPromptSlot();
    void            ShowPromptConditionallySlot(iterator it);

    iterator    m_drop_point;
    bool        m_show_drop_point;
    bool        m_order_issuing_enabled;
    bool        m_showing_prompt;
    std::string m_prompt_str;
};

#endif
