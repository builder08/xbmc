/*
* XBoxMediaPlayer
* Copyright (c) 2002 d7o3g4q and RUNTiME
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

// IoSupport.cpp: implementation of the CIoSupport class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "IoSupport.h"
#ifdef HAS_UNDOCUMENTED
#ifdef _XBOX
#include "Undocumented.h"
#include "../xbox/XKExports.h"
#else
#include "ntddcdrm.h"
#endif
#endif

typedef struct
{
  char cDriveLetter;
  char* szDevice;
  int iPartition;
}
stDriveMapping;

#ifdef _XBOX
stDriveMapping driveMapping[] =
  {
    { 'C', "Harddisk0\\Partition2", 2},
    { 'D', "Cdrom0", -1},
    { 'E', "Harddisk0\\Partition1", 1},
    { 'X', "Harddisk0\\Partition3", 3},
    { 'Y', "Harddisk0\\Partition4", 4},
    { 'Z', "Harddisk0\\Partition5", 5},
  };
#else
stDriveMapping driveMapping[] =
  {
    { 'C', "C:", 2},
    { 'D', "D:", -1},
    { 'E', "E:", 1},
    { 'X', "X:", 3},
    { 'Y', "Y:", 4},
    { 'Z', "Z:", 5},
  };

#include "../../Tools/Win32/XBMC_PC.h"
#endif
#define NUM_OF_DRIVES ( sizeof( driveMapping) / sizeof( driveMapping[0] ) )
PVOID CIoSupport::m_rawXferBuffer;


// cDriveLetter e.g. 'D'
// szDevice e.g. "Cdrom0" or "Harddisk0\Partition6"
HRESULT CIoSupport::MapDriveLetter(char cDriveLetter, char * szDevice)
{
#ifdef _XBOX
  char szSourceDevice[MAX_PATH+32];
  char szDestinationDrive[16];
  NTSTATUS status;

  CLog::Log(LOGNOTICE, "Mapping drive %c to %s", cDriveLetter, szDevice);

  sprintf(szSourceDevice, "\\Device\\%s", szDevice);
  sprintf(szDestinationDrive, "\\??\\%c:", cDriveLetter);

  ANSI_STRING DeviceName, LinkName;

  RtlInitAnsiString(&DeviceName, szSourceDevice);
  RtlInitAnsiString(&LinkName, szDestinationDrive);

  status = IoCreateSymbolicLink(&LinkName, &DeviceName);

  if (!NT_SUCCESS(status))
    CLog::Log(LOGERROR, "Failed to create symbolic link!  (status=0x%08x)", status);

  return status;
#else
  if ((strnicmp(szDevice, "Harddisk0", 9) == 0) ||
      (strnicmp(szDevice, "Cdrom", 5) == 0))
    return S_OK;
#endif
}

// cDriveLetter e.g. 'D'
HRESULT CIoSupport::UnmapDriveLetter(char cDriveLetter)
{
#ifdef _XBOX
  char szDestinationDrive[16];
  ANSI_STRING LinkName;
  NTSTATUS status;

  CLog::Log(LOGNOTICE, "Unmapping drive %c", cDriveLetter);

  sprintf(szDestinationDrive, "\\??\\%c:", cDriveLetter);
  RtlInitAnsiString(&LinkName, szDestinationDrive);

  status =  IoDeleteSymbolicLink(&LinkName);

  if (!NT_SUCCESS(status))
    CLog::Log(LOGERROR, "Failed to delete symbolic link!  (status=0x%08x)", status);

  return status;
#else 
  return S_OK;
#endif
}

HRESULT CIoSupport::RemapDriveLetter(char cDriveLetter, char * szDevice)
{
  UnmapDriveLetter(cDriveLetter);

  return MapDriveLetter(cDriveLetter, szDevice);
}
// to be used with CdRom devices.
HRESULT CIoSupport::Dismount(char * szDevice)
{
#ifdef _XBOX
  char szSourceDevice[MAX_PATH+32];
  ANSI_STRING DeviceName;
  NTSTATUS status;

  CLog::Log(LOGNOTICE, "Dismounting %s", szDevice);

  sprintf(szSourceDevice, "\\Device\\%s", szDevice);

  RtlInitAnsiString(&DeviceName, szSourceDevice);

  status = IoDismountVolumeByName(&DeviceName);

  if (!NT_SUCCESS(status))
    CLog::Log(LOGERROR, "Failed to dismount volume!  (status=0x%08x)", status);

  return status;
#else
  return S_OK;
#endif
}

void CIoSupport::GetPartition(char cDriveLetter, char * szPartition)
{
  char upperLetter = toupper(cDriveLetter);
  if (upperLetter >= 'F' && upperLetter <= 'O')
  {
    sprintf(szPartition, "Harddisk0\\Partition%u", upperLetter - 'A' + 1);
    return;
  }
  for (int i=0; i < NUM_OF_DRIVES; i++)
    if (driveMapping[i].cDriveLetter == upperLetter)
    {
      strcpy(szPartition, driveMapping[i].szDevice);
      return;
    }
  *szPartition = 0;
}

void CIoSupport::GetDrive(char * szPartition, char * cDriveLetter)
{
  int part_str_len = strlen(szPartition);
  int part_num;

  if (part_str_len < 19)
  {
    *cDriveLetter = 0;
    return;
  }
  
  part_num = atoi(szPartition + 19);

  if (part_num >= 6)
  {
    *cDriveLetter = part_num + 'A' - 1;
    return;
  }
  for (int i=0; i < NUM_OF_DRIVES; i++)
    if (strnicmp(driveMapping[i].szDevice, szPartition, strlen(driveMapping[i].szDevice)) == 0)
    {
      *cDriveLetter = driveMapping[i].cDriveLetter;
      return;
    }
  *cDriveLetter = 0;
}

HRESULT CIoSupport::EjectTray()
{
#ifdef _XBOX
  HalWriteSMBusValue(0x20, 0x0C, FALSE, 0);  // eject tray
#endif
  return S_OK;
}

HRESULT CIoSupport::CloseTray()
{
#ifdef _XBOX
  HalWriteSMBusValue(0x20, 0x0C, FALSE, 1);  // close tray
#endif
  return S_OK;
}

DWORD CIoSupport::GetTrayState()
{
#ifdef _XBOX
  DWORD dwTrayState, dwTrayCount;
  if (g_advancedSettings.m_usePCDVDROM)
  {
    dwTrayState = TRAY_CLOSED_MEDIA_PRESENT;
  }
  else
  {
    HalReadSMCTrayState(&dwTrayState, &dwTrayCount);
  }

  return dwTrayState;
#endif
  return DRIVE_NOT_READY;
}

HRESULT CIoSupport::Shutdown()
{
#ifdef _XBOX
  // fails assertion on debug bios (symptom lockup unless running dr watson)
  // so you can continue past the failed assertion).
  if (IsDebug())
    return E_FAIL;
  KeRaiseIrqlToDpcLevel();
  HalInitiateShutdown();
#endif
  return S_OK;
}

HANDLE CIoSupport::OpenCDROM()
{
  HANDLE hDevice;

#ifdef _XBOX
  IO_STATUS_BLOCK status;
  ANSI_STRING filename;
  OBJECT_ATTRIBUTES attributes;
  RtlInitAnsiString(&filename, "\\Device\\Cdrom0");
  InitializeObjectAttributes(&attributes, &filename, OBJ_CASE_INSENSITIVE, NULL);
  if (!NT_SUCCESS(NtOpenFile(&hDevice,
                             GENERIC_READ | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                             &attributes,
                             &status,
                             FILE_SHARE_READ,
                             FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT)))
  {
    return NULL;
  }
#else

  hDevice = CreateFile("\\\\.\\Cdrom0", GENERIC_READ, FILE_SHARE_READ,
                       NULL, OPEN_EXISTING,
                       FILE_FLAG_RANDOM_ACCESS, NULL );

#endif
  return hDevice;
}

void CIoSupport::AllocReadBuffer()
{
  m_rawXferBuffer = GlobalAlloc(GPTR, RAW_SECTOR_SIZE);
}

void CIoSupport::FreeReadBuffer()
{
  GlobalFree(m_rawXferBuffer);
}

INT CIoSupport::ReadSector(HANDLE hDevice, DWORD dwSector, LPSTR lpczBuffer)

{
  DWORD dwRead;
  DWORD dwSectorSize = 2048;
  LARGE_INTEGER Displacement;

  Displacement.QuadPart = ((INT64)dwSector) * dwSectorSize;

  for (int i = 0; i < 5; i++)
  {
    SetFilePointer(hDevice, Displacement.LowPart, &Displacement.HighPart, FILE_BEGIN);

    if (ReadFile(hDevice, m_rawXferBuffer, dwSectorSize, &dwRead, NULL))
    {
      memcpy(lpczBuffer, m_rawXferBuffer, dwSectorSize);
      return dwRead;
    }
  }

  OutputDebugString("CD Read error\n");
  return -1;
}


INT CIoSupport::ReadSectorMode2(HANDLE hDevice, DWORD dwSector, LPSTR lpczBuffer)
{
#ifdef HAS_DVD_DRIVE
  DWORD dwBytesReturned;
  RAW_READ_INFO rawRead = {0};

  // Oddly enough, DiskOffset uses the Red Book sector size
  rawRead.DiskOffset.QuadPart = 2048 * dwSector;
  rawRead.SectorCount = 1;
  rawRead.TrackMode = XAForm2;

  for (int i = 0; i < 5; i++)
  {
    if ( DeviceIoControl( hDevice,
                          IOCTL_CDROM_RAW_READ,
                          &rawRead,
                          sizeof(RAW_READ_INFO),
                          m_rawXferBuffer,
                          RAW_SECTOR_SIZE,
                          &dwBytesReturned,
                          NULL ) != 0 )
    {
      memcpy(lpczBuffer, m_rawXferBuffer, MODE2_DATA_SIZE);
      return MODE2_DATA_SIZE;
    }
    else
    {
      int iErr = GetLastError();
      //   printf("%i\n", iErr);
    }
  }
#endif
  return -1;
}

INT CIoSupport::ReadSectorCDDA(HANDLE hDevice, DWORD dwSector, LPSTR lpczBuffer)
{
#ifdef HAS_DVD_DRIVE
  DWORD dwBytesReturned;
  RAW_READ_INFO rawRead;

  // Oddly enough, DiskOffset uses the Red Book sector size
  rawRead.DiskOffset.QuadPart = 2048 * dwSector;
  rawRead.SectorCount = 1;
  rawRead.TrackMode = CDDA;

  for (int i = 0; i < 5; i++)
  {
    if ( DeviceIoControl( hDevice,
                          IOCTL_CDROM_RAW_READ,
                          &rawRead,
                          sizeof(RAW_READ_INFO),
                          m_rawXferBuffer,
                          sizeof(RAW_SECTOR_SIZE),
                          &dwBytesReturned,
                          NULL ) != 0 )
    {
      memcpy(lpczBuffer, m_rawXferBuffer, RAW_SECTOR_SIZE);
      return RAW_SECTOR_SIZE;
    }
  }
#endif
  return -1;
}

VOID CIoSupport::CloseCDROM(HANDLE hDevice)
{
  CloseHandle(hDevice);
}

// returns true if this is a debug machine
BOOL CIoSupport::IsDebug()
{
#ifdef _XBOX
  return (XboxKrnlVersion->Qfe & 0x8000) || ((DWORD)XboxHardwareInfo & 0x10);
#else
  return FALSE;
#endif
}


VOID CIoSupport::GetXbePath(char* szDest)
{
#ifdef _XBOX
  //Function to get the XBE Path like:
  //E:\DevKit\xbplayer\xbplayer.xbe

  char szTemp[MAX_PATH];
  char cDriveLetter = 0;

  strncpy(szTemp, XeImageFileName->Buffer + 8, XeImageFileName->Length - 8);
  szTemp[20] = 0;
  GetDrive(szTemp, &cDriveLetter);

  strncpy(szTemp, XeImageFileName->Buffer + 29, XeImageFileName->Length - 29);
  szTemp[XeImageFileName->Length - 29] = 0;

  sprintf(szDest, "%c:\\%s", cDriveLetter, szTemp);

#else
  GetCurrentDirectory(XBMC_MAX_PATH, szDest);
  strcat(szDest, "\\XBMC_PC.exe");
#endif
}

bool CIoSupport::DriveExists(char cDriveLetter)
{
#ifdef _XBOX
  char szDrive[32];
  ANSI_STRING drive_string;
  NTSTATUS status;
  HANDLE hTemp;
  OBJECT_ATTRIBUTES oa;
  IO_STATUS_BLOCK iosb;

  sprintf(szDrive, "\\??\\%c:", cDriveLetter);
  RtlInitAnsiString(&drive_string, szDrive);

  oa.Attributes = OBJ_CASE_INSENSITIVE;
  oa.ObjectName = &drive_string;
  oa.RootDirectory = 0;

  status = NtOpenFile(&hTemp, GENERIC_READ | GENERIC_WRITE, &oa, &iosb, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_SYNCHRONOUS_IO_ALERT);

  if (NT_SUCCESS(status))
  {
    CloseHandle(hTemp);
    return true;
  }

  return false;
#else
  cDriveLetter = toupper(cDriveLetter);

  if (cDriveLetter < 'A' || cDriveLetter > 'Z')
    return false;

  DWORD drivelist;
  DWORD bitposition = cDriveLetter - 'A';

  drivelist = GetLogicalDrives();

  if (!drivelist)
    return false;

  return (drivelist >> bitposition) & 1;
#endif
}

bool CIoSupport::PartitionExists(int nPartition)
{
#ifdef _XBOX
  char szPartition[32];
  ANSI_STRING part_string;
  NTSTATUS status;
  HANDLE hTemp;
  OBJECT_ATTRIBUTES oa;
  IO_STATUS_BLOCK iosb;

  sprintf(szPartition, "\\Device\\Harddisk0\\Partition%u", nPartition);
  RtlInitAnsiString(&part_string, szPartition);

  oa.Attributes = OBJ_CASE_INSENSITIVE;
  oa.ObjectName = &part_string;
  oa.RootDirectory = 0;

  status = NtOpenFile(&hTemp, GENERIC_READ | GENERIC_WRITE, &oa, &iosb, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_SYNCHRONOUS_IO_ALERT);

  if (NT_SUCCESS(status))
  {
    CloseHandle(hTemp);
    return true;
  }
 
  return false;
#else
  return false;
#endif
}