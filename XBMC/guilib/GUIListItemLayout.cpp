/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "include.h"
#include "GUIListItemLayout.h"
#include "FileItem.h"
#include "GUIControlFactory.h"
#include "SkinInfo.h"
#include "utils/GUIInfoManager.h"
#include "GUIListLabel.h"
#include "GUIImage.h"

using namespace std;

CGUIListItemLayout::CGUIListItemLayout()
: m_group(0, 0, 0, 0, 0, 0)
{
  m_width = 0;
  m_height = 0;
  m_condition = 0;
  m_focused = false;
  m_invalidated = true;
  m_isPlaying = false;
}

CGUIListItemLayout::CGUIListItemLayout(const CGUIListItemLayout &from)
: m_group(from.m_group)
{
  m_width = from.m_width;
  m_height = from.m_height;
  m_focused = from.m_focused;
  m_condition = from.m_condition;
  m_invalidated = true;
  m_isPlaying = false;
}

CGUIListItemLayout::~CGUIListItemLayout()
{
}

bool CGUIListItemLayout::IsAnimating(ANIMATION_TYPE animType)
{
  return m_group.IsAnimating(animType);
}

void CGUIListItemLayout::ResetAnimation(ANIMATION_TYPE animType)
{
  return m_group.ResetAnimation(animType);
}

float CGUIListItemLayout::Size(ORIENTATION orientation) const
{
  return (orientation == HORIZONTAL) ? m_width : m_height;
}

void CGUIListItemLayout::Render(CGUIListItem *item, DWORD parentID, DWORD time)
{
  if (m_invalidated)
  { // need to update our item
    // could use a dynamic cast here if RTTI was enabled.  As it's not,
    // let's use a static cast with a virtual base function
    CFileItem *fileItem = item->IsFileItem() ? (CFileItem *)item : new CFileItem(*item);
    m_isPlaying = g_infoManager.GetBool(LISTITEM_ISPLAYING, parentID, item);
    m_group.UpdateInfo(fileItem);
    m_invalidated = false;
    // delete our temporary fileitem
    if (!item->IsFileItem())
      delete fileItem;
  }

  // update visibility, and render
  m_group.SetState(item->IsSelected() || m_isPlaying, m_focused);
  m_group.UpdateVisibility(item);
  m_group.DoRender(time);
}

void CGUIListItemLayout::SetFocusedItem(unsigned int focus)
{
  m_group.SetFocusedItem(focus);
}

unsigned int CGUIListItemLayout::GetFocusedItem() const
{
  return m_group.GetFocusedItem();
}

void CGUIListItemLayout::SelectItemFromPoint(const CPoint &point)
{
  m_group.SelectItemFromPoint(point);
}

bool CGUIListItemLayout::MoveLeft()
{
  return m_group.MoveLeft();
}

bool CGUIListItemLayout::MoveRight()
{
  return m_group.MoveRight();
}

void CGUIListItemLayout::LoadControl(TiXmlElement *child, CGUIControlGroup *group)
{
  if (!group) return;

  FRECT rect = { group->GetXPosition(), group->GetYPosition(), group->GetXPosition() + group->GetWidth(), group->GetYPosition() + group->GetHeight() };

  CGUIControlFactory factory;
  CGUIControl *control = factory.Create(0, rect, child, true);  // true indicating we're inside a list for the
                                                                // different label control + defaults.
  if (control)
  {
    group->AddControl(control);
    if (control->IsGroup())
    {
      TiXmlElement *grandChild = child->FirstChildElement("control");
      while (grandChild)
      {
        LoadControl(grandChild, (CGUIControlGroup *)control);
        grandChild = grandChild->NextSiblingElement("control");
      }
    }
  }
}

void CGUIListItemLayout::LoadLayout(TiXmlElement *layout, bool focused)
{
  m_focused = focused;
  g_SkinInfo.ResolveIncludes(layout);
  g_SkinInfo.ResolveConstant(layout->Attribute("width"), m_width);
  g_SkinInfo.ResolveConstant(layout->Attribute("height"), m_height);
  const char *condition = layout->Attribute("condition");
  if (condition)
    m_condition = g_infoManager.TranslateString(condition);
  TiXmlElement *child = layout->FirstChildElement("control");
  m_group.SetWidth(m_width);
  m_group.SetHeight(m_height);
  while (child)
  {
    LoadControl(child, &m_group);
    child = child->NextSiblingElement("control");
  }
}

