/*
    This file is part of libscrobbler. Modified for XBMC by Bobbin007

    libscrobbler is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    libscrobbler is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libscrobbler; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Copyright � 2003 Russell Garrett (russ-scrobbler@garrett.co.uk)
*/
#include "../../stdafx.h"
#include "scrobbler.h"
#include "errors.h"
#include "md5.h"
#include "../../utils/CharsetConverter.h"
#include "../../utils/log.h"
#include "../../utils/http.h"
#include "../../util.h"
#include "../../application.h"

#define LS_VER    "1.4"
#define MINLENGTH 30
#define MAXLENGTH 10800
#define HS_FAIL_WAIT 100000

//#define CLIENT_ID "tst"
#define CLIENT_ID "xbm"
#define CLIENT_VERSION "0.1"

#define CACHE_FILE "Z:\\scrobbler.cha"

CScrobbler* CScrobbler::m_pInstance=NULL;

CScrobbler::CScrobbler()
{
  m_bShouldSubmit=false;
  m_bUpdateWarningDone=false;
  m_bConnectionWarningDone=false;
  m_strClientId = CLIENT_ID; 
  m_strClientVer = CLIENT_VERSION;
  m_bCloseThread = false;
  m_Interval = m_LastConnect = m_SongStartTime = 0;
  m_iSecsTillSubmit=0;
  m_bSubmitInProgress = false;
  m_bReadyToSubmit=false;
  m_iSongNum=0;
  DWORD threadid; // Needed for Win9x
  m_hWorkerEvent = CreateEvent(NULL, false, false, NULL);
  if (!m_hWorkerEvent)
    throw EOutOfMemory();
  m_hHttpMutex = CreateMutex(NULL, false, NULL);
  if (!m_hHttpMutex)
    throw EOutOfMemory();
  m_hWorkerThread = CreateThread(NULL, 0, threadProc, (LPVOID)this, 0, &threadid);
  if (!m_hWorkerThread)
    throw EOutOfMemory();
}

CScrobbler::~CScrobbler()
{
  m_bCloseThread = true;
  SetEvent(m_hWorkerEvent);
  WaitForSingleObject(m_hHttpMutex, INFINITE);
  CloseHandle(m_hHttpMutex);
  Sleep(0);
}

CScrobbler* CScrobbler::GetInstance()
{
  if (!m_pInstance)
    m_pInstance=new CScrobbler;

  return m_pInstance;
}

void CScrobbler::Init()
{
  if (!g_guiSettings.GetBool("MusicLibrary.UseAudioScrobbler") || !g_guiSettings.GetBool("Network.EnableInternet"))
    return;

  CStdString strPassword=g_guiSettings.GetString("MusicLibrary.AudioScrobblerPassword");
  CStdString strUserName=g_guiSettings.GetString("MusicLibrary.AudioScrobblerUserName");

  if (strPassword.IsEmpty() || strUserName.IsEmpty())
    return;

  SetPassword(strPassword);
  SetUsername(strUserName);

  ClearCache();
  LoadCache();
  DoHandshake();
}

void CScrobbler::Term()
{
  m_bReadyToSubmit=false;
  SaveCache(m_strPostString.c_str(), m_iSongNum);
}

void CScrobbler::SetPassword(const CStdString& strPass)
{
  if (strPass.IsEmpty())
    return;
  md5_state_t md5state;
  unsigned char md5pword[16];
  md5_init(&md5state);
  md5_append(&md5state, (unsigned const char *)strPass.c_str(), (int)strPass.size());
  md5_close(&md5state, md5pword);
  char tmp[33];
  strncpy(tmp, "\0", sizeof(tmp));
  for (int j = 0;j < 16;j++) 
  {
    char a[3];
    sprintf(a, "%02x", md5pword[j]);
    tmp[2*j] = a[0];
    tmp[2*j+1] = a[1];
  }
  m_strPassword = tmp;
  GenSessionKey();
}

void CScrobbler::SetUsername(const CStdString& strUser)
{
  if (strUser.IsEmpty())
    return;

  m_strUserName=strUser;

  CUtil::URLEncode(m_strUserName);

  m_strHsString = "http://post.audioscrobbler.com/?hs=true&p=1.1&c=" + m_strClientId + "&v=" + m_strClientVer + "&u=" + m_strUserName;
}

