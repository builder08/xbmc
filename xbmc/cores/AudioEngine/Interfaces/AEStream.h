#pragma once
/*
 *      Copyright (C) 2005-2010 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "../AEAudioFormat.h"
#include "cores/IAudioCallback.h"
#include <stdint.h>

/**
 * Bit options to pass to IAE::GetStream
 */
enum AEStreamOptions {
  AESTREAM_FORCE_RESAMPLE = 0x01, /* force resample even if rates match */
  AESTREAM_PAUSED         = 0x02  /* create the stream paused */
};

/**
 * IAEStream Stream Interface for streaming audio
 */
class IAEStream
{
protected:
  friend class IAE;
  IAEStream() {}
  virtual ~IAEStream() {}

public:
  /**
   * Call this to destroy the stream
   * @note Do not use delete
   */
  virtual void Destroy() = 0;

  /**
   * Returns the amount of space available in the stream
   * @return The number of bytes AddData will consume
   */
  virtual unsigned int GetSpace() = 0;

  /**
   * Add interleaved PCM data to the stream
   * @param data The interleaved PCM data
   * @param size The size in bytes of data
   * @return The number of bytes consumed
   */
  virtual unsigned int AddData(void *data, unsigned int size) = 0;

  /**
   * Returns how long until new data will be played
   * @return The delay in seconds
   */
  virtual float GetDelay() = 0;

  /**
   * Returns how long until playback will start
   * @return The delay in seconds
   */
  virtual float GetCacheTime() = 0;

  /**
   * Returns the total length of the cache before playback will start
   * @return The delay in seconds
   */
  virtual float GetCacheTotal() = 0;

  /**
   * Pauses the stream playback
   */
  virtual void Pause() = 0;

  /**
   * Resumes the stream after pausing
   */
  virtual void Resume  () = 0;

  /**
   * Start draining the stream
   * @note Once called AddData will not consume more data.
   */
  virtual void Drain() = 0;

  /**
   * Returns true if the is stream draining
   */
  virtual bool IsDraining() = 0;

  /**
   * Returns true if the is stream has finished draining
   */
  virtual bool IsDrained() { return true; } /*FIXME: this should = 0 when done */
  
  /**
   * Flush all buffers dropping the audio data
   */
  virtual void Flush() = 0;

  /**
   * Return the stream's current volume level
   * @return The volume level between 0.0 and 1.0
   */
  virtual float GetVolume() = 0;

  /**
   * Set the stream's volume level
   * @param volume The new volume level between 0.0 and 1.0
   */
  virtual void  SetVolume(float volume) = 0;

  /**
   * Returns the stream's current replay gain factor
   * @return The replay gain factor between 0.0 and 1.0
   */
  virtual float GetReplayGain() = 0;

  /**
   * Sets the stream's replay gain factor, this is used by formats such as MP3 that have attenuation information in their streams
   * @param factor The replay gain factor
   */
  virtual void  SetReplayGain(float factor) = 0;

  /**
   * Returns the size of one audio frame in bytes (channelCount * resolution)
   * @return The size in bytes of one frame
  */
  virtual unsigned int GetFrameSize() = 0;

  /**
   * Returns the number of channels the stream is configured to accept
   * @return The channel count
   */
  virtual unsigned int GetChannelCount() = 0;

  /**
   * Returns the stream's sample rate, if the stream is using a dynamic sample rate, this value will NOT reflect any changes made by calls to SetResampleRatio()
   * @return The stream's sample rate (eg, 48000)
   */
  virtual unsigned int GetSampleRate() = 0;

  /**
   * Return the data format the stream has been configured with
   * @return The stream's data format (eg, AE_FMT_S16LE)
   */
  virtual enum AEDataFormat GetDataFormat() = 0;

  /**
   * Return the resample ratio
   * @note This will return an undefined value if the stream is not resampling
   * @return the current resample ratio or undefined if the stream is not resampling
   */
  virtual double GetResampleRatio() = 0;

  /**
   * Sets the resample ratio
   * @note This function will silently fail if the stream is not resampling, if you wish to use this be sure to set the AESTREAM_FORCE_RESAMPLE option
   * @param ratio the new sample rate ratio, calculated by ((double)desiredRate / (double)GetSampleRate())
   */
  virtual void   SetResampleRatio(double ratio) = 0;

  /**
   * Registers the audio callback to call with each block of data, this is used by Audio Visualizations
   * @warning Currently the callbacks require stereo float data in blocks of 512 samples, any deviation from this may crash XBMC, or cause junk to be rendered
   * @param pCallback The callback
   */
  virtual void RegisterAudioCallback(IAudioCallback* pCallback) = 0;

  /**
   * Unregisters the current audio callback
   */
  virtual void UnRegisterAudioCallback() = 0;

  /**
    * Fade the volume level over the specified time
    * @param from The volume level to fade from (0.0f-1.0f) - See notes
    * @param target The volume level to fade to (0.0f-1.0f)
    * @param time The amount of time in milliseconds for the fade to occur
    * @note The from parameter does not set the streams volume, it is only used to calculate the fade time properly 
    */
  virtual void FadeVolume(float from, float target, unsigned int time) {} /* FIXME: once all the engines have these new methods */

  /**
   * Returns if a fade is still running
   * @return true if a fade is in progress, otherwise false
   */
  virtual bool IsFading() { return false; }
};

