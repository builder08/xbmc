
#include "../stdafx.h" 
/*
 * XBoxMediaPlayer
 * Copyright (c) 2002 Frodo
 * Portions Copyright (c) by the authors of ffmpeg and xvid
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include "FileHD.h"
#include "../Util.h"
#include <sys/stat.h>
using namespace XFILE;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//*********************************************************************************************
CFileHD::CFileHD()
    : m_hFile(INVALID_HANDLE_VALUE)
{}

//*********************************************************************************************
CFileHD::~CFileHD()
{
  if (m_hFile != INVALID_HANDLE_VALUE) Close();
}
//*********************************************************************************************
bool CFileHD::Open(const CURL& url, bool bBinary)
{
  CStdString strFile(url.GetFileName());
  strFile.Replace("/", "\\");

  m_hFile.attach(CreateFile(strFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL));
  if (!m_hFile.isValid()) return false;

  m_i64FilePos = 0;
  LARGE_INTEGER i64Size;
  GetFileSizeEx((HANDLE)m_hFile, &i64Size);
  m_i64FileLength = i64Size.QuadPart;
  Seek(0, SEEK_SET);

  return true;
}

bool CFileHD::Exists(const CURL& url)
{
  CStdString strFile(url.GetFileName());
  strFile.Replace("/", "\\");

  struct __stat64 buffer;
  return (_stat64(strFile.c_str(), &buffer)==0);
}

int CFileHD::Stat(const CURL& url, struct __stat64* buffer)
{
  CStdString strFile(url.GetFileName());
  strFile.Replace("/", "\\");

  return _stat64(strFile.c_str(), buffer);
}


//*********************************************************************************************
bool CFileHD::OpenForWrite(const CURL& url, bool bBinary, bool bOverWrite)
{
  // make sure it's a legal FATX filename (we are writing to the harddisk)
  CStdString strFileNameAndPath, strPath, strFileName;
  url.GetURL(strFileNameAndPath);

  CUtil::Split(strFileNameAndPath, strPath, strFileName);
  CStdString strNewFile = CUtil::MakeLegalFileName(strFileName.c_str(), true);

  strPath += strNewFile;
  strPath.Replace("/", "\\");

  m_hFile.attach(CreateFile(strPath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, bOverWrite ? CREATE_ALWAYS : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));
  if (!m_hFile.isValid()) return false;

  m_i64FilePos = 0;
  LARGE_INTEGER i64Size;
  GetFileSizeEx((HANDLE)m_hFile, &i64Size);
  m_i64FileLength = i64Size.QuadPart;
  Seek(0, SEEK_SET);

  return true;
}

//*********************************************************************************************
unsigned int CFileHD::Read(void *lpBuf, __int64 uiBufSize)
{
  if (!m_hFile.isValid()) return 0;
  DWORD nBytesRead;
  if ( ReadFile((HANDLE)m_hFile, lpBuf, (DWORD)uiBufSize, &nBytesRead, NULL) )
  {
    m_i64FilePos += nBytesRead;
    return nBytesRead;
  }
  return 0;
}

//*********************************************************************************************
unsigned int CFileHD::Write(void *lpBuf, __int64 uiBufSize)
{
  if (!m_hFile.isValid()) return 0;
  DWORD nBytesWriten;
  if ( WriteFile((HANDLE)m_hFile, lpBuf, (DWORD)uiBufSize, &nBytesWriten, NULL) )
  {
    return nBytesWriten;
  }
  return 0;
}

//*********************************************************************************************
void CFileHD::Close()
{
  m_hFile.reset();
}

//*********************************************************************************************
__int64 CFileHD::Seek(__int64 iFilePosition, int iWhence)
{
  LARGE_INTEGER lPos, lNewPos;
  lPos.QuadPart = iFilePosition;
  int bSuccess;
  switch (iWhence)
  {
  case SEEK_SET:
    bSuccess = SetFilePointerEx((HANDLE)m_hFile, lPos, &lNewPos, FILE_BEGIN);
    break;

  case SEEK_CUR:
    bSuccess = SetFilePointerEx((HANDLE)m_hFile, lPos, &lNewPos, FILE_CURRENT);
    break;

  case SEEK_END:
    bSuccess = SetFilePointerEx((HANDLE)m_hFile, lPos, &lNewPos, FILE_END);
    break;
  }
  m_i64FilePos = lNewPos.QuadPart;
  if (bSuccess)
    return (lNewPos.QuadPart);
  else
    return -1;
}

//*********************************************************************************************
__int64 CFileHD::GetLength()
{
  LARGE_INTEGER i64Size;
  GetFileSizeEx((HANDLE)m_hFile, &i64Size);
  return i64Size.QuadPart;

  // return m_i64FileLength;
}

//*********************************************************************************************
__int64 CFileHD::GetPosition()
{
  return m_i64FilePos;
}


//*********************************************************************************************
bool CFileHD::ReadString(char *szLine, int iLineLength)
{
  szLine[0] = 0;
  if (!m_hFile.isValid()) return false;
  __int64 iFilePos = GetPosition();

  int iBytesRead = Read( (unsigned char*)szLine, iLineLength);
  if (iBytesRead <= 0)
  {
    return false;
  }

  szLine[iBytesRead] = 0;

  for (int i = 0; i < iBytesRead; i++)
  {
    if ('\n' == szLine[i])
    {
      if ('\r' == szLine[i + 1])
      {
        szLine[i + 1] = 0;

        Seek(iFilePos + i + 2, SEEK_SET);
      }
      else
      {
        // end of line
        szLine[i + 1] = 0;

        Seek(iFilePos + i + 1, SEEK_SET);
      }
      break;
    }
    else if ('\r' == szLine[i])
    {
      if ('\n' == szLine[i + 1])
      {
        szLine[i + 1] = 0;

        Seek(iFilePos + i + 2, SEEK_SET);
      }
      else
      {
        // end of line
        szLine[i + 1] = 0;
        Seek(iFilePos + i + 1, SEEK_SET);

      }
      break;
    }
  }
  if (iBytesRead > 0)
  {
    return true;
  }
  return false;
}


int CFileHD::Write(const void* lpBuf, __int64 uiBufSize)
{
  if (!m_hFile.isValid()) return -1;
  DWORD dwNumberOfBytesWritten = 0;
  WriteFile((HANDLE)m_hFile, lpBuf, (DWORD)uiBufSize, &dwNumberOfBytesWritten, NULL);
  return (int)dwNumberOfBytesWritten;
}

bool CFileHD::Delete(const char* strFileName)
{
  return ::DeleteFile(strFileName) ? true : false;
}

bool CFileHD::Rename(const char* strFileName, const char* strNewFileName)
{
  return ::MoveFile(strFileName, strNewFileName) ? true : false;
}

void CFileHD::Flush()
{
  ::FlushFileBuffers(m_hFile);
}