void CScrobbler::ClearCache()
{
  m_iSongNum = 0;
  m_strPostString = "";
}

void CScrobbler::SetCache(const CStdString& strCache, int iNumEntries)
{
  m_strPostString = strCache;
  m_iSongNum = iNumEntries;
}

int CScrobbler::AddSong(const CMusicInfoTag& tag)
{
  if (!g_guiSettings.GetBool("MusicLibrary.UseAudioScrobbler") || !g_guiSettings.GetBool("Network.EnableInternet"))
    return 0;

  if (tag.GetDuration() <= MINLENGTH || tag.GetDuration() > MAXLENGTH) // made <= to minlength to stop iTMS previews being submitted in iTunes
    return 0;

  if (!tag.Loaded() || tag.GetArtist().IsEmpty() || tag.GetTitle().IsEmpty())
    return 0;

  if(m_bSubmitInProgress)
  {
    StatusUpdate(S_NOT_SUBMITTING,"Previous submission still in progress");
    return 0;
  }

  char ti[20];
  struct tm *today = gmtime(&m_SongStartTime);
  strftime(ti, sizeof(ti), "%Y-%m-%d %H:%M:%S", today);

  CStdString a, b, t;
  g_charsetConverter.stringCharsetToUtf8(tag.GetArtist(), a);
  g_charsetConverter.stringCharsetToUtf8(tag.GetAlbum(), b);
  g_charsetConverter.stringCharsetToUtf8(tag.GetTitle(), t);
  CStdString i=ti;
  CStdString m=tag.GetMusicBrainzTrackID();
  CUtil::URLEncode(a);
  CUtil::URLEncode(b);
  CUtil::URLEncode(t);
  CUtil::URLEncode(i);
  CUtil::URLEncode(m);

  CStdString strSubmitStr;
  strSubmitStr.Format("a[%i]=%s&t[%i]=%s&b[%i]=%s&m[%i]=%s&l[%i]=%i&i[%i]=%s&", m_iSongNum, a.c_str(), m_iSongNum, t.c_str(), m_iSongNum, b.c_str(), m_iSongNum, m.c_str(), m_iSongNum, tag.GetDuration(), m_iSongNum, i.c_str());

  if(m_strPostString.find(ti) != m_strPostString.npos)
  {
    // we have already tried to add a song at this time stamp
    // I have no idea how this could happen but apparently it does so
    // we stop it now

    StatusUpdate(S_NOT_SUBMITTING,strSubmitStr);
    StatusUpdate(S_NOT_SUBMITTING,m_strPostString);

    StatusUpdate(S_NOT_SUBMITTING,"Submission error, duplicate subbmission time found");
    return 3;
  }

  m_strPostString += strSubmitStr;
  m_iSongNum++;

  time_t now;
  time (&now);
  if ((m_Interval + m_LastConnect) < now) 
  {
    DoSubmit();
    return 1;
  } 
  else 
  {
    CStdString strMsg;
    strMsg.Format("Not submitting, caching for %i more seconds. Cache is %i entries.", (int)(m_Interval + m_LastConnect - now), m_iSongNum);
    StatusUpdate(S_NOT_SUBMITTING,strMsg);
    return 2;
  }
}

void CScrobbler::DoHandshake()
{
  m_bReadyToSubmit = false;
  time_t now; 
  time (&now);
  m_LastConnect = now;
  m_strHsString = "http://post.audioscrobbler.com/?hs=true&p=1.1&c=" + m_strClientId + "&v=" + m_strClientVer + "&u=" + m_strUserName;
  SetEvent(m_hWorkerEvent);
}

void CScrobbler::DoSubmit()
{
  if(m_bSubmitInProgress)
  {
    StatusUpdate(S_NOT_SUBMITTING,"Previous submission still in progress");
    return;
  }
  m_bSubmitInProgress = true;


  if (m_strUserName == "" || m_strPassword == "" || !m_bReadyToSubmit)
    return;
  StatusUpdate(S_SUBMITTING,"Submitting cache...");
  time_t now;
  time (&now);
  m_LastConnect = now;
  m_strSubmit.Format("u=%s&s=%s&%s", m_strUserName.c_str(), m_strSessionKey.c_str(), m_strPostString.c_str());
  SetEvent(m_hWorkerEvent);
}

