/*
 *      Copyright (C) 2015-present Team Kodi
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

#include "utils/ScreenshotAML.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>

// taken from linux/amlogic/amports/amvideocap.h - needs to be synced - no changes expected though
#define AMVIDEOCAP_IOC_MAGIC  'V'
#define AMVIDEOCAP_IOW_SET_WANTFRAME_WIDTH      _IOW(AMVIDEOCAP_IOC_MAGIC, 0x02, int)
#define AMVIDEOCAP_IOW_SET_WANTFRAME_HEIGHT     _IOW(AMVIDEOCAP_IOC_MAGIC, 0x03, int)
#define AMVIDEOCAP_IOW_SET_CANCEL_CAPTURE       _IOW(AMVIDEOCAP_IOC_MAGIC, 0x33, int)

// capture format already defaults to GE2D_FORMAT_S24_RGB - no need to pull in all the ge2d headers :)

#define CAPTURE_DEVICEPATH "/dev/amvideocap0"

//the buffer format is BGRA (4 byte)
void CScreenshotAML::CaptureVideoFrame(unsigned char *buffer, int iWidth, int iHeight, bool bBlendToBuffer)
{
  int captureFd = open(CAPTURE_DEVICEPATH, O_RDWR, 0);
  if (captureFd >= 0)
  {
    int stride = ((iWidth + 31) & ~31) * 3;
    int buffSize = stride * iHeight;
    int readSize = 0;
    // videobuffer should be rgb according to docu - but it is bgr ...
    unsigned char *videoBuffer = new unsigned char[buffSize];

    if (videoBuffer != NULL)
    {
      // configure destination
      ioctl(captureFd, AMVIDEOCAP_IOW_SET_WANTFRAME_WIDTH, stride / 3);
      ioctl(captureFd, AMVIDEOCAP_IOW_SET_WANTFRAME_HEIGHT, iHeight);
      readSize = pread(captureFd, videoBuffer, buffSize, 0);
    }

    close(captureFd);

    if (readSize == buffSize)
    {
      if (!bBlendToBuffer)
      {
        memset(buffer, 0xff, buffSize);
      }

      for (int y = 0; y < iHeight; ++y)
      {
        unsigned char *videoPtr = videoBuffer + y * stride;

        for (int x = 0; x < iWidth; ++x, buffer += 4, videoPtr += 3)
        {
          float alpha = buffer[3] / (float)255;

          if (bBlendToBuffer)
          {
            //B
            buffer[0] = alpha * (float)buffer[0] + (1 - alpha) * (float)videoPtr[0];
            //G
            buffer[1] = alpha * (float)buffer[1] + (1 - alpha) * (float)videoPtr[1];
            //R
            buffer[2] = alpha * (float)buffer[2] + (1 - alpha) * (float)videoPtr[2];
            //A
            buffer[3] = 0xff;// we are solid now
          }
          else
          {
            memcpy(buffer, videoPtr, 3);
          }
        }
      }
    }
    delete [] videoBuffer;
  }
}
