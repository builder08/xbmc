#include "include.h"
#include "GUIScrollBarControl.h"

#define MIN_NIB_SIZE 4

CGUIScrollBar::CGUIScrollBar(DWORD dwParentID, DWORD dwControlId, float posX, float posY, float width, float height, const CImage& backGroundTexture, const CImage& barTexture, const CImage& barTextureFocus, const CImage& nibTexture, const CImage& nibTextureFocus, ORIENTATION orientation, bool showOnePage)
    : CGUIControl(dwParentID, dwControlId, posX, posY, width, height)
    , m_guiBackground(dwParentID, dwControlId, posX, posY, width, height, backGroundTexture)
    , m_guiBarNoFocus(dwParentID, dwControlId, posX, posY, width, height, barTexture)
    , m_guiBarFocus(dwParentID, dwControlId, posX, posY, width, height, barTextureFocus)
    , m_guiNibNoFocus(dwParentID, dwControlId, posX, posY, width, height, nibTexture)
    , m_guiNibFocus(dwParentID, dwControlId, posX, posY, width, height, nibTextureFocus)
{
  m_guiNibNoFocus.SetAspectRatio(CGUIImage::ASPECT_RATIO_CENTER);
  m_guiNibFocus.SetAspectRatio(CGUIImage::ASPECT_RATIO_CENTER);
  m_numItems = 100;
  m_offset = 0;
  m_pageSize = 10;
  ControlType = GUICONTROL_SCROLLBAR;
  m_orientation = orientation;
  m_showOnePage = showOnePage;
}

CGUIScrollBar::~CGUIScrollBar(void)
{
}


void CGUIScrollBar::Render()
{
  if (m_bInvalidated)
    UpdateBarSize();

  if (!IsVisible()) return;

  m_guiBackground.Render();
  if (m_bHasFocus)
  {
    m_guiBarFocus.Render();
    m_guiNibFocus.Render();
  }
  else
  {
    m_guiBarNoFocus.Render();
    m_guiNibNoFocus.Render();
  }
  CGUIControl::Render();
}

bool CGUIScrollBar::OnMessage(CGUIMessage& message)
{
  switch (message.GetMessage())
  {
  case GUI_MSG_ITEM_SELECT:
    SetValue(message.GetParam1());
    return true;
  case GUI_MSG_LABEL_RESET:
    SetRange(message.GetParam1(), message.GetParam2());
    return true;
  }
  return CGUIControl::OnMessage(message);
}

bool CGUIScrollBar::OnAction(const CAction &action)
{
  switch ( action.wID )
  {
  case ACTION_MOVE_LEFT:
    if (m_orientation == HORIZONTAL)
    {
      Move( -1);
      return true;
    }
    break;

  case ACTION_MOVE_RIGHT:
    if (m_orientation == HORIZONTAL)
    {
      Move(1);
      return true;
    }
    break;
  case ACTION_MOVE_UP:
    if (m_orientation == VERTICAL)
    {
      Move(-1);
      return true;
    }
    break;

  case ACTION_MOVE_DOWN:
    if (m_orientation == VERTICAL)
    {
      Move(1);
      return true;
    }
    break;
  }
  return CGUIControl::OnAction(action);
}

void CGUIScrollBar::Move(int numSteps)
{
  m_offset += numSteps * m_pageSize;
  if (m_offset > m_numItems - m_pageSize) m_offset = m_numItems - m_pageSize;
  if (m_offset < 0) m_offset = 0;
  CGUIMessage message(GUI_MSG_NOTIFY_ALL, GetParentID(), GetID(), GUI_MSG_PAGE_CHANGE, m_offset);
  SendWindowMessage(message);
  Update();
}

void CGUIScrollBar::SetRange(int pageSize, int numItems)
{
  m_pageSize = pageSize;
  m_numItems = numItems;
  m_offset = 0;
  Update();
}

void CGUIScrollBar::SetValue(int value)
{
  m_offset = value;
  Update();
}

void CGUIScrollBar::FreeResources()
{
  CGUIControl::FreeResources();
  m_guiBackground.FreeResources();
  m_guiBarNoFocus.FreeResources();
  m_guiBarFocus.FreeResources();
  m_guiNibNoFocus.FreeResources();
  m_guiNibFocus.FreeResources();
}

void CGUIScrollBar::DynamicResourceAlloc(bool bOnOff)
{
  CGUIControl::DynamicResourceAlloc(bOnOff);
  m_guiBackground.DynamicResourceAlloc(bOnOff);
  m_guiBarNoFocus.DynamicResourceAlloc(bOnOff);
  m_guiBarFocus.DynamicResourceAlloc(bOnOff);
  m_guiNibNoFocus.DynamicResourceAlloc(bOnOff);
  m_guiNibFocus.DynamicResourceAlloc(bOnOff);
}

void CGUIScrollBar::PreAllocResources()
{
  CGUIControl::PreAllocResources();
  m_guiBackground.PreAllocResources();
  m_guiBarNoFocus.PreAllocResources();
  m_guiBarFocus.PreAllocResources();
  m_guiNibNoFocus.PreAllocResources();
  m_guiNibFocus.PreAllocResources();
}

