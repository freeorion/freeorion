#include "LinkText.h"

#include <boost/lexical_cast.hpp>

// initialize static(s)
bool LinkText::s_link_tags_registered = false;


LinkText::LinkText(int x, int y, int w, int h, const std::string& str, const boost::shared_ptr<GG::Font>& font, 
                   Uint32 text_fmt/* = 0*/, GG::Clr color/* = GG::CLR_BLACK*/, Uint32 flags/* = 0*/) : 
   GG::DynamicText(x, y, w, h, str, font, text_fmt, color, flags),
   m_old_sel_link(-1),
   m_old_rollover_link(-1)
{
   Init();
}

LinkText::LinkText(int x, int y, int w, int h, const std::string& str, const std::string& font_filename, int pts, 
                   Uint32 text_fmt/* = 0*/, GG::Clr color/* = GG::CLR_BLACK*/, Uint32 flags/* = 0*/) : 
   GG::DynamicText(x, y, w, h, str, font_filename, pts, text_fmt, color, flags),
   m_old_sel_link(-1),
   m_old_rollover_link(-1)
{
   Init();
}

LinkText::LinkText(int x, int y, const std::string& str, const boost::shared_ptr<GG::Font>& font, 
                   GG::Clr color/* = GG::CLR_BLACK*/, Uint32 flags/* = 0*/) : 
   GG::DynamicText(x, y, str, font, color, flags),
   m_old_sel_link(-1),
   m_old_rollover_link(-1)
{
   Init();
}

LinkText::LinkText(int x, int y, const std::string& str, const std::string& font_filename, int pts, 
                   GG::Clr color/* = GG::CLR_BLACK*/, Uint32 flags/* = 0*/) : 
   GG::DynamicText(x, y, str, font_filename, pts, color, flags),
   m_old_sel_link(-1),
   m_old_rollover_link(-1)
{
   Init();
}

LinkText::LinkText(const GG::XMLElement& elem) : 
   GG::DynamicText(elem.Child("GG::DynamicText")),
   m_old_sel_link(-1),
   m_old_rollover_link(-1)
{
   if (elem.Tag() != "LinkText")
      throw std::invalid_argument("Attempted to construct a LinkText from an XMLElement that had a tag other than \"LinkText\"");
   
   FindLinks();
}

int LinkText::LButtonDown(const GG::Pt& pt, Uint32 keys)
{
   m_old_sel_link = GetLinkUnderPt(pt);
   return 1;
}

int LinkText::LButtonUp(const GG::Pt& pt, Uint32 keys)
{
   m_old_sel_link = -1;
   return 1;
}

int LinkText::LClick(const GG::Pt& pt, Uint32 keys)
{
   int sel_link = GetLinkUnderPt(pt);
   if (sel_link == m_old_sel_link && sel_link != -1) {
      if (m_links[sel_link].type == "planet") {
         m_planet_sig(boost::lexical_cast<int>(m_links[sel_link].data));
      } else if (m_links[sel_link].type == "system") {
         m_system_sig(boost::lexical_cast<int>(m_links[sel_link].data));
      } else if (m_links[sel_link].type == "fleet") {
         m_fleet_sig(boost::lexical_cast<int>(m_links[sel_link].data));
      } else if (m_links[sel_link].type == "ship") {
         m_ship_sig(boost::lexical_cast<int>(m_links[sel_link].data));
      } else if (m_links[sel_link].type == "tech") {
         m_tech_sig(boost::lexical_cast<int>(m_links[sel_link].data));
      } else if (m_links[sel_link].type == "encyclopedia") {
         m_encyclopedia_sig(m_links[sel_link].data);
      }
   }
   m_old_sel_link = -1;
   return 1;
}

int LinkText::MouseHere(const GG::Pt& pt, Uint32 keys)
{
   int rollover_link = GetLinkUnderPt(pt);
   if (rollover_link != m_old_rollover_link) {
      ClearOldRollover();
      if (rollover_link != -1) {
         std::string text = WindowText();
         text.insert(m_links[rollover_link].text_posn.first, "<u>");
         text.insert(m_links[rollover_link].text_posn.second + 3, "</u>");
         SetText(text);
         m_old_rollover_link = rollover_link;
      }
   }
   return 1;
}

int LinkText::MouseLeave(const GG::Pt& pt, Uint32 keys)
{
   ClearOldRollover();
}
   