void CScrobbler::HandleHandshake(char *handshake)
{
  // Doesn't take into account multiple-packet returns (not that I've seen one yet)...

  // Ian says: strtok() is not re-entrant, but since it's only being called
  //  in only one function at a time, it's ok so far. 

  char seps[] = " \n\r";
  char *response = strtok(handshake, seps);
  if (!response) 
  {
    StatusUpdate(S_HANDSHAKE_INVALID_RESPONSE,"Handshake failed: Response invalid");
    return;
  }
  do 
  {
    if (stricmp("UPTODATE", response) == 0) 
    {
      StatusUpdate(S_HANDSHAKE_UP_TO_DATE,"Handshaking: Client up to date.");
    } 
    else if (stricmp("UPDATE", response) == 0) 
    {
      char *updateurl = strtok(NULL, seps);
      if (!updateurl)
        break;
      string msg = "Handshaking: Please update your client at: ";
      msg += updateurl;
      StatusUpdate(S_HANDSHAKE_OLD_CLIENT,msg.c_str());
    } 
    else if (stricmp("BADUSER", response) == 0) 
    {
      StatusUpdate(S_HANDSHAKE_BAD_USERNAME,"Handshake failed: Bad username");
      return;
    } 
    else 
    {
      break;
    }
    m_strChallenge = strtok(NULL, seps);
    m_strSubmitUrl = strtok(NULL, seps);
    char *inttext = strtok(NULL, seps);
    if (!inttext || !(stricmp("INTERVAL", inttext) == 0))
      break;
    SetInterval(atoi(strtok(NULL, seps)));
    GenSessionKey();
    m_bReadyToSubmit = true;
    StatusUpdate( S_HANDSHAKE_SUCCESS,"Handshake successful.");
    return;
  } while (0);
  CStdString strBuf;
  strBuf.Format("Handshake failed: %s", strtok(NULL, "\n"));
  StatusUpdate(S_HANDSHAKE_ERROR, strBuf);
}

void CScrobbler::HandleSubmit(char *data)
{
  //  submit returned 
  m_bSubmitInProgress = false; //- this should already have been cancelled by the header callback

  StatusUpdate(S_DEBUG,data);

  // Doesn't take into account multiple-packet returns (not that I've seen one yet)...
  char seps[] = " \n\r";
  char * response = strtok(data, seps);
  if (!response) 
  {
    StatusUpdate(S_SUBMIT_INVALID_RESPONSE,"Submission failed: Response invalid");
    return;
  }
  if (stricmp("OK", response) == 0) 
  {
    StatusUpdate(S_SUBMIT_SUCCESS,"Submission succeeded.");

    StatusUpdate(S_DEBUG,m_strPostString.c_str());

    ClearCache();
    char *inttext = strtok(NULL, seps);
    if (inttext && (stricmp("INTERVAL", inttext) == 0)) 
    {
      m_Interval = atoi(strtok(NULL, seps));
      CStdString strBuf;
      strBuf.Format("Submit interval set to %i seconds.", m_Interval);
      StatusUpdate(S_SUBMIT_INTERVAL, strBuf);
    }
  } 
  else if (stricmp("BADPASS", response) == 0) 
  {
    StatusUpdate(S_SUBMIT_BAD_PASSWORD,"Submission failed: bad password.");
  } 
  else if (stricmp("FAILED", response) == 0) 
  {
    CStdString strBuf;
    strBuf.Format("Submission failed: %s", strtok(NULL, "\n"));
    StatusUpdate(S_SUBMIT_FAILED, strBuf);
    char *inttext = strtok(NULL, seps);
    if (inttext && (stricmp("INTERVAL", inttext) == 0)) 
    {
      m_Interval = atoi(strtok(NULL, seps));
      strBuf.Format("Submit interval set to %i seconds.", m_Interval);
      StatusUpdate(S_SUBMIT_INTERVAL, strBuf);
    }
  } 
  else if (stricmp("BADAUTH",response) == 0) 
  {
    StatusUpdate(S_SUBMIT_BADAUTH,"Submission failed: bad authorization.");
  } 
  else 
  {
    CStdString strBuf;
    strBuf.Format("Submission failed: %s", strtok(NULL, "\n"));
    StatusUpdate(S_SUBMIT_FAILED, strBuf);
  }
}

