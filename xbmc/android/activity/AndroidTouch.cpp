/*
 *      Copyright (C) 2012-2013 Team XBMC
 *      http://xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ACTIVITY_ANDROIDTOUCH_H_INCLUDED
#define ACTIVITY_ANDROIDTOUCH_H_INCLUDED
#include "AndroidTouch.h"
#endif

#ifndef ACTIVITY_ANDROID_ACTIVITY_XBMCAPP_H_INCLUDED
#define ACTIVITY_ANDROID_ACTIVITY_XBMCAPP_H_INCLUDED
#include "android/activity/XBMCApp.h"
#endif

#ifndef ACTIVITY_INPUT_TOUCH_GENERIC_GENERICTOUCHACTIONHANDLER_H_INCLUDED
#define ACTIVITY_INPUT_TOUCH_GENERIC_GENERICTOUCHACTIONHANDLER_H_INCLUDED
#include "input/touch/generic/GenericTouchActionHandler.h"
#endif

#ifndef ACTIVITY_INPUT_TOUCH_GENERIC_GENERICTOUCHINPUTHANDLER_H_INCLUDED
#define ACTIVITY_INPUT_TOUCH_GENERIC_GENERICTOUCHINPUTHANDLER_H_INCLUDED
#include "input/touch/generic/GenericTouchInputHandler.h"
#endif


CAndroidTouch::CAndroidTouch() : m_dpi(160)
{
  CGenericTouchInputHandler::Get().RegisterHandler(&CGenericTouchActionHandler::Get());
}

CAndroidTouch::~CAndroidTouch()
{
  CGenericTouchInputHandler::Get().UnregisterHandler();
}

bool CAndroidTouch::onTouchEvent(AInputEvent* event)
{
  if (event == NULL)
    return false;

  size_t numPointers = AMotionEvent_getPointerCount(event);
  if (numPointers <= 0)
  {
    CXBMCApp::android_printf(" => aborting touch event because there are no active pointers");
    return false;
  }

  if (numPointers > TOUCH_MAX_POINTERS)
    numPointers = TOUCH_MAX_POINTERS;

  int32_t eventAction = AMotionEvent_getAction(event);
  int8_t touchAction = eventAction & AMOTION_EVENT_ACTION_MASK;
  size_t touchPointer = eventAction >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
  
  TouchInput touchEvent = TouchInputAbort;
  switch (touchAction)
  {
    case AMOTION_EVENT_ACTION_DOWN:
    case AMOTION_EVENT_ACTION_POINTER_DOWN:
      touchEvent = TouchInputDown;
      break;

    case AMOTION_EVENT_ACTION_UP:
    case AMOTION_EVENT_ACTION_POINTER_UP:
      touchEvent = TouchInputUp;
      break;

    case AMOTION_EVENT_ACTION_MOVE:
      touchEvent = TouchInputMove;
      break;

    case AMOTION_EVENT_ACTION_OUTSIDE:
    case AMOTION_EVENT_ACTION_CANCEL:
    default:
      break;
  }

  float x = AMotionEvent_getX(event, touchPointer);
  float y = AMotionEvent_getY(event, touchPointer);
  float size = m_dpi / 16.0f;
  int64_t time = AMotionEvent_getEventTime(event);

  // first update all touch pointers
  for (unsigned int pointer = 0; pointer < numPointers; pointer++)
    CGenericTouchInputHandler::Get().UpdateTouchPointer(pointer, AMotionEvent_getX(event, pointer), AMotionEvent_getY(event, pointer),
    AMotionEvent_getEventTime(event), m_dpi / 16.0f);

  // now send the event
  return CGenericTouchInputHandler::Get().HandleTouchInput(touchEvent, x, y, time, touchPointer, size);
}

void CAndroidTouch::setDPI(uint32_t dpi)
{
  if (dpi != 0)
  {
    m_dpi = dpi;

    CGenericTouchInputHandler::Get().SetScreenDPI(m_dpi);
  }
}
