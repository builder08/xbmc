/*
 *      Copyright (C) 2017-present Team Kodi
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

namespace KODI
{
namespace KEYBOARD
{
  class IKeyboardInputHandler;

  /*!
   * \ingroup mouse
   * \brief Interface for classes that can provide keyboard input
   */
  class IKeyboardInputProvider
  {
  public:
    virtual ~IKeyboardInputProvider() = default;

    /*!
     * \brief Registers a handler to be called on keyboard input
     *
     * \param handler The handler to receive keyboard input provided by this class
     * \param bPromiscuous True to observe all events without affecting the
     *        input's destination
     */
    virtual void RegisterKeyboardHandler(IKeyboardInputHandler* handler, bool bPromiscuous) = 0;

    /*!
     * \brief Unregisters handler from keyboard input
     *
     * \param handler The handler that was receiving keyboard input
     */
    virtual void UnregisterKeyboardHandler(IKeyboardInputHandler* handler) = 0;
  };
}
}
