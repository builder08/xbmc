/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "stdio.h"
#include "stdafx.h"
#include "Win32DllLoader.h"
#include "DllLoader.h"
#include "DllLoaderContainer.h"
#include "StdString.h"
#include "Util.h"
#include "utils/log.h"

#include "dll_tracker_library.h"
#include "dll_tracker_file.h"
#include "exports/emu_kernel32.h"
#include "exports/emu_msvcrt.h"

extern "C" FILE _iob[];

// our exports
Export win32_exports[] =
{
  // kernel32
  { "FindFirstFileA",                               -1, (void*)dllFindFirstFileA,                               NULL },
  { "GetFileAttributesA",                           -1, (void*)dllGetFileAttributesA,                        NULL },
  { "LoadLibraryA",                                 -1, (void*)dllLoadLibraryA,                              (void*)track_LoadLibraryA },
  { "FreeLibrary",                                  -1, (void*)dllFreeLibrary,                               (void*)track_FreeLibrary },
  { "GetProcAddress",                               -1, (void*)dllGetProcAddress,                            NULL },
  { "SetEvent",                                     -1, (void*)SetEvent,                                     NULL },
  { "GetModuleHandleA",                             -1, (void*)dllGetModuleHandleA,                          NULL },
  { "CreateFileA",                                  -1, (void*)dllCreateFileA,                               NULL },
  { "LoadLibraryExA",                               -1, (void*)dllLoadLibraryExA,                            (void*)track_LoadLibraryExA }, 
  { "GetModuleFileNameA",                           -1, (void*)dllGetModuleFileNameA,                        NULL },
// potential vfs stuff
//  { "CreateDirectoryA",                             -1, (void*)dllCreateDirectoryA,                          NULL },
//  { "LockFile",                                     -1, (void*)dllLockFile,                                  NULL },
//  { "LockFileEx",                                   -1, (void*)dllLockFileEx,                                NULL },
//  { "UnlockFile",                                   -1, (void*)dllUnlockFile,                                NULL },
//  { "CreateFileW",                                  -1, (void*)CreateFileW,                                  NULL },
//  { "GetFullPathNameW",                             -1, (void*)GetFullPathNameW,                             NULL },
//  { "GetTempPathW",                                 -1, (void*)GetTempPathW,                                 NULL },
//  { "GetFileAttributesW",                           -1, (void*)GetFileAttributesW,                           NULL },
//  { "DeleteFileW",                                  -1, (void*)DeleteFileW,                                  NULL },
//  { "GetFileSize",                                  -1, (void*)GetFileSize,                                  NULL },

// msvcrt
  { "_close",                     -1, (void*)dll_close,                     (void*)track_close},
  { "_lseek",                     -1, (void*)dll_lseek,                     NULL },
  { "_read",                      -1, (void*)dll_read,                      NULL },
  { "_write",                     -1, (void*)dll_write,                     NULL },
  { "_lseeki64",                  -1, (void*)dll_lseeki64,                  NULL },
  { "_open",                      -1, (void*)dll_open,                      (void*)track_open },
  { "fflush",                     -1, (void*)dll_fflush,                    NULL },
  { "fprintf",                    -1, (void*)dll_fprintf,                   NULL },
  { "fwrite",                     -1, (void*)dll_fwrite,                    NULL },
  { "putchar",                    -1, (void*)dll_putchar,                   NULL },
  { "_fstat",                     -1, (void*)dll_fstat,                     NULL },
  { "_mkdir",                     -1, (void*)dll_mkdir,                     NULL },
  { "_stat",                      -1, (void*)dll_stat,                      NULL },
  { "_findclose",                 -1, (void*)dll_findclose,                 NULL },
  { "_findfirst",                 -1, (void*)dll_findfirst,                 NULL },
  { "_findnext",                  -1, (void*)dll_findnext,                  NULL },
  { "fclose",                     -1, (void*)dll_fclose,                    (void*)track_fclose},
  { "feof",                       -1, (void*)dll_feof,                      NULL },
  { "fgets",                      -1, (void*)dll_fgets,                     NULL },
  { "fopen",                      -1, (void*)dll_fopen,                     (void*)track_fopen},
  { "putc",                       -1, (void*)dll_putc,                      NULL },
  { "fputc",                      -1, (void*)dll_fputc,                     NULL },
  { "fputs",                      -1, (void*)dll_fputs,                     NULL },
  { "fread",                      -1, (void*)dll_fread,                     NULL },
  { "fseek",                      -1, (void*)dll_fseek,                     NULL },
  { "ftell",                      -1, (void*)dll_ftell,                     NULL },
  { "getc",                       -1, (void*)dll_getc,                      NULL },
  { "fgetc",                      -1, (void*)dll_getc,                      NULL },
  { "rewind",                     -1, (void*)dll_rewind,                    NULL },
  { "vfprintf",                   -1, (void*)dll_vfprintf,                  NULL },
  { "fgetpos",                    -1, (void*)dll_fgetpos,                   NULL },
  { "fsetpos",                    -1, (void*)dll_fsetpos,                   NULL },
  { "_stati64",                   -1, (void*)dll_stati64,                   NULL },
  { "_fstati64",                  -1, (void*)dll_fstati64,                  NULL },
  { "_telli64",                   -1, (void*)dll_telli64,                   NULL },
  { "_tell",                      -1, (void*)dll_tell,                      NULL },
  { "_fileno",                    -1, (void*)dll_fileno,                    NULL },
  { "ferror",                     -1, (void*)dll_ferror,                    NULL },
  { "freopen",                    -1, (void*)dll_freopen,                   (void*)track_freopen},
  { "fscanf",                     -1, (void*)fscanf,                        NULL },
  { "ungetc",                     -1, (void*)dll_ungetc,                    NULL },
  { "_fdopen",                    -1, (void*)dll_fdopen,                    NULL },
  { "clearerr",                   -1, (void*)dll_clearerr,                  NULL },
  // for debugging
  { "printf",                     -1, (void*)dllprintf,                     NULL },
  { "vprintf",                    -1, (void*)dllvprintf,                    NULL },
  { "perror",                     -1, (void*)dllperror,                     NULL },
  { "puts",                       -1, (void*)dllputs,                       NULL },
  // workarounds for non-win32 signals
  { "signal",                     -1, (void*)dll_signal,                    NULL },

  // reading/writing from stdin/stdout needs this
  { "_iob",                       -1, (void*)&_iob,                         NULL },

  // libdvdnav + python need this (due to us using dll_putenv() to put stuff only?)
  { "getenv",                     -1, (void*)dll_getenv,                    NULL },

  { NULL,                          -1, NULL,                                NULL }
};

