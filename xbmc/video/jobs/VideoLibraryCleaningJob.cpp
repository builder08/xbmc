/*
 *      Copyright (C) 2014-present Team Kodi
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

#include "VideoLibraryCleaningJob.h"
#include "dialogs/GUIDialogExtendedProgressBar.h"
#include "video/VideoDatabase.h"

CVideoLibraryCleaningJob::CVideoLibraryCleaningJob(const std::set<int>& paths /* = std::set<int>() */, bool showDialog /* = false */)
  : CVideoLibraryProgressJob(NULL),
    m_paths(paths),
    m_showDialog(showDialog)
{ }

CVideoLibraryCleaningJob::CVideoLibraryCleaningJob(const std::set<int>& paths, CGUIDialogProgressBarHandle* progressBar)
  : CVideoLibraryProgressJob(progressBar),
    m_paths(paths),
    m_showDialog(false)
{ }

CVideoLibraryCleaningJob::~CVideoLibraryCleaningJob() = default;

bool CVideoLibraryCleaningJob::operator==(const CJob* job) const
{
  if (strcmp(job->GetType(), GetType()) != 0)
    return false;

  const CVideoLibraryCleaningJob* cleaningJob = dynamic_cast<const CVideoLibraryCleaningJob*>(job);
  if (cleaningJob == NULL)
    return false;

  return m_paths == cleaningJob->m_paths &&
         m_showDialog == cleaningJob->m_showDialog;
}

bool CVideoLibraryCleaningJob::Work(CVideoDatabase &db)
{
  db.CleanDatabase(GetProgressBar(), m_paths, m_showDialog);
  return true;
}
