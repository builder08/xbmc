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

#ifdef TARGET_WINDOWS
#error "The threading options for the cryptography libraries don't need to be and shouldn't be set on Windows. Do not include CryptThreading in your windows project."
#endif

#include "CryptThreading.h"
#include "threads/Thread.h"
#include "utils/log.h"

#ifndef HAVE_OPENSSL
#define HAVE_OPENSSL
#endif

#ifdef HAVE_OPENSSL
#include <openssl/crypto.h>
#endif

#ifdef HAVE_GCRYPT
#include <gcrypt.h>
#include <errno.h>

#if GCRYPT_VERSION_NUMBER < 0x010600
GCRY_THREAD_OPTION_PTHREAD_IMPL;
#endif

#endif

/* ========================================================================= */
/* openssl locking implementation for curl */
#if defined(HAVE_OPENSSL) && OPENSSL_VERSION_NUMBER < 0x10100000L
static CCriticalSection* getlock(int index)
{
  return g_cryptThreadingInitializer.get_lock(index);
}

static void lock_callback(int mode, int type, const char* file, int line)
{
  if (mode & CRYPTO_LOCK)
    getlock(type)->lock();
  else
    getlock(type)->unlock();
}

static unsigned long thread_id()
{
  return (unsigned long)CThread::GetCurrentThreadId();
}
#endif
/* ========================================================================= */

CryptThreadingInitializer::CryptThreadingInitializer()
{
  bool attemptedToSetSSLMTHook = false;
#if defined(HAVE_OPENSSL) && OPENSSL_VERSION_NUMBER < 0x10100000L
  // set up OpenSSL
  numlocks = CRYPTO_num_locks();
  CRYPTO_set_id_callback(thread_id);
  CRYPTO_set_locking_callback(lock_callback);
  attemptedToSetSSLMTHook = true;
#else
  numlocks = 1;
#endif

  locks = new CCriticalSection*[numlocks];
  for (int i = 0; i < numlocks; i++)
    locks[i] = nullptr;

#ifdef HAVE_GCRYPT
#if GCRYPT_VERSION_NUMBER < 0x010600
  // set up gcrypt
  gcry_control(GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread);
  attemptedToSetSSLMTHook = true;
#endif
#endif

  if (!attemptedToSetSSLMTHook)
    CLog::Log(LOGWARNING, "Could not determine the libcurl security library to set the locking scheme. This may cause problem with multithreaded use of ssl or libraries that depend on it (libcurl).");
  
}

CryptThreadingInitializer::~CryptThreadingInitializer()
{
  CSingleLock l(locksLock);
#if defined(HAVE_OPENSSL) && OPENSSL_VERSION_NUMBER < 0x10100000L
  CRYPTO_set_locking_callback(nullptr);
#endif

  for (int i = 0; i < numlocks; i++)
    delete locks[i]; // I always forget ... delete is NULL safe.

  delete [] locks;
}

CCriticalSection* CryptThreadingInitializer::get_lock(int index)
{
  CSingleLock l(locksLock);
  CCriticalSection* curlock = locks[index];
  if (curlock == nullptr)
  {
    curlock = new CCriticalSection();
    locks[index] = curlock;
  }

  return curlock;
}