// stuff for python
extern "C"
{
  char* xbp_getcwd(char *buf, int size);
  int xbp_chdir(const char *dirname);
  int xbp_access(const char *path, int mode);
  int xbp_unlink(const char *filename);
  int xbp_chmod(const char *filename, int pmode);
  int xbp_rmdir(const char *dirname);
  int xbp_utime(const char *filename, struct utimbuf *times);
  int xbp_rename(const char *oldname, const char *newname);
  int xbp_mkdir(const char *dirname);
  int xbp_open(const char *filename, int oflag, int pmode);
};

Export win32_python_exports[] =
{
  // these just correct for path separators and call the base
  { "access",                               -1, (void*)xbp_access,                               NULL },
  { "unlink",                               -1, (void*)xbp_unlink,                               NULL },
  { "chmod",                               -1, (void*)xbp_chmod,                               NULL },
  { "rmdir",                               -1, (void*)xbp_rmdir,                               NULL },
  { "utime",                               -1, (void*)xbp_utime,                               NULL },
  { "rename",                               -1, (void*)xbp_rename,                               NULL },
  { "mkdir",                               -1, (void*)xbp_mkdir,                               NULL },
  { "open",                               -1, (void*)xbp_open,                               NULL },
//  { "opendir",                               -1, (void*)xbp_opendir,                               NULL }, _LINUX only

  // special workaround just for python
  { "_chdir",                               -1, (void*)xbp_chdir,                               NULL },
  { "_getcwd",                              -1, (void*)xbp_getcwd,                               NULL },
  { "_putenv",                              -1, (void*)dll_putenv,                              NULL },
  { "__p__environ",               -1, (void*)dll___p__environ,              NULL },
  { NULL,                          -1, NULL,                                NULL }
};

Win32DllLoader::Win32DllLoader(const char *dll) : LibraryLoader(dll)
{
  m_dllHandle = NULL;
  DllLoaderContainer::RegisterDll(this);
}

