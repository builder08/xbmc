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

#include "RetroPlayerInput.h"
#include "peripherals/Peripherals.h"
#include "peripherals/EventPollHandle.h"
#include "utils/log.h"

using namespace KODI;
using namespace RETRO;

CRetroPlayerInput::CRetroPlayerInput(PERIPHERALS::CPeripherals &peripheralManager) :
  m_peripheralManager(peripheralManager)
{
  CLog::Log(LOGDEBUG, "RetroPlayer[INPUT]: Initializing input");

  m_inputPollHandle = m_peripheralManager.RegisterEventPoller();
}

CRetroPlayerInput::~CRetroPlayerInput()
{
  CLog::Log(LOGDEBUG, "RetroPlayer[INPUT]: Deinitializing input");

  m_inputPollHandle.reset();
}

void CRetroPlayerInput::SetSpeed(double speed)
{
  if (speed != 0)
    m_inputPollHandle->Activate();
  else
    m_inputPollHandle->Deactivate();
}

void CRetroPlayerInput::PollInput()
{
  m_inputPollHandle->HandleEvents(true);
}
