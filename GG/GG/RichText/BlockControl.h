#ifndef BLOCKCONTROL_H
#define BLOCKCONTROL_H

#include <GG/Control.h>

namespace GG {

/** \brief BlockControl is an abstract base class for controls that can determine their height
 * when you set their width.
 *
 * BlockControls are used for embedding controls in text.
 */
class GG_API BlockControl : public Control
{
public:
    //! Create a block control.
    BlockControl(X x, Y y, X w, GG::Flags<GG::WndFlag> flags);

    //! Set the maximum width of the block control. Returns the size based on the width.
    virtual Pt SetMaxWidth (X width) = 0;

    //! Redirect size move to setmaxwidth.
    virtual void SizeMove (const Pt& ul, const Pt& lr);
};

}

#endif // BLOCKCONTROL_H
