//AnimatedGraphic.cpp

#include "AnimatedGraphic.h"
#include "GGDrawUtil.h"

/*AnimatedGraphic::AnimatedGraphic()
{
}//AnimatedGraphic()
*/

//AnimatedGraphic::AnimatedGraphic(int x, int y, int w, int h, AnimatedTexture* texture, Uint32 style, Uint32 flags):
AnimatedGraphic::AnimatedGraphic(int x, int y, int w, int h, boost::shared_ptr<AnimatedTexture> texture, Uint32 style, Uint32 flags):
    GG::Control(x,y,w,h,flags),
    m_style(style)
{
    m_anim_graphic=texture;
    ValidateStyle();
    SetColor(GG::CLR_WHITE);
}//AnimatedGraphic(int, int, int, int, shared_ptr, Uint32, Uint32)

AnimatedGraphic::~AnimatedGraphic()
{

}

int AnimatedGraphic::Render()
{
    using namespace GG;
//ripped from GG::StaticGraphic
   Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
   glColor4ubv(color_to_use.v);

   Pt ul = UpperLeft(), lr = LowerRight();
   Pt window_sz(lr - ul);
   Pt graphic_sz(m_anim_graphic->Width(), m_anim_graphic->Height());
   Pt pt1, pt2(graphic_sz); // (unscaled) default graphic size
   if (m_style & SG_FITGRAPHIC) {
      if (m_style & SG_PROPSCALE) {
         double scale_x = window_sz.x / double(graphic_sz.x), 
                scale_y = window_sz.y / double(graphic_sz.y);
         double scale = (scale_x < scale_y) ? scale_x : scale_y;
         pt2.x = int(graphic_sz.x * scale);
         pt2.y = int(graphic_sz.y * scale);
      } else {
         pt2 = window_sz;
      }
   } else if (m_style & SG_SHRINKFIT) {
      if (m_style & SG_PROPSCALE) {
         double scale_x = (graphic_sz.x > window_sz.x) ? window_sz.x / double(graphic_sz.x) : 1.0, 
                scale_y = (graphic_sz.y > window_sz.y) ? window_sz.y / double(graphic_sz.y) : 1.0;
         double scale = (scale_x < scale_y) ? scale_x : scale_y;
         pt2.x = int(graphic_sz.x * scale);
         pt2.y = int(graphic_sz.y * scale);
      } else {
         pt2 = window_sz;
      }
   }

   int shift = 0;
   if (m_style & SG_LEFT) {
      shift = ul.x;
   } else if (m_style & SG_CENTER) {
      shift = ul.x + (window_sz.x - (pt2.x - pt1.x)) / 2;
   } else { // m_style & SG_RIGHT
      shift = lr.x - (pt2.x - pt1.x);
   }
   pt1.x += shift;
   pt2.x += shift;

   if (m_style & SG_TOP) {
      shift = ul.y;
   } else if (m_style & SG_VCENTER) {
      shift = ul.y + (window_sz.y - (pt2.y - pt1.y)) / 2;
   } else { // m_style & SG_BOTTOM
      shift = lr.y - (pt2.y - pt1.y);
   }
   pt1.y += shift;
   pt2.y += shift;

   m_anim_graphic->OrthoBlit(pt1, pt2, false);

   return 1;
}

void AnimatedGraphic::ValidateStyle()
{
//from GG::StaticGraphic
    using namespace GG;
    
   int dup_ct = 0;   // duplication count
   if (m_style & SG_LEFT) ++dup_ct;
   if (m_style & SG_RIGHT) ++dup_ct;
   if (m_style & SG_CENTER) ++dup_ct;
   if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use SG_CENTER by default
      m_style &= ~(SG_RIGHT | SG_LEFT);
      m_style |= SG_CENTER;
   }
   dup_ct = 0;
   if (m_style & SG_TOP) ++dup_ct;
   if (m_style & SG_BOTTOM) ++dup_ct;
   if (m_style & SG_VCENTER) ++dup_ct;
   if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use SG_VCENTER by default
      m_style &= ~(SG_TOP | SG_BOTTOM);
      m_style |= SG_VCENTER;
   }
   dup_ct = 0;
   if (m_style & SG_FITGRAPHIC) ++dup_ct;
   if (m_style & SG_SHRINKFIT) ++dup_ct;
   if (dup_ct > 1) {   // mo more than one may be picked; when both are picked, use SG_SHRINKFIT by default
      m_style &= ~SG_FITGRAPHIC;
      m_style |= SG_SHRINKFIT;
   }
}