void LinkText::SetText(const std::string& str)
{
   GG::DynamicText::SetText(str);
   FindLinks();
}

GG::XMLElement LinkText::XMLEncode() const
{
   GG::XMLElement retval("LinkText");
   retval.AppendChild(GG::DynamicText::XMLEncode());
   return retval;
}

void LinkText::Init()
{
   // if this is the first LinkText created, it needs to register the tags that it knows about
   if (!s_link_tags_registered) {
      GG::Font::RegisterKnownTag("planet");
      GG::Font::RegisterKnownTag("system");
      GG::Font::RegisterKnownTag("fleet");
      GG::Font::RegisterKnownTag("ship");
      GG::Font::RegisterKnownTag("tech");
      GG::Font::RegisterKnownTag("encyclopedia");
      s_link_tags_registered = true;
      SetText(WindowText());
   } else {
      FindLinks();
   }
}

void LinkText::FindLinks()
{
   m_links.clear();

   bool ignore_tags = false; // this toggles true/false based on the presence of <pre> and </pre> tags

   const std::string& text = WindowText();
   const std::vector<GG::Font::LineData>& line_data = GetLineData();
   const boost::shared_ptr<GG::Font>& font = GetFont();

   int y_posn = 0; // y-coordinate of the top of the current line
   Link link;

   for (std::vector<GG::Font::LineData>::const_iterator cit = line_data.begin(); 
        cit != line_data.end();
        ++cit, y_posn += font->Lineskip()) {
      const GG::Font::LineData& curr_line = *cit;
      
      // if the last line ended without the current tag ending
      if (link.type != "") {
         link.rects.push_back(GG::Rect(0, y_posn, 0, y_posn + font->Height()));
      }

      for (int i = curr_line.begin_idx; i < curr_line.end_idx; ++i) {
         GG::Font::Tag tag;
         GG::Font::FindFormatTag(text, i, tag, ignore_tags);
         if (tag.char_length) {
            if (tag.tokens[0] == "pre") {
               ignore_tags = !tag.close_tag;
            } else if (!ignore_tags && 
                       (tag.tokens[0] == "planet" || tag.tokens[0] == "system" || tag.tokens[0] == "fleet" || 
                       tag.tokens[0] == "ship" || tag.tokens[0] == "tech" || tag.tokens[0] == "encyclopedia")) {
               link.type = tag.tokens[0];
               if (!tag.close_tag) {
                  link.data = tag.tokens[1];
                  link.text_posn.first = i + tag.char_length;
                  link.rects.push_back(GG::Rect((link.text_posn.first - curr_line.begin_idx) ? 
                                                curr_line.extents[link.text_posn.first - curr_line.begin_idx - 1] : 0,
                                                y_posn,
                                                0,
                                                y_posn + font->Height()));
               } else {
                  link.text_posn.second = i;
                  link.rects.back().lr.x = (link.text_posn.second - curr_line.begin_idx) ? 
                                           curr_line.extents[link.text_posn.second - curr_line.begin_idx - 1] : 0;
                  m_links.push_back(link);
                  link = Link();
               }
            }
            i += tag.char_length - 1; // "- 1" because i gets incremented at the end of the loop iteration
         }
      }

      // if a line is ending without the current tag ending
      if (link.type != "") {
         link.rects.back().lr.x = curr_line.extents.empty() ? 0 : curr_line.extents.back();
      }
   }
}

int LinkText::GetLinkUnderPt(const GG::Pt& pt)
{
   int retval = -1;
   for (unsigned int i = 0; i < m_links.size(); ++i) {
      for (unsigned int j = 0; j < m_links[i].rects.size(); ++j) {
         if (m_links[i].rects[j].Contains(ScreenToWindow(pt))) {
            retval = i;
            break;
         }
      }
   }
   return retval;
}

void LinkText::ClearOldRollover()
{
   if (m_old_rollover_link != -1) {
      std::string text = WindowText();
      std::string::size_type pos = text.find("<u>", m_links[m_old_rollover_link].text_posn.first);
      text.erase(pos, 3);
      pos = text.rfind("</u>", m_links[m_old_rollover_link].text_posn.second - 3);
      text.erase(pos, 4);
      SetText(text);
   }
   m_old_rollover_link = -1;
}