void CScrobbler::SetInterval(int in) 
{
  m_Interval = in;
  CStdString ret;
  ret.Format("Submit interval set to %d seconds.", in);
  StatusUpdate(S_SUBMIT_INTERVAL, ret);
}

void CScrobbler::GenSessionKey() 
{
  CStdString clear = m_strPassword + m_strChallenge;
  md5_state_t md5state;
  unsigned char md5pword[16];
  md5_init(&md5state);
  md5_append(&md5state, (unsigned const char *)clear.c_str(), (int)clear.length());
  md5_close(&md5state, md5pword);
  char key[33];
  strncpy(key, "\0", sizeof(key));
  for (int j = 0;j < 16;j++) 
  {
    char a[3];
    sprintf(a, "%02x", md5pword[j]);
    key[2*j] = a[0];
    key[2*j+1] = a[1];
  }
  m_strSessionKey = key;
}

int CScrobbler::LoadCache() 
{ 
  StatusUpdate(S_SUBMITTING,"load cache");

  int iNumEntries=0;
  CStdString strCache;

  CFile file;
  if (file.Open(CACHE_FILE))
  {
    CArchive ar(&file, CArchive::load);
    ar >> iNumEntries;
    ar >> strCache;
    ar.Close();
    file.Close();
    ::DeleteFile(CACHE_FILE);
    SetCache(strCache, iNumEntries);
    return 1;
  }
  return 0; 
}

int CScrobbler::SaveCache(const CStdString& strCache, int iNumEntries)
{ 
  if (iNumEntries<=0)
    return 0;

  CFile file;
  if (file.OpenForWrite(CACHE_FILE))
  {
    CArchive ar(&file, CArchive::store);
    ar << iNumEntries;
    ar << strCache;
    ar.Close();
    file.Close();
    return 1;
  }
  return 0; 
}

void CScrobbler::StatusUpdate(ScrobbleStatus status, const CStdString& strText)
{
  if(status == S_SUBMIT_INTERVAL || status == S_DEBUG)
  {
    CLog::Log(LOGDEBUG, "AudioScrobbler: %s", strText.c_str());
  }
  else if(S_HANDSHAKE_ERROR == status || S_SUBMIT_BAD_PASSWORD == status || 
        S_SUBMIT_FAILED == status   || S_HANDSHAKE_INVALID_RESPONSE == status ||
      S_SUBMIT_INVALID_RESPONSE == status || S_SUBMIT_BAD_PASSWORD == status ||
      S_HANDSHAKE_BAD_USERNAME  == status || S_CONNECT_ERROR == status )

  {
    // these are the bad ones just log
    CLog::Log(LOGINFO, "AudioScrobbler: %s", strText.c_str());
  }
  else if(S_HANDHAKE_NOTREADY == status)
  {
    CLog::Log(LOGNOTICE, "AudioScrobbler: %s", strText.c_str());
    if(!m_bConnectionWarningDone)
    {
      m_bConnectionWarningDone=true;
      CStdString strMsg=g_localizeStrings.Get(15204); // Unable to handshake: sleeping...
      StatusUpdate(strMsg);
    }
  }
  else if(S_HANDSHAKE_OLD_CLIENT == status)
  {
    CLog::Log(LOGNOTICE, "AudioScrobbler: %s", strText.c_str());
    if(!m_bUpdateWarningDone)
    {
      m_bUpdateWarningDone=true;
      CStdString strMsg=g_localizeStrings.Get(15205); // Please update xbmc
      StatusUpdate(strMsg);
    }

  }
  else if (S_SUBMIT_BADAUTH == status)
  {
    CLog::Log(LOGNOTICE, "AudioScrobbler: %s", strText.c_str());
    CStdString strMsg=g_localizeStrings.Get(15206); // Submission failed: bad authorization.
    StatusUpdate(strMsg);
  }
  else
  {
    CLog::Log(LOGNOTICE, "AudioScrobbler: %s", strText.c_str());
  }
}

