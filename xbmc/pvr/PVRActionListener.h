#pragma once

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

#include "interfaces/IActionListener.h"
#include "settings/lib/ISettingCallback.h"

namespace PVR
{

enum class ChannelSwitchMode;

class CPVRActionListener : public IActionListener, public ISettingCallback
{
public:
  CPVRActionListener();
  ~CPVRActionListener() override;

  // IActionListener implementation
  bool OnAction(const CAction &action) override;

  // ISettingCallback implementation
  void OnSettingChanged(std::shared_ptr<const CSetting> setting) override;
  void OnSettingAction(std::shared_ptr<const CSetting> setting) override;

private:
  CPVRActionListener(const CPVRActionListener&) = delete;
  CPVRActionListener& operator=(const CPVRActionListener&) = delete;

  static ChannelSwitchMode GetChannelSwitchMode(int iAction);
};

} // namespace PVR