void CGUIScrollBar::AllocResources()
{
  CGUIControl::AllocResources();
  m_guiBackground.AllocResources();
  m_guiBarNoFocus.AllocResources();
  m_guiBarFocus.AllocResources();
  m_guiNibNoFocus.AllocResources();
  m_guiNibFocus.AllocResources();
}

void CGUIScrollBar::UpdateBarSize()
{
  SetVisible(m_numItems > m_pageSize || m_showOnePage);

  // scale our textures to suit
  if (m_orientation == VERTICAL)
  {
    // calculate the height to display the nib at
    float percent = (float)m_pageSize / m_numItems;
    float nibSize = GetHeight() * percent;
    if (nibSize < MIN_NIB_SIZE) nibSize = MIN_NIB_SIZE;
    if (nibSize > GetHeight()) nibSize = GetHeight();

    m_guiBarNoFocus.SetHeight(nibSize);
    m_guiBarFocus.SetHeight(nibSize);
    m_guiNibNoFocus.SetHeight(nibSize);
    m_guiNibFocus.SetHeight(nibSize);

    // and the position
    percent = (float)m_offset / m_numItems;
    float nibPos = GetHeight() * percent;
    if (nibPos < 0) nibPos = 0;
    if (nibPos > GetHeight() - nibSize) nibPos = GetHeight() - nibSize;
    m_guiBarNoFocus.SetPosition(GetXPosition(), GetYPosition() + nibPos);
    m_guiBarFocus.SetPosition(GetXPosition(), GetYPosition() + nibPos);
    m_guiNibNoFocus.SetPosition(GetXPosition(), GetYPosition() + nibPos);
    m_guiNibFocus.SetPosition(GetXPosition(), GetYPosition() + nibPos);
  }
  else
  {
    // calculate the height to display the nib at
    float percent = (float)m_pageSize / m_numItems;
    float nibSize = GetWidth() * percent + 0.5f;
    if (nibSize < MIN_NIB_SIZE) nibSize = MIN_NIB_SIZE;
    if (nibSize > GetWidth()) nibSize = GetWidth();

    m_guiBarNoFocus.SetWidth(nibSize);
    m_guiBarFocus.SetWidth(nibSize);
    m_guiNibNoFocus.SetWidth(nibSize);
    m_guiNibFocus.SetWidth(nibSize);

    // and the position
    percent = (float)m_offset / m_numItems;
    float nibPos = GetWidth() * percent;
    if (nibPos < 0) nibPos = 0;
    if (nibPos > GetWidth() - nibSize) nibPos = GetWidth() - nibSize;
    m_guiBarNoFocus.SetPosition(GetXPosition() + nibPos, GetYPosition());
    m_guiBarFocus.SetPosition(GetXPosition() + nibPos, GetYPosition());
    m_guiNibNoFocus.SetPosition(GetXPosition() + nibPos, GetYPosition());
    m_guiNibFocus.SetPosition(GetXPosition() + nibPos, GetYPosition());
  }
}

bool CGUIScrollBar::HitTest(float posX, float posY) const
{
  if (m_guiBackground.HitTest(posX, posY)) return true;
  if (m_guiBarNoFocus.HitTest(posX, posY)) return true;
  return false;
}

void CGUIScrollBar::SetFromPosition(float posX, float posY)
{
  float fPercent;
  if (m_orientation == VERTICAL)
    fPercent = (posY - m_guiBackground.GetYPosition() - 0.5f*m_guiBarFocus.GetHeight()) / m_guiBackground.GetHeight();
  else
    fPercent = (posX - m_guiBackground.GetXPosition() - 0.5f*m_guiBarFocus.GetWidth()) / m_guiBackground.GetWidth();
  if (fPercent < 0) fPercent = 0;
  if (fPercent > 1) fPercent = 1;
  m_offset = (int)(floor(fPercent * m_numItems + 0.5f));
  CGUIMessage message(GUI_MSG_NOTIFY_ALL, GetParentID(), GetID(), GUI_MSG_PAGE_CHANGE, m_offset);
  SendWindowMessage(message);
  Update();
}

bool CGUIScrollBar::OnMouseClick(DWORD dwButton)
{
  g_Mouse.SetState(MOUSE_STATE_CLICK);
  // turn off any exclusive access, if it's on...
  g_Mouse.EndExclusiveAccess(GetID(), GetParentID());
  if (m_guiBackground.HitTest(g_Mouse.posX, g_Mouse.posY))
  { // set the position
    SetFromPosition(g_Mouse.posX, g_Mouse.posY);
    return true;
  }
  return false;
}

bool CGUIScrollBar::OnMouseDrag()
{
  g_Mouse.SetState(MOUSE_STATE_DRAG);
  // get exclusive access to the mouse
  g_Mouse.SetExclusiveAccess(GetID(), GetParentID());
  // get the position of the mouse
  SetFromPosition(g_Mouse.posX, g_Mouse.posY);
  return true;
}

bool CGUIScrollBar::OnMouseWheel()
{
  Move(g_Mouse.cWheel);
  return true;
}

CStdString CGUIScrollBar::GetDescription() const
{
  CStdString description;
  description.Format("%i/%i", m_offset, m_numItems);
  return description;
}