void CScrobbler::StatusUpdate(const CStdString& strText)
{
  CStdString strAudioScrobbler=g_localizeStrings.Get(15200);  // AudioScrobbler
  g_application.m_guiDialogKaiToast.QueueNotification(strAudioScrobbler, strText);
}

void CScrobbler::WorkerThread()
{
  while (1) {
    WaitForSingleObject(m_hWorkerEvent, INFINITE);
    if (m_bCloseThread)
      break;

    StatusUpdate(S_DEBUG,"...");

    WaitForSingleObject(m_hHttpMutex, INFINITE);

    CHTTP http;
    CStdString strHtml;
    bool bSuccess;
    if (!m_bReadyToSubmit) 
    {
      bSuccess=http.Get(m_strHsString, strHtml);
      if (bSuccess)
      {
        LPSTR lphtml=strHtml.GetBuffer();
        HandleHandshake(lphtml);
        strHtml.ReleaseBuffer();
      }
    } 
    else 
    {
      bSuccess=http.Post(m_strSubmitUrl, m_strSubmit, strHtml);
      if (bSuccess)
      {
        LPSTR lphtml=strHtml.GetBuffer();
        HandleSubmit(lphtml);
        strHtml.ReleaseBuffer();
      }
    }

    if(!bSuccess)
    {
      if(m_bReadyToSubmit)
        m_bSubmitInProgress = false; // failed to post, means post is over

      StatusUpdate(S_CONNECT_ERROR,"Could not connect to server.");
    }


    ReleaseMutex(m_hHttpMutex);
    // OK, if this was a handshake, it failed since m_bReadyToSubmit isn't true. Submissions get cached.
    while (!m_bReadyToSubmit && !m_bCloseThread) 
    {
      StatusUpdate(S_HANDHAKE_NOTREADY,"Unable to handshake: sleeping...");
      Sleep(HS_FAIL_WAIT);
      // and try again.
      WaitForSingleObject(m_hHttpMutex, INFINITE);
      bSuccess=http.Get(m_strHsString, strHtml);
      if (bSuccess)
      {
        LPSTR lphtml=strHtml.GetBuffer();
        HandleHandshake(lphtml);
        strHtml.ReleaseBuffer();
      }
      ReleaseMutex(m_hHttpMutex);
    }
  }
}

void CScrobbler::SetSongStartTime()
{
  time(&m_SongStartTime);
}

CStdString CScrobbler::GetConnectionState()
{
  return (m_bReadyToSubmit ? g_localizeStrings.Get(15207) : g_localizeStrings.Get(15208));  // Connected : Not Connected
}

CStdString CScrobbler::GetSubmitInterval()
{
  CStdString strFormat=g_localizeStrings.Get(15209);  // Submit Interval %i
  CStdString strInterval;
  strInterval.Format(strFormat.c_str(), m_Interval);

  return strInterval;
}

CStdString CScrobbler::GetFilesCached()
{
  CStdString strFormat=g_localizeStrings.Get(15210);  // Cached %i Songs
  CStdString strCachedFiles;
  strCachedFiles.Format(strFormat.c_str(), m_iSongNum);

  return strCachedFiles;
}

void CScrobbler::SetSecsTillSubmit(int iSecs)
{
  m_iSecsTillSubmit=iSecs;
}

CStdString CScrobbler::GetSubmitState()
{
  CStdString strText;
  if (m_bSubmitInProgress)
  {
    strText=g_localizeStrings.Get(15211);  // Submitting...
    return strText;
  }

  if (m_bReadyToSubmit && m_bShouldSubmit)
  {
    CStdString strFormat=g_localizeStrings.Get(15212);  // Submitting in %i secs
    strText.Format(strFormat.c_str(), m_iSecsTillSubmit);
    return strText;
  }

  return strText;
}

void CScrobbler::SetSubmitSong(bool bSubmit)
{
  m_bShouldSubmit=bSubmit;
  m_iSecsTillSubmit=0;
}

bool CScrobbler::ShouldSubmit()
{
  return m_bShouldSubmit;
}
