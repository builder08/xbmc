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

#include "MusicLibraryQueue.h"

#include <utility>

#include "ServiceBroker.h"
#include "dialogs/GUIDialogProgress.h"
#include "guilib/GUIComponent.h"
#include "guilib/GUIWindowManager.h"
#include "GUIUserMessages.h"
#include "music/jobs/MusicLibraryCleaningJob.h"
#include "music/jobs/MusicLibraryExportJob.h"
#include "music/jobs/MusicLibraryScanningJob.h"
#include "music/jobs/MusicLibraryJob.h"
#include "threads/SingleLock.h"
#include "Util.h"
#include "utils/Variant.h"

CMusicLibraryQueue::CMusicLibraryQueue()
  : CJobQueue(false, 1, CJob::PRIORITY_LOW),
    m_jobs(),
    m_modal(false),
    m_exporting(false),
    m_cleaning(false)
{ }

CMusicLibraryQueue::~CMusicLibraryQueue()
{
  CSingleLock lock(m_critical);
  m_jobs.clear();
}

CMusicLibraryQueue& CMusicLibraryQueue::GetInstance()
{
  static CMusicLibraryQueue s_instance;
  return s_instance;
}

void CMusicLibraryQueue::ExportLibrary(const CLibExportSettings& settings, bool showDialog /* = false */)
{
  CGUIDialogProgress* progress = NULL;
  if (showDialog)
  {
    progress = CServiceBroker::GetGUI()->GetWindowManager().GetWindow<CGUIDialogProgress>(WINDOW_DIALOG_PROGRESS);
    if (progress)
    {
      progress->SetHeading(CVariant{ 20196 }); //"Export music library"
      progress->SetText(CVariant{ 650 });   //"Exporting"
      progress->SetPercentage(0);
      progress->Open();
      progress->ShowProgressBar(true);
    }
  }

  CMusicLibraryExportJob* exportJob = new CMusicLibraryExportJob(settings, progress);
  if (showDialog)
  {    
    AddJob(exportJob);

    // Wait for export to complete or be canceled, but render every 10ms so that the 
    // pointer movements work on dialog even when export is reporting progress infrequently
    if (progress)
      progress->Wait();
  }
  else
  {
    m_modal = true;
    exportJob->DoWork();

    delete exportJob;
    m_modal = false;
    Refresh();
  }
}

void CMusicLibraryQueue::ScanLibrary(const std::string& strDirectory, int flags /* = 0 */, bool showProgress /* = true */)
{
  AddJob(new CMusicLibraryScanningJob(strDirectory, flags, showProgress));
}

void CMusicLibraryQueue::StartAlbumScan(const std::string & strDirectory, bool refresh)
{
  int flags = MUSIC_INFO::CMusicInfoScanner::SCAN_ALBUMS;
  if (refresh)
    flags |= MUSIC_INFO::CMusicInfoScanner::SCAN_RESCAN;
  AddJob(new CMusicLibraryScanningJob(strDirectory, flags, true));
}

void CMusicLibraryQueue::StartArtistScan(const std::string& strDirectory, bool refresh)
{
  int flags = MUSIC_INFO::CMusicInfoScanner::SCAN_ARTISTS;
  if (refresh)
    flags |= MUSIC_INFO::CMusicInfoScanner::SCAN_RESCAN;
  AddJob(new CMusicLibraryScanningJob(strDirectory, flags, true));
}

bool CMusicLibraryQueue::IsScanningLibrary() const
{
  // check if the library is being cleaned synchronously
  if (m_cleaning)
    return true;

  // check if the library is being scanned asynchronously
  MusicLibraryJobMap::const_iterator scanningJobs = m_jobs.find("MusicLibraryScanningJob");
  if (scanningJobs != m_jobs.end() && !scanningJobs->second.empty())
    return true;

  // check if the library is being cleaned asynchronously
  MusicLibraryJobMap::const_iterator cleaningJobs = m_jobs.find("MusicLibraryCleaningJob");
  if (cleaningJobs != m_jobs.end() && !cleaningJobs->second.empty())
    return true;

  return false;
}

void CMusicLibraryQueue::StopLibraryScanning()
{
  CSingleLock lock(m_critical);
  MusicLibraryJobMap::const_iterator scanningJobs = m_jobs.find("MusicLibraryScanningJob");
  if (scanningJobs == m_jobs.end())
    return;

  // get a copy of the scanning jobs because CancelJob() will modify m_scanningJobs
  MusicLibraryJobs tmpScanningJobs(scanningJobs->second.begin(), scanningJobs->second.end());

  // cancel all scanning jobs
  for (const auto& job : tmpScanningJobs)
    CancelJob(job);
  Refresh();
}

