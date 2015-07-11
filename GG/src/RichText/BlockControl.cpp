/** \file \brief The implementation of BlockControl.
 */

#include <GG/RichText/BlockControl.h>

namespace GG {

BlockControl::BlockControl(X x, Y y, X w, GG::Flags< GG::WndFlag > flags):
    Control(x, y, w, Y0, flags)
{

}

void BlockControl::SizeMove(Pt ul, Pt lr){
    Pt previous_ul = UpperLeft();
    Pt previous_lr = LowerRight();

    X previous_width = previous_lr.x - previous_ul.x;
    X new_width = lr.x - ul.x;

    Control::SizeMove( ul, lr );
    
    if( new_width != previous_width ){
        SetMaxWidth( lr.x - ul.x );
    }
}

}
