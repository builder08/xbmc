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

#pragma once

#include "GUIControl.h"
#include <memory>

typedef std::shared_ptr<CGUIListItem> CGUIListItemPtr;

/*!
 \ingroup controls
 \brief
 */

class IGUIContainer : public CGUIControl
{
protected:
  VIEW_TYPE m_type;
  std::string m_label;
public:
  IGUIContainer(int parentID, int controlID, float posX, float posY, float width, float height)
   : CGUIControl(parentID, controlID, posX, posY, width, height), m_type(VIEW_TYPE_NONE) {}

  bool IsContainer() const override { return true; };

  VIEW_TYPE GetType() const { return m_type; };
  const std::string &GetLabel() const { return m_label; };
  void SetType(VIEW_TYPE type, const std::string &label)
  {
    m_type = type;
    m_label = label;
  }

  virtual CGUIListItemPtr GetListItem(int offset, unsigned int flag = 0) const = 0;
  virtual std::string GetLabel(int info) const                                 = 0;
};
