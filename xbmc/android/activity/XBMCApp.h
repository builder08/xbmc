#pragma once
/*
 *      Copyright (C) 2012-2013 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <math.h>
#include <pthread.h>
#include <string>
#include <vector>

#include <android/native_activity.h>

#include "IActivityHandler.h"
#include "IInputHandler.h"

#include "xbmc.h"
#include "utils/GlobalsHandling.h"

#ifdef HAVE_LIBSTAGEFRIGHT
namespace android {
  class SurfaceTexture;
}
#endif

// forward delares
class CAESinkAUDIOTRACK;
typedef struct _JNIEnv JNIEnv;

struct androidIcon
{
  unsigned int width;
  unsigned int height;
  void *pixels;
};  

struct androidPackage
{
  std::string packageName;
  std::string packageLabel;
};


class CXBMCApp : public IActivityHandler
{
public:
  CXBMCApp();
  void SetActivity(ANativeActivity *nativeActivity);
  virtual ~CXBMCApp();

  bool isValid() { return m_activity != NULL; }

  void onStart();
  void onResume();
  void onPause();
  void onStop();
  void onDestroy();

  void onSaveState(void **data, size_t *size);
  void onConfigurationChanged();
  void onLowMemory();

  void onCreateWindow(ANativeWindow* window);
  void onResizeWindow();
  void onDestroyWindow();
  void onGainFocus();
  void onLostFocus();


  static ANativeWindow* GetNativeWindow() { return m_window; };
  static int SetBuffersGeometry(int width, int height, int format);
  static int android_printf(const char *format, ...);
  
  static int GetBatteryLevel();
  static bool StartActivity(const std::string &package, const std::string &intent = std::string(), const std::string &dataType = std::string(), const std::string &dataURI = std::string());
  static bool ListApplications(std::vector <androidPackage> *applications);
  static bool GetIconSize(const std::string &packageName, int *width, int *height);
  static bool GetIcon(const std::string &packageName, void* buffer, unsigned int bufSize); 

  /*!
   * \brief If external storage is available, it returns the path for the external storage (for the specified type)
   * \param path will contain the path of the external storage (for the specified type)
   * \param type optional type. Possible values are "", "files", "music", "videos", "pictures", "photos, "downloads"
   * \return true if external storage is available and a valid path has been stored in the path parameter
   */
  static bool GetExternalStorage(std::string &path, const std::string &type = "");
  static bool GetStorageUsage(const std::string &path, std::string &usage);
  static int GetMaxSystemVolume();

  static int GetDPI();
  
#ifdef HAVE_LIBSTAGEFRIGHT
  bool InitStagefrightSurface();
  void UninitStagefrightSurface();
  void UpdateStagefrightTexture();
  void GetStagefrightTransformMatrix(float* transformMatrix);

  ANativeWindow* GetAndroidVideoWindow() const { return m_VideoNativeWindow;}
  const unsigned int GetAndroidTexture() const { return m_VideoTextureId; }
  android::SurfaceTexture* GetSurfaceTexture() const { return m_SurfaceTexture; }
#endif

protected:
  // limit who can access Volume
  friend class CAESinkAUDIOTRACK;

  static int GetMaxSystemVolume(JNIEnv *env);
  static void SetSystemVolume(JNIEnv *env, float percent);

private:
  static bool HasLaunchIntent(const std::string &package);
  bool getWakeLock(JNIEnv *env);
  void acquireWakeLock();
  void releaseWakeLock();
  void run();
  void stop();

  static ANativeActivity *m_activity;
  jobject m_wakeLock;
  
  bool m_firstrun;
  bool m_exiting;
  pthread_t m_thread;
  
  static ANativeWindow* m_window;
#ifdef HAVE_LIBSTAGEFRIGHT
  unsigned int m_VideoTextureId;
  jobject m_SurfTexture;
  jobject m_Surface;
  jmethodID m_midUpdateTexImage;
  jmethodID m_midGetTransformMatrix;
  jmethodID midSurfaceTextureRelease;
  jmethodID midSurfaceRelease;
  ANativeWindow* m_VideoNativeWindow;
  android::SurfaceTexture* m_SurfaceTexture;
#endif
  
  void XBMC_Pause(bool pause);
  void XBMC_Stop();
  bool XBMC_DestroyDisplay();
  bool XBMC_SetupDisplay();
};

XBMC_GLOBAL_REF(CXBMCApp,g_xbmcapp);
#define g_xbmcapp XBMC_GLOBAL_USE(CXBMCApp)
