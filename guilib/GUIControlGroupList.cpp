#include "include.h"
#include "GUIControlGroupList.h"

#define TIME_TO_SCROLL 200;

CGUIControlGroupList::CGUIControlGroupList(DWORD dwParentID, DWORD dwControlId, float posX, float posY, float width, float height, float itemGap, DWORD pageControl, ORIENTATION orientation, bool useControlPositions)
: CGUIControlGroup(dwParentID, dwControlId, posX, posY, width, height)
{
  m_itemGap = itemGap;
  m_pageControl = pageControl;
  m_offset = 0;
  m_totalSize = 10;
  m_orientation = orientation;
  m_scrollOffset = 0;
  m_scrollSpeed = 0;
  m_useControlPositions = useControlPositions;
  ControlType = GUICONTROL_GROUPLIST;
}

CGUIControlGroupList::~CGUIControlGroupList(void)
{
}

void CGUIControlGroupList::Render()
{
  if (IsVisible())
  {
    if (m_scrollSpeed < 0)
    {
      m_offset += m_scrollSpeed * (m_renderTime - m_scrollTime);
      if (m_offset < m_scrollOffset)
      {
        m_offset = m_scrollOffset;
        m_scrollSpeed = 0;
      }
    }
    if (m_scrollSpeed > 0)
    {
      m_offset += m_scrollSpeed * (m_renderTime - m_scrollTime);
      if (m_offset > m_scrollOffset)
      {
        m_offset = m_scrollOffset;
        m_scrollSpeed = 0;
      }
    }
    m_scrollTime = m_renderTime;

    ValidateOffset();
    if (m_pageControl)
    {
      CGUIMessage message(GUI_MSG_LABEL_RESET, GetParentID(), m_pageControl, (DWORD)m_height, (DWORD)m_totalSize);
      SendWindowMessage(message);
      CGUIMessage message2(GUI_MSG_ITEM_SELECT, GetParentID(), m_pageControl, (DWORD)m_offset);
      SendWindowMessage(message2);
    }
    // we run through the controls, rendering as we go
    bool render(g_graphicsContext.SetViewPort(m_posX, m_posY, m_width, m_height));
    float pos = 0;
    for (iControls it = m_children.begin(); it != m_children.end(); ++it)
    {
      CGUIControl *control = *it;
      control->UpdateEffectState(m_renderTime);
      if (control->IsVisible() && render)
      {
        if (pos + Size(control) > m_offset && pos < m_offset + Size())
        { // we can render
          if (m_orientation == VERTICAL)
            g_graphicsContext.AddGroupTransform(TransformMatrix::CreateTranslation(m_posX, m_posY + pos - m_offset));
          else
            g_graphicsContext.AddGroupTransform(TransformMatrix::CreateTranslation(m_posX + pos - m_offset, m_posY));
          control->Render();
          g_graphicsContext.RemoveGroupTransform();
        }
        pos += Size(control) + m_itemGap;
      }
    }
    if (render) g_graphicsContext.RestoreViewPort();
    CGUIControl::Render();
  }
  g_graphicsContext.RemoveGroupTransform();
}

bool CGUIControlGroupList::OnMessage(CGUIMessage& message)
{
  switch (message.GetMessage() )
  {
  case GUI_MSG_FOCUSED:
    { // a control has been focused
      // scroll if we need to and update our page control
      ValidateOffset();
      float offset = 0;
      bool done = false;
      for (iControls it = m_children.begin(); it != m_children.end(); ++it)
      {
        CGUIControl *control = *it;
        if (!control->IsVisible())
          continue;
        if (control->GetID() == message.GetControlId())
        {
          if (offset < m_offset)
            ScrollTo(offset);
          else if (offset + Size(control) > m_offset + Size())
            ScrollTo(offset + Size(control) - Size());
          break;
        }
        offset += Size(control) + m_itemGap;
      }
    }
    break;
  case GUI_MSG_SETFOCUS:
    {
      // we've been asked to focus.  We focus the last control if it's on this page,
      // else we'll focus the first focusable control from our offset (after verifying it)
      ValidateOffset();
      // now check the focusControl's offset
      float offset = 0;
      for (iControls it = m_children.begin(); it != m_children.end(); ++it)
      {
        CGUIControl *control = *it;
        if (!control->IsVisible())
          continue;
        if (control->GetID() == m_focusedControl)
        {
          if (offset >= m_offset && offset + Size(control) <= m_offset + Size())
            return CGUIControlGroup::OnMessage(message);
          break;
        }
        offset += Size(control) + m_itemGap;
      }
      // find the first control on this page
      offset = 0;
      for (iControls it = m_children.begin(); it != m_children.end(); ++it)
      {
        CGUIControl *control = *it;
        if (!control->IsVisible())
          continue;
        if (control->CanFocus() && offset >= m_offset && offset + Size(control) <= m_offset + Size())
        {
          m_focusedControl = control->GetID();
          break;
        }
        offset += Size(control) + m_itemGap;
      }
    }
    break;
  case GUI_MSG_PAGE_CHANGE:
    {
      if (message.GetSenderId() == m_pageControl)
      { // it's from our page control
        ScrollTo((float)message.GetParam1());
        return true;
      }
    }
    break;
  }
  return CGUIControlGroup::OnMessage(message);
}

