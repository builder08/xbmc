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

#include "IKeyboardHandler.h"
#include "input/XBMC_vkeys.h"

#include <vector>

namespace KODI
{
namespace KEYBOARD
{
  /*!
   * \brief Hush!!!
   */
  class CKeyboardEasterEgg : public IKeyboardHandler
  {
  public:
    CKeyboardEasterEgg(void);
    virtual ~CKeyboardEasterEgg() = default;

    // implementation of IKeyboardHandler
    virtual bool OnKeyPress(const CKey& key);
    virtual void OnKeyRelease(const CKey& key) { }

  private:
    static std::vector<XBMCVKey> m_sequence;

    unsigned int m_state;
  };
}
}
