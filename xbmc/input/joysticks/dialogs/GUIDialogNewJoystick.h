/*
 *      Copyright (C) 2016-2017 Team Kodi
 *      http://kodi.tv
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
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "threads/Thread.h"

#include <string>

namespace KODI
{
namespace JOYSTICK
{
  class CGUIDialogNewJoystick : protected CThread
  {
  public:
    CGUIDialogNewJoystick();
    virtual ~CGUIDialogNewJoystick() = default;

    void ShowAsync(const std::string& deviceName);

  protected:
    // implementation of CThread
    virtual void Process() override;

  private:
    std::string m_strDeviceName;
  };
}
}
