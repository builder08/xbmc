#pragma once
/*
 *      Copyright (C) 2015-present Team Kodi
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

#include "events/UniqueEvent.h"

class CNotificationEvent : public CUniqueEvent
{
public:
  CNotificationEvent(const CVariant& label, const CVariant& description, EventLevel level = EventLevel::Information)
    : CUniqueEvent(label, description, level)
  { }
  CNotificationEvent(const CVariant& label, const CVariant& description, const std::string& icon, EventLevel level = EventLevel::Information)
    : CUniqueEvent(label, description, icon, level)
  { }
  CNotificationEvent(const CVariant& label, const CVariant& description, const std::string& icon, const CVariant& details, EventLevel level = EventLevel::Information)
    : CUniqueEvent(label, description, icon, details, level)
  { }
  CNotificationEvent(const CVariant& label, const CVariant& description, const std::string& icon, const CVariant& details, const CVariant& executionLabel, EventLevel level = EventLevel::Information)
    : CUniqueEvent(label, description, icon, details, executionLabel, level)
  { }
  ~CNotificationEvent() override = default;

  const char* GetType() const override { return "NotificationEvent"; }

  bool CanExecute() const override { return false; }
  bool Execute() const override { return true; }
};
