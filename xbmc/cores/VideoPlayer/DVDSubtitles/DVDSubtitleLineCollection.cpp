/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "DVDSubtitleLineCollection.h"

#include <stddef.h>


CDVDSubtitleLineCollection::CDVDSubtitleLineCollection()
{
  m_pHead = NULL;
  m_pCurrent = NULL;
  m_pTail = NULL;

  m_iSize = 0;
}

CDVDSubtitleLineCollection::~CDVDSubtitleLineCollection()
{
  Clear();
}

void CDVDSubtitleLineCollection::Add(CDVDOverlay* pOverlay)
{
  ListElement* pElement = new ListElement;
  pElement->pOverlay = pOverlay;
  pElement->pNext = NULL;

  if (!m_pHead)
  {
    m_pHead = m_pTail = pElement;
    m_pCurrent = m_pHead;
  }
  else
  {
    m_pTail->pNext = pElement;
    m_pTail = pElement;
  }

  m_iSize++;
}

void CDVDSubtitleLineCollection::Sort()
{
  if (!m_pHead || !m_pHead->pNext)
    return;

  for (ListElement* p1 = m_pHead; p1->pNext != NULL; p1 = p1->pNext)
  {
    for (ListElement* p2 = p1->pNext; p2 != NULL; p2 = p2->pNext)
    {
      if (p1->pOverlay->iPTSStartTime > p2->pOverlay->iPTSStartTime)
      {
        CDVDOverlay* temp = p1->pOverlay;
        p1->pOverlay = p2->pOverlay;
        p2->pOverlay = temp;
      }
    }
  }
}

void CDVDSubtitleLineCollection::MakeSequential()
{
  if (!m_pHead || !m_pHead->pNext)
    return;

  ListElement* p1 = m_pHead;
  ListElement* p2 = m_pHead;

  while (p1->pNext != nullptr)
  {
    p2 = p1->pNext;

    if (p2->pOverlay->replace)
    {
      // remove overlay if it overlaps with the previous one
      if (p2->pOverlay->iPTSStartTime > p1->pOverlay->iPTSStartTime &&
          p2->pOverlay->iPTSStopTime < p1->pOverlay->iPTSStopTime)
      {
        if (p2->pNext)
        {
          p1->pNext = p2->pNext;
          delete p2;
          continue;
        }
      }
      // set previous subtitle stop time to the new one start time
      else if (p2->pOverlay->iPTSStartTime < p1->pOverlay->iPTSStopTime)
        p1->pOverlay->iPTSStopTime = p2->pOverlay->iPTSStartTime;
    }

    // move to next subtitle on the list
    p1 = p2;
  }
}

CDVDOverlay* CDVDSubtitleLineCollection::Get(double iPts)
{
  CDVDOverlay* pOverlay = NULL;

  if (m_pCurrent)
  {
    while (m_pCurrent && m_pCurrent->pOverlay->iPTSStopTime < iPts)
    {
      m_pCurrent = m_pCurrent->pNext;
    }

    if (m_pCurrent)
    {
      pOverlay = m_pCurrent->pOverlay;

      // advance to the next overlay
      m_pCurrent = m_pCurrent->pNext;
    }
  }
  return pOverlay;
}

void CDVDSubtitleLineCollection::Reset()
{
  m_pCurrent = m_pHead;
}

void CDVDSubtitleLineCollection::Clear()
{
  ListElement* pElement = NULL;

  while (m_pHead)
  {
    pElement = m_pHead;
    m_pHead = pElement->pNext;

    pElement->pOverlay->Release();
    delete pElement;
  }

  m_pTail    = NULL;
  m_pHead    = NULL;
  m_pCurrent = NULL;
  m_iSize    = 0;
}
