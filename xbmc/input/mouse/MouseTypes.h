/*
 *      Copyright (C) 2018-present Team Kodi
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

#include "input/InputTypes.h"

#include <string>

namespace KODI
{
namespace MOUSE
{
  /*!
   * \brief Buttons on a mouse
   */
  enum class BUTTON_ID
  {
    UNKNOWN,
    LEFT,
    RIGHT,
    MIDDLE,
    BUTTON4,
    BUTTON5,
    WHEEL_UP,
    WHEEL_DOWN,
    HORIZ_WHEEL_LEFT,
    HORIZ_WHEEL_RIGHT,
  };

  /*!
   * \brief Name of a mouse button
   *
   * Names are defined in the mouse's controller profile.
   */
  using ButtonName = std::string;

  /*!
   * \brief Directions of motion for a mouse pointer
   */
  using POINTER_DIRECTION = INPUT::CARDINAL_DIRECTION;

  /*!
   * \brief Name of the mouse pointer
   *
   * Names are defined in the mouse's controller profile.
   */
  using PointerName = std::string;
}
}