void CMusicLibraryQueue::CleanLibrary(bool showDialog /* = false */)
{
  CGUIDialogProgress* progress = NULL;
  if (showDialog)
  {
    progress = CServiceBroker::GetGUI()->GetWindowManager().GetWindow<CGUIDialogProgress>(WINDOW_DIALOG_PROGRESS);
    if (progress)
    {
      progress->SetHeading(CVariant{ 700 });
      progress->SetPercentage(0);
      progress->Open();
      progress->ShowProgressBar(true);
    }
  }

  CMusicLibraryCleaningJob* cleaningJob = new CMusicLibraryCleaningJob(progress);  
  AddJob(cleaningJob);

  // Wait for cleaning to complete or be canceled, but render every 20ms so that the 
  // pointer movements work on dialog even when cleaning is reporting progress infrequently
  if (progress)
    progress->Wait(20);
}

void CMusicLibraryQueue::CleanLibraryModal()
{
  // We can't perform a modal library cleaning if other jobs are running
  if (IsRunning())
    return;

  CGUIDialogProgress* progress = nullptr;
  progress = CServiceBroker::GetGUI()->GetWindowManager().GetWindow<CGUIDialogProgress>(WINDOW_DIALOG_PROGRESS);
  if (progress)
  {
    progress->SetHeading(CVariant{ 700 });
    progress->SetPercentage(0);
    progress->Open();
    progress->ShowProgressBar(true);
  }

  m_modal = true;
  m_cleaning = true;
  CMusicLibraryCleaningJob cleaningJob(progress);
  cleaningJob.DoWork();
  m_cleaning = false;
  m_modal = false;
  Refresh();
}

void CMusicLibraryQueue::AddJob(CMusicLibraryJob *job)
{
  if (job == NULL)
    return;

  CSingleLock lock(m_critical);
  if (!CJobQueue::AddJob(job))
    return;

  // add the job to our list of queued/running jobs
  std::string jobType = job->GetType();
  MusicLibraryJobMap::iterator jobsIt = m_jobs.find(jobType);
  if (jobsIt == m_jobs.end())
  {
    MusicLibraryJobs jobs;
    jobs.insert(job);
    m_jobs.insert(std::make_pair(jobType, jobs));
  }
  else
    jobsIt->second.insert(job);
}

void CMusicLibraryQueue::CancelJob(CMusicLibraryJob *job)
{
  if (job == NULL)
    return;

  CSingleLock lock(m_critical);
  // remember the job type needed later because the job might be deleted
  // in the call to CJobQueue::CancelJob()
  std::string jobType;
  if (job->GetType() != NULL)
    jobType = job->GetType();

  // check if the job supports cancellation and cancel it
  if (job->CanBeCancelled())
    job->Cancel();

  // remove the job from the job queue
  CJobQueue::CancelJob(job);

  // remove the job from our list of queued/running jobs
  MusicLibraryJobMap::iterator jobsIt = m_jobs.find(jobType);
  if (jobsIt != m_jobs.end())
    jobsIt->second.erase(job);
}

void CMusicLibraryQueue::CancelAllJobs()
{
  CSingleLock lock(m_critical);
  CJobQueue::CancelJobs();

  // remove all scanning jobs
  m_jobs.clear();
}

bool CMusicLibraryQueue::IsRunning() const
{
  return CJobQueue::IsProcessing() || m_modal;
}

void CMusicLibraryQueue::Refresh()
{
  CUtil::DeleteMusicDatabaseDirectoryCache();
  CGUIMessage msg(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE);
  CServiceBroker::GetGUI()->GetWindowManager().SendThreadMessage(msg);
}

void CMusicLibraryQueue::OnJobComplete(unsigned int jobID, bool success, CJob *job)
{
  if (success)
  {
    if (QueueEmpty())
      Refresh();
  }

  {
    CSingleLock lock(m_critical);
    // remove the job from our list of queued/running jobs
    MusicLibraryJobMap::iterator jobsIt = m_jobs.find(job->GetType());
    if (jobsIt != m_jobs.end())
      jobsIt->second.erase(static_cast<CMusicLibraryJob*>(job));
  }

  return CJobQueue::OnJobComplete(jobID, success, job);
}