Win32DllLoader::~Win32DllLoader()
{
  if (m_dllHandle)
    Unload();
  DllLoaderContainer::UnRegisterDll(this);
}

bool Win32DllLoader::Load()
{
  if (m_dllHandle != NULL)
    return true;

  CStdString strFileName= _P(GetFileName());
  CLog::Log(LOGDEBUG, "%s(%s)\n", __FUNCTION__, strFileName.c_str());
  //int flags = RTLD_LAZY;
  //if (m_bGlobal) flags |= RTLD_GLOBAL;
  //m_soHandle = dlopen(strFileName.c_str(), flags);

  // make sure we set working directory
  CStdString path;
  CUtil::GetParentPath(strFileName, path);
  char currentPath[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, currentPath);
  SetCurrentDirectory(path.c_str());
  m_dllHandle = LoadLibrary(strFileName.c_str());
  SetCurrentDirectory(currentPath);
  if (!m_dllHandle)
  {
    CLog::Log(LOGERROR, "%s: Unable to load %s (%d)", __FUNCTION__, strFileName.c_str(), GetLastError());
    return false;
  }

  // handle functions that the dll imports
  OverrideImports(strFileName);

  return true;
}

void Win32DllLoader::Unload()
{
  CLog::Log(LOGDEBUG, "%s %s\n", __FUNCTION__, GetName());

  // restore our imports
  RestoreImports();

  if (m_dllHandle)
  {
    if (!FreeLibrary(m_dllHandle))
       CLog::Log(LOGERROR, "%s Unable to unload %s", __FUNCTION__, GetName());
  }

  m_dllHandle = NULL;
}

int Win32DllLoader::ResolveExport(const char* symbol, void** f)
{
  if (!m_dllHandle && !Load())
  {
    CLog::Log(LOGWARNING, "%s - Unable to resolve: %s %s, reason: DLL not loaded", __FUNCTION__, GetName(), symbol);
    return 0;
  }

  void *s = GetProcAddress(m_dllHandle, symbol);

  if (!s)
  {
    CLog::Log(LOGWARNING, "%s - Unable to resolve: %s %s", __FUNCTION__, GetName(), symbol);
    return 0;
  }

  *f = s;
  return 1;
}

bool Win32DllLoader::IsSystemDll()
{
  return false;
}

HMODULE Win32DllLoader::GetHModule()
{
  return m_dllHandle;
}

bool Win32DllLoader::HasSymbols()
{
  return false;
}

void Win32DllLoader::OverrideImports(const CStdString &dll)
{
  BYTE* image_base = (BYTE*)GetModuleHandle(dll.c_str());

  if (!image_base)
  {
    CLog::Log(LOGERROR, "%s - unable to GetModuleHandle for dll %s", dll.c_str());
    return;
  }

  PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)image_base;
  PIMAGE_NT_HEADERS nt_header = (PIMAGE_NT_HEADERS)(image_base + dos_header->e_lfanew); // e_lfanew = value at 0x3c

  PIMAGE_IMPORT_DESCRIPTOR imp_desc = (PIMAGE_IMPORT_DESCRIPTOR)(
    image_base + nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

  if (!imp_desc)
  {
    CLog::Log(LOGERROR, "%s - unable to get import directory for dll %s", dll.c_str());
    return;
  }

  // loop over all imported dlls 
  for (int i = 0; imp_desc[i].Characteristics != 0; i++)
  {
    char *dllName = (char*)(image_base + imp_desc[i].Name);

    // check whether this is one of our dll's.
    if (NeedsHooking(dllName))
    {
      // this will do a loadlibrary on it, which should effectively make sure that it's hooked
      // Note that the library has obviously already been loaded by the OS (as it's implicitly linked)
      // so all this will do is insert our hook and make sure our DllLoaderContainer knows about it
      HMODULE hModule = dllLoadLibraryA(dllName); 
      if (hModule)
        m_referencedDlls.push_back(hModule);
    }

    PIMAGE_THUNK_DATA orig_first_thunk = (PIMAGE_THUNK_DATA)(image_base + imp_desc[i].OriginalFirstThunk);
    PIMAGE_THUNK_DATA first_thunk = (PIMAGE_THUNK_DATA)(image_base + imp_desc[i].FirstThunk);

    // and then loop over all imported functions
    for (int j = 0; orig_first_thunk[j].u1.Function != 0; j++)
    {
      void *fixup = NULL;
      if (orig_first_thunk[j].u1.Function & 0x80000000)
        ResolveOrdinal(dllName, (orig_first_thunk[j].u1.Ordinal & 0x7fffffff), &fixup);
      else
      { // resolve by name
        PIMAGE_IMPORT_BY_NAME orig_imports_by_name = (PIMAGE_IMPORT_BY_NAME)(
          image_base + orig_first_thunk[j].u1.AddressOfData);

        ResolveImport(dllName, (char*)orig_imports_by_name->Name, &fixup);
      }/*
      if (!fixup)
      { // create a dummy function for tracking purposes
        PIMAGE_IMPORT_BY_NAME orig_imports_by_name = (PIMAGE_IMPORT_BY_NAME)(
          image_base + orig_first_thunk[j].u1.AddressOfData);
        fixup = CreateDummyFunction(dllName, (char*)orig_imports_by_name->Name);
      }*/
      if (fixup)
      {
        // save the old function
        Import import;
        import.table = &first_thunk[j].u1.Function;
        import.function = first_thunk[j].u1.Function;
        m_overriddenImports.push_back(import);

        DWORD old_prot = 0;

        // change to protection settings so we can write to memory area
        VirtualProtect((PVOID)&first_thunk[j].u1.Function, 4, PAGE_EXECUTE_READWRITE, &old_prot);

        // patch the address of function to point to our overridden version
        first_thunk[j].u1.Function = (DWORD)fixup;

        // reset to old settings
        VirtualProtect((PVOID)&first_thunk[j].u1.Function, 4, old_prot, &old_prot);
      }
    }
  }
}

