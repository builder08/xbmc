/*
 *      Copyright (C) 2005-present Team Kodi
 *      This file is part of Kodi - https://kodi.tv
 *
 *  Kodi is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Kodi is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kodi. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "GUIMessage.h"
#include "LocalizeStrings.h"

std::string CGUIMessage::empty_string;

CGUIMessage::CGUIMessage(int msg, int senderID, int controlID, int param1, int param2)
{
  m_message = msg;
  m_senderID = senderID;
  m_controlID = controlID;
  m_param1 = param1;
  m_param2 = param2;
  m_pointer = NULL;
}

CGUIMessage::CGUIMessage(int msg, int senderID, int controlID, int param1, int param2, CFileItemList *item)
{
  m_message = msg;
  m_senderID = senderID;
  m_controlID = controlID;
  m_param1 = param1;
  m_param2 = param2;
  m_pointer = item;
}

CGUIMessage::CGUIMessage(int msg, int senderID, int controlID, int param1, int param2, const CGUIListItemPtr &item)
{
  m_message = msg;
  m_senderID = senderID;
  m_controlID = controlID;
  m_param1 = param1;
  m_param2 = param2;
  m_pointer = NULL;
  m_item = item;
}

CGUIMessage::CGUIMessage(const CGUIMessage& msg) = default;

CGUIMessage::~CGUIMessage(void) = default;


int CGUIMessage::GetControlId() const
{
  return m_controlID;
}

int CGUIMessage::GetMessage() const
{
  return m_message;
}

void* CGUIMessage::GetPointer() const
{
  return m_pointer;
}

CGUIListItemPtr CGUIMessage::GetItem() const
{
  return m_item;
}

int CGUIMessage::GetParam1() const
{
  return m_param1;
}

int CGUIMessage::GetParam2() const
{
  return m_param2;
}

int CGUIMessage::GetSenderId() const
{
  return m_senderID;
}

CGUIMessage& CGUIMessage::operator = (const CGUIMessage& msg) = default;

void CGUIMessage::SetParam1(int param1)
{
  m_param1 = param1;
}

void CGUIMessage::SetParam2(int param2)
{
  m_param2 = param2;
}

void CGUIMessage::SetPointer(void* lpVoid)
{
  m_pointer = lpVoid;
}

void CGUIMessage::SetLabel(const std::string& strLabel)
{
  m_strLabel = strLabel;
}

const std::string& CGUIMessage::GetLabel() const
{
  return m_strLabel;
}

void CGUIMessage::SetLabel(int iString)
{
  m_strLabel = g_localizeStrings.Get(iString);
}

void CGUIMessage::SetStringParam(const std::string& strParam)
{
  m_params.clear();
  if (strParam.size())
    m_params.push_back(strParam);
}

void CGUIMessage::SetStringParams(const std::vector<std::string> &params)
{
  m_params = params;
}

const std::string& CGUIMessage::GetStringParam(size_t param) const
{
  if (param >= m_params.size())
    return empty_string;
  return m_params[param];
}

size_t CGUIMessage::GetNumStringParams() const
{
  return m_params.size();
}
