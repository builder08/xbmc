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

#include <sys/mman.h>

#include <cstddef>

namespace KODI
{
namespace UTILS
{
namespace POSIX
{

/**
 * Wrapper for mapped memory that automatically calls munmap on destruction
 */
class CMmap
{
public:
  /**
   * See mmap(3p) for parameter description
   */
  CMmap(void* addr, std::size_t length, int prot, int flags, int fildes, off_t offset);
  ~CMmap();

  void* Data() const
  {
    return m_memory;
  }
  std::size_t Size() const
  {
    return m_size;
  }

private:
  CMmap(CMmap const& other) = delete;
  CMmap& operator=(CMmap const& other) = delete;

  std::size_t m_size;
  void* m_memory;
};

}
}
}