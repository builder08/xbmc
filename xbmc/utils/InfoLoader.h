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

#include "Job.h"
#include <string>

class CInfoLoader : public IJobCallback
{
public:
  explicit CInfoLoader(unsigned int timeToRefresh = 5 * 60 * 1000);
  ~CInfoLoader() override;

  std::string GetInfo(int info);
  void Refresh();

  void OnJobComplete(unsigned int jobID, bool success, CJob *job) override;
protected:
  virtual CJob *GetJob() const=0;
  virtual std::string TranslateInfo(int info) const;
  virtual std::string BusyInfo(int info) const;
private:
  unsigned int m_refreshTime;
  unsigned int m_timeToRefresh;
  bool m_busy;
};
