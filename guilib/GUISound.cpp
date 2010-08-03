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

#include "system.h"
#include "GUISound.h"
#include "AudioContext.h"
#include "Settings.h"
#include "FileSystem/File.h"
#include "utils/log.h"

CGUISound::CGUISound() :
  m_sound(NULL)
{
}

CGUISound::~CGUISound()
{
  delete m_sound;
}

// \brief Loads a wav file by filename
bool CGUISound::Load(const CStdString& strFile)
{
  m_sound = new CAESound(strFile);
  if (!m_sound->Initialize())
  {
    delete m_sound;
    m_sound = NULL;
    return false;
  }

  return true;
}

// \brief Starts playback of the sound
void CGUISound::Play()
{
  if (m_sound) m_sound->Play();
}

// \brief returns true if the sound is playing
bool CGUISound::IsPlaying()
{
  if (m_sound) return m_sound->IsPlaying();
  return false;
}

// \brief Stops playback if the sound
void CGUISound::Stop()
{
  if (m_sound) m_sound->Stop();
}

// \brief Sets the volume of the sound
void CGUISound::SetVolume(int level)
{
  if (m_sound) m_sound->SetVolume(level);
}

