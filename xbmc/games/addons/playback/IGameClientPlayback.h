/*
 *      Copyright (C) 2016-present Team Kodi
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

#include <stdint.h>
#include <string>

namespace KODI
{
namespace GAME
{
  class IGameClientPlayback
  {
  public:
    virtual ~IGameClientPlayback() = default;

    // Playback capabilities
    virtual bool CanPause() const = 0;
    virtual bool CanSeek() const = 0;

    // Control playback
    virtual unsigned int GetTimeMs() const = 0;
    virtual unsigned int GetTotalTimeMs() const = 0;
    virtual unsigned int GetCacheTimeMs() const = 0;
    virtual void SeekTimeMs(unsigned int timeMs) = 0;
    virtual double GetSpeed() const = 0;
    virtual void SetSpeed(double speedFactor) = 0;
    virtual void PauseAsync() = 0; // Pauses after the following frame

    // Savestates
    virtual std::string CreateSavestate() = 0; // Returns the path of savestate on success
    virtual bool LoadSavestate(const std::string& path) = 0;
  };
}
}
