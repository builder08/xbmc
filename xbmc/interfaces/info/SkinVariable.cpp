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

#include "SkinVariable.h"
#include "GUIInfoManager.h"
#include "ServiceBroker.h"
#include "guilib/GUIComponent.h"
#include "utils/XBMCTinyXML.h"

using namespace INFO;
using namespace KODI;

const CSkinVariableString* CSkinVariable::CreateFromXML(const TiXmlElement& node, int context)
{
  const char* name = node.Attribute("name");
  if (name)
  {
    CSkinVariableString* tmp = new CSkinVariableString;
    tmp->m_name = name;
    tmp->m_context = context;
    const TiXmlElement* valuenode = node.FirstChildElement("value");
    while (valuenode)
    {
      CSkinVariableString::ConditionLabelPair pair;
      const char *condition = valuenode->Attribute("condition");
      if (condition)
        pair.m_condition = CServiceBroker::GetGUI()->GetInfoManager().Register(condition, context);

      auto label = valuenode->FirstChild() ? valuenode->FirstChild()->ValueStr() : "";
      pair.m_label = GUILIB::GUIINFO::CGUIInfoLabel(label);
      tmp->m_conditionLabelPairs.push_back(pair);
      if (!pair.m_condition)
        break; // once we reach default value (without condition) break iterating

      valuenode = valuenode->NextSiblingElement("value");
    }
    if (!tmp->m_conditionLabelPairs.empty())
      return tmp;
    delete tmp;
  }
  return NULL;
}

CSkinVariableString::CSkinVariableString() = default;

int CSkinVariableString::GetContext() const
{
  return m_context;
}

const std::string& CSkinVariableString::GetName() const
{
  return m_name;
}

std::string CSkinVariableString::GetValue(bool preferImage /* = false */, const CGUIListItem *item /* = nullptr */) const
{
  for (VECCONDITIONLABELPAIR::const_iterator it = m_conditionLabelPairs.begin() ; it != m_conditionLabelPairs.end(); ++it)
  {
    if (!it->m_condition || it->m_condition->Get(item))
    {
      if (item)
        return it->m_label.GetItemLabel(item, preferImage);
      else
        return it->m_label.GetLabel(m_context, preferImage);
    }
  }
  return "";
}
