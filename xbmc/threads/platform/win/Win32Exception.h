#pragma once

/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <windows.h>
#include <exception>
#ifndef WIN_COMMONS_EXCEPTION_H_INCLUDED
#define WIN_COMMONS_EXCEPTION_H_INCLUDED
#include "commons/Exception.h"
#endif


class win32_exception: public XbmcCommons::UncheckedException
{
public:
    typedef const void* Address; // OK on Win32 platform

    static void install_handler();
    static void set_version(std::string version) { mVersion = version; };
    virtual const char* what() const { return mWhat; };
    Address where() const { return mWhere; };
    unsigned code() const { return mCode; };
    virtual void LogThrowMessage(const char *prefix) const;
    static bool write_minidump(EXCEPTION_POINTERS* pEp);
    static bool write_stacktrace(EXCEPTION_POINTERS* pEp);
protected:
    win32_exception(EXCEPTION_POINTERS*, const char* classname = NULL);
    static void translate(unsigned code, EXCEPTION_POINTERS* info);

    inline bool write_minidump() const { return write_minidump(mExceptionPointers); };
    inline bool write_stacktrace() const { return write_stacktrace(mExceptionPointers); };
private:
    const char* mWhat;
    Address mWhere;
    unsigned mCode;
    EXCEPTION_POINTERS *mExceptionPointers;
    static std::string mVersion;
};

class access_violation: public win32_exception
{
  enum access_type
  {
    Invalid,
    Read,
    Write,
    DEP
  };

public:
    Address address() const { return mBadAddress; };
    virtual void LogThrowMessage(const char *prefix) const;
protected:
    friend void win32_exception::translate(unsigned code, EXCEPTION_POINTERS* info);
private:
    access_type mAccessType;
    Address mBadAddress;
    access_violation(EXCEPTION_POINTERS* info);
};