bool Win32DllLoader::NeedsHooking(const char *dllName)
{
  LibraryLoader *loader = DllLoaderContainer::GetModule(dllName);
  if (loader)
  {
    // may have hooked this already (we can have repeats in the import table)
    for (unsigned int i = 0; i < m_referencedDlls.size(); i++)
    {
      if (loader->GetHModule() == m_referencedDlls[i])
        return false;
    }
  }
  HMODULE hModule = GetModuleHandle(dllName);
  char filepath[MAX_PATH];
  GetModuleFileName(hModule, filepath, MAX_PATH);
  CStdString dllPath = filepath;

  // compare this filepath with Q:
  CStdString homePath = _P("Q:");
  return strncmp(homePath.c_str(), filepath, homePath.GetLength()) == 0;
}

void Win32DllLoader::RestoreImports()
{
  // first unhook any referenced dll's
  for (unsigned int i = 0; i < m_referencedDlls.size(); i++)
  {
    HMODULE module = m_referencedDlls[i];
    dllFreeLibrary(module);  // should unhook things for us
  }
  m_referencedDlls.clear();

  for (unsigned int i = 0; i < m_overriddenImports.size(); i++)
  {
    Import &import = m_overriddenImports[i];

    // change to protection settings so we can write to memory area
    DWORD old_prot = 0;
    VirtualProtect(import.table, 4, PAGE_EXECUTE_READWRITE, &old_prot);

    *(DWORD *)import.table = import.function;

    // reset to old settings
    VirtualProtect(import.table, 4, old_prot, &old_prot);
  }
}

bool Win32DllLoader::ResolveImport(const char *dllName, const char *functionName, void **fixup)
{
  char *dll = GetName();
  if (strstr(dll, "python24.dll") || strstr(dll, ".pyd"))
  { // special case for python
    Export *exp = win32_python_exports;
    while (exp->name)
    {
      if (strcmp(exp->name, functionName) == 0)
      {
        *fixup = exp->function;
        return true;
      }
      exp++;
    }
  }
  Export *exp = win32_exports;
  while (exp->name)
  {
    if (strcmp(exp->name, functionName) == 0)
    { // TODO: Should we be tracking stuff?
      if (0)
        *fixup = exp->track_function;
      else
        *fixup = exp->function;
      return true;
    }
    exp++;
  }
  return false;
}

bool Win32DllLoader::ResolveOrdinal(const char *dllName, unsigned long ordinal, void **fixup)
{
  Export *exp = win32_exports;
  while (exp->name)
  {
    if (exp->ordinal == ordinal)
    { // TODO: Should we be tracking stuff?
      if (0)
        *fixup = exp->track_function;
      else
        *fixup = exp->function;
      return true;
    }
    exp++;
  }
  return false;
}