//#ifdef PRE_SKIN_VERSION_2_1_COMPATIBILITY
void CGUIListItemLayout::CreateListControlLayouts(float width, float height, bool focused, const CLabelInfo &labelInfo, const CLabelInfo &labelInfo2, const CTextureInfo &texture, const CTextureInfo &textureFocus, float texHeight, float iconWidth, float iconHeight, int nofocusCondition, int focusCondition)
{
  m_width = width;
  m_height = height;
  m_focused = focused;
  CGUIImage *tex = new CGUIImage(0, 0, 0, 0, width, texHeight, texture);
  tex->SetVisibleCondition(nofocusCondition, false);
  m_group.AddControl(tex);
  if (focused)
  {
    CGUIImage *tex = new CGUIImage(0, 0, 0, 0, width, texHeight, textureFocus);
    tex->SetVisibleCondition(focusCondition, false);
    m_group.AddControl(tex);
  }
  CGUIImage *image = new CGUIImage(0, 0, 8, 0, iconWidth, texHeight, CTextureInfo(""));
  image->SetInfo(CGUIInfoLabel("$INFO[ListItem.Icon]"));
  image->SetAspectRatio(CAspectRatio::AR_KEEP);
  m_group.AddControl(image);
  float x = iconWidth + labelInfo.offsetX + 10;
  CGUIListLabel *label = new CGUIListLabel(0, 0, x, labelInfo.offsetY, width - x - 18, height, labelInfo, CGUIInfoLabel("$INFO[ListItem.Label]"), false, CScrollInfo::defaultSpeed);
  m_group.AddControl(label);
  x = labelInfo2.offsetX ? labelInfo2.offsetX : m_width - 16;
  label = new CGUIListLabel(0, 0, x, labelInfo2.offsetY, x - iconWidth - 20, height, labelInfo2, CGUIInfoLabel("$INFO[ListItem.Label2]"), false, CScrollInfo::defaultSpeed);
  m_group.AddControl(label);
}

void CGUIListItemLayout::CreateThumbnailPanelLayouts(float width, float height, bool focused, const CTextureInfo &image, float texWidth, float texHeight, float thumbPosX, float thumbPosY, float thumbWidth, float thumbHeight, DWORD thumbAlign, const CAspectRatio &thumbAspect, const CLabelInfo &labelInfo, bool hideLabels)
{
  m_width = width;
  m_height = height;
  m_focused = focused;
  float centeredPosX = (m_width - texWidth)*0.5f;
  CGUIImage *tex = new CGUIImage(0, 0, centeredPosX, 0, texWidth, texHeight, image);
  m_group.AddControl(tex);
  // thumbnail
  float xOff = 0;
  float yOff = 0;
  if (thumbAlign != 0)
  {
    xOff += (texWidth - thumbWidth) * 0.5f;
    yOff += (texHeight - thumbHeight) * 0.5f;
    //if thumbPosX or thumbPosX != 0 the thumb will be bumped off-center
  }
  CGUIImage *thumb = new CGUIImage(0, 0, thumbPosX + centeredPosX + xOff, thumbPosY + yOff, thumbWidth, thumbHeight, CTextureInfo(""));
  thumb->SetInfo(CGUIInfoLabel("$INFO[ListItem.Icon]"));
  thumb->SetAspectRatio(thumbAspect);
  m_group.AddControl(thumb);
  // overlay
  CGUIImage *overlay = new CGUIImage(0, 0, thumbPosX + centeredPosX + xOff + thumbWidth - 32, thumbPosY + yOff + thumbHeight - 32, 32, 32, CTextureInfo(""));
  overlay->SetInfo(CGUIInfoLabel("$INFO[ListItem.Overlay]"));
  overlay->SetAspectRatio(thumbAspect);
  m_group.AddControl(overlay);
  // label
  if (hideLabels) return;
  CGUIListLabel *label = new CGUIListLabel(0, 0, width*0.5f, texHeight, width, height, labelInfo, CGUIInfoLabel("$INFO[ListItem.Label]"), false, CScrollInfo::defaultSpeed);
  m_group.AddControl(label);
}
//#endif

#ifdef _DEBUG
void CGUIListItemLayout::DumpTextureUse()
{
  m_group.DumpTextureUse();
}
#endif