void CGUIControlGroupList::ValidateOffset()
{
  // calculate how many items we have on this page
  m_totalSize = 0;
  for (iControls it = m_children.begin(); it != m_children.end(); ++it)
  {
    CGUIControl *control = *it;
    if (!control->IsVisible()) continue;
    m_totalSize += Size(control) + m_itemGap;
  }
  if (m_totalSize > 0) m_totalSize -= m_itemGap;
  // check our m_offset range
  if (m_offset > m_totalSize - Size())
    m_offset = m_totalSize - Size();
  if (m_offset < 0) m_offset = 0;
}

void CGUIControlGroupList::AddControl(CGUIControl *control)
{
  if (control)
  { // set the navigation of items so that they form a list
    if (m_orientation == VERTICAL)
    {
      DWORD upID = GetControlIdUp();
      DWORD downID = GetControlIdDown();
      if (m_children.size())
      {
        CGUIControl *top = m_children[0];
        if (downID == GetID())
          downID = top->GetID();
        if (upID == GetID())
          top->SetNavigation(control->GetID(), top->GetControlIdDown(), GetControlIdLeft(), GetControlIdRight());
        CGUIControl *prev = m_children[m_children.size() - 1];
        upID = prev->GetID();
        prev->SetNavigation(prev->GetControlIdUp(), control->GetID(), GetControlIdLeft(), GetControlIdRight());
      }
      control->SetNavigation(upID, downID, GetControlIdLeft(), GetControlIdRight());
    }
    else
    {
      DWORD leftID = GetControlIdLeft();
      DWORD rightID = GetControlIdRight();
      if (m_children.size())
      {
        CGUIControl *left = m_children[0];
        if (rightID == GetID())
          rightID = left->GetID();
        if (leftID == GetID())
          left->SetNavigation(GetControlIdUp(), GetControlIdDown(), control->GetID(), left->GetControlIdRight());
        CGUIControl *prev = m_children[m_children.size() - 1];
        leftID = prev->GetID();
        prev->SetNavigation(GetControlIdUp(), GetControlIdDown(), prev->GetControlIdLeft(), control->GetID());
      }
      control->SetNavigation(GetControlIdUp(), GetControlIdDown(), leftID, rightID);
    }
    // old versions of the controllist (type="grouplist") used to set the positions of all controls
    // directly.  The new version (type="controllist") allows offsets to be set via the posx, posy coordinates.
    if (!m_useControlPositions)
      control->SetPosition(0,0);
    CGUIControlGroup::AddControl(control);
  }
}

void CGUIControlGroupList::ClearAll()
{
  CGUIControlGroup::ClearAll();
  m_offset = 0;
}

inline float CGUIControlGroupList::Size(const CGUIControl *control) const
{
  return (m_orientation == VERTICAL) ? control->GetYPosition() + control->GetHeight() : control->GetXPosition() + control->GetWidth();
}

inline float CGUIControlGroupList::Size() const
{
  return (m_orientation == VERTICAL) ? m_height : m_width;
}

void CGUIControlGroupList::ScrollTo(float offset)
{
  m_scrollOffset = offset;
  m_scrollSpeed = (m_scrollOffset - m_offset) / TIME_TO_SCROLL;
}